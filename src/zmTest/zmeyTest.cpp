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

using namespace std;

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
      bool ff = false;  
    }
  }
  ~ZmeyTest() {
    zmey::zmDisconnect(_zc);
  }
protected:
  zmey::zmConn _zc = nullptr; 
};

TEST_F(ZmeyTest, startSchedr){
 
  uint64_t* sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);
  
  if (scnt == 0){
    zmey::zmSchedr cng;
    cng.capacityTask = 10000;
    strcpy(cng.connectPnt, "localhost:4444");
    sId = new uint64_t();
    EXPECT_TRUE(zmey::zmAddScheduler(_zc, cng, sId));
  } 

  zmey::zmStartScheduler(_zc, sId[0]);

  ZM_Aux::sleepMs(3000);

  zmey::zmStateType state;
  zmey::zmSchedulerState(_zc, sId[0], &state);

  EXPECT_TRUE(state == zmey::zmStateType::zmRunning);
}
TEST_F(ZmeyTest, pauseSchedr){
 
  uint64_t *sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);
   
  if (scnt == 0){
    zmey::zmSchedr cng;
    cng.capacityTask = 10000;
    strcpy(cng.connectPnt, "localhost:4444");
    sId = new uint64_t();
    EXPECT_TRUE(zmey::zmAddScheduler(_zc, cng, sId));
  } 

  zmey::zmPauseScheduler(_zc, sId[0]);

  ZM_Aux::sleepMs(3000);

  zmey::zmStateType state;
  zmey::zmSchedulerState(_zc, sId[0], &state);

  EXPECT_TRUE(state == zmey::zmStateType::zmPause);
}
TEST_F(ZmeyTest, pingSchedr){
 
  uint64_t *sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);
   
  if (scnt == 0){
    zmey::zmSchedr cng;
    cng.capacityTask = 10000;
    strcpy(cng.connectPnt, "localhost:4444");
    sId = new uint64_t();
    EXPECT_TRUE(zmey::zmAddScheduler(_zc, cng, sId));
  }

  EXPECT_TRUE(zmey::zmPingScheduler(_zc, sId[0]));
}

TEST_F(ZmeyTest, startWorker){
 
  uint64_t *sId = nullptr;
  int scnt = zmey::zmGetAllSchedulers(_zc, zmey::zmStateType::undefined, &sId);
   
  if (scnt == 0){
    zmey::zmSchedr cng;
    cng.capacityTask = 10000;
    strcpy(cng.connectPnt, "localhost:4444");
    sId = new uint64_t();
    EXPECT_TRUE(zmey::zmAddScheduler(_zc, cng, sId));
  } 

  uint64_t *wId = nullptr;
  int wcnt = zmey::zmGetAllWorkers(_zc, sId[0], zmey::zmStateType::undefined, &wId);

  if (wcnt < 2){
    zmey::zmWorker cng;
    cng.capacityTask = 10;
    strcpy(cng.connectPnt, "localhost:4445");
    wId = new uint64_t[2];
    EXPECT_TRUE(zmey::zmAddWorker(_zc, cng, wId));

    strcpy(cng.connectPnt, "localhost:4446");
    EXPECT_TRUE(zmey::zmAddWorker(_zc, cng, wId + 1));
  }

  zmey::zmWorker cng;
  zmGetWorker(_zc, wId[0], &cng);
 
  zmey::zmStartWorker(_zc, wId[0]);

  ZM_Aux::sleepMs(3000);

  zmey::zmStateType state[2];
  zmey::zmWorkerState(_zc, wId, 2, state);

  EXPECT_TRUE(state[0] == zmey::zmStateType::zmRunning);
}