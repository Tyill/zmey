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
#include <cassert>

using namespace std;

void Executor::getPrevTaskFromDB(db::DbProvider& db)
{
  vector<base::Task> tasks;
  if (db.getTasksOfSchedr(m_schedr.sId, tasks)){
    for(auto& t : tasks){
      m_tasks.push(move(t));
    }
  }else{
    m_app.statusMess("getPrevTaskFromDB db error: " + db.getLastError());
  }
};

void Executor::addTaskForWorker(int wId, const base::Task& t)
{
  lock_guard<mutex> lk(*m_workerLocks[wId]);
  assert(m_workerTasks.count(wId));

  m_workerTasks[wId].push_back(t);
}
void Executor::removeTaskForWorker(int wId, const base::Task& t)
{
  lock_guard<mutex> lk(*m_workerLocks[wId]);
  assert(m_workerTasks.count(wId));

  if (auto it = std::find_if(m_workerTasks[wId].begin(), m_workerTasks[wId].end(), [tid = t.tId](const auto& t){
    return t.tId == tid;
  }); it != m_workerTasks[wId].end()){
    m_workerTasks[wId].erase(it);
  }
}
std::vector<base::Task> Executor::getWorkerTasks(int wId)
{
  lock_guard<mutex> lk(*m_workerLocks[wId]);
  assert(m_workerTasks.count(wId));

  return m_workerTasks[wId];
}
void Executor::clearWorkerTasks(int wId)
{
  lock_guard<mutex> lk(*m_workerLocks[wId]);
  assert(m_workerTasks.count(wId));

  m_workerTasks[wId].clear();
}