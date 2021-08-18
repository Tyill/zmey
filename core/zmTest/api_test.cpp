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

#include "prepare.h"
#include "zmDbProvider/db_provider.h"
#include "zmCommon/aux_func.h"
#include "zmClient/zmClient.h"

using namespace std;
using namespace zmey;

class APITest : public ::testing::Test {
public:
  APITest() { 

    string connStr = "host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10";
    char err[256]{0};
    _zc = zmey::zmCreateConnection(zmey::zmConfig{ (char*)connStr.c_str() },
                                                   err);
    if (strlen(err) > 0){    
      TEST_COUT << err << endl;
      exit(-1);
    } 
    zmey::zmSetErrorCBack(_zc, [](const char* mess, zmey::zmUData){
      TEST_COUT << mess << endl;
    }, nullptr); 

    ZM_DB::ConnectCng cng;
    cng.connectStr = connStr;
    ZM_DB::DbProvider dbp(cng);
    EXPECT_TRUE(dbp.delAllTables()) << dbp.getLastError();
    
    EXPECT_TRUE(zmey::zmCreateTables(_zc));
  }
  ~APITest() {
    zmey::zmDisconnect(_zc);
  }
protected:
  zmey::zmConn _zc = nullptr; 
};

TEST_F(APITest, addSchedr){  
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0)); 

  strcpy(schedr.connectPnt, "localhost4444");  
  sId = 0;  
  EXPECT_TRUE(!zmAddScheduler(_zc, schedr, &sId) && (sId == 0));               
}
TEST_F(APITest, getSchedr){  
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0)); 

  schedr.capacityTask = 1;
  strcpy(schedr.connectPnt, "");    
  EXPECT_TRUE(zmGetScheduler(_zc, sId, &schedr) &&
             (strcmp(schedr.connectPnt, "localhost:4444") == 0) &&
             (schedr.capacityTask == 10000)); 

  schedr.capacityTask = 1;   
  strcpy(schedr.connectPnt, "");   
  EXPECT_TRUE(!zmGetScheduler(_zc, sId + 1, &schedr) &&
             (strcmp(schedr.connectPnt, "") == 0) &&
             (schedr.capacityTask == 1));                                                      
}
TEST_F(APITest, changeSchedr){  
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0)); 

  strcpy(schedr.connectPnt, "localhost:1234"); 
  schedr.capacityTask = 10;   
  EXPECT_TRUE(zmChangeScheduler(_zc, sId, schedr)); 

  strcpy(schedr.connectPnt, ""); 
  schedr.capacityTask = 1;      
  EXPECT_TRUE(zmGetScheduler(_zc, sId, &schedr) &&
             (strcmp(schedr.connectPnt, "localhost:1234") == 0) &&
             (schedr.capacityTask == 10));                                                      
}
TEST_F(APITest, delSchedr){
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));
    
  EXPECT_TRUE(zmDelScheduler(_zc, sId));
       
  EXPECT_TRUE(!zmGetScheduler(_zc, sId, &schedr));  
}
TEST_F(APITest, schedrState){  
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));

  zmStateType state = zmStateType::zmSTATE_STOP;
  EXPECT_TRUE(zmSchedulerState(_zc, sId, &state) && 
             (state == zmStateType::zmSTATE_STOP));                                                      
}
TEST_F(APITest, getAllSchedrs){  
  uint64_t* pSId = nullptr;
  auto sCnt = zmGetAllSchedulers(_zc, zmStateType::zmSTATE_UNDEFINED, &pSId);
  EXPECT_TRUE((sCnt == 0) && !pSId);   

  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId1 = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId1) && (sId1 > 0)); 

  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:5555");  
  uint64_t sId2 = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId2) && (sId2 > 0)); 

  sCnt = zmGetAllSchedulers(_zc, zmStateType::zmSTATE_STOP, &pSId);
  EXPECT_TRUE((sCnt == 2) && (pSId[0] == sId1) && (pSId[1] == sId2)); 

  sCnt = zmGetAllSchedulers(_zc, zmStateType::zmSTATE_UNDEFINED, &pSId);
  EXPECT_TRUE((sCnt == 2) && (pSId[0] == sId1) && (pSId[1] == sId2));   

  sCnt = zmGetAllSchedulers(_zc, zmStateType::zmSTATE_START, &pSId);
  EXPECT_TRUE(sCnt == 0);                     
}

