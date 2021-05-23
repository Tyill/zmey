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
#include "zmWorker/executor.h"
#include "zmCommon/serial.h"

using namespace std;

#define ERROR_MESS(mstr)  \
  m_app.statusMess(mstr); \
  m_errMess.push(mstr);   \

#ifdef DEBUG
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
#else
  #define checkFieldNum(field)
  #define checkField(field)
#endif

void Executor::receiveHandler(const string& remcp, const string& data)
{
  auto mess = ZM_Aux::deserialn(data);
  if (mess.empty()){
    ERROR_MESS("worker::receiveHandler error deserialn data from: " + remcp);
    return;
  }  

  string cp = remcp;
  checkField(connectPnt);
  cp = mess["connectPnt"];
  checkFieldNum(command);
  ZM_Base::MessType mtype = ZM_Base::MessType(stoi(mess["command"]));  
  if (mtype == ZM_Base::MessType::NEW_TASK){
    checkFieldNum(taskId);
    checkField(params);
    checkField(script);
    checkFieldNum(averDurationSec);
    checkFieldNum(maxDurationSec);
    ZM_Base::Task t;
    t.id = stoull(mess["taskId"]);
    t.averDurationSec = stoi(mess["averDurationSec"]);
    t.maxDurationSec = stoi(mess["maxDurationSec"]);
    t.script = mess["script"];
    t.state = ZM_Base::StateType::READY;
    t.params = mess["params"];
    m_newTasks.push(move(t)); 
    Application::loopNotify();
  }
  else if (mtype == ZM_Base::MessType::PING_WORKER){  // only check
    return;
  }
  else{
    checkFieldNum(taskId);
    uint64_t tId = stoull(mess["taskId"]);
    { std::lock_guard<std::mutex> lock(m_mtxProcess);
      
      auto iPrc = find_if(m_procs.begin(), m_procs.end(), [tId](const Process& p){
        return p.getTask().id == tId;
      });
      if (iPrc != m_procs.end()){
        switch (mtype){
          case ZM_Base::MessType::TASK_PAUSE:    iPrc->pause(); break;
          case ZM_Base::MessType::TASK_CONTINUE: iPrc->contin(); break;
          case ZM_Base::MessType::TASK_STOP:     iPrc->stop(); break;
          default:{
            ERROR_MESS("worker::receiveHandler wrong command: " + mess["command"]);
          }
          break;
        }
      }else{
        ERROR_MESS("worker::receiveHandler iPrc == _procs.end() for taskId: " + mess["taskId"]);
      }
    }
  }  
}