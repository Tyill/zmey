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
#include <gtest/gtest.h>

#include "db_provider/db_provider.h"
#include "common/misc.h"

using namespace std;

extern bool isTables;
class DBSchedrTest : public ::testing::Test {
public:
  DBSchedrTest() { 
      
    DB::ConnectCng cng;
    cng.connectStr = "host=localhost port=5432 password=123 dbname=dagdb connect_timeout=10";
    _pDb = new DB::DbProvider(cng);
    if (_pDb){
      EXPECT_TRUE(_pDb->delAllTables())   << _pDb->getLastError();
      EXPECT_TRUE(_pDb->createTables());
    }
  }
  ~DBSchedrTest() {
    delete _pDb;
  }
protected:
  DB::DbProvider* _pDb = nullptr; 
};

TEST_F(DBSchedrTest, getSchedrByCP){
  base::Scheduler schedr{0};
  schedr.state = base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  int sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  schedr.state = base::StateType::ERRORT;
  schedr.capacityTask = 1;   
  schedr.id = 0;
  EXPECT_TRUE(_pDb->getSchedr(schedr.connectPnt, schedr) && (schedr.id == sId) &&
                                              (schedr.state == base::StateType::READY) &&
                                              (schedr.connectPnt == "localhost:4444") &&
                                              (schedr.capacityTask == 105)) << _pDb->getLastError(); 

  schedr.state = base::StateType::ERRORT;
  schedr.connectPnt = ""; 
  schedr.capacityTask = 1;
  EXPECT_TRUE(!_pDb->getSchedr(schedr.connectPnt, schedr) && (schedr.state == base::StateType::ERRORT) &&
                                              (schedr.connectPnt == "") &&
                                              (schedr.capacityTask == 1)) << _pDb->getLastError();                                                      
}
TEST_F(DBSchedrTest, getTaskOfSchedr){
  base::Scheduler schedr{0};
  schedr.state = base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  int sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = base::StateType::READY;
  worker.connectPnt = "localhost:4445";
  int wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();  
    
  int tId1 = 0;  
  base::Task task{0};
  EXPECT_TRUE(_pDb->startTask(0, task, tId1)) << _pDb->getLastError();        

  int tId2 = 0;  
  EXPECT_TRUE(_pDb->startTask(0, task, tId2 )) << _pDb->getLastError();        

  vector<base::Task> tasks;
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 2) &&
              (tasks[0].id == tId1) && 
              (tasks[0].wId == 0)) << _pDb->getLastError();

  vector<DB::MessSchedr> mess;
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_RUNNING, wId, tasks[0].id});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  mess.clear();
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_COMPLETED, wId, tasks[0].id});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

          
}
TEST_F(DBSchedrTest, getWorkerOfSchedr){
  base::Scheduler schedr{0};
  schedr.state = base::StateType::READY;
  schedr.connectPnt = "localhost:4444";
  schedr.capacityTask = 10000;
  int sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 
  
  base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = base::StateType::READY;
  worker.connectPnt = "localhost:4444";
  int wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();   
 
  vector<base::Worker> workers;
  EXPECT_TRUE(_pDb->getWorkersOfSchedr(sId, workers) && 
              (workers.size() == 1) &&
              (workers[0].id == wId)) << _pDb->getLastError(); 
}
TEST_F(DBSchedrTest, getNewTasksForSchedr){
  base::Scheduler schedr{0};
  schedr.state = base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  int sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = base::StateType::READY;
  worker.connectPnt = "localhost:4445";
  int wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();  
      
  int tId1 = 0;  
  base::Task task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  EXPECT_TRUE(_pDb->startTask(0, task, tId1)) << _pDb->getLastError();        

  int tId2 = 0;  
  EXPECT_TRUE(_pDb->startTask(0, task, tId2)) << _pDb->getLastError();        

  vector<base::Task> tasks;
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 2) &&
              (tasks[0].id == tId1)) << _pDb->getLastError();

  vector<DB::MessSchedr> mess;
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_RUNNING, wId, tasks[0].id});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  mess.clear();
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_COMPLETED, wId, tasks[0].id});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();
  
}
TEST_F(DBSchedrTest, getWorkerByTask){
  base::Scheduler schedr{0};
  schedr.state = base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  int sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = base::StateType::READY;
  worker.connectPnt = "localhost:4445";
  int wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();  
     
  int tId1 = 0;  
  base::Task task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  EXPECT_TRUE(_pDb->startTask(0, task, tId1)) << _pDb->getLastError();        

  int tId2 = 0;  
  EXPECT_TRUE(_pDb->startTask(0, task, tId2)) << _pDb->getLastError();        

  vector<base::Task> tasks;
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 2) &&
              (tasks[0].id == tId1)) << _pDb->getLastError();

  vector<DB::MessSchedr> mess;
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_RUNNING, wId, tasks[0].id});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();
  
  tasks.clear();
  worker.id = 0;
  EXPECT_TRUE(_pDb->getWorkerByTask(tId1, worker) && 
              (worker.id == wId)) << _pDb->getLastError();      
}
TEST_F(DBSchedrTest, sendAllMessFromSchedr){
  base::Scheduler schedr{0};
  schedr.state = base::StateType::READY;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  int sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 
  
  base::Worker worker{0};
  worker.capacityTask = 10;
  worker.sId = sId;
  worker.state = base::StateType::READY;
  worker.connectPnt = "localhost:4445";
  int wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();   
  
  vector<DB::MessSchedr> mess;
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_ERROR, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_COMPLETED, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_RUNNING, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_PAUSE, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_STOP, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::JUST_START_WORKER, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::TASK_PROGRESS, wId, 0, "0"});
  mess.push_back(DB::MessSchedr{mess::MessType::PAUSE_SCHEDR, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::PAUSE_WORKER, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::START_SCHEDR, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::START_WORKER, wId, 0, "result"});
  mess.push_back(DB::MessSchedr{mess::MessType::WORKER_NOT_RESPONDING, wId, 0, "result"});

  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();
}

#endif // DBSCHEDRTEST