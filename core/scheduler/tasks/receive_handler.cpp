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
#include "base/messages.h"

using namespace std;

#define ERROR_MESS(mess, wId)                               \
  m_messToDB.push(DB::MessSchedr::errorMess(wId, mess)); \
  m_app.statusMess(mess);

void Executor::receiveHandler(const string& remcp, const string& data)
{
  auto mtype = mess::getMessType(data);

  if (mtype == base::MessType::UNDEFINED){
    ERROR_MESS("receiveHandler error mtype from: " + remcp, 0);    
    return;
  } 

  string cp = mess::getConnectPnt(data);
  if (cp.empty()){
    ERROR_MESS("receiveHandler error connectPnt from: " + remcp, 0);    
    return;
  }
       
  // from worker
  if(m_workers.find(cp) != m_workers.end()){
    auto& worker = m_workers[cp];
    int wId = worker.base.id;
    switch (mtype){
      case base::MessType::TASK_ERROR:
      case base::MessType::TASK_COMPLETED: 
      case base::MessType::TASK_RUNNING:        
      case base::MessType::TASK_PAUSE:
      case base::MessType::TASK_CONTINUE:
      case base::MessType::TASK_STOP:{
        mess::TaskStatus tm(mtype, cp);
        if (!tm.deserialn(data)){
          ERROR_MESS("receiveHandler error deserialn from: " + cp, wId);    
          return;
        }
        worker.base.activeTask = tm.activeTaskCount;
        worker.base.load = tm.loadCPU;
        int tid = tm.taskId; 
        bool isExist = false;       
        for(auto& t : worker.taskList){
          if (t == tid){
            isExist = true;
            if ((mtype == base::MessType::TASK_ERROR) || 
                (mtype == base::MessType::TASK_COMPLETED) ||
                (mtype == base::MessType::TASK_STOP)){
              t = 0;
            }
            break;
          }
        }   
        if (isExist){   
          m_messToDB.push(DB::MessSchedr(mtype, wId, tid));            
        }
        break;
      }        
      case base::MessType::TASK_PROGRESS:{
        mess::TaskProgress tm(cp);
        if (!tm.deserialn(data)){
          ERROR_MESS("receiveHandler error deserialn from: " + cp, wId);    
          return;
        }
        for (const auto& t : tm.taskProgress){
          if (find_if(worker.taskList.begin(), worker.taskList.end(), [t](int tId){
            return tId == t.first;
          }) != worker.taskList.end())
            m_messToDB.push(DB::MessSchedr(mtype, wId, t.first, to_string(t.second)));
        }
        break;
      }
      case base::MessType::JUST_START_WORKER:
      case base::MessType::STOP_WORKER:{
          m_messToDB.push(DB::MessSchedr(mtype, wId));        
          vector<base::Task> tasks;
          if (m_db.getTasksById(m_schedr.id, worker.taskList, tasks)){
            for(auto& t : tasks){
              m_tasks.push(move(t));
            }
          }else{
            m_app.statusMess("getTasksById db error: " + m_db.getLastError());
          }
          for(auto& t : worker.taskList)
            t = 0;
        }
        break;
      case base::MessType::INTERN_ERROR:{
          mess::InternError tm(cp);
          if (!tm.deserialn(data)){
            ERROR_MESS("receiveHandler error deserialn from: " + cp, wId);    
            return;
          }
          m_messToDB.push(DB::MessSchedr::errorMess(wId, tm.message));
        }
        break;
      case base::MessType::PING_WORKER:
        m_messToDB.push(DB::MessSchedr(mtype, wId, 0, ""));
        break;
      default:
        ERROR_MESS("receiveHandler unknown command from worker: ", wId);
        return;
    }

    if (mtype == base::MessType::STOP_WORKER){
      worker.isActive = false;
      worker.base.state = worker.stateMem = base::StateType::STOP;
    }
    else{
      worker.isActive = true;
      if (worker.base.state == base::StateType::STOP){
        worker.base.state = worker.stateMem = base::StateType::RUNNING;
        m_messToDB.push(DB::MessSchedr{base::MessType::START_WORKER,
                                        worker.base.id});
      }
      else if (worker.base.state == base::StateType::NOT_RESPONDING){
        if (worker.stateMem != base::StateType::NOT_RESPONDING){ 
          worker.base.state = worker.stateMem;
        }else{
          worker.base.state = worker.stateMem = base::StateType::RUNNING;
        }
        m_messToDB.push(DB::MessSchedr{base::MessType::START_WORKER,
                                        worker.base.id});
      }
    }
    Application::loopNotify();
    return;
  }

  // from manager
  switch (mtype){
    case base::MessType::PING_SCHEDR:     // only check
      break;
    case base::MessType::PAUSE_SCHEDR:
      if (m_schedr.state != base::StateType::PAUSE){
        m_messToDB.push(DB::MessSchedr{mtype});
      }
      m_schedr.state = base::StateType::PAUSE;
      break;
    case base::MessType::START_AFTER_PAUSE_SCHEDR:
      if (m_schedr.state != base::StateType::RUNNING){
        m_messToDB.push(DB::MessSchedr{mtype});
      }
      m_schedr.state = base::StateType::RUNNING;
      break;
    case base::MessType::PAUSE_WORKER:{
      // checkField(Link::workerConnPnt);
      // if (mess.count(Link::workerConnPnt)){ 
      //   auto& worker = m_workers[mess[Link::workerConnPnt]];
      //   if ((worker.base.state != base::StateType::NOT_RESPONDING) &&
      //       (worker.base.state != base::StateType::STOP)){
      //     if (worker.base.state != base::StateType::PAUSE){
      //       m_messToDB.push(DB::MessSchedr{mtype, worker.base.id});
      //     }
      //     worker.base.state = worker.stateMem = base::StateType::PAUSE;
      // }}}
      }
      break;
    case base::MessType::START_AFTER_PAUSE_WORKER:{
      // checkField(Link::workerConnPnt);
      // if (mess.count(Link::workerConnPnt)){ 
      //   auto& worker = m_workers[mess[Link::workerConnPnt]]; 
      //   if ((worker.base.state != base::StateType::NOT_RESPONDING) &&
      //       (worker.base.state != base::StateType::STOP)){
      //     if (worker.base.state != base::StateType::RUNNING){
      //       m_messToDB.push(DB::MessSchedr{mtype, worker.base.id});
      //     }
      //     worker.base.state = worker.stateMem = base::StateType::RUNNING;
      // }}} 
      }
      break;
    default:
      ERROR_MESS("receiveHandler unknown worker: " + cp, 0);
      return;
  }
    
  Application::loopNotify();  
}