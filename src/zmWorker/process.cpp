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
#pragma once

#include "process.h"

namespace ZM_Aux{

Process::Process(const std::string& applicationWithArgs){}
void Process::exec(){

  if (pid_ != -1)
      throw exception{"process already started"};

  auto pid = fork();
  if (pid == -1){
      perror("fork()");
      throw exception{"Failed to fork child process"};
  }
  else if (pid == 0){
      err_pipe.close(pipe_t::read_end());
      pipe_buf_.stdin_pipe().close(pipe_t::write_end());
      pipe_buf_.stdout_pipe().close(pipe_t::read_end());
      pipe_buf_.stdout_pipe().dup(pipe_t::write_end(), STDOUT_FILENO);
      err_buf_.stdout_pipe().close(pipe_t::read_end());
      err_buf_.stdout_pipe().dup(pipe_t::write_end(), STDERR_FILENO);

      if (read_from_){
      read_from_->recursive_close_stdin();
      pipe_buf_.stdin_pipe().close(pipe_t::read_end());
      read_from_->pipe_buf_.stdout_pipe().dup(pipe_t::read_end(),
                                              STDIN_FILENO);
      }else{
      pipe_buf_.stdin_pipe().dup(pipe_t::read_end(), STDIN_FILENO);
      }

      std::vector<char*> args;
      args.reserve(args_.size() + 1);
      for (auto& arg : args_)
      args.push_back(const_cast<char*>(arg.c_str()));
      args.push_back(nullptr);

      limits_.set_limits();
      execvp(args[0], args.data());

      char err[sizeof(int)];
      std::memcpy(err, &errno, sizeof(int));
      err_pipe.write(err, sizeof(int));
      err_pipe.close();
      std::_Exit(EXIT_FAILURE);

  }else{
      err_pipe.close(pipe_t::write_end());
      pipe_buf_.stdout_pipe().close(pipe_t::write_end());
      err_buf_.stdout_pipe().close(pipe_t::write_end());
      pipe_buf_.stdin_pipe().close(pipe_t::read_end());

      if (read_from_){
      pipe_buf_.stdin_pipe().close(pipe_t::write_end());
      read_from_->pipe_buf_.stdout_pipe().close(pipe_t::read_end());
      read_from_->err_buf_.stdout_pipe().close(pipe_t::read_end());
      }
      pid_ = pid;

      char err[sizeof(int)];
      auto bytes = err_pipe.read(err, sizeof(int));
      if (bytes == sizeof(int)){
      int ec = 0;
      std::memcpy(&ec, err, sizeof(int));
      throw exception{"Failed to exec process: "
                      + std::system_category().message(ec)};
      }else{
      err_pipe.close();
      }
  }
  }
  Process::~process(){
      wait();
  }
  pid_t Process::id() const{
      return pid_;
  }
  void Process::wait(){
      if (!waited_){
          waitpid(pid_, &status_, 0);
          pid_ = -1;
          waited_ = true;
      }
  }
  bool Process::waited() const{
      return waited_;
  }
  bool Process::running() const{
      return ::procxx::running(*this);
  }
  bool Process::exited() const
  {
      if (!waited_)
          throw exception{"process::wait() not yet called"};
      return WIFEXITED(status_);
  }
  bool Process::killed() const
  {
      if (!waited_)
          throw exception{"process::wait() not yet called"};
      return WIFSIGNALED(status_);
  }
  bool Process::stopped() const
  {
      if (!waited_)
          throw exception{"process::wait() not yet called"};
      return WIFSTOPPED(status_);
  }
  int Process::code() const
  {
      if (!waited_)
          throw exception{"process::wait() not yet called"};
      if (exited())
          return WEXITSTATUS(status_);
      if (killed())
          return WTERMSIG(status_);
      if (stopped())
          return WSTOPSIG(status_);
      return -1;
  }   
  void Process::close()
  {
      pipe_buf_.close(end);
      err_buf_.close(end);
  }
};
