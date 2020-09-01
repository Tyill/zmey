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
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/auxFunc.h"
#include "zmClient/zmClient.h"

using namespace std;
using namespace zmey;

class APITest : public ::testing::Test {
public:
  APITest() { 

    string connStr = "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10";
    char err[256]{0};
    _zc = zmey::zmCreateConnection(zmey::zmConnect{ (char*)connStr.c_str() },
                                                   err);
    if (strlen(err) > 0){    
      TEST_COUT << err << endl;
      exit(-1);
    } 
    zmey::zmSetErrorCBack(_zc, [](const char* mess, zmey::zmUData){
      TEST_COUT << mess << endl;
    }, nullptr); 

    ZM_DB::connectCng cng;
    cng.connectStr = connStr;
    auto pDb = new ZM_DB::DbProvider(cng);
    if (pDb){
      EXPECT_TRUE(pDb->delAllTables())   << pDb->getLastError();
      delete pDb;
    }
    EXPECT_TRUE(zmey::zmCreateTables(_zc));
  }
  ~APITest() {
    zmey::zmDisconnect(_zc);
  }
protected:
  zmey::zmConn _zc = nullptr; 
};

TEST_F(APITest, addUser){  
  zmUser usr;
  usr.name[0] = '\0';
  usr.passw[0] = '\0'; 
  uint64_t uId = 0;  
  EXPECT_TRUE(!zmAddUser(_zc, usr, &uId) && (uId == 0));

  strcpy(usr.name, "alm1");
  uId = 0;
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  uId = 0;
  EXPECT_TRUE(!zmAddUser(_zc, usr, &uId) && (uId == 0));

  strcpy(usr.name, "alm2");
  strcpy(usr.passw, "123");
  uint64_t uId2 = 0;
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId2) && (uId2 > 0));
  
  EXPECT_TRUE(uId2 > uId);
}
TEST_F(APITest, getUser){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  strcpy(usr.name, "");
  strcpy(usr.passw, "");
  uId = 0;  
  EXPECT_TRUE(!zmGetUserId(_zc, usr, &uId) && (uId == 0));

  strcpy(usr.name, "alm");
  strcpy(usr.passw, "");
  uId = 0;  
  EXPECT_TRUE(!zmGetUserId(_zc, usr, &uId) && (uId == 0));

  strcpy(usr.name, "alm1");
  strcpy(usr.passw, "123");
  uId = 0;  
  EXPECT_TRUE(!zmGetUserId(_zc, usr, &uId) && (uId == 0));

  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uId = 0;  
  EXPECT_TRUE(zmGetUserId(_zc, usr, &uId) && (uId > 0));

  EXPECT_TRUE(zmDelUser(_zc, uId));

  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uId = 0;  
  EXPECT_TRUE(!zmGetUserId(_zc, usr, &uId) && (uId == 0));

  /////////////////////////

  strcpy(usr.name, "alm2");
  strcpy(usr.passw, "123");
  usr.description = new char[16];
  strcpy(usr.description, "abc");
  uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  strcpy(usr.name, "");
  strcpy(usr.passw, "");
  strcpy(usr.description, "");
  EXPECT_TRUE(zmGetUserCng(_zc, uId, &usr) &&
             (strcmp(usr.name, "alm2") == 0) &&
             (strcmp(usr.passw, "") == 0) &&
             (strcmp(usr.description, "abc") == 0));
  delete usr.description;
}
TEST_F(APITest, changeUser){   
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  usr.description = new char[16];
  strcpy(usr.description, "abc");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  strcpy(usr.name, "mla");
  strcpy(usr.passw, "321");
  strcpy(usr.description, "cba");
  EXPECT_TRUE(zmChangeUser(_zc, uId, usr));

  strcpy(usr.name, "");
  strcpy(usr.passw, "");
  strcpy(usr.description, "");
  EXPECT_TRUE(zmGetUserCng(_zc, uId, &usr) &&
             (strcmp(usr.name, "mla") == 0) &&
             (strcmp(usr.passw, "") == 0) &&
             (strcmp(usr.description, "cba") == 0));    
  delete usr.description;                        
}
TEST_F(APITest, delUser){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  usr.description = new char[16];
  strcpy(usr.description, "abc");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));
    
  EXPECT_TRUE(zmDelUser(_zc, uId));
  
  EXPECT_TRUE(!zmGetUserCng(_zc, uId, &usr));  

  uId = 0;
  EXPECT_TRUE(!zmGetUserId(_zc, usr, &uId) && (uId == 0));

  delete usr.description;                               
}
TEST_F(APITest, getAllUsers){  
  uint64_t* pUId = nullptr;
  auto uCnt = zmGetAllUsers(_zc, &pUId);
  EXPECT_TRUE((uCnt == 0) && !pUId);   

  zmUser usr;
  strcpy(usr.name, "alm1");
  strcpy(usr.passw, "123");
  usr.description = new char[16];
  strcpy(usr.description, "abc");
  uint64_t uId1 = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId1) && (uId1 > 0));

  strcpy(usr.name, "alm2");
  strcpy(usr.passw, "1234");
  usr.description = new char[16];
  strcpy(usr.description, "abcd");
  uint64_t uId2 = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId2) && (uId2 > 0));

  uCnt = zmGetAllUsers(_zc, &pUId);
  EXPECT_TRUE((uCnt == 2) && (pUId[0] == uId1) && (pUId[1] == uId2));

  delete usr.description;                    
}

