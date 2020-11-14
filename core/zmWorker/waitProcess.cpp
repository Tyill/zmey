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
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <list>
#include <algorithm>

#include "zmCommon/auxFunc.h"
#include "zmCommon/queue.h"
#include "process.h"

using namespace std;

extern ZM_Aux::QueueThrSave<string> _errMess;

void waitProcess(ZM_Base::Worker& worker, list<Process>& procs, ZM_Aux::QueueThrSave<MessForSchedr>& listMessForSchedr){
  
#define ERROR_MESS(mstr) \
  statusMess(mstr);      \
  _errMess.push(mstr);   \

  pid_t pid;
  int sts = 0;
  
  while ((pid = waitpid(-1, &sts, WNOHANG | WUNTRACED | WCONTINUED)) > 0){

    auto itPrc = find_if(procs.begin(), procs.end(),[pid](const Process& p){
        return p.getPid() == pid;
      });    
    if (itPrc == procs.end()){
      ERROR_MESS("worker::waitProcess error not found process " + to_string(pid));
      continue;
    }
    // completed or error
    if (WIFEXITED(sts) || WIFSIGNALED(sts)){
                
      auto tId = itPrc->getTask().base.id;
    
      string resultFile = to_string(tId) + ".result",
             result;
      bool isRes = true;       
      int fdRes = open(resultFile.c_str(), O_RDONLY);
      if (fdRes >= 0){
        off_t fsz = lseek(fdRes, 0, SEEK_END);
        lseek(fdRes, 0, SEEK_SET);
        result.resize(fsz);

        if (read(fdRes, (char*)result.data(), fsz) == -1){
          ERROR_MESS("worker::waitProcess error read " + resultFile + ": " + string(strerror(errno))); 
          isRes = false;
        }
        close(fdRes);
      }
      else{
        ERROR_MESS("worker::waitProcess error open " + resultFile + ": " + string(strerror(errno)));
        isRes = false;
      }

      ZM_Base::MessType mt = ZM_Base::MessType::TASK_COMPLETED;
      ZM_Base::StateType st = ZM_Base::StateType::COMPLETED;
      if (WIFEXITED(sts) && isRes){
        sts = WEXITSTATUS(sts);
        if (sts != 0){
          mt = ZM_Base::MessType::TASK_ERROR;
          st = ZM_Base::StateType::ERROR;
        }
      }else{
        mt = ZM_Base::MessType::TASK_ERROR;
        st = ZM_Base::StateType::ERROR;
      }
      itPrc->setTaskState(st);

      listMessForSchedr.push(MessForSchedr{itPrc->getTask().base.id,
                                          mt,
                                          result});

      if (remove(resultFile.c_str()) == -1){
        ERROR_MESS("worker::waitProcess error remove " + resultFile + ": " + string(strerror(errno)));
      }
      string scriptFile = to_string(tId) + ".script";
      if (remove(scriptFile.c_str()) == -1){
        ERROR_MESS("worker::waitProcess error remove " + scriptFile + ": " + string(strerror(errno)));
      }    
    }    
    // stop
    else if (WIFSTOPPED(sts)){
      itPrc->setTaskState(ZM_Base::StateType::PAUSE);
      listMessForSchedr.push(MessForSchedr{itPrc->getTask().base.id,
                                          ZM_Base::MessType::TASK_PAUSE,
                                          ""});
    } 
    // continue
    else if (WIFCONTINUED(sts)){
      itPrc->setTaskState(ZM_Base::StateType::RUNNING);
      listMessForSchedr.push(MessForSchedr{itPrc->getTask().base.id,
                                          ZM_Base::MessType::TASK_CONTINUE,
                                          ""});    
    } 
  }  
  
  // check max run time
  for(auto& p : procs){
    if (p.checkMaxRunTime() && (p.getTask().state == ZM_Base::StateType::RUNNING)){
      p.stop();
    }
  }
}