TEST_F(APITest, addWorker){    
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  uint64_t wId = 0;  
  EXPECT_TRUE(zmAddWorker(_zc, worker, &wId) && (wId > 0));  

  worker.sId = sId + 1;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4446");  
  wId = 0;  
  EXPECT_TRUE(!zmAddWorker(_zc, worker, &wId) && (wId == 0));           
}
TEST_F(APITest, getWorker){ 
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  uint64_t wId = 0;  
  EXPECT_TRUE(zmAddWorker(_zc, worker, &wId) && (wId > 0));  

  strcpy(worker.connectPnt, "");  
  worker.capacityTask = 1;
  EXPECT_TRUE(zmGetWorker(_zc, wId, &worker) &&
             (strcmp(worker.connectPnt, "localhost:4445") == 0) &&
             (worker.capacityTask == 10)); 

  strcpy(worker.connectPnt, "");  
  worker.capacityTask = 1;   
  EXPECT_TRUE(!zmGetWorker(_zc, wId + 1, &worker) &&
             (strcmp(worker.connectPnt, "") == 0) &&
             (worker.capacityTask == 1));                                                       
}
TEST_F(APITest, changeWorker){ 
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  uint64_t wId = 0;  
  EXPECT_TRUE(zmAddWorker(_zc, worker, &wId) && (wId > 0)); 
  
  strcpy(worker.connectPnt, "localhost:1234"); 
  worker.capacityTask = 100;   
  EXPECT_TRUE(zmChangeWorker(_zc, wId, worker)); 

  strcpy(worker.connectPnt, "");  
  worker.capacityTask = 1;
  EXPECT_TRUE(zmGetWorker(_zc, wId, &worker) &&
             (strcmp(worker.connectPnt, "localhost:1234") == 0) &&
             (worker.capacityTask == 100)); 

  worker.sId = sId + 1;
  EXPECT_TRUE(!zmChangeWorker(_zc, wId, worker)); 
}
TEST_F(APITest, delWorker){    
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  uint64_t wId = 0;  
  EXPECT_TRUE(zmAddWorker(_zc, worker, &wId) && (wId > 0)); 
    
  EXPECT_TRUE(zmDelWorker(_zc, wId));
  
  EXPECT_TRUE(!zmGetWorker(_zc, wId, &worker));  
}
TEST_F(APITest, workerState){    
  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  uint64_t wId1 = 0;  
  EXPECT_TRUE(zmAddWorker(_zc, worker, &wId1) && (wId1 > 0)); 

  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4446");  
  uint64_t wId2 = 0;  
  EXPECT_TRUE(zmAddWorker(_zc, worker, &wId2) && (wId2 > 0));  

  zmStateType* wstate = new zmStateType[2];
  uint64_t* pWId = new uint64_t[2]{ wId1, wId2};
  EXPECT_TRUE(zmWorkerState(_zc, pWId, 2, wstate) &&
              (wstate[0] == zmStateType::zmSTATE_STOP) && 
              (wstate[1] == zmStateType::zmSTATE_STOP));                                                      
}
TEST_F(APITest, getAllWorkers){  
  uint64_t* pWId = nullptr;
  auto wCnt = zmGetAllWorkers(_zc, 1, zmStateType::zmSTATE_UNDEFINED, &pWId);
  EXPECT_TRUE((wCnt == 0) && !pWId);   

  zmSchedr schedr{0};
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker{0};
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  uint64_t wId1 = 0;  
  EXPECT_TRUE(zmAddWorker(_zc, worker, &wId1) && (wId1 > 0)); 

  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4446");  
  uint64_t wId2 = 0;  
  EXPECT_TRUE(zmAddWorker(_zc, worker, &wId2) && (wId2 > 0));
    
  wCnt = zmGetAllWorkers(_zc, sId, zmStateType::zmSTATE_STOP, &pWId);
  EXPECT_TRUE((wCnt == 2) && (pWId[0] == wId1) && (pWId[1] == wId2)); 

  wCnt = zmGetAllWorkers(_zc, sId, zmStateType::zmSTATE_UNDEFINED, &pWId);
  EXPECT_TRUE((wCnt == 2) && (pWId[0] == wId1) && (pWId[1] == wId2));   

  wCnt = zmGetAllWorkers(_zc, sId, zmStateType::zmSTATE_START, &pWId);
  EXPECT_TRUE(wCnt == 0);        

  wCnt = zmGetAllWorkers(_zc, sId + 1, zmStateType::zmSTATE_UNDEFINED, &pWId);
  EXPECT_TRUE(wCnt == 0);                  
}

