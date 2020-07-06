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
//#define PGTEST
#ifdef PGTEST

#include <vector>
#include <algorithm>
#include "prepareTest.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/auxFunc.h"

using namespace std;
bool isTables = false;
class DBTest : public ::testing::Test {
public:
  DBTest() { 
    ZM_DB::connectCng cng;
    cng.selType = ZM_DB::dbType::PostgreSQL;
    cng.connectStr = "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10";

    _pDb = ZM_DB::makeDbProvider(cng);
    
    if (!isTables){
      isTables = true;
      _pDb->createTables();
    }

    auto err = _pDb->getLastError();
    if (!err.empty()){    
      TEST_COUT << err << endl;
      bool ff = false;  
    }    
  }
  ~DBTest() {
    if (_pDb){
      delete _pDb;
    }
  }
protected:
  ZM_DB::DbProvider* _pDb = nullptr; 
};

TEST_F(DBTest, addUser){
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();

  ZM_Base::user usr;
  usr.name = "";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(!_pDb->addUser(usr, uId) && (uId == 0)) << _pDb->getLastError();

  usr.name = "alm1";
  usr.passw = "";  
  uId = 0;
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  usr.name = "alm1";
  usr.passw = "";
  uId = 0;
  EXPECT_TRUE(!_pDb->addUser(usr, uId) && (uId == 0)) << _pDb->getLastError();

  usr.name = "alm2";
  usr.passw = "123";
  uint64_t uId2 = 0;
  EXPECT_TRUE(_pDb->addUser(usr, uId2) && (uId2 > 0)) << _pDb->getLastError();
  
  EXPECT_TRUE(uId2 > uId) << _pDb->getLastError();
}
TEST_F(DBTest, getUser){ 
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "alm";
  usr.passw = "123";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  uId = 0;  
  EXPECT_TRUE(!_pDb->getUserId("", "", uId) && (uId == 0)) << _pDb->getLastError();

  uId = 0;  
  EXPECT_TRUE(!_pDb->getUserId("alm", "", uId) && (uId == 0)) << _pDb->getLastError();

  uId = 0;  
  EXPECT_TRUE(!_pDb->getUserId("alm1", "123", uId) && (uId == 0)) << _pDb->getLastError();

  uId = 0;  
  EXPECT_TRUE(_pDb->getUserId("alm", "123", uId) && (uId > 0)) << _pDb->getLastError();

  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();

  uId = 0;  
  EXPECT_TRUE(!_pDb->getUserId("alm", "123", uId) && (uId == 0)) << _pDb->getLastError();

  /////////////////////////

  uId = 0;  
  usr.name = "alm";
  usr.passw = "123";  
  usr.description = "abc";  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  EXPECT_TRUE(_pDb->getUserCng(uId, usr) &&
                           (usr.name == "alm") &&
                           (usr.description == "abc")) << _pDb->getLastError();
}
TEST_F(DBTest, changeUser){
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "alm";
  usr.passw = "123";  
  usr.description = "abc"; 
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  usr.name = "mla";
  usr.passw = "321";  
  usr.description = "cba"; 
  EXPECT_TRUE(_pDb->changeUser(uId, usr)) << _pDb->getLastError();

  usr.name = "";
  usr.passw = "";  
  usr.description = ""; 
  EXPECT_TRUE(_pDb->getUserCng(uId, usr) &&
                           (usr.name == "mla") &&
                           (usr.description == "cba")) << _pDb->getLastError();                            
}
TEST_F(DBTest, delUser){
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "alm";
  usr.passw = "123";  
  usr.description = "abc"; 
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();
    
  EXPECT_TRUE(_pDb->delUser(uId)) << _pDb->getLastError();
  
  EXPECT_TRUE(!_pDb->getUserCng(uId, usr)) << _pDb->getLastError();  

  uId = 0;
  EXPECT_TRUE(!_pDb->getUserId(usr.name, usr.passw, uId) && (uId == 0)) << _pDb->getLastError();                            
}
TEST_F(DBTest, getAllUsers){
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  auto users = _pDb->getAllUsers();
  EXPECT_TRUE(users.empty()) << _pDb->getLastError();   

  ZM_Base::user usr;
  usr.name = "alm";
  usr.passw = "123";  
  usr.description = "abc"; 
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  usr.name = "alm1";
  usr.passw = "1234";  
  usr.description = "abcd"; 
  uint64_t uId1 = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId1) && (uId1 > 0)) << _pDb->getLastError();
        
  users = _pDb->getAllUsers();
  EXPECT_TRUE((users.size() == 2) && (users[0] == uId) && (users[1] == uId1)) << _pDb->getLastError();                        
}

