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
#include "common/misc.h"

#ifdef __linux__ 

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

Process::Process(Application& app, Executor& exr, const base::Task& tsk):
  m_app(app),
  m_executor(exr),
  m_task(tsk){

  switch (m_pid = fork()){
    // error
    case -1:{    
      m_err = "Process child error fork: " + string(strerror(errno));
      m_app.statusMess(m_err);
      m_executor.addErrMess(m_err);
    }
      break;
    // children                        
    case 0:{
      #define CHECK(fd, err)   \
                if (fd == -1){ \
                  perror(err); \
                  _exit(127);  \
                }  
                
      misc::createSubDirectory(tsk.tResultPath);
      string resultPath = tsk.tResultPath + to_string(tsk.tId) + ".dat";
      int fdRes = open(resultPath.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
      CHECK(fdRes, "create");
      CHECK(dup2(fdRes, 1), "dup2(fdRes, 1)");// stdout -> fdRes
      CHECK(dup2(1, 2), "dup2(1, 2)");        // stderr -> stdout
     
      char** argVec = new char*[!tsk.tParams.empty() ? 3 : 2];
      argVec[0] = (char*)tsk.tScriptPath.c_str();
      if (!tsk.tParams.empty()){
        argVec[1] = (char*)tsk.tParams.data();
        argVec[2] = NULL;
      }
      else{
        argVec[1] = NULL;
      }
      execv(tsk.tScriptPath.c_str(), argVec);
      perror("execv");
      _exit(127);
    }
      break;
    // parent                
    default:
      m_timerProgress.updateCycTime();
      m_timerDuration.updateCycTime();
      m_task.tState = base::StateType::RUNNING;
      break;
  }
}

base::Task Process::getTask() const{
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
  return min(100, dt * 100 / max(1, m_task.tAverDurationSec));
}
bool Process::checkMaxRunTime(){
  if (!m_isPause){
    m_cdeltaTimeDuration += m_timerDuration.getDeltaTimeMS();
  }
  m_timerDuration.updateCycTime();
  return (m_task.tMaxDurationSec > 0) && (int(m_cdeltaTimeDuration / 1000) > m_task.tMaxDurationSec);
}
void Process::setTaskState(base::StateType st){
  m_task.tState = st;
  m_isPause = (st == base::StateType::PAUSE);
}
void Process::pause(){
  if (kill(m_pid, SIGSTOP) == -1){
    m_err = "Process error pause: " + string(strerror(errno));
    m_app.statusMess(m_err);
    m_executor.addErrMess(m_err);
  }
}
void Process::continueTask(){
  if (kill(m_pid, SIGCONT) == -1){
    m_err = "Process error continue: " + string(strerror(errno));
    m_app.statusMess(m_err);
    m_executor.addErrMess(m_err);
  }
}
void Process::stop(){
  m_err = "Stopping by command from user";
  if (kill(m_pid, SIGTERM) == -1){
    m_err = "Process error stop: " + string(strerror(errno));
    m_app.statusMess(m_err);
    m_executor.addErrMess(m_err);
  }
}
void Process::stopByTimeout(){
  m_err = "Stopping by timeout";
  if (kill(m_pid, SIGTERM) == -1) {
      m_err = "Process error stop: " + string(strerror(errno));
      m_app.statusMess(m_err);
      m_executor.addErrMess(m_err);
  }
}

#elif _WIN32

#include <fstream>
#include <algorithm>
#include <system_error>
#include <string>

#include <windows.h>

std::string getLastErrorStr(){
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); 
    } else {
        return std::system_category().message(errorMessageID);
    }
}

#define ERR_RETURN(err)   \
        m_pid = -1;       \
        m_err = err;      \
        m_app.statusMess(m_err);      \
        m_executor.addErrMess(m_err); \
        if (hOutFile != INVALID_HANDLE_VALUE){ \
          CloseHandle(hOutFile);               \
        }                                      \
        return;