TEST_F(APITest, addTaskTemplate){  
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0;
  templ.workerPresetId = 0;
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId) && (tId > 0));   
}
TEST_F(APITest, getTaskTemplate){  
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0; 
  templ.workerPresetId = 0;
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId) && (tId > 0));    
  
  templ.averDurationSec = 11;
  templ.maxDurationSec = 110;
  templ.schedrPresetId= 10;
  templ.workerPresetId= 10;
  strcpy(templ.script, "1000");
  templ.userId = 1; 
  strcpy(templ.description, "dfsd");
  strcpy(templ.name, "new11ask");
  EXPECT_TRUE(zmGetTaskTemplate(_zc, tId, &templ) &&
             (templ.averDurationSec == 10) &&
             (templ.maxDurationSec == 100) &&
             (templ.schedrPresetId == 0) &&
             (templ.workerPresetId == 0) &&
             (strcmp(templ.script, "100500") == 0) &&
             (templ.userId == 0) &&
             (strcmp(templ.description, "descr") == 0) &&
             (strcmp(templ.name, "NEW_TASK") == 0)); 

  templ.averDurationSec = 1;
  templ.maxDurationSec = 10;
  EXPECT_TRUE(!zmGetTaskTemplate(_zc, tId + 1, &templ) &&
             (templ.averDurationSec == 1) &&
             (templ.maxDurationSec == 10));                         
}
TEST_F(APITest, delTaskTemplate){
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0;
  templ.workerPresetId = 0; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId) && (tId > 0));
  
  EXPECT_TRUE(zmDelTaskTemplate(_zc, tId));   

  templ.averDurationSec = 1;
  templ.maxDurationSec = 10;
  EXPECT_TRUE(!zmGetTaskTemplate(_zc, tId, &templ) && 
              (templ.averDurationSec == 1) &&
              (templ.maxDurationSec == 10));    
}
TEST_F(APITest, changeTaskTemplate){
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0;
  templ.workerPresetId = 0; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId) && (tId > 0));  
  
  templ.averDurationSec = 1;
  templ.maxDurationSec = 10;
  strcpy(templ.script, "100");
  templ.userId = 1; 
  strcpy(templ.description, "de");
  strcpy(templ.name, "new");
  EXPECT_TRUE(zmChangeTaskTemplate(_zc, tId, templ));   

  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  strcpy(templ.script, "1000");
  templ.userId = 2; 
  strcpy(templ.description, "d00");
  strcpy(templ.name, "new00");
  EXPECT_TRUE(zmGetTaskTemplate(_zc, tId, &templ) &&
             (templ.averDurationSec == 1) &&
             (templ.maxDurationSec == 10) &&
             (strcmp(templ.script, "100") == 0) &&
             (templ.userId == 1) &&
             (strcmp(templ.description, "de") == 0) &&
             (strcmp(templ.name, "new") == 0));   
}
TEST_F(APITest, getAllTaskTemplate){
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0; 
  templ.workerPresetId = 0; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId1) && (tId1 > 0));  

  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  strcpy(templ.script, "100500");
  templ.userId = 1; 
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t tId2 = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId2) && (tId2 > 0)); 
    
  uint64_t* pTT = nullptr;    
  auto tCnt = zmGetAllTaskTemplates(_zc, 0, &pTT);
  EXPECT_TRUE((tCnt == 1) && (pTT[0] == tId1)); 

  tCnt = zmGetAllTaskTemplates(_zc, 1, &pTT);
  EXPECT_TRUE((tCnt == 1) && (pTT[0] == tId2));   

  tCnt = zmGetAllTaskTemplates(_zc, 2, &pTT);
  EXPECT_TRUE(tCnt == 0);
}

