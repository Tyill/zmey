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
#include "zmScheduler/executor.h"
#include "zmCommon/serial.h"
#include "zmCommon/tcp.h"

using namespace std;

#define ERROR_MESS(mess, wId)                                                   \
  m_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::INTERN_ERROR, wId, mess}); \
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
  
  while (!m_tasks.empty()){
    sort(m_refWorkers.begin(), m_refWorkers.end(), [](const ZM_Base::Worker* l, const ZM_Base::Worker* r){
      return float(l->activeTask + l->load / 10.f) / l->rating < float(r->activeTask + r->load / 10.f) / r->rating;
    });
    auto iWr = find_if(m_refWorkers.begin(), m_refWorkers.end(),
      [](const ZM_Base::Worker* w){
        return (w->state == ZM_Base::StateType::RUNNING) && 
               (w->activeTask <= w->capacityTask) && 
               (w->rating > 1);
      }); 
    if(iWr != m_refWorkers.end()){
      ZM_Base::Task t;
      m_tasks.tryPop(t);
      map<string, string> data{
        {"command",         to_string((int)ZM_Base::MessType::NEW_TASK)},
        {"connectPnt",      m_schedr.connectPnt},
        {"taskId",          to_string(t.id)},
        {"params",          t.params}, 
        {"script",          t.script},
        {"averDurationSec", to_string(t.averDurationSec)}, 
        {"maxDurationSec",  to_string(t.maxDurationSec)}        
      };
      if (ZM_Tcp::asyncSendData((*iWr)->connectPnt, ZM_Aux::serialn(data))){
        ++(*iWr)->activeTask;
        m_workers[(*iWr)->connectPnt].base.activeTask = (*iWr)->activeTask; 
        m_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_START, (*iWr)->id, t.id}); 
      }else{
        (*iWr)->rating = std::max(1, (*iWr)->rating - 1);
      }      
      m_ctickTaskToWorker.reset();
    }
    else{
      if (m_ctickTaskToWorker(1000)){ // every 1000 cycle
        ERROR_MESS("schedr::sendTaskToWorker not found available worker", 0);
      }
      return false;
    }
  }
  return true;
}