TEST_F(DBTest, addSchedr){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  
  ZM_Base::scheduler schedr{0};
  schedr.capacityTask = 10000;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444";  
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost4444";  
  sId = 0;  
  EXPECT_TRUE(!_pDb->addSchedr(schedr, sId) && (sId == 0)) << _pDb->getLastError();               
}
TEST_F(DBTest, getSchedr){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::stateType::error;
  schedr.connectPnt = ""; 
  schedr.capacityTask = 1;   
  EXPECT_TRUE(_pDb->getSchedr(sId, schedr) && (schedr.state == ZM_Base::stateType::ready) &&
                                              (schedr.connectPnt == "localhost:4444") &&
                                              (schedr.capacityTask == 105)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::stateType::error;
  schedr.connectPnt = ""; 
  schedr.capacityTask = 1;   
  EXPECT_TRUE(!_pDb->getSchedr(sId + 1, schedr) && (schedr.state == ZM_Base::stateType::error) &&
                                              (schedr.connectPnt == "") &&
                                              (schedr.capacityTask == 1)) << _pDb->getLastError();                                                      
}
TEST_F(DBTest, changeSchedr){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::stateType::error;
  schedr.connectPnt = "localhost:1234"; 
  schedr.capacityTask = 10;   
  EXPECT_TRUE(_pDb->changeSchedr(sId, schedr)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = ""; 
  schedr.capacityTask = 1;   
  EXPECT_TRUE(_pDb->getSchedr(sId, schedr) && (schedr.state == ZM_Base::stateType::error) &&
                                              (schedr.connectPnt == "localhost:1234") &&
                                              (schedr.capacityTask == 10)) << _pDb->getLastError();                                                      
}
TEST_F(DBTest, delSchedr){
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 
    
  EXPECT_TRUE(_pDb->delSchedr(sId)) << _pDb->getLastError();
  
  EXPECT_TRUE(!_pDb->getSchedr(sId, schedr)) << _pDb->getLastError();  
}
TEST_F(DBTest, schedrState){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::stateType::error;
  EXPECT_TRUE(_pDb->schedrState(sId, schedr.state) && 
             (schedr.state == ZM_Base::stateType::ready)) << _pDb->getLastError();                                                      
}
TEST_F(DBTest, getAllSchedrs){

  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  
  auto scheds = _pDb->getAllSchedrs(ZM_Base::stateType::undefined);
  EXPECT_TRUE(scheds.empty()) << _pDb->getLastError();   

  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId1 = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId1) && (sId1 > 0)) << _pDb->getLastError(); 

  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:5555"; 
  schedr.capacityTask = 105; 
  uint64_t sId2 = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId2) && (sId2 > 0)) << _pDb->getLastError(); 

  scheds.clear();      
  scheds = _pDb->getAllSchedrs(ZM_Base::stateType::ready);
  EXPECT_TRUE((scheds.size() == 2) && (scheds[0] == sId1) && (scheds[1] == sId2)) << _pDb->getLastError(); 

  scheds.clear();
  scheds = _pDb->getAllSchedrs(ZM_Base::stateType::undefined);
  EXPECT_TRUE((scheds.size() == 2) && (scheds[0] == sId1) && (scheds[1] == sId2)) << _pDb->getLastError();   

  scheds.clear();
  scheds = _pDb->getAllSchedrs(ZM_Base::stateType::start);
  EXPECT_TRUE(scheds.size() == 0) << _pDb->getLastError();                     
}

