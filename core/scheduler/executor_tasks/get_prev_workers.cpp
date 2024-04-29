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
#include "common/tcp.h"

#include <cassert>

using namespace std;

void Executor::getPrevWorkersFromDB(db::DbProvider& db)
{   
  vector<base::Worker> workers; 
  if (db.getWorkersOfSchedr(m_schedr.sId, workers)){
    for(auto& w : workers){
      w.wIsActive = w.wState != base::StateType::NOT_RESPONDING;
      vector<base::Task> tasks;
      if (db.getTasksOfWorker(m_schedr.sId, w.wId, tasks)){
        m_workers[w.wConnectPnt] = w;
        m_workerTasks[w.wId] = tasks;
        m_workerLocks[w.wId] = new std::mutex;
      }else{
        m_app.statusMess("getTasksOfWorker db error: " + db.getLastError());
      }      
    }
  }
  else{
    m_app.statusMess("getPrevWorkersFromDB db error: " + db.getLastError());
  }  
}

std::optional<base::Worker> Executor::getWorkerByConnPnt(const std::string& cp)
{
  lock_guard<mutex> lk(m_mtxWorker);
  if (m_workers.count(cp)){
    return m_workers[cp];
  }
  return {};
}

std::vector<base::Worker> Executor::getWorkers()
{
  lock_guard<mutex> lk(m_mtxWorker);
  
  std::vector<base::Worker> out;
  for(auto& w : m_workers){
    out.push_back(w.second);
  }
  return out;
}

void Executor::updateWorkerState(const std::string& cp, base::StateType state)
{
  lock_guard<mutex> lk(m_mtxWorker);
  assert(m_workers.count(cp));

  m_workers[cp].wStateMem = m_workers[cp].wState;
  m_workers[cp].wState = state;
}

void Executor::updateWorker(const base::Worker& w)
{
  lock_guard<mutex> lk(m_mtxWorker);
  assert(m_workers.count(w.wConnectPnt));

  m_workers[w.wConnectPnt] = w;
}

void Executor::updateWorkers(const std::vector<base::Worker>& workers)
{
  lock_guard<mutex> lk(m_mtxWorker);
  for(const auto& w : workers){
    m_workers[w.wConnectPnt] = w;
  }
}

