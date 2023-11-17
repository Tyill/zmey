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

#define APITEST
#ifdef APITEST

#include <vector>
#include <algorithm>
#include <memory>
#include <gtest/gtest.h>

#include "db_provider/db_provider.h"
#include "common/misc.h"
#include "client/zmclient.h"

using namespace std;

class APITest : public ::testing::Test {
public:
  APITest() { 

    string connStr = "host=localhost port=5432 password=123 dbname=dagdb connect_timeout=10";
    char err[256]{0};
    _zc = ddag::ddCreateConnection(ddag::ddConfig{ (char*)connStr.c_str() },
                                                   err);
    if (strlen(err) > 0){    
      TEST_COUT << err << endl;
      exit(-1);
    } 
    ddag::ddSetErrorCBack(_zc, [](const char* mess, ddag::ddUData){
      TEST_COUT << mess << endl;
    }, nullptr); 

    db::ConnectCng cng;
    cng.connectStr = connStr;
    db::DbProvider dbp(cng);
    EXPECT_TRUE(dbp.delAllTables()) << dbp.getLastError();
    
    EXPECT_TRUE(ddag::ddCreateTables(_zc));
  }
  ~APITest() {
    ddag::ddDisconnect(_zc);
  }
protected:
  ddag::ddConn _zc = nullptr; 
};

TEST_F(APITest, addSchedr){  
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0));         
}
TEST_F(APITest, getSchedr){  
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0)); 

  schedr.capacityTask = 1;
  strcpy(schedr.connectPnt, "");    
  EXPECT_TRUE(ddGetScheduler(_zc, sId, &schedr) &&
             (strcmp(schedr.connectPnt, "localhost:4444") == 0) &&
             (schedr.capacityTask == 10000)); 

  schedr.capacityTask = 1;   
  strcpy(schedr.connectPnt, "");   
  EXPECT_TRUE(!ddGetScheduler(_zc, sId + 1, &schedr) &&
             (strcmp(schedr.connectPnt, "") == 0) &&
             (schedr.capacityTask == 1));                                                      
}
TEST_F(APITest, changeSchedr){  
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0)); 

  strcpy(schedr.connectPnt, "localhost:1234"); 
  schedr.capacityTask = 10;   
  EXPECT_TRUE(ddChangeScheduler(_zc, sId, schedr)); 

  strcpy(schedr.connectPnt, ""); 
  schedr.capacityTask = 1;      
  EXPECT_TRUE(ddGetScheduler(_zc, sId, &schedr) &&
             (strcmp(schedr.connectPnt, "localhost:1234") == 0) &&
             (schedr.capacityTask == 10));                                                      
}
TEST_F(APITest, delSchedr){
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0));
    
  EXPECT_TRUE(ddDelScheduler(_zc, sId));
       
  EXPECT_TRUE(!ddGetScheduler(_zc, sId, &schedr));  
}
TEST_F(APITest, schedrState){  
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0));

  ddSchedulerState state;
  EXPECT_TRUE(ddStateOfScheduler(_zc, sId, &state) && 
             (state.state == ddStateType::ddSTATE_STOP));                                                      
}
TEST_F(APITest, getAllSchedrs){  
  int* pSId = nullptr;
  auto sCnt = ddGetAllSchedulers(_zc, ddStateType::ddSTATE_UNDEFINED, &pSId);
  EXPECT_TRUE((sCnt == 0) && !pSId);   

  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId1 = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId1) && (sId1 > 0)); 

  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:5555");  
  int sId2 = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId2) && (sId2 > 0)); 

  sCnt = ddGetAllSchedulers(_zc, ddStateType::ddSTATE_STOP, &pSId);
  EXPECT_TRUE((sCnt == 2) && (pSId[0] == sId1) && (pSId[1] == sId2)); 

  sCnt = ddGetAllSchedulers(_zc, ddStateType::ddSTATE_UNDEFINED, &pSId);
  EXPECT_TRUE((sCnt == 2) && (pSId[0] == sId1) && (pSId[1] == sId2));   

  sCnt = ddGetAllSchedulers(_zc, ddStateType::ddSTATE_START, &pSId);
  EXPECT_TRUE(sCnt == 0);                     
}

