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
#include <iostream>
#include <gtest/gtest.h>

#include "db_provider/db_provider.h"
#include "common/misc.h"
#include "client/zmclient.h"

using namespace std;

class APITest : public ::testing::Test {
public:
  APITest() { 

    string connStr = "host=localhost port=5432 user=postgres dbname=zmeydb connect_timeout=10";
    char err[256]{0};
    zc_ = zmCreateConnection(zmConfig{ (char*)connStr.c_str() }, err);
    if (strlen(err) > 0){    
      cout << err << endl;
      exit(-1);
    } 
   
  }
  ~APITest() {
    zmDisconnect(zc_);
  }
protected:
  zmConn zc_ = nullptr; 
};

TEST_F(APITest, addSchedr){  
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0));         
}
TEST_F(APITest, getSchedr){  
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0)); 

  schedr.capacityTask = 1;
  strcpy(schedr.connectPnt, "");    
  EXPECT_TRUE(zmGetScheduler(zc_, sId, &schedr) &&
             (strcmp(schedr.connectPnt, "localhost:4444") == 0) &&
             (schedr.capacityTask == 10000)); 

  schedr.capacityTask = 1;   
  strcpy(schedr.connectPnt, "");   
  EXPECT_TRUE(!zmGetScheduler(zc_, sId + 1, &schedr) &&
             (strcmp(schedr.connectPnt, "") == 0) &&
             (schedr.capacityTask == 1));                                                      
}
TEST_F(APITest, changeSchedr){  
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0)); 

  strcpy(schedr.connectPnt, "localhost:1234"); 
  schedr.capacityTask = 10;   
  EXPECT_TRUE(zmChangeScheduler(zc_, sId, schedr)); 

  strcpy(schedr.connectPnt, ""); 
  schedr.capacityTask = 1;      
  EXPECT_TRUE(zmGetScheduler(zc_, sId, &schedr) &&
             (strcmp(schedr.connectPnt, "localhost:1234") == 0) &&
             (schedr.capacityTask == 10));                                                      
}
TEST_F(APITest, delSchedr){
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0));
    
  EXPECT_TRUE(zmDelScheduler(zc_, sId));
       
  EXPECT_TRUE(!zmGetScheduler(zc_, sId, &schedr));  
}
TEST_F(APITest, schedrState){  
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0));

  ddSchedulerState state;
  EXPECT_TRUE(zmStateOfScheduler(zc_, sId, &state) && 
             (state.state == zmStateType::ddSTATE_STOP));                                                      
}
TEST_F(APITest, getAllSchedrs){  
  int* pSId = nullptr;
  auto sCnt = zmGetAllSchedulers(zc_, zmStateType::ddSTATE_UNDEFINED, &pSId);
  EXPECT_TRUE((sCnt == 0) && !pSId);   

  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId1 = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId1) && (sId1 > 0)); 

  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:5555");  
  int sId2 = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId2) && (sId2 > 0)); 

  sCnt = zmGetAllSchedulers(zc_, zmStateType::ddSTATE_STOP, &pSId);
  EXPECT_TRUE((sCnt == 2) && (pSId[0] == sId1) && (pSId[1] == sId2)); 

  sCnt = zmGetAllSchedulers(zc_, zmStateType::ddSTATE_UNDEFINED, &pSId);
  EXPECT_TRUE((sCnt == 2) && (pSId[0] == sId1) && (pSId[1] == sId2));   

  sCnt = zmGetAllSchedulers(zc_, zmStateType::ddSTATE_START, &pSId);
  EXPECT_TRUE(sCnt == 0);                     
}

