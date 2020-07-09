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
#define DBSCHEDRTEST
#ifdef DBSCHEDRTEST

#include <vector>
#include <algorithm>
#include <memory>
#include "prepare.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/auxFunc.h"

using namespace std;

extern bool isTables;
class DBSchedrTest : public ::testing::Test {
public:
  DBSchedrTest() { 
      
    ZM_DB::connectCng cng;
    cng.selType = ZM_DB::dbType::PostgreSQL;
    cng.connectStr = "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10";
    _pDb = ZM_DB::makeDbProvider(cng);
    if (_pDb){
   
      if (!isTables){
        isTables = true;
        EXPECT_TRUE(_pDb->createTables());
      }
   
      EXPECT_TRUE(_pDb->delAllSchedrs())   << _pDb->getLastError();
      EXPECT_TRUE(_pDb->delAllTask())      << _pDb->getLastError();
      EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
      EXPECT_TRUE(_pDb->delAllUsers())     << _pDb->getLastError();
      EXPECT_TRUE(_pDb->delAllWorkers())   << _pDb->getLastError();
    }
  }
  ~DBSchedrTest() {
    delete _pDb;
  }
protected:
  ZM_DB::DbProvider* _pDb = nullptr; 
};

TEST_F(DBSchedrTest, getSchedrByCP){
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::stateType::error;
  schedr.capacityTask = 1;   
  schedr.id = 0;
  EXPECT_TRUE(_pDb->getSchedr(schedr.connectPnt, schedr) && (schedr.id == sId) &&
                                              (schedr.state == ZM_Base::stateType::ready) &&
                                              (schedr.connectPnt == "localhost:4444") &&
                                              (schedr.capacityTask == 105)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::stateType::error;
  schedr.connectPnt = ""; 
  schedr.capacityTask = 1;
  EXPECT_TRUE(!_pDb->getSchedr(schedr.connectPnt, schedr) && (schedr.state == ZM_Base::stateType::error) &&
                                              (schedr.connectPnt == "") &&
                                              (schedr.capacityTask == 1)) << _pDb->getLastError();                                                      
}
TEST_F(DBSchedrTest, getTaskOfSchedr){
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4445";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();  

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  ppline.isShared = 0;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 
    
  ZM_Base::task base;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  templ.isShared = 0;
  uint64_t ttId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, ttId) && (ttId > 0)) << _pDb->getLastError(); 

  ZM_Base::uTask task;
  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "['param11','param12','param13']";
  task.screenRect = "11, 12, 13, 14";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId1)) << _pDb->getLastError();        

  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "['param21','param22','param23']";
  task.screenRect = "21, 22, 23, 24";
  task.nextTasks = "[]";
  task.prevTasks = "[" + to_string(tId1) + "]";
  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId2) && (tId2 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId2)) << _pDb->getLastError();        

  vector<ZM_DB::schedrTask> tasks;
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) &&
              (tasks[0].params == "param11,param12,param13") &&
              (tasks[0].base.id == ttId)) << _pDb->getLastError();

  tasks.clear();
  EXPECT_TRUE(_pDb->getTasksOfSchedr(sId, tasks) && 
             (tasks.size() == 1) &&
             (tasks[0].params == "param11,param12,param13") &&
             (tasks[0].base.id == ttId)) << _pDb->getLastError(); 

  vector<ZM_DB::messSchedr> mess;
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskStart, wId, tasks[0].qTaskId, 0, 10, "result"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  mess.clear();
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskCompleted, wId, tasks[0].qTaskId, 0, 0, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  tasks.clear();
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) && 
              (tasks[0].params == "param21,param22,param23,result1") &&
              (tasks[0].base.id == ttId)) << _pDb->getLastError(); 

  tasks.clear();
  EXPECT_TRUE(_pDb->getTasksOfSchedr(sId, tasks) && 
             (tasks.size() == 1) &&
             (tasks[0].params == "param21,param22,param23,result1") &&
             (tasks[0].base.id == ttId)) << _pDb->getLastError();          
}
TEST_F(DBSchedrTest, getWorkerOfSchedr){
  ZM_Base::scheduler schedr{0};
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444";
  schedr.capacityTask = 10000;
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 
  
  ZM_Base::worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();   
 
  vector<ZM_Base::worker> workers;
  EXPECT_TRUE(_pDb->getWorkersOfSchedr(sId, workers) && 
              (workers.size() == 1) &&
              (workers[0].id == wId)) << _pDb->getLastError(); 
}
TEST_F(DBSchedrTest, getNewTasksForSchedr){
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4445";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();  

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  ppline.isShared = 0;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 
    
  ZM_Base::task base;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  templ.isShared = 0;
  uint64_t ttId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, ttId) && (ttId > 0)) << _pDb->getLastError(); 

  ZM_Base::uTask task;
  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "['param11','param12','param13']";
  task.screenRect = "11, 12, 13, 14";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "result1";
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId1)) << _pDb->getLastError();        

  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "['param21','param22','param23']";
  task.screenRect = "21, 22, 23, 24";
  task.nextTasks = "[]";
  task.prevTasks = "[" + to_string(tId1) + "]";
  task.base.result = "result2";
  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId2) && (tId2 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId2)) << _pDb->getLastError();        

  vector<ZM_DB::schedrTask> tasks;
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) &&
              (tasks[0].params == "param11,param12,param13") &&
              (tasks[0].base.id == ttId)) << _pDb->getLastError();

  vector<ZM_DB::messSchedr> mess;
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskStart, wId, tasks[0].qTaskId, 0, 0, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  mess.clear();
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskCompleted, wId, tasks[0].qTaskId, 0, 0, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  tasks.clear();
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) && 
              (tasks[0].params == "param21,param22,param23,result1") &&
              (tasks[0].base.id == ttId)) << _pDb->getLastError();    
}
TEST_F(DBSchedrTest, getWorkerByTask){
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4445";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();  

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  ppline.isShared = 0;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 
    
  ZM_Base::task base;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  templ.isShared = 0;
  uint64_t ttId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, ttId) && (ttId > 0)) << _pDb->getLastError(); 

  ZM_Base::uTask task;
  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "['param11','param12','param13']";
  task.screenRect = "11, 12, 13, 14";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "result1";
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId1)) << _pDb->getLastError();        

  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "['param21','param22','param23']";
  task.screenRect = "21, 22, 23, 24";
  task.nextTasks = "[]";
  task.prevTasks = "[" + to_string(tId1) + "]";
  task.base.result = "result2";
  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId2) && (tId2 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId2)) << _pDb->getLastError();        

  vector<ZM_DB::schedrTask> tasks;
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) &&
              (tasks[0].params == "param11,param12,param13") &&
              (tasks[0].base.id == ttId)) << _pDb->getLastError();

  vector<ZM_DB::messSchedr> mess;
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskStart, wId, tasks[0].qTaskId, 0, 0, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();
  
  tasks.clear();
  uint64_t qId = 0;
  worker.id = 0;
  EXPECT_TRUE(_pDb->getWorkerByTask(tId1, qId, worker) && 
              (worker.id == wId)) << _pDb->getLastError();      
}
TEST_F(DBSchedrTest, sendAllMessFromSchedr){
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 
  
  ZM_Base::worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4445";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();   
  
  vector<ZM_DB::messSchedr> mess;
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskError, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskCompleted, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskStart, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskRunning, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskPause, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskStop, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::justStartWorker, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::progress, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::pauseSchedr, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::pauseWorker, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::startSchedr, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::startWorker, wId, 0, 0, 10, "result"});
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::workerNotResponding, wId, 0, 0, 10, "result"});

  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();
}

#endif // DBSCHEDRTEST