//
// zmey Project
// Copyright (C) 2018 by Contributors <https://github.com/Tyill/zmey>
//
// This code is licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "scheduler/executor.h"
#include "scheduler/loop.h"
#include "base/messages.h"
#include "db_provider/db_provider.h"

using namespace std;


void Executor::receiveHandler(const string& remcp, const string& data)
{
  auto mtype = mess::getMessType(data);
  if (mtype == mess::MessType::UNDEFINED){
    errorMessage("receiveHandler error mtype from: " + remcp, 0);    
    return;
  }
  string cp = mess::getConnectPnt(data);
  if (cp.empty()){
    errorMessage("receiveHandler error connectPnt from: " + remcp, 0);    
    return;
  }

  auto schedr = getScheduler();
  auto worker = getWorkerByConnPnt(cp);

  // from manager
  if(!worker){
    switch (mtype){
      case mess::MessType::PING_SCHEDR:     // only check
        break;
      case mess::MessType::PAUSE_SCHEDR:
        if (schedr.sState != base::StateType::PAUSE){
          m_messToDB.push(db::MessSchedr{mtype});
        }
        schedr.sState = base::StateType::PAUSE;
        updateScheduler(schedr);
        break;
      case mess::MessType::START_AFTER_PAUSE_SCHEDR:
        if (schedr.sState != base::StateType::RUNNING){
          m_messToDB.push(db::MessSchedr{mtype});
        }
        schedr.sState = base::StateType::RUNNING;
        updateScheduler(schedr);
        break;    
      default: 
        errorMessage("receiveHandler unknown command", 0);
        return;
    }
  }
  else{
    if (worker->wState == base::StateType::STOP || worker->wState == base::StateType::NOT_RESPONDING){
      if (mtype != mess::MessType::JUST_START_WORKER){
        errorMessage("receiveHandler error: mess from not running worker: " + cp, worker->wId);
        m_messToWorker.push(mess::InfoMess(mess::MessType::REQUEST_START_WORKER, cp));
        return;
      }
    }
    switch (mtype){
      case mess::MessType::TASK_COMPLETED: 
      case mess::MessType::TASK_RUNNING:        
      case mess::MessType::TASK_ERROR:
      case mess::MessType::TASK_PAUSE:
      case mess::MessType::TASK_CONTINUE:
      case mess::MessType::TASK_STOP:{
          mess::TaskStatus tm(mtype, cp);
          if (!tm.deserialn(data)){
            errorMessage("receiveHandler error deserialn from: " + cp, worker->wId);    
            return;
          }
          worker->wActiveTaskCount = tm.activeTaskCount;
          worker->wLoadCPU = tm.loadCPU;
          updateWorker(worker.value());
          int tid = tm.taskId;
          const auto wtasks = getWorkerTasks(worker->wId);
          if (auto it = std::find_if(wtasks.begin(), wtasks.end(), [tid](const auto& t){
            return t.tId == tid;
          }); it != wtasks.end()){
            m_messToDB.push(db::MessSchedr(mtype, worker->wId, tid));

            if (mtype == mess::MessType::TASK_COMPLETED || mtype == mess::MessType::TASK_ERROR){
              removeTaskForWorker(worker->wId, *it);
            }
          }
        }
        break;      
      case mess::MessType::PING_WORKER:{
          mess::TaskStatus tm(mtype, cp);
          if (!tm.deserialn(data)){
            errorMessage("receiveHandler error deserialn from: " + cp, worker->wId);    
            return;
          }
          m_messToDB.push(db::MessSchedr::pingWorkerMess(worker->wId, tm.activeTaskCount, tm.loadCPU));
        }
        break;
      case mess::MessType::TASK_PROGRESS:{
          mess::TaskProgress tm;
          if (!tm.deserialn(data)){
            errorMessage("receiveHandler error deserialn from: " + cp, worker->wId);    
            return;
          }
          worker->wActiveTaskCount = tm.activeTaskCount;
          worker->wLoadCPU = tm.loadCPU;
          updateWorker(worker.value());
          const auto wtasks = getWorkerTasks(worker->wId);
          for (int i = 0; i < tm.taskProgress.size() && i < tm.taskIds.size(); ++i){
            int tid = tm.taskIds[i];
            if (auto it = std::find_if(wtasks.begin(), wtasks.end(), [tid](const auto& t){
              return t.tId == tid;
            }); it != wtasks.end()){
              m_messToDB.push(db::MessSchedr::taskProgressMess(worker->wId, tid, tm.taskProgress[i]));
            }
          }
        }
        break;
      case mess::MessType::JUST_START_WORKER:
      case mess::MessType::STOP_WORKER:{
          m_messToDB.push(db::MessSchedr(mtype, worker->wId)); 
          const auto wtasks = getWorkerTasks(worker->wId);
          for(auto t : wtasks){
            m_tasks.push(move(t));
          }          
          clearWorkerTasks(worker->wId); 
        }
        break;
      case mess::MessType::PAUSE_WORKER:
        if (worker->wState != base::StateType::NOT_RESPONDING &&
            worker->wState != base::StateType::STOP){
          if (worker->wState != base::StateType::PAUSE){
            m_messToDB.push(db::MessSchedr{mtype, worker->wId});
          }
          worker->wState = worker->wStateMem = base::StateType::PAUSE;
          updateWorker(worker.value());
        }
        break;
      case mess::MessType::START_AFTER_PAUSE_WORKER:
        if (worker->wState != base::StateType::NOT_RESPONDING &&
            worker->wState != base::StateType::STOP){
          if (worker->wState != base::StateType::RUNNING){
            m_messToDB.push(db::MessSchedr{mtype, worker->wId});
          }
          worker->wState = worker->wStateMem = base::StateType::RUNNING;
          updateWorker(worker.value());
        }
        break;
      case mess::MessType::INTERN_ERROR:{
          mess::InternError tm(cp);
          if (!tm.deserialn(data)){
            errorMessage("receiveHandler error deserialn from: " + cp, worker->wId);    
            return;
          }
          m_messToDB.push(db::MessSchedr::errorMess(worker->wId, tm.message));
        }
        break;      
      default:
        errorMessage("receiveHandler unknown command from worker: ", worker->wId);
        return;
    }
    if (mtype == mess::MessType::STOP_WORKER){
      worker->wIsActive = false;
      worker->wState = worker->wStateMem = base::StateType::STOP;
      updateWorker(worker.value());
    }
    else{
      if (!worker->wIsActive){
        worker->wIsActive = true;
        updateWorker(worker.value());
      }
      if (worker->wState == base::StateType::STOP){
        worker->wState = worker->wStateMem = base::StateType::RUNNING;
        m_messToDB.push(db::MessSchedr{mess::MessType::START_WORKER, worker->wId});
        updateWorker(worker.value());
      }
      else if (worker->wState == base::StateType::NOT_RESPONDING){
        if (worker->wStateMem != base::StateType::NOT_RESPONDING){ 
          worker->wState = worker->wStateMem;
        }else{
          worker->wState = worker->wStateMem = base::StateType::RUNNING;
        }
        m_messToDB.push(db::MessSchedr{mess::MessType::START_WORKER, worker->wId});
        updateWorker(worker.value());
      }
    }
  }
  m_loop->standUpNotify();  
}