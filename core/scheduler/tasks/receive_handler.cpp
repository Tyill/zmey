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
#include "db_provider/db_provider.h"
#include "../application.h"

using namespace std;

#define ERROR_MESS(mess, wId)                               \
  m_messToDB.push(db::MessSchedr::errorMess(wId, mess)); \
  m_app.statusMess(mess);

void Executor::receiveHandler(const string& remcp, const string& data)
{
  auto mtype = mess::getMessType(data);

  if (mtype == mess::MessType::UNDEFINED){
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
      case mess::MessType::TASK_ERROR:
      case mess::MessType::TASK_COMPLETED: 
      case mess::MessType::TASK_RUNNING:        
      case mess::MessType::TASK_PAUSE:
      case mess::MessType::TASK_CONTINUE:
      case mess::MessType::TASK_STOP:{
        mess::TaskStatus tm(mtype, cp);
        if (!tm.deserialn(data)){
          ERROR_MESS("receiveHandler error deserialn from: " + cp, wId);    
          return;
        }
        worker.base.activeTask = tm.activeTaskCount;
        int tid = tm.taskId; 
        bool isExist = false;       
        for(auto& t : worker.taskList){
          if (t == tid){
            isExist = true;
            if ((mtype == mess::MessType::TASK_ERROR) || 
                (mtype == mess::MessType::TASK_COMPLETED) ||
                (mtype == mess::MessType::TASK_STOP)){
              t = 0;
            }
            break;
          }
        }   
        if (isExist){   
          m_messToDB.push(db::MessSchedr(mtype, wId, tid));            
        }
        break;
      }
      case mess::MessType::JUST_START_WORKER:
      case mess::MessType::STOP_WORKER:{
          m_messToDB.push(db::MessSchedr(mtype, wId)); 
          vector<base::Task> tasks;
          if (m_db.getTasksById(m_schedr.id, worker.taskList, tasks)){
            for(auto& t : tasks){
              m_tasks.push(move(t));
            }
          }else{
            m_app.statusMess("getTasksById db error: " + m_db.getLastError());
          }
          for(auto& t : worker.taskList){
            t = 0;
          }
        }
        break;
      case mess::MessType::INTERN_ERROR:{
          mess::InternError tm(cp);
          if (!tm.deserialn(data)){
            ERROR_MESS("receiveHandler error deserialn from: " + cp, wId);    
            return;
          }
          m_messToDB.push(db::MessSchedr::errorMess(wId, tm.message));
        }
        break;
      case mess::MessType::PING_WORKER:
        m_messToDB.push(db::MessSchedr(mtype, wId, 0, ""));
        break;
      default:
        ERROR_MESS("receiveHandler unknown command from worker: ", wId);
        return;
    }

    if (mtype == mess::MessType::STOP_WORKER){
      worker.isActive = false;
      worker.base.state = worker.stateMem = base::StateType::STOP;
    }
    else{
      worker.isActive = true;
      if (worker.base.state == base::StateType::STOP){
        worker.base.state = worker.stateMem = base::StateType::RUNNING;
        m_messToDB.push(db::MessSchedr{mess::MessType::START_WORKER,
                                        worker.base.id});
      }
      else if (worker.base.state == base::StateType::NOT_RESPONDING){
        if (worker.stateMem != base::StateType::NOT_RESPONDING){ 
          worker.base.state = worker.stateMem;
        }else{
          worker.base.state = worker.stateMem = base::StateType::RUNNING;
        }
        m_messToDB.push(db::MessSchedr{mess::MessType::START_WORKER,
                                        worker.base.id});
      }
    }
    loopStandUpNotify();
    return;
  }

  // from manager
  switch (mtype){
    case mess::MessType::PING_SCHEDR:     // only check
      break;
    case mess::MessType::PAUSE_SCHEDR:
      if (m_schedr.state != base::StateType::PAUSE){
        m_messToDB.push(db::MessSchedr{mtype});
      }
      m_schedr.state = base::StateType::PAUSE;
      break;
    case mess::MessType::START_AFTER_PAUSE_SCHEDR:
      if (m_schedr.state != base::StateType::RUNNING){
        m_messToDB.push(db::MessSchedr{mtype});
      }
      m_schedr.state = base::StateType::RUNNING;
      break;
    case mess::MessType::PAUSE_WORKER:{
      // checkField(Link::workerConnPnt);
      // if (mess.count(Link::workerConnPnt)){ 
      //   auto& worker = m_workers[mess[Link::workerConnPnt]];
      //   if ((worker.base.state != base::StateType::NOT_RESPONDING) &&
      //       (worker.base.state != base::StateType::STOP)){
      //     if (worker.base.state != base::StateType::PAUSE){
      //       m_messToDB.push(db::MessSchedr{mtype, worker.base.id});
      //     }
      //     worker.base.state = worker.stateMem = base::StateType::PAUSE;
      // }}}
      }
      break;
    case mess::MessType::START_AFTER_PAUSE_WORKER:{
      // checkField(Link::workerConnPnt);
      // if (mess.count(Link::workerConnPnt)){ 
      //   auto& worker = m_workers[mess[Link::workerConnPnt]]; 
      //   if ((worker.base.state != base::StateType::NOT_RESPONDING) &&
      //       (worker.base.state != base::StateType::STOP)){
      //     if (worker.base.state != base::StateType::RUNNING){
      //       m_messToDB.push(db::MessSchedr{mtype, worker.base.id});
      //     }
      //     worker.base.state = worker.stateMem = base::StateType::RUNNING;
      // }}} 
      }
      break;
    default:
      ERROR_MESS("receiveHandler unknown worker: " + cp, 0);
      return;
  }
    
  loopStandUpNotify();  
}