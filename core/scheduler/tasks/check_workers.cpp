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

#include <cmath>

using namespace std;

void Executor::checkStatusWorkers(db::DbProvider& db)
{
  vector<base::Worker*> wkrNotResp;
  for(auto& w : m_workers){
    if (!w.second->wIsActive && (w.second->wState != int(base::StateType::STOP))){            
      wkrNotResp.push_back(w.second);
    }else{
      w.second->wIsActive = false;
    }
  }
  if (wkrNotResp.size() <= round(m_workers.size() * 0.75)){ 
    for(auto w : wkrNotResp){
      workerNotResponding(db, w);
    }
  }else{
    string mess = "checkStatusWorkers error all workers are not available";
    errorMessage(mess, 0);
  }
}

void Executor::workerNotResponding(db::DbProvider& db, base::Worker* w)
{
  if (w->wState != int(base::StateType::NOT_RESPONDING)){
    m_messToDB.push(db::MessSchedr(mess::MessType::WORKER_NOT_RESPONDING, w->wId));
    m_messToDB.push(db::MessSchedr::errorMess(w->wId, "worker not responding"));          
    w->wStateMem = +w->wState;
    w->wState = int(base::StateType::NOT_RESPONDING);
  }
  const auto wTasks = getWorkerTasks(w->wId);
  for(auto t : wTasks){
    m_tasks.push(move(t));
  }
  clearWorkerTasks(w->wId);
}