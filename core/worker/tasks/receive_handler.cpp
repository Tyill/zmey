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

#include "worker/executor.h"
#include "base/messages.h"

using namespace std;

#define ERROR_MESS(mstr)  \
  m_app.statusMess(mstr); \
  m_errMess.push(mstr);   \

void Executor::receiveHandler(const string& remcp, const string& data)
{
  auto mtype = mess::getMessType(data);
  if (mtype == base::MessType::UNDEFINED){
    ERROR_MESS("receiveHandler error mtype from: " + remcp);    
    return;
  } 

  string cp = mess::getConnectPnt(data);
  if (cp.empty()){
    ERROR_MESS("receiveHandler error connectPnt from: " + remcp);    
    return;
  }
 
  if (mtype == base::MessType::NEW_TASK){
    mess::NewTask tm(cp);
    if (!tm.deserialn(data)){
      ERROR_MESS("receiveHandler error deserialn MessType::NEW_TASK from: " + cp);    
      return;
    }
    base::Task t;
    t.id = tm.taskId;
    t.averDurationSec = tm.averDurationSec;
    t.maxDurationSec = tm.maxDurationSec;
    t.scriptPath = tm.scriptPath;
    t.resultPath = tm.resultPath;
    t.state = base::StateType::READY;
    t.params = tm.params;
    m_newTasks.push(move(t)); 
    Application::loopNotify();
  }
  else if (mtype == base::MessType::PING_WORKER){  // only check
    return;
  }
  else{
    mess::TaskStatus tm(mtype, cp);
    if (!tm.deserialn(data)){
      ERROR_MESS("receiveHandler error deserialn MessType::TASK_STATUS from: " + cp);    
      return;
    }
    int tId = tm.taskId;
    { std::lock_guard<std::mutex> lock(m_mtxProcess);
      
      auto iPrc = find_if(m_procs.begin(), m_procs.end(), [tId](const Process& p){
        return p.getTask().id == tId;
      });
      if (iPrc != m_procs.end()){
        switch (mtype){
          case base::MessType::TASK_PAUSE:    iPrc->pause(); break;
          case base::MessType::TASK_CONTINUE: iPrc->continueTask(); break;
          case base::MessType::TASK_STOP:     iPrc->stop(); break;
          default:{
            ERROR_MESS("receiveHandler wrong task status");
          }
          break;
        }
      }else{
        ERROR_MESS("receiveHandler iPrc == _procs.end() for taskId: " + to_string(tId);
      }
    }
  }  
}