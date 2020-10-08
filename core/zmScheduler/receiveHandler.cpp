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
#include <map>
#include "zmCommon/serial.h"
#include "zmCommon/queue.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/tcp.h"
#include "zmCommon/auxFunc.h"
#include "structurs.h"

using namespace std;

extern ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
extern map<std::string, sWorker> _workers;
extern ZM_Base::scheduler _schedr;

void receiveHandler(const string& remcp, const string& data){

#define ERROR_MESS(mess, wId)                                      \
  _messToDB.push(ZM_DB::messSchedr{ZM_Base::MessType::INTERN_ERROR, \
                                   wId,                            \
                                   0,                              \
                                   0,                              \
                                   0,                              \
                                   0,                              \
                                   0,                              \
                                   mess});                         \
  statusMess(mess);

  auto mess = ZM_Aux::deserialn(data);
  if (mess.empty()){
    ERROR_MESS("schedr::receiveHandler error deserialn data from: " + remcp, 0);    
    return;
  } 
#define checkFieldNum(field) \
  if (mess.find(#field) == mess.end()){ \
    ERROR_MESS(string("schedr::receiveHandler error mess.find ") + #field + " from: " + cp, wId); \
    return;  \
  } \
  if (!ZM_Aux::isNumber(mess[#field])){ \
    ERROR_MESS("schedr::receiveHandler error !ZM_Aux::isNumber " + mess[#field] + " from: " + cp, wId); \
    return; \
  }
#define checkField(field) \
  if (mess.find(#field) == mess.end()){  \
    ERROR_MESS(string("schedr::receiveHandler error mess.find ") + #field + " from: " + cp, wId);  \
    return;  \
  }
  uint64_t wId = 0;
  string cp = remcp;
  checkFieldNum(command);
  checkField(connectPnt);
       
  cp = mess["connectPnt"];
  ZM_Base::MessType mtype = ZM_Base::MessType(stoi(mess["command"]));
  
  // from worker
  if(_workers.find(cp) != _workers.end()){
    auto& worker = _workers[cp];
    wId = worker.base.id;
    switch (mtype){
      case ZM_Base::MessType::TASK_ERROR:
      case ZM_Base::MessType::TASK_COMPLETED: 
      case ZM_Base::MessType::TASK_RUNNING:        
      case ZM_Base::MessType::TASK_PAUSE:
      case ZM_Base::MessType::TASK_CONTINUE:
      case ZM_Base::MessType::TASK_STOP:
        checkFieldNum(taskId);
        checkFieldNum(activeTask);
        checkField(taskResult);
        worker.base.activeTask = stoi(mess["activeTask"]);
        _messToDB.push(ZM_DB::messSchedr{mtype, 
                                         wId,
                                         stoull(mess["taskId"]),
                                         0,
                                         0,
                                         _schedr.activeTask,
                                         worker.base.activeTask,
                                         mess["taskResult"]});
        break;
      case ZM_Base::MessType::JUST_START_WORKER:
        _messToDB.push(ZM_DB::messSchedr{mtype, wId});
        break;
      case ZM_Base::MessType::PROGRESS:{
        int tCnt = 0;
        while(mess.find("taskId" + to_string(tCnt)) != mess.end()){
          _messToDB.push(ZM_DB::messSchedr{mtype, 
                                           wId,
                                           stoull(mess["taskId" + to_string(tCnt)]),
                                           stoi(mess["PROGRESS" + to_string(tCnt)])});
          ++tCnt;
        }
        }
        break;
      case ZM_Base::MessType::INTERN_ERROR:{
        checkField(message);
        ERROR_MESS(mess["message"], wId);
        }
        break;
      case ZM_Base::MessType::PING_WORKER:
        checkFieldNum(activeTask);
        worker.base.activeTask = stoi(mess["activeTask"]);        
        break;
      default:
        ERROR_MESS("schedr::receiveHandler unknown command from worker: " + mess["command"], wId);
        break;
    }    
    worker.isActive = true;
    if (worker.base.state == ZM_Base::StateType::READY){
      worker.base.state = worker.stateMem = ZM_Base::StateType::running;
      _messToDB.push(ZM_DB::messSchedr{ZM_Base::MessType::START_WORKER,
                                       worker.base.id});
    }
    else if (worker.base.state == ZM_Base::StateType::notResponding){
      if (worker.stateMem != ZM_Base::StateType::notResponding){ 
        worker.base.state = worker.stateMem;
      }else{
        worker.base.state = worker.stateMem = ZM_Base::StateType::running;
      }
      _messToDB.push(ZM_DB::messSchedr{ZM_Base::MessType::START_WORKER,
                                       worker.base.id});
    }
    if (worker.base.rating < ZM_Base::worker::RATING_MAX){
      ++worker.base.rating;
      _messToDB.push(ZM_DB::messSchedr{ZM_Base::MessType::WORKER_RATING,
                                       wId,
                                       0,
                                       0,
                                       worker.base.rating});      
    }    
  }
  // from manager
  else{
    switch (mtype){
      case ZM_Base::MessType::PING_SCHEDR:     // only check
        break;
      case ZM_Base::MessType::PAUSE_SCHEDR:
        if (_schedr.state != ZM_Base::StateType::pause){
          _messToDB.push(ZM_DB::messSchedr{mtype});
        }
        _schedr.state = ZM_Base::StateType::pause;
        break;
      case ZM_Base::MessType::START_SCHEDR:
        if (_schedr.state != ZM_Base::StateType::running){
          _messToDB.push(ZM_DB::messSchedr{mtype});
        }
        _schedr.state = ZM_Base::StateType::running;
        break;
      case ZM_Base::MessType::PAUSE_WORKER:{
        checkField(workerConnPnt);
        auto& worker = _workers[mess["workerConnPnt"]];
        if (worker.base.state != ZM_Base::StateType::pause){
          _messToDB.push(ZM_DB::messSchedr{mtype, worker.base.id});
        }
        worker.base.state = worker.stateMem = ZM_Base::StateType::pause;
        } break;
      case ZM_Base::MessType::START_WORKER:{
        checkField(workerConnPnt);
        auto& worker = _workers[mess["workerConnPnt"]];        
        if (worker.base.state != ZM_Base::StateType::running){
          _messToDB.push(ZM_DB::messSchedr{mtype, worker.base.id});
        }
        worker.base.state = worker.stateMem = ZM_Base::StateType::running;
        } break;
      default:
        ERROR_MESS("schedr::receiveHandler wrong command from manager: " + mess["command"], 0);
        break;
    }
  }
}