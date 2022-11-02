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
  m_messToDB.push(DB::MessSchedr::errorMess(wId, mess)); \
  m_app.statusMess(mess);

#ifdef DEBUG
  #define checkFieldNum(field) \
    if (mess.find(field) == mess.end()){ \
      ERROR_MESS(string("receiveHandler error mess.find ") + field + " from: " + cp, wId); \
      return;  \
    } \
    if (!Aux::isNumber(mess[field])){ \
      ERROR_MESS(string("receiveHandler error !Aux::isNumber ") + field + " " + mess[field] + " from: " + cp, wId); \
      return; \
    }
  #define checkField(field) \
    if (mess.find(field) == mess.end()){  \
      ERROR_MESS(string("receiveHandler error mess.find ") + field + " from: " + cp, wId);  \
      return;  \
    }
#else
  #define checkFieldNum(field)
  #define checkField(field)
#endif

void Executor::receiveHandler(const string& remcp, const string& data)
{
  auto mess = Aux::deserialn(data);
  if (mess.empty()){
    ERROR_MESS("receiveHandler error deserialn data from: " + remcp, 0);    
    return;
  } 

  int wId = 0;
  string cp = remcp;
  checkFieldNum(Link::command);
  checkField(Link::connectPnt);
       
  cp = mess[Link::connectPnt];
  Base::MessType mtype = Base::MessType(stoi(mess[Link::command]));
  
  // from worker
  if(m_workers.find(cp) != m_workers.end()){
    auto& worker = m_workers[cp];
    wId = worker.base.id;
    switch (mtype){
      case Base::MessType::TASK_ERROR:
      case Base::MessType::TASK_COMPLETED: 
      case Base::MessType::TASK_RUNNING:        
      case Base::MessType::TASK_PAUSE:
      case Base::MessType::TASK_CONTINUE:
      case Base::MessType::TASK_STOP:{
        checkFieldNum(Link::taskId);        
        checkFieldNum(Link::load);
        checkFieldNum(Link::activeTask);
        worker.base.activeTask = stoi(mess[Link::activeTask]);
        worker.base.load = stoi(mess[Link::load]);
        int tid = stoull(mess[Link::taskId]); 
        bool taskExist = false;       
        for(auto& t : worker.taskList){
          if (t == tid){
            taskExist = true;
            if ((mtype == Base::MessType::TASK_ERROR) || (mtype == Base::MessType::TASK_COMPLETED) ||
                (mtype == Base::MessType::TASK_STOP)){
              t = 0;
            }
            break;
          }
        }   
        if (taskExist){   
          m_messToDB.push(DB::MessSchedr(mtype, wId, tid));            
        }
        break;
      }        
      case Base::MessType::TASK_PROGRESS:{
        checkField(Link::tasks);
        Json::Reader readerJs;
        Json::Value obj;
        readerJs.parse(mess[Link::tasks], obj); 

        if (obj.isObject() && obj.isMember(Link::tasks) && obj[Link::tasks].isArray()){
          Json::Value takskJs = obj[Link::tasks];
          for (const auto& t : takskJs){
            if (t.isMember(Link::taskId) && t[Link::taskId].isUInt64() &&
                t.isMember(Link::progress) && t[Link::progress].isString()){
              
              if (find_if(worker.taskList.begin(), worker.taskList.end(), [t](int tId){
                return tId == t[Link::taskId].asUInt64();
              }) != worker.taskList.end())
                m_messToDB.push(DB::MessSchedr(mtype, wId, t[Link::taskId].asUInt64(), t[Link::progress].asString()));
            }
          }
        }
        break;
      }
      case Base::MessType::JUST_START_WORKER:
      case Base::MessType::STOP_WORKER:{
          m_messToDB.push(DB::MessSchedr(mtype, wId));        
          vector<Base::Task> tasks;
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
      case Base::MessType::INTERN_ERROR:
        checkField(Link::message);
        m_messToDB.push(DB::MessSchedr::errorMess(wId, mess[Link::message]));
        break;
      case Base::MessType::PING_WORKER:
        checkFieldNum(Link::load);
        checkFieldNum(Link::activeTask);
        worker.base.activeTask = stoi(mess[Link::activeTask]);
        worker.base.load = stoi(mess[Link::load]);
        m_messToDB.push(DB::MessSchedr(mtype, wId, 0, mess[Link::activeTask] + '\t' + mess[Link::load]));
        break;
      default:
        ERROR_MESS("receiveHandler unknown command from worker: " + mess[Link::command], wId);
        break;
    }

    if (mtype == Base::MessType::STOP_WORKER){
      worker.isActive = false;
      worker.base.state = worker.stateMem = Base::StateType::STOP;
    }
    else{
      worker.isActive = true;
      if (worker.base.state == Base::StateType::STOP){
        worker.base.state = worker.stateMem = Base::StateType::RUNNING;
        m_messToDB.push(DB::MessSchedr{Base::MessType::START_WORKER,
                                        worker.base.id});
      }
      else if (worker.base.state == Base::StateType::NOT_RESPONDING){
        if (worker.stateMem != Base::StateType::NOT_RESPONDING){ 
          worker.base.state = worker.stateMem;
        }else{
          worker.base.state = worker.stateMem = Base::StateType::RUNNING;
        }
        m_messToDB.push(DB::MessSchedr{Base::MessType::START_WORKER,
                                        worker.base.id});
      }
      if (worker.base.rating < Base::Worker::RATING_MAX)
        ++worker.base.rating;
    }
  }
  // from manager
  else{
    switch (mtype){
      case Base::MessType::PING_SCHEDR:     // only check
        break;
      case Base::MessType::PAUSE_SCHEDR:
        if (m_schedr.state != Base::StateType::PAUSE){
          m_messToDB.push(DB::MessSchedr{mtype});
        }
        m_schedr.state = Base::StateType::PAUSE;
        break;
      case Base::MessType::START_AFTER_PAUSE_SCHEDR:
        if (m_schedr.state != Base::StateType::RUNNING){
          m_messToDB.push(DB::MessSchedr{mtype});
        }
        m_schedr.state = Base::StateType::RUNNING;
        break;
      case Base::MessType::PAUSE_WORKER:{
        checkField(Link::workerConnPnt);
        if (mess.count(Link::workerConnPnt)){ 
          auto& worker = m_workers[mess[Link::workerConnPnt]];
          if ((worker.base.state != Base::StateType::NOT_RESPONDING) &&
              (worker.base.state != Base::StateType::STOP)){
            if (worker.base.state != Base::StateType::PAUSE){
              m_messToDB.push(DB::MessSchedr{mtype, worker.base.id});
            }
            worker.base.state = worker.stateMem = Base::StateType::PAUSE;
        }}}
        break;
      case Base::MessType::START_AFTER_PAUSE_WORKER:{
        checkField(Link::workerConnPnt);
        if (mess.count(Link::workerConnPnt)){ 
          auto& worker = m_workers[mess[Link::workerConnPnt]]; 
          if ((worker.base.state != Base::StateType::NOT_RESPONDING) &&
              (worker.base.state != Base::StateType::STOP)){
            if (worker.base.state != Base::StateType::RUNNING){
              m_messToDB.push(DB::MessSchedr{mtype, worker.base.id});
            }
            worker.base.state = worker.stateMem = Base::StateType::RUNNING;
        }}} 
        break;
      default:
        ERROR_MESS("receiveHandler unknown worker: " + cp, 0);
        break;
    }
  }
  
  Application::loopNotify();  
}