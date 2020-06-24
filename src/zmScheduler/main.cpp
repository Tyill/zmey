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
#include <future>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include "zmCommon/tcp.h"
#include "zmCommon/timerDelay.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/logger.h"
#include "structurs.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void getNewTaskFromDB(ZM_DB::DbProvider& db);
void sendTaskToWorker(unordered_map<std::string, sWorker>&,
                      ZM_Aux::QueueThrSave<sTask>&);
void sendAllMessToDB(ZM_DB::DbProvider& db);
void checkStatusWorkers(const ZM_Base::scheduler&,
                        unordered_map<std::string, sWorker>&,
                        ZM_Aux::QueueThrSave<ZM_DB::messSchedr>&);
void getPrevTaskFromDB(ZM_DB::DbProvider& db, ZM_Base::scheduler&,  ZM_Aux::QueueThrSave<sTask>&);
void getPrevWorkersFromDB(ZM_DB::DbProvider& db, ZM_Base::scheduler&, unordered_map<std::string, sWorker>&);

unordered_map<std::string, sWorker> _workers;   // key - connectPnt
ZM_Aux::QueueThrSave<sTask> _tasks;
ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
unique_ptr<ZM_Aux::Logger> _pLog = nullptr;
ZM_Base::scheduler _schedr;
bool _fClose = false;

struct config{
  bool logEna = false;
  int capacityTask = 10000;
  int sendAllMessTOutMS = 500;
  int checkWorkerTOutSec = 120; 
  std::string dbType;
  std::string connectPnt;
  ZM_DB::connectCng dbConnCng;
};
config _cng;

void statusMess(const string& mess){
  cout << ZM_Aux::currDateTime() << " " << mess << std::endl;
  if (_pLog)
    _pLog->writeMess(mess);
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
  if (sprms.find("log") != sprms.end()){
    outCng.logEna = true;
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

int main(int argc, char* argv[]){

  parseArgs(argc, argv, _cng);
  
  if (_cng.logEna){
    _pLog = unique_ptr<ZM_Aux::Logger>(new ZM_Aux::Logger("zmSchedr.log", ""));
  }    
 // signal(SIGHUP, initHandler);
  signal(SIGINT, closeHandler);
  signal(SIGTERM, closeHandler);

  // TCP server
  string err;
  if (ZM_Tcp::startServer(_cng.connectPnt, err)){
    ZM_Tcp::setReceiveCBack(receiveHandler);
    ZM_Tcp::setStsSendCBack(sendHandler);
    statusMess("Tcp server running: " + _cng.connectPnt);
  }else{
    statusMess("Tcp server error, busy -connectPnt: " + _cng.connectPnt + " " + err);
    return -1;
  }
  unique_ptr<ZM_DB::DbProvider> db(ZM_DB::makeDbProvider(_cng.dbConnCng));
  if (db && db->getLastError().empty()){
    statusMess(
      "DB connect success: " + _cng.dbType + " " + _cng.dbConnCng.connectStr);
  }else{
    statusMess(
      "DB connect error " + db->getLastError() + ": " + _cng.dbType + " " + _cng.dbConnCng.connectStr);
    ZM_Tcp::stopServer();
    return -1;
  }
  // schedr from DB
  db->getSchedr(_cng.connectPnt, _schedr);
  if (_schedr.id == 0){
    statusMess("Schedr not found in DB for connectPnt " + _cng.connectPnt);
    ZM_Tcp::stopServer();
    return -1;
  }
  // prev tasks and workers
  getPrevTaskFromDB(*db, _schedr, _tasks);
  getPrevWorkersFromDB(*db, _schedr, _workers);
  
  future<void> frGetNewTask,
               frSendAllMessToDB; 
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 5;

#define FUTURE_RUN(fut, func)                                                     \
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
      FUTURE_RUN(frGetNewTask, getNewTaskFromDB);
    }        
    // send task to worker    
    sendTaskToWorker(_workers, _tasks);    

    // send all mess to DB
    if(timer.onDelTmMS(true, _cng.sendAllMessTOutMS, 0) && !_messToDB.empty()){
      timer.onDelTmMS(false, _cng.sendAllMessTOutMS, 0);
      FUTURE_RUN(frSendAllMessToDB, sendAllMessToDB);
    }    
    // check status of workers
    if(timer.onDelTmSec(true, _cng.checkWorkerTOutSec, 1)){
      timer.onDelTmSec(false, _cng.checkWorkerTOutSec, 1);    
      checkStatusWorkers(_schedr, _workers, _messToDB);
    }
    // added delay
    if (timer.getCTime() < minCycleTimeMS){
      ZM_Aux::sleepMs(minCycleTimeMS - timer.getCTime());
    }
  }
  ZM_Tcp::stopServer();

  return 0;
}