TEST_F(APITest, startTask){
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0; 
  templ.workerPresetId = 0; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task{0};
  task.ttlId = ttId; 
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmStartTask(_zc, task, &tId1)); 

  uint64_t tId2 = 0;  
  EXPECT_TRUE(zmStartTask(_zc, task, &tId2));           
}
TEST_F(APITest, cancelTask){
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0; 
  templ.workerPresetId = 0; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task{0};
  task.ttlId = ttId; 
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmStartTask(_zc, task, &tId1));        

  EXPECT_TRUE(zmCancelTask(_zc, tId1));           
}
TEST_F(APITest, taskState){
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0; 
  templ.workerPresetId = 0; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 
  
  zmTask task{0};
  task.ttlId = ttId; 
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmStartTask(_zc, task, &tId1));  
  
  task.ttlId = ttId; 
  uint64_t tId2 = 0;  
  EXPECT_TRUE(zmStartTask(_zc, task, &tId2));  
  
  uint64_t* tIds = new uint64_t[2] {tId1, tId2};
  zmTaskState* tState = new zmTaskState[2];
  EXPECT_TRUE(zmStateOfTask(_zc, tIds, 2, tState) && 
              (tState[0].progress == 0) &&
              (tState[0].state == zmStateType::zmSTATE_READY) &&
              (tState[1].progress == 0) &&
              (tState[1].state == zmStateType::zmSTATE_READY)); 
}
TEST_F(APITest, taskResult){
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0; 
  templ.workerPresetId = 0; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task{0};
  task.ttlId = ttId; 
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmStartTask(_zc, task, &tId1));  

  char* result = nullptr;
  EXPECT_TRUE(zmResultOfTask(_zc, tId1, &result)); 
}
TEST_F(APITest, TaskTime){
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0; 
  templ.workerPresetId = 0; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 
  
  zmTask task{0};
  task.ttlId = ttId; 
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmStartTask(_zc, task,  &tId1));  

  zmTaskTime result;
  EXPECT_TRUE(zmTimeOfTask(_zc, tId1, &result));
}
TEST_F(APITest, getAllTask){
  zmTaskTemplate templ{0};
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = 0; 
  templ.schedrPresetId = 0; 
  templ.workerPresetId = 0; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "NEW_TASK");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task{0};
  task.ttlId = ttId; 
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmStartTask(_zc, task, &tId1));  
 
  uint64_t tId2 = 0;  
  EXPECT_TRUE(zmStartTask(_zc, task, &tId2));    
  
  // tCnt = zmGetAllTasks(_zc, pId, zmStateType::zmStart, &pTT);
  // EXPECT_TRUE(tCnt == 0); 

  // tCnt = zmGetAllTasks(_zc, pId + 1, zmStateType::zmReady, &pTT);
  // EXPECT_TRUE(tCnt == 0); 

  // tCnt = zmGetAllTasks(_zc, pId + 1, zmStateType::zmUndefined, &pTT);
  // EXPECT_TRUE(tCnt == 0);  
}

#endif //APITEST