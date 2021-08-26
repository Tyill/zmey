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

#include "zmCommon/timer_delay.h"
#include "zmBase/structurs.h"

class Application;
class Executor;

class Process{
public:
  Process(Application&, Executor&, const ZM_Base::Task&);

#ifdef _WIN32 
  typedef int64_t pid_t;
#endif 

  ZM_Base::Task getTask() const;
  pid_t getPid() const;
  int getProgress();
  bool checkMaxRunTime();
  void setTaskState(ZM_Base::StateType);
  void pause();
  void contin();
  void stop();    

private:
  Application& m_app;  
  Executor& m_executor;
  pid_t m_pid = 1; 
  ZM_Base::Task m_task;
  ZM_Aux::TimerDelay m_timerProgress,
                     m_timerDuration;
  uint64_t m_cdeltaTimeProgress = 0,
           m_cdeltaTimeDuration  = 0;
  bool m_isPause = false;
};