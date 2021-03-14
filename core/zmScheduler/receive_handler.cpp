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
#include "zmDbProvider/db_provider.h"
#include "zmCommon/tcp.h"
#include "zmCommon/aux_func.h"
#include "structurs.h"

using namespace std;

#define ERROR_MESS(mess, wId)                                                    \
  g_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::INTERN_ERROR, wId, mess}); \
  statusMess(mess);

#ifdef DEBUG
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
#else
  #define checkFieldNum(field)
  #define checkField(field)
#endif

extern ZM_Aux::Queue<ZM_DB::MessSchedr> g_messToDB;
extern map<std::string, SWorker> g_workers;
extern ZM_Base::Scheduler g_schedr;

void receiveHandler(const string& remcp, const string& data){

  auto mess = ZM_Aux::deserialn(data);
  if (mess.empty()){
    ERROR_MESS("schedr::receiveHandler error deserialn data from: " + remcp, 0);    
    return;
  } 

  uint64_t wId = 0;
  string cp = remcp;
  checkFieldNum(command);
  checkField(connectPnt);
       
  cp = mess["connectPnt"];
  ZM_Base::MessType mtype = ZM_Base::MessType(stoi(mess["command"]));
  
  // from worker
  if(g_workers.find(cp) != g_workers.end()){
    auto& worker = g_workers[cp];
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
        checkFieldNum(load);
        checkField(taskResult);
        worker.base.activeTask = stoi(mess["activeTask"]);
        worker.base.load = stoi(mess["load"]);
        g_messToDB.push(ZM_DB::MessSchedr{mtype, 
                                         wId,
                                         stoull(mess["taskId"]),
                                         0,
                                         0,
                                         worker.base.load,
                                         g_schedr.activeTask,
                                         worker.base.activeTask,
                                         mess["taskResult"]});
        break;
      case ZM_Base::MessType::JUST_START_WORKER:
        g_messToDB.push(ZM_DB::MessSchedr{mtype, wId});
        break;
      case ZM_Base::MessType::PROGRESS:{
        int tCnt = 0;
        while(mess.find("taskId" + to_string(tCnt)) != mess.end()){
          g_messToDB.push(ZM_DB::MessSchedr{mtype, 
                                           wId,
                                           stoull(mess["taskId" + to_string(tCnt)]),
                                           stoi(mess["progress" + to_string(tCnt)])});
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
        checkFieldNum(load);
        checkFieldNum(activeTask);
        worker.base.activeTask = stoi(mess["activeTask"]);
        worker.base.load = stoi(mess["load"]);
        break;
      default:
        ERROR_MESS("schedr::receiveHandler unknown command from worker: " + mess["command"], wId);
        break;
    }    
    worker.isActive = true;
    if (worker.base.state == ZM_Base::StateType::READY){
      worker.base.state = worker.stateMem = ZM_Base::StateType::RUNNING;
      g_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::START_WORKER,
                                       worker.base.id});
    }
    else if (worker.base.state == ZM_Base::StateType::NOT_RESPONDING){
      if (worker.stateMem != ZM_Base::StateType::NOT_RESPONDING){ 
        worker.base.state = worker.stateMem;
      }else{
        worker.base.state = worker.stateMem = ZM_Base::StateType::RUNNING;
      }
      g_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::START_WORKER,
                                       worker.base.id});
    }
    if (worker.base.rating < ZM_Base::Worker::RATING_MAX){
      ++worker.base.rating;
      if (worker.base.rating == ZM_Base::Worker::RATING_MAX)
        g_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::WORKER_RATING,
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
        if (g_schedr.state != ZM_Base::StateType::PAUSE){
          g_messToDB.push(ZM_DB::MessSchedr{mtype});
        }
        g_schedr.state = ZM_Base::StateType::PAUSE;
        break;
      case ZM_Base::MessType::START_AFTER_PAUSE_SCHEDR:
        if (g_schedr.state != ZM_Base::StateType::RUNNING){
          g_messToDB.push(ZM_DB::MessSchedr{mtype});
        }
        g_schedr.state = ZM_Base::StateType::RUNNING;
        break;
      case ZM_Base::MessType::PAUSE_WORKER:{
        checkField(workerConnPnt);
        auto& worker = g_workers[mess["workerConnPnt"]];
        if (worker.base.state != ZM_Base::StateType::NOT_RESPONDING){
          if (worker.base.state != ZM_Base::StateType::PAUSE){
            g_messToDB.push(ZM_DB::MessSchedr{mtype, worker.base.id});
          }
          worker.base.state = worker.stateMem = ZM_Base::StateType::PAUSE;
        }} break;
      case ZM_Base::MessType::START_AFTER_PAUSE_WORKER:{
        checkField(workerConnPnt);
        auto& worker = g_workers[mess["workerConnPnt"]]; 
        if (worker.base.state != ZM_Base::StateType::NOT_RESPONDING){
          if (worker.base.state != ZM_Base::StateType::RUNNING){
            g_messToDB.push(ZM_DB::MessSchedr{mtype, worker.base.id});
          }
          worker.base.state = worker.stateMem = ZM_Base::StateType::RUNNING;
        }} break;
      default:
        ERROR_MESS("schedr::receiveHandler wrong command from manager: " + mess["command"], 0);
        break;
    }
  }
  
  mainCycleNotify();  
}