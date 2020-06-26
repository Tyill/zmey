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
#include "prepareTest.h"
#include "zmey/zmey.h"
#include "zmCommon/auxFunc.h"
#include "zmDbProvider/dbProvider.h"

using namespace std;

bool tonc = true;
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
    if (!tonc){
      tonc = true;
      ZM_DB::connectCng cng;
      cng.selType = ZM_DB::dbType::PostgreSQL;
      cng.connectStr = "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10";
      ZM_DB::DbProvider* pDb = ZM_DB::makeDbProvider(cng);

      EXPECT_TRUE(pDb->delAllSchedrs()) << pDb->getLastError();
      EXPECT_TRUE(pDb->delAllWorkers()) << pDb->getLastError();
      delete pDb;

      zmey::zmSchedr scng;
      scng.capacityTask = 10000;
      strcpy(scng.connectPnt, "localhost:4444");
      uint64_t sId = 0;
      EXPECT_TRUE(zmey::zmAddScheduler(_zc, scng, &sId));
  
      scng = zmey::zmSchedr();
      zmey::zmGetScheduler(_zc, sId, &scng);
      EXPECT_TRUE(scng.capacityTask == 10000 && 
                  strcmp(scng.connectPnt, "localhost:4444") == 0);

      zmey::zmWorker wcng;
      wcng.capacityTask = 10;
      wcng.sId = sId;
      wcng.exr = zmey::zmExecutorType::zmBash;
      strcpy(wcng.connectPnt, "localhost:4445");
      uint64_t wId = 0;
      EXPECT_TRUE(zmey::zmAddWorker(_zc, wcng, &wId));

      wcng = zmey::zmWorker();
      zmGetWorker(_zc, wId, &wcng);
      EXPECT_TRUE(wcng.sId == sId && 
                  wcng.capacityTask == 10 && 
                  wcng.exr == zmey::zmExecutorType::zmBash && 
                  strcmp(wcng.connectPnt, "localhost:4445") == 0);
    }
  }
  ~ZmeyTest() {
    zmey::zmDisconnect(_zc);
  }
protected:
  zmey::zmConn _zc = nullptr; 
};

TEST_F(ZmeyTest, pauseSchedr){
 
  uint64_t *sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);
     
  zmey::zmPauseScheduler(_zc, sId[0]);

  ZM_Aux::sleepMs(3000);

  zmey::zmStateType state;
  zmey::zmSchedulerState(_zc, sId[0], &state);

  EXPECT_TRUE(state == zmey::zmStateType::zmPause);
}
TEST_F(ZmeyTest, startSchedr){
 
  uint64_t* sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);
 
  zmey::zmStartScheduler(_zc, sId[0]);

  ZM_Aux::sleepMs(3000);

  zmey::zmStateType state;
  zmey::zmSchedulerState(_zc, sId[0], &state);

  EXPECT_TRUE(state == zmey::zmStateType::zmRunning);
}
TEST_F(ZmeyTest, pingSchedr){
 
  uint64_t *sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);
     
  EXPECT_TRUE(zmey::zmPingScheduler(_zc, sId[0]));
}

TEST_F(ZmeyTest, pauseWorker){
 
  uint64_t *sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);
   
  uint64_t* wId = nullptr;
  int wcnt = zmey::zmGetAllWorkers(_zc, sId[0], zmey::zmStateType::undefined, &wId);
  
  zmey::zmPauseWorker(_zc, wId[0]);

  ZM_Aux::sleepMs(3000);

  zmey::zmStateType state;
  zmey::zmWorkerState(_zc, wId, 1, &state);

  EXPECT_TRUE(state == zmey::zmStateType::zmPause);
}
TEST_F(ZmeyTest, startWorker){
 
  uint64_t *sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);

  uint64_t* wId = nullptr;
  int wcnt = zmey::zmGetAllWorkers(_zc, sId[0], zmey::zmStateType::undefined, &wId);
 
  zmey::zmStartWorker(_zc, wId[0]);

  ZM_Aux::sleepMs(3000);

  zmey::zmStateType state;
  zmey::zmWorkerState(_zc, wId, 1, &state);

  EXPECT_TRUE(state == zmey::zmStateType::zmRunning);
}
TEST_F(ZmeyTest, pingWorker){
 
  uint64_t *sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);
   
  uint64_t* wId = nullptr;
  int wcnt = zmey::zmGetAllWorkers(_zc, sId[0], zmey::zmStateType::undefined, &wId);
  
  EXPECT_TRUE(zmey::zmPingWorker(_zc, wId[0]));
}
