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
#include "prepareTest.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/auxFunc.h"

using namespace std;

class DBTest : public ::testing::Test {
public:
  DBTest() { 
    ZM_DB::connectCng cng;
    cng.selType = ZM_DB::dbType::PostgreSQL;
    cng.connectStr = "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10";

    _pDb = ZM_DB::makeDbProvider(cng);
    
    auto err = _pDb->getLastError();
    if (!err.empty()){    
      TEST_COUT << err << endl;     
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
  
  ZM_Base::scheduler schedr;
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
  
  ZM_Base::scheduler schedr;
  schedr.state = ZM_Base::stateType::ready;
  schedr.connectPnt = "localhost:4444";  
  uint64_t sId = 0;  
  EXPECT_TRUE(_pDb->addSchedr(schedr, sId) && (sId > 0)) << _pDb->getLastError(); 
  
  ZM_Base::worker worker;
  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444";  
  worker.exr = ZM_Base::executorType::bash;
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
  worker.exr = ZM_Base::executorType::bash;
  worker.capacityTask = 105; 
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError(); 

  worker.state = ZM_Base::stateType::error;
  worker.connectPnt = ""; 
  worker.capacityTask = 1;   
  worker.exr = ZM_Base::executorType::cmd;
  EXPECT_TRUE(_pDb->getWorker(wId, worker) && (worker.state == ZM_Base::stateType::ready) &&
                                              (worker.connectPnt == "localhost:4444") &&
                                              (worker.exr == ZM_Base::executorType::bash) &&
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
  worker.exr = ZM_Base::executorType::bash;
  worker.capacityTask = 105; 
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError(); 

  worker.state = ZM_Base::stateType::error;
  worker.connectPnt = "localhost:1234"; 
  worker.exr = ZM_Base::executorType::cmd;
  worker.capacityTask = 10;   
  EXPECT_TRUE(_pDb->changeWorker(wId, worker)) << _pDb->getLastError(); 

  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = ""; 
  worker.exr = ZM_Base::executorType::bash;
  worker.capacityTask = 1;   
  EXPECT_TRUE(_pDb->getWorker(wId, worker) && (worker.state == ZM_Base::stateType::error) &&
                                              (worker.connectPnt == "localhost:1234") &&
                                              (worker.exr == ZM_Base::executorType::cmd) &&
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
  worker.exr = ZM_Base::executorType::bash;
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
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444";  
  worker.exr = ZM_Base::executorType::bash;
  worker.capacityTask = 105; 
  uint64_t wId = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId) && (wId > 0)) << _pDb->getLastError(); 

  worker.state = ZM_Base::stateType::error;
  EXPECT_TRUE(_pDb->workerState(wId, worker.state) && 
             (worker.state == ZM_Base::stateType::ready)) << _pDb->getLastError();                                                      
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
  worker.exr = ZM_Base::executorType::bash;
  worker.capacityTask = 105; 
  uint64_t wId1 = 0;  
  EXPECT_TRUE(_pDb->addWorker(worker, wId1) && (wId1 > 0)) << _pDb->getLastError(); 

  worker.sId = sId;
  worker.state = ZM_Base::stateType::ready;
  worker.connectPnt = "localhost:4444";  
  worker.exr = ZM_Base::executorType::bash;
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
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 

  ppline.name = ""; 
  ppline.description = "";   
  EXPECT_TRUE(_pDb->getPipeline(pId, ppline) && (ppline.uId == uId) &&
                                                (ppline.name == "newPP") &&
                                                (ppline.description == "dfsdf")) << _pDb->getLastError(); 

  ppline.uId = 0;
  ppline.name = ""; 
  ppline.description = "dd";  
  EXPECT_TRUE(!_pDb->getPipeline(pId + 1, ppline) && (ppline.uId == 0) &&
                                                     (ppline.name == "") &&
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
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 

  ppline.name = "super";
  ppline.description = "localhost:1234"; 
  ppline.uId = uId;
  EXPECT_TRUE(_pDb->changePipeline(pId, ppline)) << _pDb->getLastError(); 

  ppline.name = "";
  ppline.description = "1234"; 
  ppline.uId = 0;   
  EXPECT_TRUE(_pDb->getPipeline(pId, ppline) && (ppline.name == "super") &&
                                                (ppline.description == "localhost:1234") &&
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
  uint64_t pId1 = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId1) && (pId1 > 0)) << _pDb->getLastError(); 

  ppline.name = "newPP2";
  ppline.description = "dfsdf";
  ppline.uId = uId;
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
  base.exr = ZM_Base::executorType::bash;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId; 
  templ.description = "descr";
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
  base.exr = ZM_Base::executorType::bash;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId) && (tId > 0)) << _pDb->getLastError();   
  
