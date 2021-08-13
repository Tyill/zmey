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
#include "process.h"
#include "application.h"
#include "executor.h"
#include "zmCommon/aux_func.h"

#include <utility>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

using namespace std;

Process::Process(Application& app, Executor& exr, const ZM_Base::Task& tsk):
  m_app(app),
  m_executor(exr),
  m_task(tsk){

  switch (m_pid = fork()){
    // error
    case -1:{    
      string mstr = "worker::Process child error fork: " + string(strerror(errno));
      m_app.statusMess(mstr);
      m_executor.addErrMess(mstr);
    }
      break;
    // children                        
    case 0:{
      #define CHECK(fd, err)   \
                if (fd == -1){ \
                  perror(err); \
                  _exit(127);  \
                }    
      string scriptFile = "/tmp/" + to_string(tsk.id) + ".script";
      int fdSct = open(scriptFile.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR);
      CHECK(fdSct, "create");
      CHECK(write(fdSct, tsk.script.data(), tsk.script.size()), "write");
      CHECK(close(fdSct), "close");
      
      string resultFile = "/tmp/" + to_string(tsk.id) + ".result";
      int fdRes = open(resultFile.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
      CHECK(fdRes, "create");
      CHECK(dup2(fdRes, 1), "dup2(fdRes, 1)");// stdout -> fdRes
      CHECK(dup2(1, 2), "dup2(1, 2)");        // stderr -> stdout
      
      auto params = !tsk.params.empty() ? ZM_Aux::split(tsk.params, ',') : vector<string>();

      char** argVec = new char*[params.size() + 2];
      argVec[0] = (char*)scriptFile.c_str();
      for(size_t i = 0; i < params.size(); ++i){
        argVec[i + 1] = (char*)params[i].data();
      }    
      argVec[params.size() + 1] = NULL;
      execv(scriptFile.c_str(), argVec);
      perror("execv");
      _exit(127);
    }
      break;
    // parent                
    default:
      m_timerProgress.updateCycTime();
      m_timerDuration.updateCycTime();
      m_task.state = ZM_Base::StateType::RUNNING;
      break;
  }
}

ZM_Base::Task Process::getTask() const{
  return m_task;
}
pid_t Process::getPid() const{
  return m_pid;
}
int Process::getProgress(){
  if (!m_isPause){
    m_cdeltaTimeProgress += m_timerProgress.getDeltaTimeMS();
  }
  m_timerProgress.updateCycTime();
  int dt = (int)m_cdeltaTimeProgress / 1000;
  return min(100, dt * 100 / max(1, m_task.averDurationSec));
}
bool Process::checkMaxRunTime(){
  if (!m_isPause){
    m_cdeltaTimeDuration += m_timerDuration.getDeltaTimeMS();
  }
  m_timerDuration.updateCycTime();
  return int(m_cdeltaTimeDuration / 1000) > m_task.maxDurationSec;
}
void Process::setTaskState(ZM_Base::StateType st){
  m_task.state = st;
  m_isPause = (st == ZM_Base::StateType::PAUSE);
}
void Process::pause(){
  if (kill(m_pid, SIGSTOP) == -1){
    string mstr = "worker::Process error pause: " + string(strerror(errno));
    m_app.statusMess(mstr);
    m_executor.addErrMess(mstr);
  }
}
void Process::contin(){
  if (kill(m_pid, SIGCONT) == -1){
    string mstr = "worker::Process error continue: " + string(strerror(errno));
    m_app.statusMess(mstr);
    m_executor.addErrMess(mstr);
  }
}
void Process::stop(){
  if (kill(m_pid, SIGTERM) == -1){
    string mstr = "worker::Process error stop: " + string(strerror(errno));
    m_app.statusMess(mstr);
    m_executor.addErrMess(mstr);
  }
}