TEST_F(APITest, addWorker){    
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId = 0;  
  EXPECT_TRUE(zmAddWorker(zc_, worker, &wId) && (wId > 0));  

  worker.sId = sId + 1;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4446");  
  wId = 0;  
  EXPECT_TRUE(!zmAddWorker(zc_, worker, &wId) && (wId == 0));           
}
TEST_F(APITest, getWorker){ 
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId = 0;  
  EXPECT_TRUE(zmAddWorker(zc_, worker, &wId) && (wId > 0));  

  strcpy(worker.connectPnt, "");  
  worker.capacityTask = 1;
  EXPECT_TRUE(zmGetWorker(zc_, wId, &worker) &&
             (strcmp(worker.connectPnt, "localhost:4445") == 0) &&
             (worker.capacityTask == 10)); 

  strcpy(worker.connectPnt, "");  
  worker.capacityTask = 1;   
  EXPECT_TRUE(!zmGetWorker(zc_, wId + 1, &worker) &&
             (strcmp(worker.connectPnt, "") == 0) &&
             (worker.capacityTask == 1));                                                       
}
TEST_F(APITest, changeWorker){ 
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId = 0;  
  EXPECT_TRUE(zmAddWorker(zc_, worker, &wId) && (wId > 0)); 
  
  strcpy(worker.connectPnt, "localhost:1234"); 
  worker.capacityTask = 100;   
  EXPECT_TRUE(zmChangeWorker(zc_, wId, worker)); 

  strcpy(worker.connectPnt, "");  
  worker.capacityTask = 1;
  EXPECT_TRUE(zmGetWorker(zc_, wId, &worker) &&
             (strcmp(worker.connectPnt, "localhost:1234") == 0) &&
             (worker.capacityTask == 100)); 

  worker.sId = sId + 1;
  EXPECT_TRUE(!zmChangeWorker(zc_, wId, worker)); 
}
TEST_F(APITest, delWorker){    
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId = 0;  
  EXPECT_TRUE(zmAddWorker(zc_, worker, &wId) && (wId > 0)); 
    
  EXPECT_TRUE(zmDelWorker(zc_, wId));
  
  EXPECT_TRUE(!zmGetWorker(zc_, wId, &worker));  
}
TEST_F(APITest, workerState){    
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId1 = 0;  
  EXPECT_TRUE(zmAddWorker(zc_, worker, &wId1) && (wId1 > 0)); 

  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4446");  
  int wId2 = 0;  
  EXPECT_TRUE(zmAddWorker(zc_, worker, &wId2) && (wId2 > 0));  

  zmWorkerState* wstate = new zmWorkerState[2];
  int* pWId = new int[2]{ wId1, wId2};
  EXPECT_TRUE(ddStateOfWorker(zc_, pWId, 2, wstate) &&
              (wstate[0].state == zmStateType::ddSTATE_STOP) && 
              (wstate[1].state == zmStateType::ddSTATE_STOP));                                                      
}
TEST_F(APITest, getAllWorkers){  
  int* pWId = nullptr;
  auto wCnt = zmGetAllWorkers(zc_, 1, zmStateType::ddSTATE_UNDEFINED, &pWId);
  EXPECT_TRUE((wCnt == 0) && !pWId);   

  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(zmAddScheduler(zc_, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId1 = 0;  
  EXPECT_TRUE(zmAddWorker(zc_, worker, &wId1) && (wId1 > 0)); 

  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4446");  
  int wId2 = 0;  
  EXPECT_TRUE(zmAddWorker(zc_, worker, &wId2) && (wId2 > 0));
    
  wCnt = zmGetAllWorkers(zc_, sId, zmStateType::ddSTATE_STOP, &pWId);
  EXPECT_TRUE((wCnt == 2) && (pWId[0] == wId1) && (pWId[1] == wId2)); 

  wCnt = zmGetAllWorkers(zc_, sId, zmStateType::ddSTATE_UNDEFINED, &pWId);
  EXPECT_TRUE((wCnt == 2) && (pWId[0] == wId1) && (pWId[1] == wId2));   

  wCnt = zmGetAllWorkers(zc_, sId, zmStateType::ddSTATE_START, &pWId);
  EXPECT_TRUE(wCnt == 0);        

  wCnt = zmGetAllWorkers(zc_, sId + 1, zmStateType::ddSTATE_UNDEFINED, &pWId);
  EXPECT_TRUE(wCnt == 0);                  
}

TEST_F(APITest, startTask){
  zmTask task{0};
  int tId1 = 0; 
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  EXPECT_TRUE(ddStartTask(zc_, task, &tId1)); 

  int tId2 = 0;  
  EXPECT_TRUE(ddStartTask(zc_, task, &tId2));           
}
TEST_F(APITest, cancelTask){
  zmTask task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  int tId1 = 0;  
  EXPECT_TRUE(zmStartTask(zc_, task, &tId1));        

  EXPECT_TRUE(zmCancelTask(zc_, tId1));           
}
TEST_F(APITest, taskState){
  zmTask task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  int tId1 = 0;  
  EXPECT_TRUE(zmStartTask(zc_, task, &tId1));  
  
  int tId2 = 0;  
  EXPECT_TRUE(zmStartTask(zc_, task, &tId2));  
  
  int* tIds = new int[2] {tId1, tId2};
  zmTaskState* tState = new zmTaskState[2];
  EXPECT_TRUE(zmStateOfTask(zc_, tIds, 2, tState) && 
              (tState[0].progress == 0) &&
              (tState[0].state == zmStateType::ddSTATE_READY) &&
              (tState[1].progress == 0) &&
              (tState[1].state == zmStateType::ddSTATE_READY)); 
}
TEST_F(APITest, TaskTime){
  zmTask task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  int tId1 = 0;  
  EXPECT_TRUE(zmStartTask(zc_, task,  &tId1));  

  zmTaskTime result;
  EXPECT_TRUE(zmTimeOfTask(zc_, tId1, &result));
}
TEST_F(APITest, getAllTask){
  zmTask task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  int tId1 = 0;  
  EXPECT_TRUE(zmStartTask(zc_, task, &tId1));  
 
  int tId2 = 0;  
  EXPECT_TRUE(zmStartTask(zc_, task, &tId2));    
}
