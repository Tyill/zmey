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

#define ERROR_MESS(mess, wId)                               \
  m_messToDB.push(DB::MessSchedr::errorMess(wId, mess)); \
  m_app.statusMess(mess);

bool Executor::sendTaskToWorker()
{    
  if (m_workersList.empty()){
    for (auto& w : m_workers){
      m_workersList.push_back(w.second.base);
    }
  }
  
  std::vector<base::Worker*> refWorkers;
  auto iw = m_workers.cbegin();
  auto iwcp = m_workersList.begin();
  for (; iw != m_workers.cend(); ++iw, ++iwcp){
    iwcp->activeTask = iw->second.base.activeTask;
    iwcp->load = iw->second.base.load;
    iwcp->state = iw->second.base.state;
    refWorkers.push_back(iwcp.base());  
  }
  int cycleCount = 0;
  while (!m_tasks.empty()){
    base::Task task;
    m_tasks.front(task);
    sort(refWorkers.begin(), refWorkers.end(), [](const base::Worker* l, const base::Worker* r){
      return float(l->activeTask + l->load / 10.f) < float(r->activeTask + r->load / 10.f);
    });
    auto iWr = find_if(refWorkers.begin(), refWorkers.end(),
      [this, &task](const base::Worker* w){                
        bool isSpare = false;
        if (((task.wId == 0) || (task.wId == w->id)) && (w->state == base::StateType::RUNNING) && 
            (w->activeTask < w->capacityTask)){ 
          for(auto& wt : m_workers[w->connectPnt].taskList){
            if (wt == 0){
              isSpare = true;
              break;
            }
          } 
        }
        return isSpare;
      }); 
    
    if(iWr != refWorkers.end()){
      mess::NewTask messNewTask(m_schedr.connectPnt);{
        messNewTask.taskId = task.id;
        messNewTask.params = task.params;
        messNewTask.scriptPath = task.scriptPath;
        messNewTask.resultPath = task.resultPath;        
      } 
      const string& wConnPnt = (*iWr)->connectPnt;
      if (misc::asyncSendData(wConnPnt, messNewTask.serialn())){
        m_tasks.tryPop(task);
        ++(*iWr)->activeTask;
        m_workers[wConnPnt].base.activeTask = (*iWr)->activeTask; 

        for(auto& wt : m_workers[wConnPnt].taskList){
          if (wt == 0){
            wt = task.id;
            break;
          }
        } 
      }     
    }
    else{
      ++cycleCount;
      if (task.wId == 0 || cycleCount >= m_tasks.size()){
        return false;
      }
    }
  }
  return true;
}