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
#include <signal.h>
#include <unistd.h>
#include <future>
#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>
#include "zmCommon/tcp.h"
#include "zmCommon/timerDelay.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "zmDbProvider/dbProvider.h"
#include "structurs.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void getNewTaskFromDB(ZM_DB::DbProvider& db);
void sendTaskToWorker(const ZM_Base::scheduler&, map<std::string, sWorker>&, ZM_Aux::QueueThrSave<sTask>&, ZM_Aux::QueueThrSave<ZM_DB::messSchedr>& messToDB);
void sendAllMessToDB(ZM_DB::DbProvider& db);
void checkStatusWorkers(const ZM_Base::scheduler&, map<std::string, sWorker>&, ZM_Aux::QueueThrSave<ZM_DB::messSchedr>&);
void getPrevTaskFromDB(ZM_DB::DbProvider& db, ZM_Base::scheduler&,  ZM_Aux::QueueThrSave<sTask>&);
void getPrevWorkersFromDB(ZM_DB::DbProvider& db, ZM_Base::scheduler&, map<std::string, sWorker>&);

map<std::string, sWorker> _workers;   // key - connectPnt
ZM_Aux::QueueThrSave<sTask> _tasks;
ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
ZM_Base::scheduler _schedr;
mutex _mtxSts;
volatile bool _fClose = false;

struct config{
  int capacityTask = 10000;
  int sendAllMessTOutMS = 500;
  int checkWorkerTOutSec = 120; 
  const int currentStateTOutSec = 10; 
  std::string dbType;
  std::string connectPnt;
  ZM_DB::connectCng dbConnCng;
};

void statusMess(const string& mess){
  lock_guard<std::mutex> lock(_mtxSts);
  cout << ZM_Aux::currDateTimeMs() << " " << mess << std::endl;
}