Process::Process(Application& app, Executor& exr, const base::Task& tsk):
  m_app(app),
  m_executor(exr),
  m_task(tsk){
  
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;
 
  misc::createSubDirectory(tsk.resultPath);
  std::string resultPath = tsk.resultPath + std::to_string(tsk.id) + ".dat";

  HANDLE hOutFile = CreateFileA(resultPath.c_str(),
      FILE_WRITE_DATA,
      FILE_SHARE_WRITE | FILE_SHARE_READ,
      &sa,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_TEMPORARY,
      NULL);

  if (hOutFile == INVALID_HANDLE_VALUE){
    ERR_RETURN("worker::Process CreateFileA " + std::to_string(tsk.id) + ".res error: " + getLastErrorStr());
  }
  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

  STARTUPINFOA si;
  ZeroMemory(&si, sizeof(STARTUPINFOA));

  DWORD flags = CREATE_NO_WINDOW;
  
  si.cb = sizeof(STARTUPINFO);
  si.dwFlags |= STARTF_USESTDHANDLES;
  si.hStdInput = NULL;
  si.hStdError = hOutFile;
  si.hStdOutput = hOutFile;
      
  std::string script;
  std::ifstream ifs(tsk.scriptPath, std::ifstream::in);
  if (ifs.good()){      
    ifs.seekg(0, std::ios::end);   
    script.resize(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    ifs.read(script.data(), script.size());
    ifs.close();
  }     
  else {
    ERR_RETURN("Process read of script file: " + tsk.scriptPath);
  }

  std::string cmd;
  size_t shebPos = script.find("#!");
  size_t endline = script.find('\n');
  if ((shebPos != std::string::npos) && (endline != std::string::npos)) {
      cmd = misc::trim(script.substr(shebPos + 2, endline - shebPos - 2));      
      cmd += " " + tsk.scriptPath + " " + tsk.params;
  }
  else {
    ERR_RETURN("Process shebang of script error");    
  }
  BOOL ret = CreateProcessA(NULL, (char*)cmd.c_str(), NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);
 
  if (ret){
    m_hProcess = pi.hProcess;
    m_hThread = pi.hThread;
    m_hResFile = hOutFile;
  }
  else{
    ERR_RETURN("Process CreateProcessA " + cmd + " error: " + getLastErrorStr());
  }
  m_timerProgress.updateCycTime();
  m_timerDuration.updateCycTime();
  m_task.state = base::StateType::RUNNING;  
}

HANDLE Process::getHandle() const{
  return m_hProcess;
}

void Process::closeHandle(){
  if (m_hProcess && m_hThread){
    CloseHandle(m_hProcess);
    CloseHandle(m_hThread);
    CloseHandle(m_hResFile);
  }
}

base::Task Process::getTask() const{
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
  return std::min<int>(100, dt * 100 / std::max<int>(1, m_task.averDurationSec));
}
bool Process::checkMaxRunTime(){
  if (!m_isPause){
    m_cdeltaTimeDuration += m_timerDuration.getDeltaTimeMS();
  }
  m_timerDuration.updateCycTime();
  return (m_task.maxDurationSec > 0) && (int(m_cdeltaTimeDuration / 1000) > m_task.maxDurationSec);
}
void Process::setTaskState(base::StateType st){
  m_task.state = st;
  m_isPause = (st == base::StateType::PAUSE);
}

void Process::pause(){
  if (SuspendThread(m_hThread) == DWORD(-1)){
    m_err = "Process error pause: " + getLastErrorStr();
    m_app.statusMess(m_err);
    m_executor.addErrMess(m_err);
  }
  else {
      m_executor.addMessForSchedr(mess::TaskStatus{ m_task.id, mess::MessType::TASK_PAUSE });
      setTaskState(base::StateType::PAUSE);
  }
}
void Process::continueTask(){
  if (ResumeThread(m_hThread) == DWORD(-1)){
    m_err = "Process error continue: " + getLastErrorStr();
    m_app.statusMess(m_err);
    m_executor.addErrMess(m_err);
  }
  else {
      m_executor.addMessForSchedr(mess::TaskStatus{ m_task.id, mess::MessType::TASK_CONTINUE });
      setTaskState(base::StateType::RUNNING);
  }
}
void Process::stopByTimeout(){
  m_err = "Stopping by timeout";
  TerminateProcess(m_hProcess, -1);
}
void Process::stop(){
  m_err = "Stopping by command from user";
  TerminateProcess(m_hProcess, -1);
}


#endif  // _WIN32