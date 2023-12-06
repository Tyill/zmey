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

#ifdef __linux__ 

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <list>
#include <algorithm>


using namespace std;
  
void Executor::waitProcess()
{
  pid_t pid;
  int sts = 0;
  
  while ((pid = waitpid(-1, &sts, WNOHANG | WUNTRACED | WCONTINUED)) > 0){

    auto itPrc = find_if(m_procs.begin(), m_procs.end(),[pid](const Process& p){
      return p.getPid() == pid;
    });    
    if (itPrc == m_procs.end()){
      errorMessage("waitProcess error not found process " + to_string(pid));
      continue;
    }
    // completed or error
    if (WIFEXITED(sts) || WIFSIGNALED(sts)){               
      mess::MessType mt = mess::MessType::TASK_COMPLETED;
      base::StateType st = base::StateType::COMPLETED;
      if (WIFEXITED(sts)){
        sts = WEXITSTATUS(sts);
        if (sts != 0){
          mt = mess::MessType::TASK_ERROR;
          st = base::StateType::ERRORT;
        }
      }else{
        mt = mess::MessType::TASK_ERROR;
        st = base::StateType::ERRORT;
      }
      itPrc->setTaskState(st);      
      m_messForSchedr.push(mess::TaskStatus{itPrc->getTask().tId, mt});
    }    
    // stop
    else if (WIFSTOPPED(sts)){
      itPrc->setTaskState(base::StateType::PAUSE);
      m_messForSchedr.push(mess::TaskStatus{itPrc->getTask().tId, mess::MessType::TASK_PAUSE});
    } 
    // continue
    else if (WIFCONTINUED(sts)){
      itPrc->setTaskState(base::StateType::RUNNING);
      m_messForSchedr.push(mess::TaskStatus{itPrc->getTask().tId, mess::MessType::TASK_CONTINUE});    
    } 
  }  
  // check max run time
  for(auto& p : m_procs){
    if (p.checkMaxRunTime() && (p.getTask().tState == base::StateType::RUNNING)){
      p.stopByTimeout();
    }
  }
  
  { std::lock_guard<std::mutex> lock(m_mtxProcess);    
    for (auto p = m_procs.begin(); p != m_procs.end();){
      base::StateType TaskState = p->getTask().tState;
      if ((TaskState == base::StateType::COMPLETED) ||
          (TaskState == base::StateType::ERRORT)){
        p = m_procs.erase(p);
      }else{
        ++p;
      }
    }
  }
}

#elif _WIN32

#include <fstream>
#include <system_error>
#include <windows.h>

void Executor::waitProcess()
{
  for (auto& p : m_procs){
    
    DWORD status = WaitForSingleObject(p.getHandle(), 0);

    if (status == WAIT_TIMEOUT)
      continue;

    if (!GetExitCodeProcess(p.getHandle(), &status))
      status = -1;

    p.closeHandle();   
  
    mess::MessType mt = status == 0 ? mess::MessType::TASK_COMPLETED : mess::MessType::TASK_ERROR;
    base::StateType st = status == 0 ? base::StateType::COMPLETED : base::StateType::ERRORT;

    p.setTaskState(st);

    m_listMessForSchedr.push(MessForSchedr{p.getTask().id,
                                           mt});
  }

  // check max run time
  for(auto& p : m_procs){
    if (p.checkMaxRunTime() && (p.getTask().state == base::StateType::RUNNING)){
      p.stopByTimeout();
    }
  }

  { std::lock_guard<std::mutex> lock(m_mtxProcess);    
    for (auto p = m_procs.begin(); p != m_procs.end();){
      base::StateType TaskState = p->getTask().state;
      if ((TaskState == base::StateType::COMPLETED) ||
          (TaskState == base::StateType::ERRORT)){
        p = m_procs.erase(p);
      }else{
        ++p;
      }
    }
  }
}

#endif // _WIN32