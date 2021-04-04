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
#include "zmDbProvider/db_provider.h"
#include "zmCommon/queue.h"
#include "zmCommon/aux_func.h"
#include "structurs.h"

using namespace std;

static ZM_Aux::CounterTick m_ctickNT;
extern ZM_Base::Scheduler g_schedr;
extern ZM_Aux::Queue<ZM_Base::Task> g_tasks;
extern map<std::string, SWorker> g_workers;

void getNewTaskFromDB(ZM_DB::DbProvider& db){
  
  int actSz = 0,
      capSz = g_schedr.capacityTask,
      newSz = 0;
  for (auto& w : g_workers){
    actSz += w.second.base.activeTask;
  }
  actSz += g_tasks.size();
  
  if ((capSz - actSz) > 0){ 
    vector<ZM_Base::Task> newTasks;
    if (db.getNewTasksForSchedr(g_schedr.id, capSz - actSz, newTasks)){
      newSz = (int)newTasks.size();
      for(auto& t : newTasks){
        g_tasks.push(move(t));
      }      
      m_ctickNT.reset();
    }
    else if (m_ctickNT(1000)){ // every 1000 cycle
      statusMess("getNewTaskFromDB db error: " + db.getLastError());
    }
  }
  g_schedr.activeTask = actSz + newSz;  

  if (newSz > 0){
    mainCycleNotify();
  }
};