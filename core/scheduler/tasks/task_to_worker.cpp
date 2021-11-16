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
#include "common/serial.h"
#include "common/tcp.h"
#include "base/link.h"

using namespace std;

#define ERROR_MESS(mess, wId)                               \
  m_messToDB.push(DB::MessSchedr::errorMess(wId, mess)); \
  m_app.statusMess(mess);

bool Executor::sendTaskToWorker()
{    
  if (m_workersCpy.empty()){
    for (auto& w : m_workers){
      m_workersCpy.push_back(w.second.base);
    }
    for (auto& w : m_workersCpy){
      m_refWorkers.push_back(&w);
    }
  } 
  auto iw = m_workers.cbegin();
  auto iwcp = m_workersCpy.begin();
  for (; iw != m_workers.cend(); ++iw, ++iwcp){
    iwcp->activeTask = iw->second.base.activeTask;
    iwcp->rating = iw->second.base.rating;
    iwcp->load = iw->second.base.load;
    iwcp->state = iw->second.base.state;
  }
  int cycleCount = 0;
  while (!m_tasks.empty()){
    Base::Task task;
    m_tasks.front(task);
    sort(m_refWorkers.begin(), m_refWorkers.end(), [](const Base::Worker* l, const Base::Worker* r){
      return float(l->activeTask + l->load / 10.f) / l->rating < float(r->activeTask + r->load / 10.f) / r->rating;
    });
    auto iWr = find_if(m_refWorkers.begin(), m_refWorkers.end(),
      [this, &task](const Base::Worker* w){                
        bool isSpare = false;
        if (((task.wId == 0) || (task.wId == w->id)) && (w->state == Base::StateType::RUNNING) && 
            (w->activeTask < w->capacityTask) && (w->rating > 1)){ 
          for(auto& wt : m_workers[w->connectPnt].taskList){
            if (wt == 0){
              isSpare = true;
              break;
            }
          } 
        }
        return isSpare;
      }); 
    
    if(iWr != m_refWorkers.end()){
      m_tasks.tryPop(task);
      map<string, string> data{
        {Link::command,         to_string((int)Base::MessType::NEW_TASK)},
        {Link::connectPnt,      m_schedr.connectPnt},
        {Link::taskId,          to_string(task.id)},
        {Link::params,          task.params}, 
        {Link::scriptPath,      task.scriptPath},
        {Link::resultPath,      task.resultPath},
        {Link::averDurationSec, to_string(task.averDurationSec)}, 
        {Link::maxDurationSec,  to_string(task.maxDurationSec)}        
      };
      const string& wConnPnt = (*iWr)->connectPnt;
      if (Tcp::asyncSendData(wConnPnt, Aux::serialn(data))){
        ++(*iWr)->activeTask;
        m_workers[wConnPnt].base.activeTask = (*iWr)->activeTask; 
       
        for(auto& wt : m_workers[wConnPnt].taskList){
          if (wt == 0){
            wt = task.id;
            break;
          }
        } 
      }else{
        (*iWr)->rating = std::max(1, (*iWr)->rating - 1);
      }      
    }
    else{
      if (task.wId != 0){ 
        m_tasks.tryPop(task);     // transfer task to end 
        m_tasks.push(move(task)); 
      }
      else{
        return false;
      }
      ++cycleCount;
      if (cycleCount >= m_tasks.size()) 
        return false;
    }
  }
  return true;
}