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

#include "scheduler/executor.h"
#include "base/messages.h"
#include "common/tcp.h"

using namespace std;


bool Executor::sendTaskToWorker()
{   
  const auto schedr = getScheduler();
  auto workers = getWorkers();
  int cycleCount = 0;
  while (!m_tasks.empty()){
    base::Task task;
    m_tasks.front(task);
    sort(workers.begin(), workers.end(), [](const base::Worker& l, const base::Worker& r){
      return l.wActiveTaskCount + l.wLoadCPU / 10.f < r.wActiveTaskCount + r.wLoadCPU / 10.f;
    });
    auto iWr = find_if(workers.begin(), workers.end(), [this, task](const base::Worker& w){                
      return ((task.tWId == 0 || task.tWId == w.wId) && 
              (w.wState == base::StateType::RUNNING) && 
              (w.wActiveTaskCount < w.wCapacityTaskCount));         
    });
    if(iWr != workers.end()){
      mess::NewTask messNewTask(schedr.sConnectPnt);{
        messNewTask.taskId = task.tId;
        messNewTask.params = task.tParams;
        messNewTask.scriptPath = task.tScriptPath;
        messNewTask.resultPath = task.tResultPath;
        messNewTask.averDurationSec = task.tAverDurationSec;
        messNewTask.maxDurationSec = task.tMaxDurationSec;        
      } 
      const string& wConnPnt = (*iWr).wConnectPnt;
      if (misc::asyncSendData(wConnPnt, messNewTask.serialn())){
        m_tasks.tryPop(task);
        addTaskForWorker((*iWr).wId, task);
        ++(*iWr).wActiveTaskCount; 
        updateWorker(*iWr);
      }     
    }else{
      ++cycleCount;
      if (task.tWId == 0 || cycleCount >= m_tasks.size()){
        return false;
      }
    }
  }
  return true;
}