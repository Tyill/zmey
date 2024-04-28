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

  // from manager
  if(!m_workers.count(cp)){
    switch (mtype){
      case mess::MessType::PING_SCHEDR:     // only check
        break;
      case mess::MessType::PAUSE_SCHEDR:
        if (m_schedr.sState != int(base::StateType::PAUSE)){
          m_messToDB.push(db::MessSchedr{mtype});
        }
        m_schedr.sState = int(base::StateType::PAUSE);
        break;
      case mess::MessType::START_AFTER_PAUSE_SCHEDR:
        if (m_schedr.sState != int(base::StateType::RUNNING)){
          m_messToDB.push(db::MessSchedr{mtype});
        }
        m_schedr.sState = int(base::StateType::RUNNING);
        break;    
      default: 
        errorMessage("receiveHandler unknown command", 0);
        return;
    }
  }
  else{
    auto w = m_workers[cp];
    if (w->wState == int(base::StateType::STOP) || w->wState == int(base::StateType::NOT_RESPONDING)){
      if (mtype != mess::MessType::JUST_START_WORKER){
        errorMessage("receiveHandler error: mess from not running worker: " + cp, w->wId);    
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
            errorMessage("receiveHandler error deserialn from: " + cp, w->wId);    
            return;
          }
          w->wActiveTaskCount = tm.activeTaskCount;
          w->wLoadCPU = tm.loadCPU;
          int tid = tm.taskId;
          const auto wtasks = getWorkerTasks(w->wId);
          if (auto it = std::find_if(wtasks.begin(), wtasks.end(), [tid](const auto& t){
            return t.tId == tid;
          }); it != wtasks.end()){
            m_messToDB.push(db::MessSchedr(mtype, w->wId, tid));

            if (mtype == mess::MessType::TASK_COMPLETED || mtype == mess::MessType::TASK_ERROR){
              removeTaskForWorker(w->wId, *it);
            }
          }
        }
        break;      
      case mess::MessType::PING_WORKER:{
          mess::TaskStatus tm(mtype, cp);
          if (!tm.deserialn(data)){
            errorMessage("receiveHandler error deserialn from: " + cp, w->wId);    
            return;
          }
          m_messToDB.push(db::MessSchedr::pingWorkerMess(w->wId, tm.activeTaskCount, tm.loadCPU));
        }
        break;
      case mess::MessType::TASK_PROGRESS:{
          mess::TaskProgress tm;
          if (!tm.deserialn(data)){
            errorMessage("receiveHandler error deserialn from: " + cp, w->wId);    
            return;
          }
          w->wActiveTaskCount = tm.activeTaskCount;
          w->wLoadCPU = tm.loadCPU;
          const auto wtasks = getWorkerTasks(w->wId);
          for (int i = 0; i < tm.taskProgress.size() && i < tm.taskIds.size(); ++i){
            int tid = tm.taskIds[i];
            if (auto it = std::find_if(wtasks.begin(), wtasks.end(), [tid](const auto& t){
              return t.tId == tid;
            }); it != wtasks.end()){
              m_messToDB.push(db::MessSchedr::taskProgressMess(w->wId, tid, tm.taskProgress[i]));
            }
          }
        }
        break;
      case mess::MessType::JUST_START_WORKER:
      case mess::MessType::STOP_WORKER:{
          m_messToDB.push(db::MessSchedr(mtype, w->wId)); 
          const auto wtasks = getWorkerTasks(w->wId);
          for(auto t : wtasks){
            m_tasks.push(move(t));
          }          
          clearWorkerTasks(w->wId); 
        }
        break;
      case mess::MessType::PAUSE_WORKER:
        if (w->wState != int(base::StateType::NOT_RESPONDING) &&
            w->wState != int(base::StateType::STOP)){
          if (w->wState != int(base::StateType::PAUSE)){
            m_messToDB.push(db::MessSchedr{mtype, w->wId});
          }
          w->wState = w->wStateMem = int(base::StateType::PAUSE);
        }
        break;
      case mess::MessType::START_AFTER_PAUSE_WORKER:
        if (w->wState != int(base::StateType::NOT_RESPONDING) &&
            w->wState != int(base::StateType::STOP)){
          if (w->wState != int(base::StateType::RUNNING)){
            m_messToDB.push(db::MessSchedr{mtype, w->wId});
          }
          w->wState = w->wStateMem = int(base::StateType::RUNNING);
        }
        break;
      case mess::MessType::INTERN_ERROR:{
          mess::InternError tm(cp);
          if (!tm.deserialn(data)){
            errorMessage("receiveHandler error deserialn from: " + cp, w->wId);    
            return;
          }
          m_messToDB.push(db::MessSchedr::errorMess(w->wId, tm.message));
        }
        break;      
      default:
        errorMessage("receiveHandler unknown command from worker: ", w->wId);
        return;
    }
    if (mtype == mess::MessType::STOP_WORKER){
      w->wIsActive = false;
      w->wState = w->wStateMem = int(base::StateType::STOP);
    }
    else{
      if (!w->wIsActive){
        w->wIsActive = true;
      }
      if (w->wState == int(base::StateType::STOP)){
        w->wState = w->wStateMem = int(base::StateType::RUNNING);
        m_messToDB.push(db::MessSchedr{mess::MessType::START_WORKER, w->wId});
      }
      else if (w->wState == int(base::StateType::NOT_RESPONDING)){
        if (w->wStateMem != int(base::StateType::NOT_RESPONDING)){ 
          w->wState = +w->wStateMem;
        }else{
          w->wState = w->wStateMem = int(base::StateType::RUNNING);
        }
        m_messToDB.push(db::MessSchedr{mess::MessType::START_WORKER, w->wId});
      }
    }
  }
  m_loop->standUpNotify();  
}