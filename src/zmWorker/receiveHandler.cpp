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
#include <string>
#include <algorithm>
#include <list>
#include <mutex>
#include "zmCommon/serial.h"
#include "zmCommon/auxFunc.h"
#include "zmCommon/queue.h"
#include "structurs.h"
#include "process.h" 

using namespace std;

extern list<Process> _procs;
extern ZM_Aux::QueueThrSave<wTask> _newTasks;
extern ZM_Aux::QueueThrSave<string> _errMess;
extern mutex _mtxPrc;

void receiveHandler(const string& remcp, const string& data){
#define ERROR_MESS(mstr) \
  statusMess(mstr);      \
  _errMess.push(mstr);   \

  auto mess = ZM_Aux::deserialn(data);
  if (mess.empty()){
    ERROR_MESS("worker::receiveHandler error deserialn data from: " + remcp);
    return;
  }  
#define checkFieldNum(field) \
  if (mess.find(#field) == mess.end()){ \
    ERROR_MESS(string("worker::receiveHandler error mess.find ") + #field + " from: " + cp); \
    return;  \
  } \
  if (!ZM_Aux::isNumber(mess[#field])){ \
    ERROR_MESS("worker::receiveHandler error !ZM_Aux::isNumber " + mess[#field] + " from: " + cp); \
    return; \
  }
#define checkField(field) \
  if (mess.find(#field) == mess.end()){  \
    ERROR_MESS(string("worker::receiveHandler error mess.find ") + #field + " from: " + cp); \
    return;  \
  }
  string cp = remcp;
  checkField(connectPnt);
  cp = mess["connectPnt"];
  checkFieldNum(command);
  ZM_Base::messType mtype = ZM_Base::messType(stoi(mess["command"]));  
  if (mtype == ZM_Base::messType::newTask){
    checkFieldNum(taskId);
    checkField(params);
    checkField(script);
    checkFieldNum(averDurationSec);
    checkFieldNum(maxDurationSec);
    ZM_Base::task t;
    t.id = stoull(mess["taskId"]);
    t.averDurationSec = stoi(mess["averDurationSec"]);
    t.maxDurationSec = stoi(mess["maxDurationSec"]);
    t.script = mess["script"];
    _newTasks.push(wTask{t, 
                         ZM_Base::stateType::ready,
                         mess["params"]});
  }
  else if (mtype == ZM_Base::messType::pingWorker){  // only check
    return;
  }
  else{
    checkFieldNum(taskId);
    uint64_t tId = stoull(mess["taskId"]);
    { std::lock_guard<std::mutex> lock(_mtxPrc);
      
      auto iPrc = find_if(_procs.begin(), _procs.end(), [tId](const Process& p){
        return p.getTask().base.id == tId;
      });
      if (iPrc != _procs.end()){
        switch (mtype){
          case ZM_Base::messType::taskPause:   iPrc->pause(); break;
          case ZM_Base::messType::taskRunning: iPrc->contin(); break;
          case ZM_Base::messType::taskStop:    iPrc->stop(); break;
          default:{
            ERROR_MESS("worker::receiveHandler unknown command: " + mess["command"]);
          }
          break;
        }
      }else{
        ERROR_MESS("worker::receiveHandler iPrc == _procs.end() for taskId: " + mess["taskId"]);
      }
    }
  }
}