TEST_F(DBTest, addWorker){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
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
  worker.connectPnt = "localhost:4445"; 
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError();   

  worker.sId = sId + 1;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444";  
  wId = 0;  
  EXPECT_TRUE(!_pDb->addWorker(worker, wId) && (wId == 0)) << _pDb->getLastError();           
}
TEST_F(DBTest, getWorker){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::worker worker;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444"; 
  worker.capacityTask = 105; 
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError(); 

  worker.state = ZM_Base::stateType::error;
  worker.connectPnt = ""; 
  worker.capacityTask = 1;
  EXPECT_TRUE(_pDb->getWorker(wId, worker) && (worker.state == ZM_Base::stateType::ready) &&
                                              (worker.connectPnt == "localhost:4444") &&
                                              (worker.capacityTask == 105)) << _pDb->getLastError(); 

  worker.state = ZM_Base::stateType::error;
  worker.connectPnt = ""; 
  worker.capacityTask = 1;   
  EXPECT_TRUE(!_pDb->getWorker(wId + 1, worker) && (worker.state == ZM_Base::stateType::error) &&
                                                   (worker.connectPnt == "") &&
                                                   (worker.capacityTask == 1)) << _pDb->getLastError();                                                      
}
TEST_F(DBTest, changeWorker){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::worker worker;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444";  
  worker.capacityTask = 105; 
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError(); 

  worker.state = ZM_Base::stateType::error;
  worker.connectPnt = "localhost:1234"; 
  worker.capacityTask = 10;   
  EXPECT_TRUE(_pDb->changeWorker(wId, worker)) << _pDb->getLastError(); 

  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = ""; 
  worker.capacityTask = 1;   
  EXPECT_TRUE(_pDb->getWorker(wId, worker) && (worker.state == ZM_Base::stateType::error) &&
                                              (worker.connectPnt == "localhost:1234") &&
                                              (worker.capacityTask == 10)) << _pDb->getLastError(); 

  worker.sId = sId + 1;
  EXPECT_TRUE(!_pDb->changeWorker(wId, worker)) << _pDb->getLastError();
}
TEST_F(DBTest, delWorker){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::worker worker;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444";  
  worker.capacityTask = 105; 
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError(); 
    
  EXPECT_TRUE(_pDb->delWorker(wId)) << _pDb->getLastError();
  
  EXPECT_TRUE(!_pDb->getWorker(wId, worker)) << _pDb->getLastError();  
}
TEST_F(DBTest, workerState){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::worker worker;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::pause;
  worker.connectPnt = "localhost:4444";  
  worker.capacityTask = 105; 
  uint64_t wId1 = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId1) && (wId1 > 0)) << _pDb->getLastError(); 

  worker.sId = sId;
  worker.state = ZM_Base::stateType::start;
  worker.connectPnt = "localhost:4444"; 
  worker.capacityTask = 105; 
  uint64_t wId2 = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId2) && (wId2 > 0)) << _pDb->getLastError(); 

  vector<ZM_Base::stateType> wstate;
  EXPECT_TRUE(_pDb->workerState(vector<uint64_t>{wId1, wId2}, wstate) &&
              (wstate[0] == ZM_Base::stateType::pause) && 
              (wstate[1] == ZM_Base::stateType::start)) << _pDb->getLastError();                                                      
}
TEST_F(DBTest, getAllWorkers){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
  auto workers = _pDb->getAllWorkers(0, ZM_Base::stateType::undefined);
  EXPECT_TRUE(workers.empty()) << _pDb->getLastError();   

  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444"; 
  schedr.capacityTask = 105; 
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 

  ZM_Base::worker worker;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444"; 
  worker.capacityTask = 105; 
  uint64_t wId1 = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId1) && (wId1 > 0)) << _pDb->getLastError(); 

  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444";  
  worker.capacityTask = 105; 
  uint64_t wId2 = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId2) && (wId2 > 0)) << _pDb->getLastError(); 
    
  workers.clear();      
  workers = _pDb->getAllWorkers(sId, ZM_Base::stateType::ready);
  EXPECT_TRUE((workers.size() == 2) && (workers[0] == wId1) && (workers[1] == wId2)) << _pDb->getLastError(); 

  workers.clear();
  workers = _pDb->getAllWorkers(sId, ZM_Base::stateType::undefined);
  EXPECT_TRUE((workers.size() == 2) && (workers[0] == wId1) && (workers[1] == wId2)) << _pDb->getLastError();   

  workers.clear();
  workers = _pDb->getAllWorkers(sId, ZM_Base::stateType::start);
  EXPECT_TRUE(workers.size() == 0) << _pDb->getLastError();        

  workers.clear();
  workers = _pDb->getAllWorkers(sId + 1, ZM_Base::stateType::undefined);
  EXPECT_TRUE(workers.size() == 0) << _pDb->getLastError();                
}