TEST_F(APITest, addSchedr){  
  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0)); 

  strcpy(schedr.connectPnt, "localhost4444");  
  sId = 0;  
  EXPECT_TRUE(!zmAddScheduler(_zc, schedr, &sId) && (sId == 0));               
}
TEST_F(APITest, getSchedr){  
  zmSchedr schedr;
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
  zmSchedr schedr;
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
  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));
    
  EXPECT_TRUE(zmDelScheduler(_zc, sId));
       
  EXPECT_TRUE(!zmGetScheduler(_zc, sId, &schedr));  
}
TEST_F(APITest, schedrState){  
  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));

  zmStateType state = zmStateType::zmError;
  EXPECT_TRUE(zmSchedulerState(_zc, sId, &state) && 
             (state == zmStateType::zmReady));                                                      
}
TEST_F(APITest, getAllSchedrs){  
  uint64_t* pSId = nullptr;
  auto sCnt = zmGetAllSchedulers(_zc, zmStateType::zmUndefined, &pSId);
  EXPECT_TRUE((sCnt == 0) && !pSId);   

  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId1 = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId1) && (sId1 > 0)); 

  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:5555");  
  uint64_t sId2 = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId2) && (sId2 > 0)); 

  sCnt = zmGetAllSchedulers(_zc, zmStateType::zmReady, &pSId);
  EXPECT_TRUE((sCnt == 2) && (pSId[0] == sId1) && (pSId[1] == sId2)); 

  sCnt = zmGetAllSchedulers(_zc, zmStateType::zmUndefined, &pSId);
  EXPECT_TRUE((sCnt == 2) && (pSId[0] == sId1) && (pSId[1] == sId2));   

  sCnt = zmGetAllSchedulers(_zc, zmStateType::zmStart, &pSId);
  EXPECT_TRUE(sCnt == 0);                     
}

