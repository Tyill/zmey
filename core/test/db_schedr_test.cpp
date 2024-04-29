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
    db::ConnectCng cng;
    cng.connectStr = "host=localhost user=postgres port=5432 dbname=zmeydb connect_timeout=10";
    pDb_ = new db::DbProvider(cng);

    pDb_->setErrorCBack([](const char* mess, void*){
      cout << mess << endl;
    }, nullptr);  
  }
  ~DBSchedrTest() {
    delete pDb_;
  }
protected:
  db::DbProvider* pDb_ = nullptr; 
};

base::Scheduler newSchedr(){
  base::Scheduler schedr;
  schedr.sState = base::StateType::READY;
  schedr.sConnectPnt = "localhost:4444"; 
  schedr.sCapacityTaskCount = 105;
  return schedr;
}

base::Worker newWorker(int sId){
  base::Worker worker;
  worker.wCapacityTaskCount= 10;
  worker.sId = sId;
  worker.wState = base::StateType::READY;
  worker.wConnectPnt = "localhost:4445";
  return worker;
}

base::Task newTask(){
  base::Task task;  
  return task;
}

TEST_F(DBSchedrTest, getSchedrByCP){

  int* pSId = nullptr;
  auto schedrs = pDb_->getAllSchedrs(base::StateType::UNDEFINED);
  for (int i = 0; i < schedrs.size(); ++i){
    EXPECT_TRUE(pDb_->delSchedr(schedrs[i]));
  }

  auto schedr = newSchedr();
  int sId = 0;  
  EXPECT_TRUE(pDb_->addSchedr(schedr, sId) && (sId > 0)) << pDb_->getLastError(); 

  schedr.sState = base::StateType::ERRORT;
  schedr.sCapacityTaskCount = 1;   
  schedr.sId = 0;
  EXPECT_TRUE(pDb_->getSchedr(schedr.sConnectPnt, schedr) && (schedr.sId == sId) &&
                             (schedr.sState == base::StateType::READY) &&
                             (schedr.sConnectPnt == "localhost:4444") &&
                             (schedr.sCapacityTaskCount== 105)) << pDb_->getLastError(); 

  schedr.sState = base::StateType::ERRORT;
  schedr.sConnectPnt = ""; 
  schedr.sCapacityTaskCount = 1;
  EXPECT_TRUE(!pDb_->getSchedr(schedr.sConnectPnt, schedr));
  EXPECT_TRUE(schedr.sState == base::StateType::ERRORT);
  EXPECT_TRUE(schedr.sConnectPnt == "");
  EXPECT_TRUE(schedr.sCapacityTaskCount == 1);                                                      
}
TEST_F(DBSchedrTest, getTaskOfSchedr){
  auto schedr = newSchedr();
  int sId = 0;  
  EXPECT_TRUE(pDb_->addSchedr(schedr, sId) && (sId > 0)) << pDb_->getLastError(); 

  auto worker = newWorker(sId);
  int wId = 0;  
  EXPECT_TRUE(pDb_->addWorker(worker, wId) && (wId > 0)) << pDb_->getLastError();  
    
  int tId1 = 0;  
  base::Task task{0};
  EXPECT_TRUE(pDb_->startTask(0, task, tId1)) << pDb_->getLastError();        

  int tId2 = 0;  
  EXPECT_TRUE(pDb_->startTask(0, task, tId2 )) << pDb_->getLastError();        

  vector<base::Task> tasks;
  EXPECT_TRUE(pDb_->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 2) &&
              (tasks[0].tId == tId1) && 
              (tasks[0].tWId == 0)) << pDb_->getLastError();

  vector<db::MessSchedr> mess;
  mess.push_back(db::MessSchedr{mess::MessType::TASK_RUNNING, wId, tasks[0].tId});
  EXPECT_TRUE(pDb_->sendAllMessFromSchedr(sId, mess)) << pDb_->getLastError();

  mess.clear();
  mess.push_back(db::MessSchedr{mess::MessType::TASK_COMPLETED, wId, tasks[0].tId});
  EXPECT_TRUE(pDb_->sendAllMessFromSchedr(sId, mess)) << pDb_->getLastError();

          
}
TEST_F(DBSchedrTest, getWorkerOfSchedr){
  auto schedr = newSchedr();
  int sId = 0;  
  EXPECT_TRUE(pDb_->addSchedr(schedr, sId) && (sId > 0)) << pDb_->getLastError(); 
  
  auto worker = newWorker(sId);
  int wId = 0;  
  EXPECT_TRUE(pDb_->addWorker(worker, wId) && (wId > 0)) << pDb_->getLastError();   
 
  vector<base::Worker> workers;
  EXPECT_TRUE(pDb_->getWorkersOfSchedr(sId, workers) && 
              (workers.size() == 1) &&
              (workers[0].wId == wId)) << pDb_->getLastError(); 
}
TEST_F(DBSchedrTest, getNewTasksForSchedr){
  auto schedr = newSchedr();
  int sId = 0;  
  EXPECT_TRUE(pDb_->addSchedr(schedr, sId) && (sId > 0)) << pDb_->getLastError(); 

  auto worker = newWorker(sId);
  int wId = 0;  
  EXPECT_TRUE(pDb_->addWorker(worker, wId) && (wId > 0)) << pDb_->getLastError();  

  auto task = newTask();  
  int tId1 = 0;  
  EXPECT_TRUE(pDb_->startTask(0, task, tId1)) << pDb_->getLastError();        

  int tId2 = 0;  
  EXPECT_TRUE(pDb_->startTask(0, task, tId2)) << pDb_->getLastError();        

  vector<base::Task> tasks;
  EXPECT_TRUE(pDb_->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 2) &&
              (tasks[0].tId == tId1)) << pDb_->getLastError();

  vector<db::MessSchedr> mess;
  mess.push_back(db::MessSchedr{mess::MessType::TASK_RUNNING, wId, tasks[0].tId});
  EXPECT_TRUE(pDb_->sendAllMessFromSchedr(sId, mess)) << pDb_->getLastError();

  mess.clear();
  mess.push_back(db::MessSchedr{mess::MessType::TASK_COMPLETED, wId, tasks[0].tId});
  EXPECT_TRUE(pDb_->sendAllMessFromSchedr(sId, mess)) << pDb_->getLastError();
  
}
TEST_F(DBSchedrTest, getWorkerByTask){
  auto schedr = newSchedr();
  int sId = 0;  
  EXPECT_TRUE(pDb_->addSchedr(schedr, sId) && (sId > 0)) << pDb_->getLastError(); 

  auto worker = newWorker(sId);
  int wId = 0;  
  EXPECT_TRUE(pDb_->addWorker(worker, wId) && (wId > 0)) << pDb_->getLastError();  

  auto task = newTask();   
  int tId1 = 0;  
  EXPECT_TRUE(pDb_->startTask(0, task, tId1)) << pDb_->getLastError();        

  int tId2 = 0;  
  EXPECT_TRUE(pDb_->startTask(0, task, tId2)) << pDb_->getLastError();        

  vector<base::Task> tasks;
  EXPECT_TRUE(pDb_->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 2) &&
              (tasks[0].tId == tId1)) << pDb_->getLastError();

  vector<db::MessSchedr> mess;
  mess.push_back(db::MessSchedr{mess::MessType::TASK_RUNNING, wId, tasks[0].tId});
  EXPECT_TRUE(pDb_->sendAllMessFromSchedr(sId, mess)) << pDb_->getLastError();
  
  tasks.clear();
  worker.wId = 0;
  EXPECT_TRUE(pDb_->getWorkerByTask(tId1, worker) && 
              (worker.wId == wId)) << pDb_->getLastError();      
}
TEST_F(DBSchedrTest, sendAllMessFromSchedr){
  auto schedr = newSchedr();
  int sId = 0;  
  EXPECT_TRUE(pDb_->addSchedr(schedr, sId) && (sId > 0)) << pDb_->getLastError(); 
  
  auto worker = newWorker(sId);
  int wId = 0;  
  EXPECT_TRUE(pDb_->addWorker(worker, wId) && (wId > 0)) << pDb_->getLastError();   
  
  vector<db::MessSchedr> mess;
  mess.push_back(db::MessSchedr{mess::MessType::TASK_ERROR, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::TASK_COMPLETED, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::TASK_RUNNING, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::TASK_PAUSE, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::TASK_STOP, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::JUST_START_WORKER, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::PAUSE_SCHEDR, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::PAUSE_WORKER, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::START_SCHEDR, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::START_WORKER, wId, 0});
  mess.push_back(db::MessSchedr{mess::MessType::WORKER_NOT_RESPONDING, wId, 0});

  EXPECT_TRUE(pDb_->sendAllMessFromSchedr(sId, mess)) << pDb_->getLastError();
}

//#endif // DBSCHEDRTEST