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
#define ZMEYTEST
#ifdef ZMEYTEST

#include <vector>
#include <algorithm>
#include "prepareTest.h"
#include "zmey/zmey.h"
#include "zmCommon/auxFunc.h"
#include "zmDbProvider/dbProvider.h"

using namespace std;

bool isSchedrAndWorker = false;
class ZmeyTest : public ::testing::Test {
public:
  ZmeyTest() { 
    
    string connStr = "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10";
    char err[256];
    _zc = zmey::zmCreateConnection(zmey::zmConnect{ zmey::zmDbType::zmPostgreSQL,
                                                    (char*)connStr.c_str() },
                                                   err);
    if (strlen(err) > 0){    
      TEST_COUT << err << endl;
    }   
    EXPECT_TRUE(zmey::zmCreateTables(_zc));
    
    zmey::zmSetErrorCBack(_zc, [](const char* mess, zmey::zmUData){
      TEST_COUT << mess << endl;
    }, nullptr);    

    ZM_DB::connectCng cng;
    cng.selType = ZM_DB::dbType::PostgreSQL;
    cng.connectStr = "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10";
    _pDb = ZM_DB::makeDbProvider(cng);
    
    EXPECT_TRUE(_pDb->delAllTask()) << _pDb->getLastError();
    EXPECT_TRUE(_pDb->delAllPipelines()) << _pDb->getLastError();
    EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
    EXPECT_TRUE(_pDb->delAllSchedrs()) << _pDb->getLastError();
    EXPECT_TRUE(_pDb->delAllWorkers()) << _pDb->getLastError();
  }
  ~ZmeyTest() {
    zmey::zmDisconnect(_zc);
  }
protected:
  ZM_DB::DbProvider* _pDb = nullptr;
  zmey::zmConn _zc = nullptr; 
};

TEST_F(ZmeyTest, addTask){

  zmey::zmSchedr scng;
  scng.capacityTask = 10000;
  strcpy(scng.connectPnt, "localhost:4444");
  uint64_t sId = 0;
  EXPECT_TRUE(zmey::zmAddScheduler(_zc, scng, &sId));
 
  zmey::zmWorker wcng;
  wcng.capacityTask = 10;
  wcng.sId = sId;
  strcpy(wcng.connectPnt, "localhost:4445");
  uint64_t wId = 0;
  EXPECT_TRUE(zmey::zmAddWorker(_zc, wcng, &wId));

  zmey::zmUser usr;
  strcpy(usr.name, "usr");
  strcpy(usr.passw, "123");
  usr.description = nullptr;  
  uint64_t uId = 0;  
  EXPECT_TRUE(zmey::zmAddUser(_zc, usr, &uId) && (uId > 0));
    
  zmey::zmPipeline ppline;
  strcpy(ppline.name, "newPP");
  ppline.isShared = 0;
  ppline.userId = uId;
  ppline.description = nullptr;
  uint64_t ppId = 0;  
  EXPECT_TRUE(zmey::zmAddPipeline(_zc, ppline, &ppId) && (ppId > 0)); 
   
  
  zmey::zmTaskTemplate ttempl;
  strcpy(ttempl.name, "ttempl");
  ttempl.isShared = 0;
  ttempl.userId = uId;
  ttempl.description = nullptr;
  ttempl.averDurationSec = 10;
  ttempl.maxDurationSec = 100;
  ttempl.script = "#!/bin/sh \n echo $1 $2 $3 $4;";
    
  uint64_t ttId = 0;  
  EXPECT_TRUE(zmey::zmAddTaskTemplate(_zc, ttempl, &ttId) && (ttId > 0)); 
   
  zmey::zmTask task;
  task.pplId = ppId; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = "['par am11','param12','param13']";
  task.screenRect = "11, 12, 13, 14";
  task.nextTasksId = "[]";
  task.prevTasksId = "[]";
  
  uint64_t tId1 = 0;  
  EXPECT_TRUE(zmey::zmAddTask(_zc, task, &tId1) && (tId1 > 0));  
  
  task.pplId = ppId; 
  task.priority = 1;
  task.ttId = ttId;
  task.params = "['param21','param22','param23']";
  task.screenRect = "21, 22, 23, 24";
  task.nextTasksId = "[]";
  task.prevTasksId = (char*)("[" + to_string(tId1) + "]").c_str();
  
  uint64_t tId2 = 0;  
  EXPECT_TRUE(zmey::zmAddTask(_zc, task, &tId2) && (tId2 > 0));  
  
  EXPECT_TRUE(zmey::zmStartTask(_zc, tId1));     

  EXPECT_TRUE(zmey::zmStartTask(_zc, tId2));  
}

#endif // ZMEYTEST