TEST_F(APITest, addWorker){    
  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker;
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
  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker;
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
  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker;
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
  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker;
  worker.sId = sId;
  worker.capacityTask = 10;
  strcpy(worker.connectPnt, "localhost:4445");  
  uint64_t wId = 0;  
  EXPECT_TRUE(zmAddWorker(_zc, worker, &wId) && (wId > 0)); 
    
  EXPECT_TRUE(zmDelWorker(_zc, wId));
  
  EXPECT_TRUE(!zmGetWorker(_zc, wId, &worker));  
}
TEST_F(APITest, workerState){    
  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker;
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
              (wstate[0] == zmStateType::zmReady) && 
              (wstate[1] == zmStateType::zmReady));                                                      
}
TEST_F(APITest, getAllWorkers){  
  uint64_t* pWId = nullptr;
  auto wCnt = zmGetAllWorkers(_zc, 1, zmStateType::zmUndefined, &pWId);
  EXPECT_TRUE((wCnt == 0) && !pWId);   

  zmSchedr schedr;
  schedr.capacityTask = 10000;
  strcpy(schedr.connectPnt, "localhost:4444");  
  uint64_t sId = 0;  
  EXPECT_TRUE(zmAddScheduler(_zc, schedr, &sId) && (sId > 0));  
  
  zmWorker worker;
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
    
  wCnt = zmGetAllWorkers(_zc, sId, zmStateType::zmReady, &pWId);
  EXPECT_TRUE((wCnt == 2) && (pWId[0] == wId1) && (pWId[1] == wId2)); 

  wCnt = zmGetAllWorkers(_zc, sId, zmStateType::zmUndefined, &pWId);
  EXPECT_TRUE((wCnt == 2) && (pWId[0] == wId1) && (pWId[1] == wId2));   

  wCnt = zmGetAllWorkers(_zc, sId, zmStateType::zmStart, &pWId);
  EXPECT_TRUE(wCnt == 0);        

  wCnt = zmGetAllWorkers(_zc, sId + 1, zmStateType::zmUndefined, &pWId);
  EXPECT_TRUE(wCnt == 0);                  
}

TEST_F(APITest, addPipeline){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 

  strcpy(ppline.name, "");
  pId = 0;  
  EXPECT_TRUE(!zmAddPipeline(_zc, ppline, &pId) && (pId == 0));

  strcpy(ppline.name, "newPP1");
  ppline.userId = uId + 1;
  pId = 0;  
  EXPECT_TRUE(!zmAddPipeline(_zc, ppline, &pId) && (pId == 0));

  delete ppline.description;              
}
TEST_F(APITest, getPipeline){ 
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 

  strcpy(ppline.name, "");
  strcpy(ppline.description, "");
  EXPECT_TRUE(zmGetPipeline(_zc, pId, &ppline) && 
                           (ppline.userId == uId) &&
                           (strcmp(ppline.name, "newPP") == 0) &&
                           (strcmp(ppline.description, "dfsdf") == 0)); 

  ppline.userId = 0;
  strcpy(ppline.name, "");
  strcpy(ppline.description, "dd");
  EXPECT_TRUE(!zmGetPipeline(_zc, pId + 1, &ppline) &&
             (ppline.userId == 0) &&
             (strcmp(ppline.name, "") == 0) &&
             (strcmp(ppline.description, "dd") == 0)); 

  delete ppline.description;                                                            
}
TEST_F(APITest, changePipeline){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0));  

  strcpy(ppline.name, "super");
  strcpy(ppline.description, "localhost:1234");
  ppline.userId = uId;
  EXPECT_TRUE(zmChangePipeline(_zc, pId, ppline)); 

  strcpy(ppline.name, "");
  strcpy(ppline.description, "1234");
  ppline.userId = 0;
  EXPECT_TRUE(zmGetPipeline(_zc, pId, &ppline) &&
                           (strcmp(ppline.name, "super") == 0) &&
                           (strcmp(ppline.description, "localhost:1234") == 0) &&
                           (ppline.userId == uId));  
                                                
  ppline.userId = uId + 1;
  EXPECT_TRUE(!zmChangePipeline(_zc, pId, ppline));  

  delete ppline.description;                                                                                             
}
TEST_F(APITest, delPipeline){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0));  
    
  EXPECT_TRUE(zmDelPipeline(_zc, pId));
  
  EXPECT_TRUE(!zmGetPipeline(_zc, pId, &ppline)); 

  delete ppline.description;     
}
TEST_F(APITest, getAllPipelines){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId1 = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId1) && (pId1 > 0));  

  strcpy(ppline.name, "newPP2");
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId2 = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId2) && (pId2 > 0));  

  uint64_t* pPP = nullptr;     
  auto ppCnt = zmGetAllPipelines(_zc, uId, &pPP);
  EXPECT_TRUE((ppCnt == 2) && (pPP[0] == pId1) && (pPP[1] == pId2)); 

  ppCnt = zmGetAllPipelines(_zc, uId + 1, &pPP);
  EXPECT_TRUE(ppCnt == 0); 

  delete ppline.description;          
}

