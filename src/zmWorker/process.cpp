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
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include "process.h"

Process::Process(const wTask& tsk, std::function<taskChangeType> taskStateChangeCBack):
  _task(tsk), _taskStateChangeCBack(taskStateChangeCBack){
  
  sigset_t blockMask, origMask;
  struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;
      
  sigemptyset(&blockMask);          /* block SIGCHLD */
  sigaddset(&blockMask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &blockMask, &origMask);

  saIgnore.sa_handler = SIG_IGN;    /* ignor SIGINT и SIGQUIT */
  saIgnore.sa_flags = 0;
  sigemptyset(&saIgnore.sa_mask);
  sigaction(SIGINT, &saIgnore, &saOrigInt);
  sigaction(SIGQUIT, &saIgnore, &saOrigQuit);
  
  switch (_pid = fork()) {
    case -1:    
      break;                        
    case 0:                        
      saDefault.sa_handler = SIG_DFL;
      saDefault.sa_flags = 0;
      sigemptyset(&saDefault.sa_mask);
      if (saOrigInt.sa_handler != SIG_IGN)
        sigaction(SIGINT, &saDefault, NULL);
      if (saOrigQuit.sa_handler != SIG_IGN)
        sigaction(SIGQUIT, &saDefault, NULL);
      sigprocmask(SIG_SETMASK, &origMask, NULL);
      //execl(application.c_str(), args.c_str(), (char *) NULL);
      _exit(127);                  
    default:
      break;
 }
 int savedErrno = errno; 
 sigprocmask(SIG_SETMASK, &origMask, NULL);
 sigaction(SIGINT, &saOrigInt, NULL);
 sigaction(SIGQUIT, &saOrigQuit, NULL);
 errno = savedErrno;
}
Process::~Process(){

}
int Process::getProgress() const{
  return 0;
}
wTask Process::getTask() const{
  return wTask();
}
void Process::pause(){

}
void Process::start(){

}
void Process::stop(){
  
}
