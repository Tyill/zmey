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
#include "common/aux_func.h"

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

Process::Process(Application& app, Executor& exr, const ZM_Base::Task& tsk):
  m_app(app),
  m_executor(exr),
  m_task(tsk){

  switch (m_pid = fork()){
    // error
    case -1:{    
      m_err = "worker::Process child error fork: " + string(strerror(errno));
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
      string scriptFile = "/tmp/" + to_string(tsk.id) + ".sh";
      int fdSct = open(scriptFile.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR);
      CHECK(fdSct, "create");
      CHECK(write(fdSct, tsk.script.data(), tsk.script.size()), "write");
      CHECK(close(fdSct), "close");
      
      string resultFile = "/tmp/" + to_string(tsk.id) + ".res";
      int fdRes = open(resultFile.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
      CHECK(fdRes, "create");
      CHECK(dup2(fdRes, 1), "dup2(fdRes, 1)");// stdout -> fdRes
      CHECK(dup2(1, 2), "dup2(1, 2)");        // stderr -> stdout
     
      char** argVec = new char*[!tsk.params.empty() ? 3 : 2];
      argVec[0] = (char*)scriptFile.c_str();
      if (!tsk.params.empty()){
        argVec[1] = (char*)tsk.params.data();
        argVec[2] = NULL;
      }
      else{
        argVec[1] = NULL;
      }
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
bool Process::getResult(string& result){
  #define ERROR_MESS(mstr)   \
    m_err = mstr;            \
    m_app.statusMess(m_err); \
    m_executor.addErrMess(m_err);

  string resultFile = "/tmp/" + to_string(m_task.id) + ".res";
  bool isOk = true;       
  int fdRes = open(resultFile.c_str(), O_RDONLY);
  if (fdRes >= 0){
    off_t fsz = lseek(fdRes, 0, SEEK_END);
    lseek(fdRes, 0, SEEK_SET);
    result.resize(fsz);

    if (read(fdRes, (char*)result.data(), fsz) == -1){
      ERROR_MESS("worker::Process::getResult error read " + resultFile + ": " + string(strerror(errno))); 
      isOk = false;
    }
    close(fdRes);
  }
  else{
    ERROR_MESS("worker::Process::getResult error open " + resultFile + ": " + string(strerror(errno)));
    isOk = false;
  }
  result.erase(remove(result.begin(), result.end(), '\0'), result.end());
  ZM_Aux::replace(result, "'", "''");
  
  if (result.empty() && !m_err.empty()){
    result = m_err;
    isOk = false;
  }

  if (remove(resultFile.c_str()) == -1){
    ERROR_MESS("worker::Process::getResult error remove " + resultFile + ": " + string(strerror(errno)));
  }
  string scriptFile = "/tmp/" + to_string(m_task.id) + ".sh";
  if (remove(scriptFile.c_str()) == -1){
    ERROR_MESS("worker::Process::getResult error remove " + scriptFile + ": " + string(strerror(errno)));
  }    
  return isOk;
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
    m_err = "worker::Process error pause: " + string(strerror(errno));
    m_app.statusMess(m_err);
    m_executor.addErrMess(m_err);
  }
}
void Process::contin(){
  if (kill(m_pid, SIGCONT) == -1){
    m_err = "worker::Process error continue: " + string(strerror(errno));
    m_app.statusMess(m_err);
    m_executor.addErrMess(m_err);
  }
}
void Process::stopByTimeout(){
  m_err = "Stopping by timeout";
  stop();
}
void Process::stop(){
  if (kill(m_pid, SIGTERM) == -1){
    m_err = "worker::Process error stop: " + string(strerror(errno));
    m_app.statusMess(m_err);
    m_executor.addErrMess(m_err);
  }
}

#elif _WIN32

#include <fstream>
#include <algorithm>
#include <system_error>

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
          remove(resultFile.c_str());          \
          remove(scriptFile.c_str());          \
        }                                      \
        return;

Process::Process(Application& app, Executor& exr, const ZM_Base::Task& tsk):
  m_app(app),
  m_executor(exr),
  m_task(tsk){
  
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;

  std::string resultFile = std::to_string(tsk.id) + ".res";
  std::string scriptFile = std::to_string(tsk.id) + ".bat";

  HANDLE hOutFile = CreateFileA(resultFile.c_str(),
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
      
  std::string cmd;
  size_t shebPos = tsk.script.find("#!");
  size_t endline = tsk.script.find('\n');
  if ((shebPos != std::string::npos) && (endline != std::string::npos)) {
      cmd = ZM_Aux::trim(tsk.script.substr(shebPos + 2, endline - shebPos - 2));

      std::ofstream ofs(scriptFile, std::ofstream::out);
      if (ofs.good()) {
        ofs << tsk.script.substr(endline + 1).c_str();
        ofs.close();
      }
      else {
        ERR_RETURN("worker::Process create of script file error: " + scriptFile);
      }
      cmd += " " + scriptFile + " " + tsk.params;
  }
  else {
    ERR_RETURN("worker::Process shebang of script error");    
  }
  BOOL ret = CreateProcessA(NULL, (char*)cmd.c_str(), NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);
 
  if (ret){
    m_hProcess = pi.hProcess;
    m_hThread = pi.hThread;
    m_hResFile = hOutFile;
  }
  else{
    ERR_RETURN("worker::Process CreateProcessA " + cmd + " error: " + getLastErrorStr());
  }
  m_timerProgress.updateCycTime();
  m_timerDuration.updateCycTime();
  m_task.state = ZM_Base::StateType::RUNNING;  
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
  return std::min<int>(100, dt * 100 / std::max<int>(1, m_task.averDurationSec));
}
bool Process::getResult(std::string& result){
 #define ERROR_MESS(mstr)  \
    m_err = mstr;           \
    m_app.statusMess(m_err); \
    m_executor.addErrMess(m_err);
    
  bool isOk = true;
  std::string resultFile = std::to_string(m_task.id) + ".res";
  std::ifstream ifs(resultFile, std::ifstream::in);
  if (ifs.good()){      
    ifs.seekg(0, std::ios::end);   
    result.resize(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    ifs.read(result.data(), result.size());
    ifs.close();
  }else{
    ERROR_MESS("worker::Process::getResult error open " + resultFile + ": " + getLastErrorStr());
    isOk = false;
  }
      
  result.erase(remove(result.begin(), result.end(), '\0'), result.end());
  ZM_Aux::replace(result, "'", "''");

  if (result.empty() && !m_err.empty()){
    result = m_err;
    isOk = false;
  }

  int toutMs = 0, 
      maxDelayMs = 1000; 
  while ((remove(resultFile.c_str()) == -1) && (toutMs < maxDelayMs)){
    toutMs += 10;
    ZM_Aux::sleepMs(10);
  }   
   
  if (toutMs == maxDelayMs){
    ERROR_MESS("worker::Process::getResult error remove " + resultFile + ": " + getLastErrorStr());    
  }
  std::string scriptFile = std::to_string(m_task.id) + ".bat";
  if (remove(scriptFile.c_str()) == -1){
    ERROR_MESS("worker::Process::getResult error remove " + scriptFile + ": " + getLastErrorStr());
  }  
  return isOk; 
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
    // TODO  
}
void Process::contin(){
    // TODO 
}
void Process::stopByTimeout(){
  m_err = "Stopping by timeout";
  stop();
}
void Process::stop(){
  TerminateProcess(m_hProcess, -1);
}


#endif  // _WIN32