TEST_F(APITest, addGroup){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 

  zmGroup group;
  strcpy(group.name, "newGrp");
  group.description = new char[24];
  strcpy(group.description, "hjghjghj");
  group.pplId = pId;
  uint64_t gId = 0;  
  EXPECT_TRUE(zmAddGroup(_zc, group, &gId) && (gId > 0)); 

  strcpy(group.name, "");
  gId = 0;  
  EXPECT_TRUE(!zmAddGroup(_zc, group, &gId) && (gId == 0));

  strcpy(group.name, "newGG");
  group.pplId = pId + 1;
  gId = 0;  
  EXPECT_TRUE(!zmAddGroup(_zc, group, &gId) && (gId == 0));

  delete ppline.description;              
}
TEST_F(APITest, getGroup){ 
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 

  zmGroup group;
  strcpy(group.name, "newGrp");
  group.description = new char[24];
  strcpy(group.description, "hjghjghj");
  group.pplId = pId;
  uint64_t gId = 0;  
  EXPECT_TRUE(zmAddGroup(_zc, group, &gId) && (gId > 0)); 

  strcpy(group.name, "");
  strcpy(group.description, "");
  EXPECT_TRUE(zmGetGroup(_zc, gId, &group) && 
                           (group.pplId == pId) &&
                           (strcmp(group.name, "newGrp") == 0) &&
                           (strcmp(group.description, "hjghjghj") == 0)); 

  group.pplId = 0;
  strcpy(group.name, "");
  strcpy(group.description, "dd");
  EXPECT_TRUE(!zmGetGroup(_zc, gId + 1, &group) &&
             (group.pplId == 0) &&
             (strcmp(group.name, "") == 0) &&
             (strcmp(group.description, "dd") == 0)); 

  delete group.description;                                                            
}
TEST_F(APITest, changeGroup){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0));  

  zmGroup group;
  strcpy(group.name, "newGrp");
  group.description = new char[24];
  strcpy(group.description, "hjghjghj");
  group.pplId = pId;
  uint64_t gId = 0;  
  EXPECT_TRUE(zmAddGroup(_zc, group, &gId) && (gId > 0)); 

  strcpy(group.name, "super");
  strcpy(group.description, "localhost:1234");
  group.pplId = pId;
  EXPECT_TRUE(zmChangeGroup(_zc, gId, group)); 

  strcpy(group.name, "");
  strcpy(group.description, "1234");
  group.pplId = 0;
  EXPECT_TRUE(zmGetGroup(_zc, gId, &group) &&
                           (strcmp(group.name, "super") == 0) &&
                           (strcmp(group.description, "localhost:1234") == 0) &&
                           (group.pplId == pId));  
                                                
  group.pplId = pId + 1;
  EXPECT_TRUE(!zmChangeGroup(_zc, gId, group));  

  delete ppline.description;                                                                                             
}
TEST_F(APITest, delGroup){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0));  
    
  zmGroup group;
  strcpy(group.name, "newGrp");
  group.description = new char[24];
  strcpy(group.description, "hjghjghj");
  group.pplId = pId;
  uint64_t gId = 0;  
  EXPECT_TRUE(zmAddGroup(_zc, group, &gId) && (gId > 0)); 

  EXPECT_TRUE(zmDelGroup(_zc, gId));
  
  EXPECT_TRUE(!zmGetGroup(_zc, gId, &group)); 

  delete ppline.description;     
}
TEST_F(APITest, getAllGroups){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId1 = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId1) && (pId1 > 0));  

  zmGroup group;
  strcpy(group.name, "newGrp1");
  group.description = new char[24];
  strcpy(group.description, "hjghjghj");
  group.pplId = pId1;
  uint64_t gId1 = 0;  
  EXPECT_TRUE(zmAddGroup(_zc, group, &gId1) && (gId1 > 0));

  strcpy(group.name, "newGrp2");
  group.description = new char[24];
  strcpy(group.description, "hjghjghj");
  group.pplId = pId1;
  uint64_t gId2 = 0;  
  EXPECT_TRUE(zmAddGroup(_zc, group, &gId2) && (gId2 > 0)); 
  
  uint64_t* pGR = nullptr;     
  auto grCnt = zmGetAllGroups(_zc, pId1, &pGR);
  EXPECT_TRUE((grCnt == 2) && (pGR[0] == gId1) && (pGR[1] == gId2)); 

  grCnt = zmGetAllGroups(_zc, pId1 + 1, &pGR);
  EXPECT_TRUE(grCnt == 0);

  delete ppline.description;          
}

