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
#include "zmDbProvider/db_provider.h"
#include "zmCommon/aux_func.h"

using namespace std;

extern bool isTables;
class DBSchedrTest : public ::testing::Test {
public:
  DBSchedrTest() { 
      
    ZM_DB::ConnectCng cng;
    cng.connectStr = "host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10";
    _pDb = new ZM_DB::DbProvider(cng);
    if (_pDb){
      EXPECT_TRUE(_pDb->delAllTables())   << _pDb->getLastError();
      EXPECT_TRUE(_pDb->createTables());
    }
  }
  ~DBSchedrTest() {
    delete _pDb;
  }
protected:
  ZM_DB::DbProvider* _pDb = nullptr; 
};

TEST_F(DBSchedrTest, getSchedrByCP){
  ZM_Base::Scheduler schedr;
  schedr.state = ZM_Base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::StateType::ERROR;
  schedr.capacityTask = 1;   
  schedr.id = 0;
  EXPECT_TRUE(_pDb->getSchedr(schedr.connectPnt, schedr) && (schedr.id == sId) &&
                                              (schedr.state == ZM_Base::StateType::READY) &&
                                              (schedr.connectPnt == "localhost:4444") &&
                                              (schedr.capacityTask == 105)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::StateType::ERROR;
  schedr.connectPnt = ""; 
  schedr.capacityTask = 1;
  EXPECT_TRUE(!_pDb->getSchedr(schedr.connectPnt, schedr) && (schedr.state == ZM_Base::StateType::ERROR) &&
                                              (schedr.connectPnt == "") &&
                                              (schedr.capacityTask == 1)) << _pDb->getLastError();                                                      
}
TEST_F(DBSchedrTest, getTaskOfSchedr){
  ZM_Base::User usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::Scheduler schedr;
  schedr.state = ZM_Base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::StateType::READY;
  worker.connectPnt = "localhost:4445";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();  

  ZM_Base::UPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 
 
  ZM_Base::UTaskTemplate templ;
  templ.uId = uId; 
  templ.sId = 0; 
  templ.description = "descr";
  templ.name = "NEW_TASK";
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = "100500";
  uint64_t ttId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, ttId) && (ttId > 0)) << _pDb->getLastError(); 

  ZM_Base::UTaskPipeline task{0};
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  uint64_t ptId1 = 0;  
  EXPECT_TRUE(_pDb->addTaskPipeline(task, ptId1) && (ptId1 > 0)) << _pDb->getLastError();  

  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->startTask(ptId1, 1, "param11,param12,param13", "", tId1)) << _pDb->getLastError();        

  task.pplId = pId; 
  task.priority = 1;
  task.ttId = ttId;
  uint64_t ptId2 = 0;  
  EXPECT_TRUE(_pDb->addTaskPipeline(task, ptId2) && (ptId2 > 0)) << _pDb->getLastError();  

  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->startTask(ptId2, 1, "param21,param22,param23", to_string(tId1), tId2 )) << _pDb->getLastError();        

  vector<ZM_Base::Task> tasks;
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) &&
              (tasks[0].params == "\"param11,param12,param13\"") &&
              (tasks[0].id == tId1)) << _pDb->getLastError();

  tasks.clear();
  EXPECT_TRUE(_pDb->getTasksOfSchedr(sId, tasks) && 
             (tasks.size() == 1) &&
             (tasks[0].params == "\"param11,param12,param13\"") &&
             (tasks[0].id == tId1)) << _pDb->getLastError(); 

  vector<ZM_DB::MessSchedr> mess;
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_START, wId, tasks[0].id, "result"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  mess.clear();
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_COMPLETED, wId, tasks[0].id, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  tasks.clear();
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) && 
              (tasks[0].params == "\"param21,param22,param23\",result1") &&
              (tasks[0].id == tId2)) << _pDb->getLastError(); 

  tasks.clear();
  EXPECT_TRUE(_pDb->getTasksOfSchedr(sId, tasks) && 
             (tasks.size() == 1) &&
             (tasks[0].params == "\"param21,param22,param23\",result1") &&
             (tasks[0].id == tId2)) << _pDb->getLastError();          
}
TEST_F(DBSchedrTest, getWorkerOfSchedr){
  ZM_Base::Scheduler schedr{0};
  schedr.state = ZM_Base::StateType::READY;
  schedr.connectPnt = "localhost:4444";
  schedr.capacityTask = 10000;
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 
  
  ZM_Base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::StateType::READY;
  worker.connectPnt = "localhost:4444";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();   
 
  vector<ZM_Base::Worker> workers;
  EXPECT_TRUE(_pDb->getWorkersOfSchedr(sId, workers) && 
              (workers.size() == 1) &&
              (workers[0].id == wId)) << _pDb->getLastError(); 
}
TEST_F(DBSchedrTest, getNewTasksForSchedr){
  ZM_Base::User usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::Scheduler schedr;
  schedr.state = ZM_Base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::StateType::READY;
  worker.connectPnt = "localhost:4445";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();  

  ZM_Base::UPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 
      
  ZM_Base::UTaskTemplate templ;
  templ.uId = uId; 
  templ.sId = 0; 
  templ.description = "descr";
  templ.name = "NEW_TASK";
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = "100500";
  uint64_t ttId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, ttId) && (ttId > 0)) << _pDb->getLastError(); 

  ZM_Base::UTaskPipeline task{0};
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  uint64_t ptId1 = 0;  
  EXPECT_TRUE(_pDb->addTaskPipeline(task, ptId1) && (ptId1 > 0)) << _pDb->getLastError();  

  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->startTask(ptId1, 1, "param11,param12,param13", "", tId1)) << _pDb->getLastError();        

  task.pplId = pId; 
  task.priority = 1;
  task.ttId = ttId;
  uint64_t ptId2 = 0;  
  EXPECT_TRUE(_pDb->addTaskPipeline(task, ptId2) && (ptId2 > 0)) << _pDb->getLastError();  

  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->startTask(ptId2, 1, "param21,param22,param23", to_string(tId1), tId2)) << _pDb->getLastError();        

  vector<ZM_Base::Task> tasks;
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) &&
              (tasks[0].params == "\"param11,param12,param13\"") &&
              (tasks[0].id == tId1)) << _pDb->getLastError();

  vector<ZM_DB::MessSchedr> mess;
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_START, wId, tasks[0].id, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  mess.clear();
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_COMPLETED, wId, tasks[0].id, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  tasks.clear();
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) && 
              (tasks[0].params == "\"param21,param22,param23\",result1") &&
              (tasks[0].id == tId2)) << _pDb->getLastError();    
}
TEST_F(DBSchedrTest, getWorkerByTask){
  ZM_Base::User usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::Scheduler schedr;
  schedr.state = ZM_Base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::StateType::READY;
  worker.connectPnt = "localhost:4445";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();  

  ZM_Base::UPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 
  
  ZM_Base::UTaskTemplate templ;
  templ.uId = uId; 
  templ.sId = 0; 
  templ.description = "descr";
  templ.name = "NEW_TASK";
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = "100500";
  uint64_t ttId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, ttId) && (ttId > 0)) << _pDb->getLastError(); 

  ZM_Base::UTaskPipeline task{0};
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  uint64_t ptId1 = 0;  
  EXPECT_TRUE(_pDb->addTaskPipeline(task, ptId1) && (ptId1 > 0)) << _pDb->getLastError();  

  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->startTask(ptId1, 1, "param11,param12,param13", "", tId1)) << _pDb->getLastError();        

  task.pplId = pId; 
  task.priority = 1;
  task.ttId = ttId;
  uint64_t ptId2 = 0;  
  EXPECT_TRUE(_pDb->addTaskPipeline(task, ptId2) && (ptId2 > 0)) << _pDb->getLastError();  

  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->startTask(ptId2, 1, "param21,param22,param23", "", tId2)) << _pDb->getLastError();        

  vector<ZM_Base::Task> tasks;
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 2) &&
              (tasks[0].params == "\"param11,param12,param13\"") &&
              (tasks[0].id == tId1)) << _pDb->getLastError();

  vector<ZM_DB::MessSchedr> mess;
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_START, wId, tasks[0].id, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();
  
  tasks.clear();
  worker.id = 0;
  EXPECT_TRUE(_pDb->getWorkerByTask(tId1, worker) && 
              (worker.id == wId)) << _pDb->getLastError();      
}
TEST_F(DBSchedrTest, sendAllMessFromSchedr){
  ZM_Base::Scheduler schedr;
  schedr.state = ZM_Base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 
  
  ZM_Base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = ZM_Base::StateType::READY;
  worker.connectPnt = "localhost:4445";
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();   
  
  vector<ZM_DB::MessSchedr> mess;
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_ERROR, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_COMPLETED, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_START, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_RUNNING, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_PAUSE, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_STOP, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::JUST_START_WORKER, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::TASK_PROGRESS, wId, 0, "0"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::PAUSE_SCHEDR, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::PAUSE_WORKER, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::START_SCHEDR, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::START_WORKER, wId, 0, "result"});
  mess.push_back(ZM_DB::MessSchedr{ZM_Base::MessType::WORKER_NOT_RESPONDING, wId, 0, "result"});

  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();
}

#endif // DBSCHEDRTEST