TEST_F(DBTest, addPipeline){
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.isShared = 0;
  ppline.uId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 

  ppline.name = "";
  pId = 0;  
  EXPECT_TRUE(!_pDb->addPipeline(ppline, pId) && (pId == 0)) << _pDb->getLastError(); 

  ppline.uId = uId + 1;
  pId = 0;  
  EXPECT_TRUE(!_pDb->addPipeline(ppline, pId) && (pId == 0)) << _pDb->getLastError();               
}
TEST_F(DBTest, getPipeline){
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  ppline.isShared = 0;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 

  ppline.name = ""; 
  ppline.description = ""; 
  ppline.isShared = 1;  
  EXPECT_TRUE(_pDb->getPipeline(pId, ppline) && (ppline.uId == uId) &&
                                                (ppline.name == "newPP") &&
                                                (ppline.isShared == 0) &&
                                                (ppline.description == "dfsdf")) << _pDb->getLastError(); 

  ppline.uId = 0;
  ppline.name = ""; 
  ppline.description = "dd";  
   ppline.isShared = 1; 
  EXPECT_TRUE(!_pDb->getPipeline(pId + 1, ppline) && (ppline.uId == 0) &&
                                                     (ppline.name == "") &&
                                                     (ppline.isShared == 1) &&
                                                     (ppline.description == "dd")) << _pDb->getLastError();                                                   
}
TEST_F(DBTest, changePipeline){
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  ppline.isShared = 0;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 

  ppline.name = "super";
  ppline.description = "localhost:1234"; 
  ppline.uId = uId;
  ppline.isShared = 1;
  EXPECT_TRUE(_pDb->changePipeline(pId, ppline)) << _pDb->getLastError(); 

  ppline.name = "";
  ppline.description = "1234"; 
  ppline.uId = 0;   
  ppline.isShared = 0;
  EXPECT_TRUE(_pDb->getPipeline(pId, ppline) && (ppline.name == "super") &&
                                                (ppline.description == "localhost:1234") &&
                                                (ppline.isShared == 1) &&
                                                (ppline.uId == uId)) << _pDb->getLastError();  
                                                
  ppline.uId = uId + 1;
  EXPECT_TRUE(!_pDb->changePipeline(pId, ppline)) << _pDb->getLastError();                                                                                           
}
TEST_F(DBTest, delPipeline){
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  ppline.isShared = 0;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 
    
  EXPECT_TRUE(_pDb->delPipeline(pId)) << _pDb->getLastError();
  
  EXPECT_TRUE(!_pDb->getPipeline(pId, ppline)) << _pDb->getLastError();  
}
TEST_F(DBTest, getAllPipelines){
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  auto pplines = _pDb->getAllPipelines(0);
  EXPECT_TRUE(pplines.empty()) << _pDb->getLastError();   

  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  ppline.isShared = 0;
  uint64_t pId1 = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId1) && (pId1 > 0)) << _pDb->getLastError(); 

  ppline.name = "newPP2";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  ppline.isShared = 0;
  uint64_t pId2 = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId2) && (pId2 > 0)) << _pDb->getLastError(); 

  pplines.clear();      
  pplines = _pDb->getAllPipelines(uId);
  EXPECT_TRUE((pplines.size() == 2) && (pplines[0] == pId1) && (pplines[1] == pId2)) << _pDb->getLastError(); 

  pplines.clear();
  pplines = _pDb->getAllPipelines(uId + 1);
  EXPECT_TRUE(pplines.size() == 0) << _pDb->getLastError();                     
}