TEST_F(APITest, addTaskTemplate){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));
  
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId) && (tId > 0));   

  templ.userId = uId + 1;  
  tId = 0;  
  EXPECT_TRUE(!zmAddTaskTemplate(_zc, templ, &tId) && (tId == 0));

  delete templ.script;   
  delete templ.description;            
}
TEST_F(APITest, getTaskTemplate){  
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));
  
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId) && (tId > 0));    
  
  templ.averDurationSec = 11;
  templ.maxDurationSec = 110;
  strcpy(templ.script, "1000");
  templ.userId = uId; 
  strcpy(templ.description, "dfsd");
  strcpy(templ.name, "new11ask");
  EXPECT_TRUE(zmGetTaskTemplate(_zc, tId, &templ) &&
             (templ.averDurationSec == 10) &&
             (templ.maxDurationSec == 100) &&
             (strcmp(templ.script, "100500") == 0) &&
             (templ.userId == uId) &&
             (strcmp(templ.description, "descr") == 0) &&
             (strcmp(templ.name, "newTask") == 0)); 

  templ.averDurationSec = 1;
  templ.maxDurationSec = 10;
  EXPECT_TRUE(!zmGetTaskTemplate(_zc, tId + 1, &templ) &&
             (templ.averDurationSec == 1) &&
             (templ.maxDurationSec == 10));  

  delete templ.script;   
  delete templ.description;                        
}
TEST_F(APITest, delTaskTemplate){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));
  
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId) && (tId > 0));
  
  EXPECT_TRUE(zmDelTaskTemplate(_zc, tId));   

  templ.averDurationSec = 1;
  templ.maxDurationSec = 10;
  EXPECT_TRUE(!zmGetTaskTemplate(_zc, tId, &templ) && 
              (templ.averDurationSec == 1) &&
              (templ.maxDurationSec == 10)); 

  delete templ.script;   
  delete templ.description;             
}
TEST_F(APITest, changeTaskTemplate){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123"); 
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));
  
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId) && (tId > 0));  
  
  templ.averDurationSec = 1;
  templ.maxDurationSec = 10;
  strcpy(templ.script, "100");
  templ.userId = uId; 
  strcpy(templ.description, "de");
  strcpy(templ.name, "new");
  EXPECT_TRUE(zmChangeTaskTemplate(_zc, tId, templ));   

  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  strcpy(templ.script, "1000");
  templ.userId = uId + 1; 
  strcpy(templ.description, "d00");
  strcpy(templ.name, "new00");
  EXPECT_TRUE(zmGetTaskTemplate(_zc, tId, &templ) &&
             (templ.averDurationSec == 1) &&
             (templ.maxDurationSec == 10) &&
             (strcmp(templ.script, "100") == 0) &&
             (templ.userId == uId) &&
             (strcmp(templ.description, "de") == 0) &&
             (strcmp(templ.name, "new") == 0));   

  delete templ.script;   
  delete templ.description;          
}
TEST_F(APITest, getAllTaskTemplate){
  zmUser usr;
  strcpy(usr.name, "alm1");
  strcpy(usr.passw, "123"); 
  uint64_t uId1 = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId1) && (uId1 > 0));

  strcpy(usr.name, "alm2");
  strcpy(usr.passw, "123"); 
  uint64_t uId2 = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId2) && (uId2 > 0));
  
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId1; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId1) && (tId1 > 0));  

  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  strcpy(templ.script, "100500");
  templ.userId = uId2; 
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t tId2 = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &tId2) && (tId2 > 0)); 
    
  uint64_t* pTT = nullptr;    
  auto tCnt = zmGetAllTaskTemplates(_zc, uId1, &pTT);
  EXPECT_TRUE((tCnt == 1) && (pTT[0] == tId1)); 

  tCnt = zmGetAllTaskTemplates(_zc, uId2, &pTT);
  EXPECT_TRUE((tCnt == 1) && (pTT[0] == tId2));   

  tCnt = zmGetAllTaskTemplates(_zc, uId2 + 1, &pTT);
  EXPECT_TRUE(tCnt == 0);         

  delete templ.script;   
  delete templ.description;    
}

