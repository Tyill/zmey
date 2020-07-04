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
#include <errno.h>
#include "process.h"

using namespace std;

static struct sigaction saDefault;

void childHandler(int sig);

Process::Process(const wTask& tsk, std::function<taskChangeType> taskStateChangeCBack):
  _task(tsk), _taskStateChangeCBack(taskStateChangeCBack){
 
  if (!saDefault.sa_handler){
    saDefault.sa_handler = childHandler;         
    saDefault.sa_flags = 0;
    sigemptyset(&saDefault.sa_mask); 
    sigaddset(&saDefault.sa_mask, SIGCHLD);
    sigaction(SIGCHLD, &saDefault, NULL);
  }
  switch (_pid = fork()){
    // error
    case -1:    
      break;
    // children                        
    case 0:
      signal(SIGCHLD, SIG_DFL);
     // execl(application.c_str(), args.c_str(), (char*) NULL);
      _exit(127);
    // parent                
    default:

      

      break;
 }

}
Process::~Process(){
}
int Process::getProgress() const{
  return 0;
}
wTask Process::getTask() const{
  return _task;
}
void Process::pause(){

}
void Process::start(){

}
void Process::stop(){
  
}