  base.exr = ZM_Base::executorType::cmd;
  base.averDurationSec = 1;
  base.maxDurationSec = 10;
  base.script = "1005";
  templ.uId = uId; 
  templ.description = "dscr";
  templ.name = "newTk";
  templ.base = base;
  EXPECT_TRUE(_pDb->getTaskTemplate(tId, templ) && (templ.base.exr == ZM_Base::executorType::bash) &&
                                                   (templ.base.averDurationSec == 10) &&
                                                   (templ.base.maxDurationSec == 100) &&
                                                   (templ.base.script == "100500") &&
                                                   (templ.uId == uId) &&
                                                   (templ.description == "descr") &&
                                                   (templ.name == "newTask")) << _pDb->getLastError(); 

  base.exr = ZM_Base::executorType::cmd;
  base.averDurationSec = 1;
  base.maxDurationSec = 10;  
  templ.base = base;
  EXPECT_TRUE(!_pDb->getTaskTemplate(tId + 1, templ) && (templ.base.exr == ZM_Base::executorType::cmd) &&
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
  base.exr = ZM_Base::executorType::bash;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId) && (tId > 0)) << _pDb->getLastError();   
  
  EXPECT_TRUE(_pDb->delTaskTemplate(tId)) << _pDb->getLastError();   

  base.exr = ZM_Base::executorType::cmd;
  base.averDurationSec = 1;
  base.maxDurationSec = 10;  
  templ.base = base;
  EXPECT_TRUE(!_pDb->getTaskTemplate(tId, templ) && (templ.base.exr == ZM_Base::executorType::cmd) &&
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
  base.exr = ZM_Base::executorType::bash;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId1; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId) && (tId > 0)) << _pDb->getLastError();   
  
  base.exr = ZM_Base::executorType::cmd;
  base.averDurationSec = 1;
  base.maxDurationSec = 10;
  base.script = "100";
  templ.uId = uId2; 
  templ.description = "de";
  templ.name = "new";
  templ.base = base;
  uint64_t tIdNew = 0;  
  EXPECT_TRUE(_pDb->changeTaskTemplate(tId, templ, tIdNew) && (tIdNew > 0)) << _pDb->getLastError();   

  base.exr = ZM_Base::executorType::bash;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";
  templ.uId = uId1; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  EXPECT_TRUE(_pDb->getTaskTemplate(tIdNew, templ) && (templ.base.exr == ZM_Base::executorType::cmd) &&
                                                   (templ.base.averDurationSec == 1) &&
                                                   (templ.base.maxDurationSec == 10) &&
                                                   (templ.base.script == "100") &&
                                                   (templ.uId == uId2) &&
                                                   (templ.description == "de") &&
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
  base.exr = ZM_Base::executorType::bash;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId1; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  uint64_t tId1 = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, tId1) && (tId1 > 0)) << _pDb->getLastError();  

  base.exr = ZM_Base::executorType::bash;
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
    
  ZM_Base::user usr;
  usr.name = "usr";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  ZM_Base::uPipeline ppline;
  ppline.name = "newPP";
  ppline.description = "dfsdf";
  ppline.uId = uId;
  uint64_t pId = 0;  
  EXPECT_TRUE(_pDb->addPipeline(ppline, pId) && (pId > 0)) << _pDb->getLastError(); 
    
  ZM_Base::task base;
  base.exr = ZM_Base::executorType::bash;
  base.averDurationSec = 10;
  base.maxDurationSec = 100;
  base.script = "100500";

  ZM_Base::uTaskTemplate templ;
  templ.uId = uId; 
  templ.description = "descr";
  templ.name = "newTask";
  templ.base = base;
  uint64_t ttId = 0;  
  EXPECT_TRUE(_pDb->addTaskTemplate(templ, ttId) && (ttId > 0)) << _pDb->getLastError(); 

  ZM_Base::uTask task;
  task.pplId = pId; 
  task.base.priority = 1;
  task.base.tId = ttId;
  task.base.params = "params";
  task.rct = ZM_Base::uScreenRect{1, 2, 3, 4};
  task.nextTasks = std::vector<uint64_t>{1,2,3,4};
  task.prevTasks = std::vector<uint64_t>{4,3,2,1};
  uint64_t tId = 0;  
  EXPECT_TRUE(_pDb->addTask(task, tId) && (tId > 0)) << _pDb->getLastError();   

  task.pplId = pId + 1;  
  tId = 0;  
  EXPECT_TRUE(!_pDb->addTask(task, tId) && (tId == 0)) << _pDb->getLastError();           
}