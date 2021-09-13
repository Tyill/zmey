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
#include "common/serial.h"
#include "common/json.h"
#include "base/link.h"

using namespace std;

#define ERROR_MESS(mess, wId)                               \
  m_messToDB.push(ZM_DB::MessSchedr::errorMess(wId, mess)); \
  m_app.statusMess(mess);

#ifdef DEBUG
  #define checkFieldNum(field) \
    if (mess.find(#field) == mess.end()){ \
      ERROR_MESS(string("schedr::receiveHandler error mess.find ") + #field + " from: " + cp, wId); \
      return;  \
    } \
    if (!ZM_Aux::isNumber(mess[#field])){ \
      ERROR_MESS(string("schedr::receiveHandler error !ZM_Aux::isNumber ") + #field + " " + mess[#field] + " from: " + cp, wId); \
      return; \
    }
  #define checkField(field) \
    if (mess.find(#field) == mess.end()){  \
      ERROR_MESS(string("schedr::receiveHandler error mess.find ") + #field + " from: " + cp, wId);  \
      return;  \
    }
#else
  #define checkFieldNum(field)
  #define checkField(field)
#endif

void Executor::receiveHandler(const string& remcp, const string& data)
{
  auto mess = ZM_Aux::deserialn(data);
  if (mess.empty()){
    ERROR_MESS("schedr::receiveHandler error deserialn data from: " + remcp, 0);    
    return;
  } 

  uint64_t wId = 0;
  string cp = remcp;
  checkFieldNum(ZM_Link::command);
  checkField(ZM_Link::connectPnt);
       
  cp = mess[ZM_Link::connectPnt];
  ZM_Base::MessType mtype = ZM_Base::MessType(stoi(mess[ZM_Link::command]));
  
  // from worker
  if(m_workers.find(cp) != m_workers.end()){
    auto& worker = m_workers[cp];
    wId = worker.base.id;
    switch (mtype){
      case ZM_Base::MessType::TASK_ERROR:
      case ZM_Base::MessType::TASK_COMPLETED: 
      case ZM_Base::MessType::TASK_RUNNING:        
      case ZM_Base::MessType::TASK_PAUSE:
      case ZM_Base::MessType::TASK_CONTINUE:
      case ZM_Base::MessType::TASK_STOP:{
        checkFieldNum(ZM_Link::taskId);        
        checkFieldNum(ZM_Link::load);
        checkFieldNum(ZM_Link::activeTask);
        checkField(ZM_Link::taskResult);     
        worker.base.activeTask = stoi(mess[ZM_Link::activeTask]);
        worker.base.load = stoi(mess[ZM_Link::load]);
        uint64_t tid = stoull(mess[ZM_Link::taskId]); 
        bool taskExist = false;       
        for(auto& t : worker.taskList){
          if (t == tid){
            taskExist = true;
            if ((mtype == ZM_Base::MessType::TASK_ERROR) || (mtype == ZM_Base::MessType::TASK_COMPLETED) ||
                (mtype == ZM_Base::MessType::TASK_STOP)){
              t = 0;
            }
            break;
          }
        }   
        if (taskExist){   
          m_messToDB.push(ZM_DB::MessSchedr(mtype, wId, tid, mess[ZM_Link::taskResult]));            
        }
        break;
      }        
      case ZM_Base::MessType::TASK_PROGRESS:{
        checkField(tasks);
        Json::Reader readerJs;
        Json::Value obj;
        readerJs.parse(mess[ZM_Link::tasks], obj); 

        if (obj.isObject() && obj.isMember(ZM_Link::tasks) && obj[ZM_Link::tasks].isArray()){
          Json::Value takskJs = obj[ZM_Link::tasks];
          for (const auto& t : takskJs){
            if (t.isMember(ZM_Link::taskId) && t[ZM_Link::taskId].isUInt64() &&
                t.isMember(ZM_Link::progress) && t[ZM_Link::progress].isString()){
              
              if (find_if(worker.taskList.begin(), worker.taskList.end(), [t](uint64_t tId){
                return tId == t[ZM_Link::taskId].asUInt64();
              }) != worker.taskList.end())
                m_messToDB.push(ZM_DB::MessSchedr(mtype, wId, t[ZM_Link::taskId].asUInt64(), t[ZM_Link::progress].asString()));
            }
          }
        }
        break;
      }
      case ZM_Base::MessType::JUST_START_WORKER:
      case ZM_Base::MessType::STOP_WORKER:{
          m_messToDB.push(ZM_DB::MessSchedr(mtype, wId));        
          vector<ZM_Base::Task> tasks;
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
      case ZM_Base::MessType::INTERN_ERROR:
        checkField(ZM_Link::message);
        m_messToDB.push(ZM_DB::MessSchedr::errorMess(wId, mess[ZM_Link::message]));
        break;
      case ZM_Base::MessType::PING_WORKER:
        checkFieldNum(ZM_Link::load);
        checkFieldNum(ZM_Link::activeTask);
        worker.base.activeTask = stoi(mess[ZM_Link::activeTask]);
        worker.base.load = stoi(mess[ZM_Link::load]);
        m_messToDB.push(ZM_DB::MessSchedr(mtype, wId, 0, mess[ZM_Link::activeTask] + '\t' + mess[ZM_Link::load]));
        break;
      default:
        ERROR_MESS("schedr::receiveHandler unknown command from worker: " + mess[ZM_Link::command], wId);
        break;
    }

    if (mtype == ZM_Base::MessType::STOP_WORKER){
      worker.isActive = false;
      worker.base.state = worker.stateMem = ZM_Base::StateType::STOP;
    }
    else{
      worker.isActive = true;
      if (worker.base.state == ZM_Base::StateType::STOP){
        worker.base.state = worker.stateMem = ZM_Base::StateType::RUNNING;
        m_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::START_WORKER,
                                        worker.base.id});
      }
      else if (worker.base.state == ZM_Base::StateType::NOT_RESPONDING){
        if (worker.stateMem != ZM_Base::StateType::NOT_RESPONDING){ 
          worker.base.state = worker.stateMem;
        }else{
          worker.base.state = worker.stateMem = ZM_Base::StateType::RUNNING;
        }
        m_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::START_WORKER,
                                        worker.base.id});
      }
      if (worker.base.rating < ZM_Base::Worker::RATING_MAX)
        ++worker.base.rating;
    }
  }
  // from manager
  else{
    switch (mtype){
      case ZM_Base::MessType::PING_SCHEDR:     // only check
        break;
      case ZM_Base::MessType::PAUSE_SCHEDR:
        if (m_schedr.state != ZM_Base::StateType::PAUSE){
          m_messToDB.push(ZM_DB::MessSchedr{mtype});
        }
        m_schedr.state = ZM_Base::StateType::PAUSE;
        break;
      case ZM_Base::MessType::START_AFTER_PAUSE_SCHEDR:
        if (m_schedr.state != ZM_Base::StateType::RUNNING){
          m_messToDB.push(ZM_DB::MessSchedr{mtype});
        }
        m_schedr.state = ZM_Base::StateType::RUNNING;
        break;
      case ZM_Base::MessType::PAUSE_WORKER:{
        checkField(ZM_Link::workerConnPnt);
        if (mess.count(ZM_Link::workerConnPnt)){ 
          auto& worker = m_workers[mess[ZM_Link::workerConnPnt]];
          if ((worker.base.state != ZM_Base::StateType::NOT_RESPONDING) &&
              (worker.base.state != ZM_Base::StateType::STOP)){
            if (worker.base.state != ZM_Base::StateType::PAUSE){
              m_messToDB.push(ZM_DB::MessSchedr{mtype, worker.base.id});
            }
            worker.base.state = worker.stateMem = ZM_Base::StateType::PAUSE;
        }}}
        break;
      case ZM_Base::MessType::START_AFTER_PAUSE_WORKER:{
        checkField(ZM_Link::workerConnPnt);
        if (mess.count(ZM_Link::workerConnPnt)){ 
          auto& worker = m_workers[mess[ZM_Link::workerConnPnt]]; 
          if ((worker.base.state != ZM_Base::StateType::NOT_RESPONDING) &&
              (worker.base.state != ZM_Base::StateType::STOP)){
            if (worker.base.state != ZM_Base::StateType::RUNNING){
              m_messToDB.push(ZM_DB::MessSchedr{mtype, worker.base.id});
            }
            worker.base.state = worker.stateMem = ZM_Base::StateType::RUNNING;
        }}} 
        break;
      default:
        ERROR_MESS("schedr::receiveHandler unknown worker: " + cp, 0);
        break;
    }
  }
  
  Application::loopNotify();  
}