TEST_F(DBTest, addTaskTemplate){
  EXPECT_TRUE(_pDb->delAllTemplateTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
    
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();
  
  ZM_Base::task base;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId; 
  templ.description = "descr";
  templ.isShared = 0;
  templ.name = "newTask";
  templ.base = base;
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId) && (tId > 0)) << _pDb->getLastError();   

  templ.uId = uId + 1;  
  tId = 0;  
  EXPECT_TRUE(!_pDb->addTaskTemplate(templ, tId) && (tId == 0)) << _pDb->getLastError();           
}
TEST_F(DBTest, getTaskTemplate){
  EXPECT_TRUE(_pDb->delAllTemplateTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
    
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();
  
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
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId) && (tId > 0)) << _pDb->getLastError();   
  
  base.averDurationSec = 1;
  base.maxDurationSec = 10;
  base.script = "1005";
  templ.uId = uId; 
  templ.description = "dscr";
  templ.name = "newTk";
  templ.base = base;
  templ.isShared = 1;
  EXPECT_TRUE(_pDb->getTaskTemplate(tId, templ) && (templ.base.averDurationSec == 10) &&
                                                   (templ.base.maxDurationSec == 100) &&
                                                   (templ.base.script == "100500") &&
                                                   (templ.uId == uId) &&
                                                   (templ.isShared == 0) &&
                                                   (templ.description == "descr") &&
                                                   (templ.name == "newTask")) << _pDb->getLastError(); 

  base.averDurationSec = 1;
  base.maxDurationSec = 10;  
  templ.base = base;
  EXPECT_TRUE(!_pDb->getTaskTemplate(tId + 1, templ) &&
              (templ.base.averDurationSec == 1) &&
              (templ.base.maxDurationSec == 10)) << _pDb->getLastError();           
}
TEST_F(DBTest, delTaskTemplate){
  EXPECT_TRUE(_pDb->delAllTemplateTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
    
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();
  
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
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId) && (tId > 0)) << _pDb->getLastError();   
  
  EXPECT_TRUE(_pDb->delTaskTemplate(tId)) << _pDb->getLastError();   

  base.averDurationSec = 1;
  base.maxDurationSec = 10;  
  templ.base = base;
  EXPECT_TRUE(!_pDb->getTaskTemplate(tId, templ) && 
              (templ.base.averDurationSec == 1) &&
              (templ.base.maxDurationSec == 10)) << _pDb->getLastError();           
}
TEST_F(DBTest, changeTaskTemplate){
  EXPECT_TRUE(_pDb->delAllTemplateTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
    
  ZM_Base::user usr;
  usr.name = "usr1";
  usr.passw = "";  
  uint64_t uId1 = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId1) && (uId1 > 0)) << _pDb->getLastError();

  usr.name = "usr2";
  usr.passw = "";  
  uint64_t uId2 = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId2) && (uId2 > 0)) << _pDb->getLastError();
  
  ZM_Base::task base;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId1; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  templ.isShared = 0;
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId) && (tId > 0)) << _pDb->getLastError();   
  
  base.averDurationSec = 1;
  base.maxDurationSec = 10;
  base.script = "100";
  templ.uId = uId2; 
  templ.description = "de";
  templ.name = "new";
  templ.base = base;
  templ.isShared = 1;
  uint64_t tIdNew = 0;  
  EXPECT_TRUE(_pDb->changeTaskTemplate(tId, templ, tIdNew) && (tIdNew > 0)) << _pDb->getLastError();   

  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";
  templ.uId = uId1; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.isShared = 0;
  templ.base = base;
  EXPECT_TRUE(_pDb->getTaskTemplate(tIdNew, templ) && 
             (templ.base.averDurationSec == 1) &&
             (templ.base.maxDurationSec == 10) &&
             (templ.base.script == "100") &&
             (templ.uId == uId2) &&
             (templ.description == "de") &&
             (templ.isShared == 1) &&
             (templ.name == "new")) << _pDb->getLastError();           
}
TEST_F(DBTest, getAllTaskTemplate){
  EXPECT_TRUE(_pDb->delAllTemplateTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
    
  auto ttempl = _pDb->getAllTaskTemplates(0);
  EXPECT_TRUE(ttempl.empty()) << _pDb->getLastError();   

  ZM_Base::user usr;
  usr.name = "usr1";
  usr.passw = "";  
  uint64_t uId1 = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId1) && (uId1 > 0)) << _pDb->getLastError();

  usr.name = "usr2";
  usr.passw = "";  
  uint64_t uId2 = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId2) && (uId2 > 0)) << _pDb->getLastError();
  
  ZM_Base::task base;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId1; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  templ.isShared = 0;
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";
  templ.uId = uId2; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId2) && (tId2 > 0)) << _pDb->getLastError(); 
    
  ttempl.clear();      
  ttempl = _pDb->getAllTaskTemplates(uId1);
  EXPECT_TRUE((ttempl.size() == 1) && (ttempl[0] == tId1)) << _pDb->getLastError(); 

  ttempl.clear();
  ttempl = _pDb->getAllTaskTemplates(uId2);
  EXPECT_TRUE((ttempl.size() == 1) && (ttempl[0] == tId2)) << _pDb->getLastError();   

  ttempl.clear();
  ttempl = _pDb->getAllTaskTemplates(uId2 + 1);
  EXPECT_TRUE(ttempl.size() == 0) << _pDb->getLastError();         
}

