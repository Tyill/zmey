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
#include <utility>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

#include "zmCommon/auxFunc.h"
#include "zmCommon/queue.h"
#include "process.h"

using namespace std;

extern ZM_Aux::QueueThrSave<mess2schedr> _messForSchedr;

Process::Process(const wTask& tsk):
  _task(tsk){

  switch (_pid = fork()){
    // error
    case -1:{    
      _task.state = ZM_Base::stateType::error;
      string err = "Process child error fork: " + string(strerror(errno));
      _messForSchedr.push(mess2schedr{tsk.base.id, ZM_Base::messType::taskError, err});
      statusMess(err);
    }
      break;
    // children                        
    case 0:{
      #define CHECK(fd, err)   \
                if (fd == -1){ \
                  perror(err); \
                  _exit(127);  \
                }    
      string scriptFile = to_string(tsk.base.id) + ".script";
      int fdSct = open(scriptFile.c_str(), O_CREAT | O_TRUNC | O_WRONLY);
      CHECK(fdSct, "create");
      CHECK(write(fdSct, tsk.base.script.data(), tsk.base.script.size()), "write");
      CHECK(close(fdSct), "close");
      CHECK(chmod(scriptFile.c_str(), S_IRUSR | S_IXUSR), "chmod");

      string resultFile = to_string(tsk.base.id) + ".result";
      int fdRes = open(resultFile.c_str(), O_CREAT | O_TRUNC | O_RDWR);
      CHECK(fdRes, "create");
      CHECK(dup2(fdRes, 1), "dup2(fdRes, 1)");// stdout -> fdRes
      CHECK(dup2(1, 2), "dup2(1, 2)");        // stderr -> stdout
     
      string params;
      if (!tsk.params.empty()){
        params = string(tsk.params.data() + 1, tsk.params.size() - 2); // remove '{' and '}'
      }
      auto prmVec = ZM_Aux::split(params, ",");
      char** argVec = new char*[prmVec.size() + 2];
      argVec[0] = (char*)scriptFile.c_str();
      size_t psz = prmVec.size();
      for (size_t i = 0; i < psz; ++i){
        argVec[i + 1] = (char*)prmVec[i].data();
      } 
      argVec[psz + 1] = NULL;
      execv(scriptFile.c_str(), argVec);
      perror("execv");
      _exit(127);
    }
      break;
    // parent                
    default:
      _timer.updateCycTime();
      _task.state = ZM_Base::stateType::running;
      _messForSchedr.push(mess2schedr{tsk.base.id, ZM_Base::messType::taskRunning, ""});
      break;
 }

}
Process::~Process(){
}
int Process::getProgress() const{
  if (!_isPause){
    _cdeltaTime += _timer.getDeltaTimeMS();
  }
  _timer.updateCycTime();
  int dt = (int)_cdeltaTime / 1000;
  return min(100, int((dt * 100.0)/ max(1, _task.base.averDurationSec)));
}
wTask Process::getTask() const{
  return _task;
}
pid_t Process::getPid() const{
  return _pid;
}
void Process::setTaskState(ZM_Base::stateType st){
  _task.state = st;
  _isPause = (st == ZM_Base::stateType::pause);
}
void Process::pause(){
  if (kill(_pid, SIGSTOP) == -1){
    statusMess("Process error pause: " + string(strerror(errno)));
  }
}
void Process::start(){
  if (kill(_pid, SIGCONT) == -1){
    statusMess("Process error start: " + string(strerror(errno)));
  }
}
void Process::stop(){
  if (kill(_pid, SIGTERM) == -1){
    statusMess("Process error stop: " + string(strerror(errno)));
  }
}
