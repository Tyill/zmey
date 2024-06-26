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

#include "common/queue.h"
#include "common/misc.h"
#include "base/base.h"
#include "base/messages.h"
#include "worker/application.h"
#include "worker/process.h" 

#include <list>
#include <atomic>

class Loop;

class Executor{
public:  
  Executor(Application&, const std::string& connPnt);

public:  
  void setLoop(Loop*);
  void loopNotify();
  void loopStop();

  void addMessForSchedr(mess::TaskStatus);
  void addErrMess(std::string);
  void setLoadCPU(int);
  bool isErrMessEmpty();
  bool isNewTasksEmpty();
  bool isMessForSchedrEmpty();

  void receiveHandler(const std::string& cp, const std::string& data);
  void sendNotifyHandler(const std::string& cp, const std::string& data, const std::error_code& ec);
  void messageToSchedr(const std::string& schedrConnPnt);
  void progressToSchedr(const std::string& schedrConnPnt);
  void pingToSchedr(const std::string& schedrConnPnt);
  void stopToSchedr(const std::string& schedrConnPnt);
  void errorToSchedr(const std::string& schedrConnPnt);
  void updateListTasks();
  void waitProcess();

private:  
  void errorMessage(std::string mess);

private:
  
  Application& m_app;  
  
  base::Worker m_worker;
  misc::Queue<mess::TaskStatus> m_messForSchedr;
  misc::Queue<base::Task> m_newTasks;
  misc::Queue<std::string> m_errMess;
  std::list<Process> m_procs;
  std::mutex m_mtxProcess;
  
  std::atomic_bool m_restartRequest{};

  Loop* m_loop{};
};