void parseArgs(int argc, char* argv[], config& outCng){ 
  string sargs;
  for (int i = 1; i < argc; ++i){
    sargs += argv[i];
  }
  map<string, string> sprms;
  auto argPair = ZM_Aux::split(sargs, "-");
  for (auto& arg : argPair){
    size_t sp = arg.find_first_of("=");
    if (sp != std::string::npos){
      sprms[ZM_Aux::trim(arg.substr(0, sp))] = ZM_Aux::trim(arg.substr(sp + 1));
    }else{
      sprms[ZM_Aux::trim(arg)] = "";
    }
  }
#define SET_PARAM(nm, prm) \
  if (sprms.find(#nm) != sprms.end()){ \
    outCng.prm = sprms[#nm]; \
  }
  SET_PARAM(cp, connectPnt);
  SET_PARAM(dbtp, dbType);
  SET_PARAM(dbcs, dbConnCng.connectStr);

  outCng.dbConnCng.selType = ZM_DB::dbTypeFromStr(outCng.dbType);
 
#define SET_PARAM_NUM(nm, prm) \
  if (sprms.find(#nm) != sprms.end() && ZM_Aux::isNumber(sprms[#nm])){ \
    outCng.prm = stoi(sprms[#nm]); \
  }  
  SET_PARAM_NUM(ctk, capacityTask);
  SET_PARAM_NUM(sdt, sendAllMessTOutMS);
  SET_PARAM_NUM(chw, checkWorkerTOutSec);

#undef SET_PARAM
#undef SET_PARAM_NUM
}

void closeHandler(int sig){
  _fClose = true;
}

unique_ptr<ZM_DB::DbProvider> 
createDbProvider(const config& cng, std::string& err){
  unique_ptr<ZM_DB::DbProvider> db(ZM_DB::makeDbProvider(cng.dbConnCng));  
  if (db && db->getLastError().empty()){
    return db;
  } else{
    err = db ? db->getLastError() : "db undefined";
    return nullptr;
  }
}

#define CHECK(fun, mess) \
  if (fun){              \
    statusMess(mess);    \
    return -1;           \
  }

int main(int argc, char* argv[]){

  config cng;
  parseArgs(argc, argv, cng);
  
  CHECK(cng.connectPnt.empty(), "Not set param '-cp' - scheduler connection point: IP or DNS:port");
  CHECK(cng.dbConnCng.selType == ZM_DB::dbType::undefined, "Check param '-dbtp', such db type is not defined");
  CHECK(cng.dbConnCng.connectStr.empty(), "Not set param '-dbcs' - db connection string");
 
  signal(SIGHUP, closeHandler);
  signal(SIGINT, closeHandler);
  signal(SIGTERM, closeHandler);
  signal(SIGQUIT, closeHandler);
  
  // db providers
  string err;
  auto dbNewTask = createDbProvider(cng, err);
  auto dbSendMess = dbNewTask ? createDbProvider(cng, err) : nullptr;
  CHECK(!dbNewTask || !dbSendMess, "Schedr DB connect error " + err + ": " + cng.dbType + " " + cng.dbConnCng.connectStr); 
    
  // schedr from DB
  dbNewTask->getSchedr(cng.connectPnt, _schedr);
  CHECK(_schedr.id == 0, "Schedr not found in DB for connectPnt " + cng.connectPnt);
      
  // prev tasks and workers
  getPrevTaskFromDB(*dbNewTask, _schedr, _tasks);
  getPrevWorkersFromDB(*dbNewTask, _schedr, _workers);
 
  // TCP server
  ZM_Tcp::setReceiveCBack(receiveHandler);
  ZM_Tcp::setStsSendCBack(sendHandler);
  CHECK(!ZM_Tcp::startServer(cng.connectPnt, err), "Schedr error: " + cng.connectPnt + " " + err);
  statusMess("Schedr running: " + cng.connectPnt);
  
  ///////////////////////////////////////////////////////

  future<void> frGetNewTask,
               frSendAllMessToDB; 
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 10;

  #define FUTURE_RUN(fut, db, func)                                                 \
    if(!fut.valid() || (fut.wait_for(chrono::seconds(0)) == future_status::ready)){ \
      fut = async(launch::async, [&db]{                                             \
        func(*db);                                                                  \
      });                                                                           \
    }
  // main cycle
  while (!_fClose){
    timer.updateCycTime();   

    // get new tasks from DB
    if((_tasks.size() < _schedr.capacityTask) && (_schedr.state != ZM_Base::stateType::pause)){
      FUTURE_RUN(frGetNewTask, dbNewTask, getNewTaskFromDB);
    }        
    // send task to worker    
    sendTaskToWorker(_schedr, _workers, _tasks, _messToDB);    

    // send all mess to DB
    if(timer.onDelayMS(true, cng.sendAllMessTOutMS, 0) && !_messToDB.empty()){
      timer.onDelayMS(false, cng.sendAllMessTOutMS, 0);
      FUTURE_RUN(frSendAllMessToDB, dbSendMess, sendAllMessToDB);
    }    
    // check status of workers
    if(timer.onDelaySec(true, cng.checkWorkerTOutSec, 1)){
      timer.onDelaySec(false, cng.checkWorkerTOutSec, 1);    
      checkStatusWorkers(_schedr, _workers, _messToDB);
    }
    // current state
    if(timer.onDelaySec(true, cng.currentStateTOutSec, 2)){
      timer.onDelaySec(false, cng.currentStateTOutSec, 2);    
      _messToDB.push(ZM_DB::messSchedr{ 
        _schedr.state == ZM_Base::stateType::running ? ZM_Base::messType::startSchedr : ZM_Base::messType::pauseSchedr
      });
    }
    // added delay
    if (timer.getDeltaTimeMS() < minCycleTimeMS){
      ZM_Aux::sleepMs(minCycleTimeMS - timer.getDeltaTimeMS());
    }
  }
  ZM_Tcp::stopServer();
  sendAllMessToDB(*dbSendMess);
  return 0;
}