TEST_F(DBTest, addTask){
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
 
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

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
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId) && (tId > 0)) << _pDb->getLastError();  
  
  task.pplId = pId + 1;  
  tId = 0;  
  EXPECT_TRUE(!_pDb->addTask(task, tId) && (tId == 0)) << _pDb->getLastError();           
}
TEST_F(DBTest, getTask){
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

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
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[" + to_string(tId1) + "]";
  task.base.result = "resut";
  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId2) && (tId2 > 0)) << _pDb->getLastError();  

  task.pplId = pId + 1; 
  task.base.priority = 2;
  task.base.tId = ttId + 1;
  task.base.params = "paramsddd";
  task.screenRect = "sdfsd fg";
  task.nextTasks = "[1,2]";
  task.prevTasks.clear();
  EXPECT_TRUE(_pDb->getTask(tId2, task) && (task.pplId == pId) &&
                                          (task.base.priority == 1) &&
                                          (task.base.params == "['param1','param2','param3']") &&
                                          (task.nextTasks == "[]") &&
                                          (task.prevTasks == "[" + to_string(tId1) + "]") &&
                                          (task.screenRect == "1, 2, 3, 4") &&
                                          (task.base.tId == ttId)) << _pDb->getLastError();   

  bool dd = false;                                                
}
TEST_F(DBTest, changeTask){
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP1";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  ppline.isShared = 0;
  uint64_t pId1 = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId1) && (pId1 > 0)) << _pDb->getLastError(); 

  ppline.name = "newPP2";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  uint64_t pId2 = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId2) && (pId2 > 0)) << _pDb->getLastError(); 
    
  ZM_Base::task base;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId; 
  templ.description = "descr";
  templ.name = "newTask1";
  templ.base = base;
  templ.isShared = 0;
  uint64_t ttId1 = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, ttId1) && (ttId1 > 0)) << _pDb->getLastError();

  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  templ.uId = uId; 
  templ.description = "descr";
  templ.name = "newTask2";
  templ.base = base;
  uint64_t ttId2 = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, ttId2) && (ttId2 > 0)) << _pDb->getLastError(); 

  ZM_Base::uTask task;
  task.pplId = pId1; 
  task.base.priority = 1;
  task.base.tId = ttId1;
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId1) && (tId1 > 0)) << _pDb->getLastError();   

  task.pplId = pId1; 
  task.base.priority = 1;
  task.base.tId = ttId1;
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId2) && (tId2 > 0)) << _pDb->getLastError();   

  task.pplId = pId2; 
  task.base.priority = 2;
  task.base.tId = ttId2;
  task.base.params = "['paramm1','paramm2','paramm3']";
  task.screenRect = "1, 2, 2, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[" + to_string(tId1) + "]";
  EXPECT_TRUE(_pDb->changeTask(tId2, task)) << _pDb->getLastError();

  task.pplId = pId1; 
  task.base.priority = 1;
  task.base.tId = ttId1;
  task.base.params = "paramsddd";
  task.screenRect = "sdfsd fg";
  task.nextTasks = "[11,22]";
  task.prevTasks.clear();
  EXPECT_TRUE(_pDb->getTask(tId2, task) && (task.pplId == pId2) &&
                                          (task.base.priority == 2) &&
                                          (task.base.params == "['paramm1','paramm2','paramm3']") &&
                                          (task.nextTasks == "[]") &&
                                          (task.prevTasks == "[" + to_string(tId1) + "]") &&
                                          (task.screenRect == "1, 2, 2, 4") &&
                                          (task.base.tId == ttId2)) << _pDb->getLastError();
}
TEST_F(DBTest, delTask){
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

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
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId) && (tId > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->delTask(tId)) << _pDb->getLastError();

  task.pplId = pId + 1; 
  task.base.priority = 2;
  task.base.tId = ttId + 1;
  task.base.params = "paramsddd";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[1, 2, 3]";
  task.prevTasks = "[4, 5, 6]";
  EXPECT_TRUE(!_pDb->getTask(tId, task) && (task.pplId == pId + 1) &&
                                          (task.base.priority == 2) &&
                                          (task.base.tId == ttId + 1) &&
                                          (task.base.params == "paramsddd") &&
                                          (task.screenRect == "1, 2, 3, 4") &&
                                          (task.nextTasks == "[1, 2, 3]") &&
                                          (task.prevTasks == "[4, 5, 6]")) << _pDb->getLastError();             
}
TEST_F(DBTest, startTask){
   EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

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
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId) && (tId > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId)) << _pDb->getLastError();           
}
TEST_F(DBTest, taskState){
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

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
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId1)) << _pDb->getLastError(); 

  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId2) && (tId2 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId2)) << _pDb->getLastError();  

  vector<uint64_t> tIds {tId1, tId2};
  vector<ZM_DB::taskPrsAState> tState;
  EXPECT_TRUE(_pDb->taskState(tIds, tState) && 
              (tState[0].progress == 0) &&
              (tState[0].state == ZM_Base::stateType::ready) &&
              (tState[1].progress == 0) &&
              (tState[1].state == ZM_Base::stateType::ready)) << _pDb->getLastError();              
}
TEST_F(DBTest, taskResult){
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

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
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId1)) << _pDb->getLastError(); 

  string result;
  EXPECT_TRUE(_pDb->taskResult(tId1, result)) << _pDb->getLastError(); 
}
TEST_F(DBTest, taskTime){
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

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
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId1)) << _pDb->getLastError(); 

  ZM_DB::taskTime result;
  EXPECT_TRUE(_pDb->taskTime(tId1, result)) << _pDb->getLastError();
}
TEST_F(DBTest, getAllTask){
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

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
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  task.base.result = "resut";
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId1)) << _pDb->getLastError(); 

  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "['param1','param2','param3']";
  task.screenRect = "1, 2, 3, 4";
  task.nextTasks = "[]";
  task.prevTasks = "[]";
  uint64_t tId2 = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId2) && (tId2 > 0)) << _pDb->getLastError();  

  EXPECT_TRUE(_pDb->startTask(tId2)) << _pDb->getLastError();  

  auto allTask = _pDb->getAllTasks(pId, ZM_Base::stateType::ready);
  EXPECT_TRUE((allTask.size() == 2) &&
              (allTask[0] == tId1) && 
              (allTask[1] == tId2)) << _pDb->getLastError();  

  allTask = _pDb->getAllTasks(pId, ZM_Base::stateType::undefined);
  EXPECT_TRUE((allTask.size() == 2) &&
              (allTask[0] == tId1) && 
              (allTask[1] == tId2)) << _pDb->getLastError();

  allTask = _pDb->getAllTasks(pId, ZM_Base::stateType::start);
  EXPECT_TRUE(allTask.size() == 0) << _pDb->getLastError(); 

  allTask = _pDb->getAllTasks(pId + 1, ZM_Base::stateType::ready);
  EXPECT_TRUE(allTask.size() == 0) << _pDb->getLastError(); 

  allTask = _pDb->getAllTasks(pId + 1, ZM_Base::stateType::undefined);
  EXPECT_TRUE(allTask.size() == 0) << _pDb->getLastError();  
}

