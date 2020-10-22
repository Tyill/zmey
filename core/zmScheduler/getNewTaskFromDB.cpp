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
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "structurs.h"

using namespace std;

ZM_Aux::CounterTick ctickNT;
extern ZM_Base::Scheduler _schedr;
extern ZM_Aux::QueueThrSave<STask> _tasks;
extern map<std::string, SWorker> _workers;

void getNewTaskFromDB(ZM_DB::DbProvider& db){
  
  int actSz = 0,
      capSz = _schedr.capacityTask;
  for (auto& w : _workers){
    actSz += w.second.base.activeTask;
  }
  actSz += _tasks.size();
  vector<ZM_DB::SchedrTask> newTasks;
  if ((capSz - actSz) > 0){ 
    if (db.getNewTasksForSchedr(_schedr.id, capSz - actSz, newTasks)){
      for(auto& t : newTasks){
        _tasks.push(STask{t.qTaskId, t.base, t.params});
      }      
      ctickNT.reset();
    }
    else if (ctickNT(1000)){ // every 1000 cycle
      statusMess("getNewTaskFromDB db error: " + db.getLastError());
    }
  }
  _schedr.activeTask = actSz + newTasks.size();  

  if (!newTasks.empty()){
    mainCycleNotify();
  }
};