TEST_F(APITest, addTask){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId;
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId) && (tId > 0));  
  
  task.pplId = pId + 1;  
  tId = 0;  
  EXPECT_TRUE(!zmAddTask(_zc, task, &tId) && (tId == 0));           
}
TEST_F(APITest, getTask){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "par am1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId) && (tId > 0));  

  task.pplId = pId + 1;
  task.gId = pId + 1; 
  task.priority = 2;
  task.ttId = ttId + 1;
  strcpy(task.params, "['e','paramr2','patyram3']");
  strcpy(task.nextTasksId, "[vv]");
  strcpy(task.prevTasksId, "[c]");
  EXPECT_TRUE(zmGetTask(_zc, tId, &task) &&
             (task.pplId == pId) &&
             (task.gId == 0) &&
             (task.priority == 1) &&
             (strcmp(task.params, "par am1,param2,param3") == 0) &&
             (strcmp(task.nextTasksId, "") == 0) &&
             (strcmp(task.prevTasksId, "") == 0) &&
             (task.ttId == ttId));          
}
TEST_F(APITest, changeTask){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId; 
  task.gId = 0;
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId1) && (tId1 > 0)); 

  task.pplId = pId; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId2 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId2) && (tId2 > 0)); 
  
  task.pplId = pId; 
  task.priority = 2;
  strcpy(task.params, "par amm1,paramm2,paramm3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, (to_string(tId1)).c_str());
  EXPECT_TRUE(zmChangeTask(_zc, tId2, task));

  task.pplId = pId + 1; 
  task.priority = 11;
  task.ttId = ttId + 1;
  strcpy(task.params, "par1,mm2,pam3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "123");
  EXPECT_TRUE(zmGetTask(_zc, tId2, &task) &&
             (task.pplId == pId) &&
             (task.priority == 2) &&
             (strcmp(task.params, "par amm1,paramm2,paramm3") == 0) &&
             (strcmp(task.nextTasksId, "") == 0) &&
             (strcmp(task.prevTasksId, (to_string(tId1)).c_str()) == 0) &&
             (task.ttId == ttId));
}
TEST_F(APITest, delTask){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId1) && (tId1 > 0)); 

  EXPECT_TRUE(zmDelTask(_zc, tId1));

  task.pplId = pId + 1; 
  task.priority = 2;
  task.ttId = ttId + 1;
  strcpy(task.params, "['e','paramr2','patyram3']");
  strcpy(task.nextTasksId, "[vv]");
  strcpy(task.prevTasksId, "[c]");
  EXPECT_TRUE(!zmGetTask(_zc, tId1, &task) &&
             (task.pplId == pId + 1) &&
             (task.priority == 2) &&
             (strcmp(task.params, "['e','paramr2','patyram3']") == 0) &&
             (strcmp(task.nextTasksId, "[vv]") == 0) &&
             (strcmp(task.prevTasksId, "[c]") == 0) &&
            (task.ttId == ttId + 1));             
}
TEST_F(APITest, startTask){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId1) && (tId1 > 0));  

  EXPECT_TRUE(zmStartTask(_zc, tId1));           
}
TEST_F(APITest, cancelTask){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId;
  task.gId = 0;  
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId1) && (tId1 > 0));  

  EXPECT_TRUE(zmStartTask(_zc, tId1));        

  EXPECT_TRUE(zmCancelTask(_zc, tId1));           
}
TEST_F(APITest, taskState){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId1) && (tId1 > 0));  
  
  EXPECT_TRUE(zmStartTask(_zc, tId1));  

  task.pplId = pId; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId2 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId2) && (tId2 > 0));

  EXPECT_TRUE(zmStartTask(_zc, tId2));  

  uint64_t* tIds = new uint64_t[2] {tId1, tId2};
  zmTskState* tState = new zmTskState[2];
  EXPECT_TRUE(zmTaskState(_zc, tIds, 2, tState) && 
              (tState[0].progress == 0) &&
              (tState[0].state == zmStateType::zmReady) &&
              (tState[1].progress == 0) &&
              (tState[1].state == zmStateType::zmReady));              
}
TEST_F(APITest, taskResult){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId1) && (tId1 > 0));  
  
  EXPECT_TRUE(zmStartTask(_zc, tId1));  

  char* result = nullptr;
  EXPECT_TRUE(zmTaskResult(_zc, tId1, &result)); 
}
TEST_F(APITest, taskTime){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId1) && (tId1 > 0));  
  
  EXPECT_TRUE(zmStartTask(_zc, tId1));  

  zmTskTime result;
  EXPECT_TRUE(zmTaskTime(_zc, tId1, &result));
}
TEST_F(APITest, getAllTask){
  zmUser usr;
  strcpy(usr.name, "alm");
  strcpy(usr.passw, "123");
  uint64_t uId = 0;  
  EXPECT_TRUE(zmAddUser(_zc, usr, &uId) && (uId > 0));

  zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.description = new char[24];
  strcpy(ppline.description, "dfsdf");
  ppline.userId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(zmAddPipeline(_zc, ppline, &pId) && (pId > 0)); 
    
  zmTaskTemplate templ;
  templ.averDurationSec = 10;
  templ.maxDurationSec = 100;
  templ.script = new char[256];
  strcpy(templ.script, "100500");
  templ.userId = uId; 
  templ.description = new char[256];
  strcpy(templ.description, "descr");
  strcpy(templ.name, "newTask");
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmAddTaskTemplate(_zc, templ, &ttId) && (ttId > 0)); 

  zmTask task;
  task.pplId = pId; 
  task.gId = 0; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId1) && (tId1 > 0));  
  
  EXPECT_TRUE(zmStartTask(_zc, tId1));  

  task.pplId = pId; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = new char[32];
  task.nextTasksId = new char[24];
  task.prevTasksId = new char[24];
  strcpy(task.params, "param1,param2,param3");
  strcpy(task.nextTasksId, "");
  strcpy(task.prevTasksId, "");
  uint64_t tId2 = 0;  
  EXPECT_TRUE(zmAddTask(_zc, task, &tId2) && (tId2 > 0));  
  
  EXPECT_TRUE(zmStartTask(_zc, tId2));    

  uint64_t* pTT = nullptr;
  auto tCnt = zmGetAllTasks(_zc, pId, zmStateType::zmReady, &pTT);
  EXPECT_TRUE((tCnt == 2) &&
              (pTT[0] == tId1) && 
              (pTT[1] == tId2));  

  tCnt = zmGetAllTasks(_zc, pId, zmStateType::zmUndefined, &pTT);
  EXPECT_TRUE((tCnt == 2) &&
              (pTT[0] == tId1) && 
              (pTT[1] == tId2));

  tCnt = zmGetAllTasks(_zc, pId, zmStateType::zmStart, &pTT);
  EXPECT_TRUE(tCnt == 0); 

  tCnt = zmGetAllTasks(_zc, pId + 1, zmStateType::zmReady, &pTT);
  EXPECT_TRUE(tCnt == 0); 

  tCnt = zmGetAllTasks(_zc, pId + 1, zmStateType::zmUndefined, &pTT);
  EXPECT_TRUE(tCnt == 0);  
}

#endif //APITEST