TEST_F(DBTest, getSchedrByCP){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  
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
TEST_F(DBTest, getTaskOfSchedr){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
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
              (tasks[0].params == "{param11,param12,param13}") &&
              (tasks[0].base.id == ttId)) << _pDb->getLastError();

  tasks.clear();
  EXPECT_TRUE(_pDb->getTasksOfSchedr(sId, tasks) && 
             (tasks.size() == 1) &&
             (tasks[0].params == "{param11,param12,param13}") &&
             (tasks[0].base.id == ttId)) << _pDb->getLastError(); 

  vector<ZM_DB::messSchedr> mess;
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskRunning, wId, tasks[0].qTaskId, 0, 10, "result"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  mess.clear();
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskCompleted, wId, tasks[0].qTaskId, 0, 0, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();

  tasks.clear();
  EXPECT_TRUE(_pDb->getNewTasksForSchedr(sId, 10, tasks) && 
              (tasks.size() == 1) && 
              (tasks[0].params == "{param21,param22,param23,result1}") &&
              (tasks[0].base.id == ttId)) << _pDb->getLastError(); 

  tasks.clear();
  EXPECT_TRUE(_pDb->getTasksOfSchedr(sId, tasks) && 
             (tasks.size() == 1) &&
             (tasks[0].params == "{param21,param22,param23,result1}") &&
             (tasks[0].base.id == ttId)) << _pDb->getLastError();          
}
TEST_F(DBTest, getWorkerOfSchedr){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
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
TEST_F(DBTest, getNewTasksForSchedr){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
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
              (tasks[0].params == "{param11,param12,param13}") &&
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
              (tasks[0].params == "{param21,param22,param23,result1}") &&
              (tasks[0].base.id == ttId)) << _pDb->getLastError();    
}
TEST_F(DBTest, getSchedrAndWorkerByTask){
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  
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
              (tasks[0].params == "{param11,param12,param13}") &&
              (tasks[0].base.id == ttId)) << _pDb->getLastError();

  vector<ZM_DB::messSchedr> mess;
  mess.push_back(ZM_DB::messSchedr{ZM_Base::messType::taskStart, wId, tasks[0].qTaskId, 0, 0, "result1"});
  EXPECT_TRUE(_pDb->sendAllMessFromSchedr(sId, mess)) << _pDb->getLastError();
  
  tasks.clear();
  uint64_t qId = 0;
  EXPECT_TRUE(_pDb->getSchedrAndWorkerByTask(tId1, qId, schedr, worker) && 
              (schedr.id == sId) && 
              (worker.id == wId)) << _pDb->getLastError();      
}
TEST_F(DBTest, sendAllMessFromSchedr){
  EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
 
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
  worker.connectPnt = "localhost:4444";
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

#endif //PGTEST