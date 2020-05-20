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

namespace ZM_Aux{

Process::Process(const std::string& application, const std::string& args){
  auto pid = fork();
  if (pid == -1){
      perror("fork()");
  }
  else if (pid == 0){
    
    execvp(application, args.data());

  }else{
    // pid_ = pid;

    // char err[sizeof(int)];
    // auto bytes = err_pipe.read(err, sizeof(int));
    // if (bytes == sizeof(int)){
    // int ec = 0;
    // std::memcpy(&ec, err, sizeof(int));
    // throw exception{"Failed to exec process: "
    //                 + std::system_category().message(ec)};
    // }else{
    // err_pipe.close();
    // }
  }
}
Process::~Process(){
  //  wait();
}
void Process::wait(){
  // if (!waited_){
  //   waitpid(pid_, &status_, 0);
  //   pid_ = -1;
  //   waited_ = true;
  // }
}
bool Process::running() const{
  //return ::procxx::running(*this);
}
bool Process::exited() const{
  // if (!waited_)
  //     throw exception{"process::wait() not yet called"};
  // return WIFEXITED(status_);
}
bool Process::killed() const{
  // if (!waited_)
  //     throw exception{"process::wait() not yet called"};
  // return WIFSIGNALED(status_);
}
bool Process::stopped() const{
  // if (!waited_)
  //     throw exception{"process::wait() not yet called"};
  // return WIFSTOPPED(status_);
}
int Process::code() const{
  // if (!waited_)
  //     throw exception{"process::wait() not yet called"};
  // if (exited())
  //     return WEXITSTATUS(status_);
  // if (killed())
  //     return WTERMSIG(status_);
  // if (stopped())
  //     return WSTOPSIG(status_);
  return -1;
}
};
