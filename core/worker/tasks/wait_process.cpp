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

  
#define ERROR_MESS(mstr)  \
  m_app.statusMess(mstr); \
  m_errMess.push(mstr);   \

void Executor::waitProcess()
{
  pid_t pid;
  int sts = 0;
  
  while ((pid = waitpid(-1, &sts, WNOHANG | WUNTRACED | WCONTINUED)) > 0){

    auto itPrc = find_if(m_procs.begin(), m_procs.end(),[pid](const Process& p){
      return p.getPid() == pid;
    });    
    if (itPrc == m_procs.end()){
      ERROR_MESS("waitProcess error not found process " + to_string(pid));
      continue;
    }
    // completed or error
    if (WIFEXITED(sts) || WIFSIGNALED(sts)){
               
      Base::MessType mt = Base::MessType::TASK_COMPLETED;
      Base::StateType st = Base::StateType::COMPLETED;
      if (WIFEXITED(sts)){
        sts = WEXITSTATUS(sts);
        if (sts != 0){
          mt = Base::MessType::TASK_ERROR;
          st = Base::StateType::ERRORT;
        }
      }else{
        mt = Base::MessType::TASK_ERROR;
        st = Base::StateType::ERRORT;
      }
      itPrc->setTaskState(st);
      
      m_listMessForSchedr.push(MessForSchedr{itPrc->getTask().id,
                                          mt});
    }    
    // stop
    else if (WIFSTOPPED(sts)){
      itPrc->setTaskState(Base::StateType::PAUSE);
      m_listMessForSchedr.push(MessForSchedr{itPrc->getTask().id,
                                          Base::MessType::TASK_PAUSE});
    } 
    // continue
    else if (WIFCONTINUED(sts)){
      itPrc->setTaskState(Base::StateType::RUNNING);
      m_listMessForSchedr.push(MessForSchedr{itPrc->getTask().id,
                                          Base::MessType::TASK_CONTINUE});    
    } 
  }  
  
  // check max run time
  for(auto& p : m_procs){
    if (p.checkMaxRunTime() && (p.getTask().state == Base::StateType::RUNNING)){
      p.stopByTimeout();
    }
  }

  { std::lock_guard<std::mutex> lock(m_mtxProcess);    
    for (auto p = m_procs.begin(); p != m_procs.end();){
      Base::StateType TaskState = p->getTask().state;
      if ((TaskState == Base::StateType::COMPLETED) ||
          (TaskState == Base::StateType::ERRORT)){
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
  
    Base::MessType mt = status == 0 ? Base::MessType::TASK_COMPLETED : Base::MessType::TASK_ERROR;
    Base::StateType st = status == 0 ? Base::StateType::COMPLETED : Base::StateType::ERRORT;

    p.setTaskState(st);

    m_listMessForSchedr.push(MessForSchedr{p.getTask().id,
                                           mt});
  }

  // check max run time
  for(auto& p : m_procs){
    if (p.checkMaxRunTime() && (p.getTask().state == Base::StateType::RUNNING)){
      p.stopByTimeout();
    }
  }

  { std::lock_guard<std::mutex> lock(m_mtxProcess);    
    for (auto p = m_procs.begin(); p != m_procs.end();){
      Base::StateType TaskState = p->getTask().state;
      if ((TaskState == Base::StateType::COMPLETED) ||
          (TaskState == Base::StateType::ERRORT)){
        p = m_procs.erase(p);
      }else{
        ++p;
      }
    }
  }
}

#endif // _WIN32