TEST_F(APITest, addWorker){    
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  ddWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId = 0;  
  EXPECT_TRUE(ddAddWorker(_zc, worker, &wId) && (wId > 0));  

  worker.sId = sId + 1;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4446");  
  wId = 0;  
  EXPECT_TRUE(!ddAddWorker(_zc, worker, &wId) && (wId == 0));           
}
TEST_F(APITest, getWorker){ 
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  ddWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId = 0;  
  EXPECT_TRUE(ddAddWorker(_zc, worker, &wId) && (wId > 0));  

  strcpy(worker.connectPnt, "");  
  worker.capacityTask = 1;
  EXPECT_TRUE(ddGetWorker(_zc, wId, &worker) &&
             (strcmp(worker.connectPnt, "localhost:4445") == 0) &&
             (worker.capacityTask == 10)); 

  strcpy(worker.connectPnt, "");  
  worker.capacityTask = 1;   
  EXPECT_TRUE(!ddGetWorker(_zc, wId + 1, &worker) &&
             (strcmp(worker.connectPnt, "") == 0) &&
             (worker.capacityTask == 1));                                                       
}
TEST_F(APITest, changeWorker){ 
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  ddWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId = 0;  
  EXPECT_TRUE(ddAddWorker(_zc, worker, &wId) && (wId > 0)); 
  
  strcpy(worker.connectPnt, "localhost:1234"); 
  worker.capacityTask = 100;   
  EXPECT_TRUE(ddChangeWorker(_zc, wId, worker)); 

  strcpy(worker.connectPnt, "");  
  worker.capacityTask = 1;
  EXPECT_TRUE(ddGetWorker(_zc, wId, &worker) &&
             (strcmp(worker.connectPnt, "localhost:1234") == 0) &&
             (worker.capacityTask == 100)); 

  worker.sId = sId + 1;
  EXPECT_TRUE(!ddChangeWorker(_zc, wId, worker)); 
}
TEST_F(APITest, delWorker){    
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  ddWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId = 0;  
  EXPECT_TRUE(ddAddWorker(_zc, worker, &wId) && (wId > 0)); 
    
  EXPECT_TRUE(ddDelWorker(_zc, wId));
  
  EXPECT_TRUE(!ddGetWorker(_zc, wId, &worker));  
}
TEST_F(APITest, workerState){    
  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  ddWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId1 = 0;  
  EXPECT_TRUE(ddAddWorker(_zc, worker, &wId1) && (wId1 > 0)); 

  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4446");  
  int wId2 = 0;  
  EXPECT_TRUE(ddAddWorker(_zc, worker, &wId2) && (wId2 > 0));  

  ddWorkerState* wstate = new ddWorkerState[2];
  int* pWId = new int[2]{ wId1, wId2};
  EXPECT_TRUE(ddStateOfWorker(_zc, pWId, 2, wstate) &&
              (wstate[0].state == ddStateType::ddSTATE_STOP) && 
              (wstate[1].state == ddStateType::ddSTATE_STOP));                                                      
}
TEST_F(APITest, getAllWorkers){  
  int* pWId = nullptr;
  auto wCnt = ddGetAllWorkers(_zc, 1, ddStateType::ddSTATE_UNDEFINED, &pWId);
  EXPECT_TRUE((wCnt == 0) && !pWId);   

  ddSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  int sId = 0;  
  EXPECT_TRUE(ddAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  ddWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  int wId1 = 0;  
  EXPECT_TRUE(ddAddWorker(_zc, worker, &wId1) && (wId1 > 0)); 

  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4446");  
  int wId2 = 0;  
  EXPECT_TRUE(ddAddWorker(_zc, worker, &wId2) && (wId2 > 0));
    
  wCnt = ddGetAllWorkers(_zc, sId, ddStateType::ddSTATE_STOP, &pWId);
  EXPECT_TRUE((wCnt == 2) && (pWId[0] == wId1) && (pWId[1] == wId2)); 

  wCnt = ddGetAllWorkers(_zc, sId, ddStateType::ddSTATE_UNDEFINED, &pWId);
  EXPECT_TRUE((wCnt == 2) && (pWId[0] == wId1) && (pWId[1] == wId2));   

  wCnt = ddGetAllWorkers(_zc, sId, ddStateType::ddSTATE_START, &pWId);
  EXPECT_TRUE(wCnt == 0);        

  wCnt = ddGetAllWorkers(_zc, sId + 1, ddStateType::ddSTATE_UNDEFINED, &pWId);
  EXPECT_TRUE(wCnt == 0);                  
}

TEST_F(APITest, startTask){
  ddTask task{0};
  int tId1 = 0; 
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  EXPECT_TRUE(ddStartTask(_zc, task, &tId1)); 

  int tId2 = 0;  
  EXPECT_TRUE(ddStartTask(_zc, task, &tId2));           
}
TEST_F(APITest, cancelTask){
  ddTask task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  int tId1 = 0;  
  EXPECT_TRUE(ddStartTask(_zc, task, &tId1));        

  EXPECT_TRUE(ddCancelTask(_zc, tId1));           
}
TEST_F(APITest, taskState){
  ddTask task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  int tId1 = 0;  
  EXPECT_TRUE(ddStartTask(_zc, task, &tId1));  
  
  int tId2 = 0;  
  EXPECT_TRUE(ddStartTask(_zc, task, &tId2));  
  
  int* tIds = new int[2] {tId1, tId2};
  ddTaskState* tState = new ddTaskState[2];
  EXPECT_TRUE(ddStateOfTask(_zc, tIds, 2, tState) && 
              (tState[0].progress == 0) &&
              (tState[0].state == ddStateType::ddSTATE_READY) &&
              (tState[1].progress == 0) &&
              (tState[1].state == ddStateType::ddSTATE_READY)); 
}
TEST_F(APITest, TaskTime){
  ddTask task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  int tId1 = 0;  
  EXPECT_TRUE(ddStartTask(_zc, task,  &tId1));  

  ddTaskTime result;
  EXPECT_TRUE(ddTimeOfTask(_zc, tId1, &result));
}
TEST_F(APITest, getAllTask){
  ddTask task{0};
  char path[256]{0};
  task.scriptPath = path; 
  task.resultPath = path;
  int tId1 = 0;  
  EXPECT_TRUE(ddStartTask(_zc, task, &tId1));  
 
  int tId2 = 0;  
  EXPECT_TRUE(ddStartTask(_zc, task, &tId2));    
}

#endif //APITEST