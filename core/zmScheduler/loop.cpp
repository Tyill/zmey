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
#include "zmCommon/timer_delay.h"
#include "zmCommon/tcp.h"

#include <future>
#include <condition_variable>
#include <mutex>

using namespace std;
 
Loop::Loop(const Application::Config& cng, Executor& exr, ZM_DB::DbProvider& dbNewTask, ZM_DB::DbProvider& dbSendMess):
  m_cng(cng),
  m_executor(exr),
  m_dbNewTask(dbNewTask),
  m_dbSendMess(dbSendMess)
{
}

void Loop::run()
{
  future<void> frGetNewTask,
               frSendAllMessToDB; 
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 10;
    
  while (!m_fClose){
    timer.updateCycTime();   

    if(m_executor.appendNewTaskAvailable()){
      if(!frGetNewTask.valid() || (frGetNewTask.wait_for(chrono::seconds(0)) == future_status::ready))
        frGetNewTask = async(launch::async, [this]{ m_executor.getNewTaskFromDB(m_dbNewTask); });                                        
    }        

    bool isAvailableWorkers = m_executor.sendTaskToWorker();    

    if(!m_executor.isMessToDBEmpty()){   
      if(!frSendAllMessToDB.valid() || (frSendAllMessToDB.wait_for(chrono::seconds(0)) == future_status::ready))
        frSendAllMessToDB = async(launch::async, [this]{ m_executor.sendAllMessToDB(m_dbSendMess); });      
    }

    if(timer.onDelayOncSec(true, m_cng.checkWorkerTOutSec, 0)){
      m_executor.checkStatusWorkers();
    }
    
    if (m_executor.isMessToDBEmpty() && (m_executor.isTasksEmpty() || !isAvailableWorkers)){ 
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