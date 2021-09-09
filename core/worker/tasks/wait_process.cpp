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
      ERROR_MESS("worker::waitProcess error not found process " + to_string(pid));
      continue;
    }
    // completed or error
    if (WIFEXITED(sts) || WIFSIGNALED(sts)){
                
      auto tId = itPrc->getTask().id;
    
      string resultFile = "/tmp/" + to_string(tId) + ".res",
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
      result.erase(remove(result.begin(), result.end(), '\0'), result.end());
      ZM_Aux::replace(result, "'", "''");

      ZM_Base::MessType mt = ZM_Base::MessType::TASK_COMPLETED;
      ZM_Base::StateType st = ZM_Base::StateType::COMPLETED;
      if (WIFEXITED(sts) && isRes){
        sts = WEXITSTATUS(sts);
        if (sts != 0){
          mt = ZM_Base::MessType::TASK_ERROR;
          st = ZM_Base::StateType::ERRORT;
        }
      }else{
        mt = ZM_Base::MessType::TASK_ERROR;
        st = ZM_Base::StateType::ERRORT;
      }
      itPrc->setTaskState(st);

      m_listMessForSchedr.push(MessForSchedr{itPrc->getTask().id,
                                          mt,
                                          result});

      if (remove(resultFile.c_str()) == -1){
        ERROR_MESS("worker::waitProcess error remove " + resultFile + ": " + string(strerror(errno)));
      }
      string scriptFile = "/tmp/" + to_string(tId) + ".sh";
      if (remove(scriptFile.c_str()) == -1){
        ERROR_MESS("worker::waitProcess error remove " + scriptFile + ": " + string(strerror(errno)));
      }    
    }    
    // stop
    else if (WIFSTOPPED(sts)){
      itPrc->setTaskState(ZM_Base::StateType::PAUSE);
      m_listMessForSchedr.push(MessForSchedr{itPrc->getTask().id,
                                          ZM_Base::MessType::TASK_PAUSE,
                                          ""});
    } 
    // continue
    else if (WIFCONTINUED(sts)){
      itPrc->setTaskState(ZM_Base::StateType::RUNNING);
      m_listMessForSchedr.push(MessForSchedr{itPrc->getTask().id,
                                          ZM_Base::MessType::TASK_CONTINUE,
                                          ""});    
    } 
  }  
  
  // check max run time
  for(auto& p : m_procs){
    if (p.checkMaxRunTime() && (p.getTask().state == ZM_Base::StateType::RUNNING)){
      p.stop();
    }
  }
}

#elif _WIN32

#include <fstream>
#include <system_error>
#include <windows.h>

std::string getLastErrorString(){
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); 
    } else {
        return std::system_category().message(errorMessageID);
    }
}

void Executor::waitProcess()
{
  for (auto& p : m_procs){
    
    DWORD status = WaitForSingleObject(p.getHandle(), 0);

    if (status == WAIT_TIMEOUT)
      continue;

    if (!GetExitCodeProcess(p.getHandle(), &status))
      status = -1;
    
    p.closeHandle();   

    auto tId = p.getTask().id; 

    std::string result;
    std::string resultFile = std::to_string(tId) + ".res";
    std::ifstream ifs(resultFile, std::ifstream::in);
    if (ifs.good()){      
      ifs.seekg(0, std::ios::end);   
      result.resize(ifs.tellg());
      ifs.seekg(0, std::ios::beg);
      ifs.read(result.data(), result.size());
      ifs.close();
    }else{
      auto mstr = "worker::waitProcess error open " + resultFile + ": " + getLastErrorString();
      m_app.statusMess(mstr);
      addErrMess(mstr);
      status = -1;
    }
    result.erase(remove(result.begin(), result.end(), '\0'), result.end());
    ZM_Aux::replace(result, "'", "''");

    int toutMs = 0, 
        maxDelayMs = 3000; 
    while ((remove(resultFile.c_str()) == -1) && (toutMs < maxDelayMs)){
      toutMs += 10;
      ZM_Aux::sleepMs(10);
    }   
    if (toutMs == maxDelayMs){
      auto mstr = "worker::waitProcess error remove " + resultFile + ": " + getLastErrorString();
      m_app.statusMess(mstr);
      addErrMess(mstr);
    }
    std::string scriptFile = std::to_string(tId) + ".bat";
    if (remove(scriptFile.c_str()) == -1){
      auto mstr = "worker::waitProcess error remove " + scriptFile + ": " + getLastErrorString();
      m_app.statusMess(mstr);
      addErrMess(mstr);
    }   

    ZM_Base::MessType mt = status == 0 ? ZM_Base::MessType::TASK_COMPLETED : ZM_Base::MessType::TASK_ERROR;
    ZM_Base::StateType st = status == 0 ? ZM_Base::StateType::COMPLETED : ZM_Base::StateType::ERRORT;

    p.setTaskState(st);

    m_listMessForSchedr.push(MessForSchedr{tId,
                                           mt,
                                           result});
  }

  // check max run time
  for(auto& p : m_procs){
    if (p.checkMaxRunTime() && (p.getTask().state == ZM_Base::StateType::RUNNING)){
      p.stop();
    }
  }
}

#endif // _WIN32