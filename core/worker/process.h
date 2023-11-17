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

#include "common/timer_delay.h"
#include "base/base.h"

#ifdef _WIN32 
typedef void* HANDLE; 
typedef int pid_t;
#endif 

class Application;
class Executor;

class Process{
public:
  Process(Application&, Executor&, const base::Task&);

  base::Task getTask() const;
  pid_t getPid() const;
  void setTaskState(base::StateType);
  void pause();
  void continueTask();
  void stop();
  std::string getErrorStr() const {
      return m_err;
  }

#ifdef _WIN32 
  HANDLE getHandle() const;
  void closeHandle();
#endif

private:
  Application& m_app;  
  Executor& m_executor;
  pid_t m_pid = 1; 
  base::Task m_task;
  bool m_isPause = false;
  std::string m_err;

#ifdef _WIN32 
  HANDLE m_hProcess = nullptr;
  HANDLE m_hThread = nullptr;
  HANDLE m_hResFile = nullptr;
#endif
};