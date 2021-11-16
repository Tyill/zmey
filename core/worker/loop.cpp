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

#include "loop.h" 
#include "executor.h"
#include "common/timer_delay.h"
#include "common/tcp.h"

#include <mutex>

using namespace std;
 
Loop::Loop(const Application::Config& cng, Executor& exr):
  m_cng(cng),
  m_executor(exr)
{
}

void Loop::run()
{      
  Aux::TimerDelay timer;
  const int minCycleTimeMS = 10;
   
  Aux::CPUData cpu;

  while (!m_fClose){
    timer.updateCycTime();   

    m_executor.waitProcess();
    
    m_executor.updateListTasks();
    
    if (!m_executor.isMessForSchedrEmpty()){ 
      m_executor.messageToSchedr(m_cng.schedrConnPnt);
    }

    if(timer.onDelayOncSec(true, m_cng.progressTasksTOutSec, 0)){
      m_executor.progressToSchedr(m_cng.schedrConnPnt);
    }
    
    if(timer.onDelayOncSec(true, m_cng.checkLoadTOutSec, 1)){
      m_executor.setLoadCPU(cpu.load());
    } 
    
    if(timer.onDelayOncSec(true, m_cng.pingSchedrTOutSec, 2)){
      m_executor.pingToSchedr(m_cng.schedrConnPnt);
    } 
         
    if (!m_executor.isErrMessEmpty()){ 
      m_executor.errorToSchedr(m_cng.schedrConnPnt);
    }    
    
    if (m_executor.isMessForSchedrEmpty() && m_executor.isNewTasksEmpty() && !m_fClose){
      mainCycleSleep(minCycleTimeMS);     
    }
  }
  
}

void Loop::stop()
{
  m_fClose = true;
  m_cvStandUp.notify_one(); 
}

void Loop::standUpNotify()
{
  m_cvStandUp.notify_one();  
}

void Loop::mainCycleSleep(int delayMS)
{
  unique_lock<mutex> lck(m_mtxNotify);
  m_cvStandUp.wait_for(lck, chrono::milliseconds(delayMS)); 
}