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

using namespace std;

static misc::CounterTick ctickNewTask;

void Executor::getNewTaskFromDB(db::DbProvider& db)
{  
  auto schedr = getScheduler();
  int actSz = 0,
      capSz = schedr.sCapacityTaskCount,
      newSz = 0;
  for (auto& w : getWorkers()){
    actSz += w.wActiveTaskCount;
  }
  actSz += m_tasks.size();
  
  if ((capSz - actSz) > 0){ 
    vector<base::Task> newTasks;
    if (db.getNewTasksForSchedr(schedr.sId, capSz - actSz, newTasks)){
      newSz = (int)newTasks.size();
      for(auto& t : newTasks){
        m_tasks.push(move(t));
      }      
      ctickNewTask.reset();
    }
    else if (ctickNewTask(1000)){ // every 1000 cycle
      m_app.statusMess("getNewTaskFromDB db error: " + db.getLastError());
    }
  }
  schedr.sActiveTaskCount = actSz + newSz; 
  updateScheduler(schedr);
};