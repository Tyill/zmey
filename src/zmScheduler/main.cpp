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
#include <iostream>
#include <unordered_map>
#include "zmCommon/tcp.h"
#include "zmCommon/timerDelay.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "zmDbProvider/dbProvider.h"
#include "zmBase/structurs.h"
#include "zmCommon/logger.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void getNewTaskFromDB(ZM_DB::DbProvider& db);
void sendTaskToWorker(unordered_map<std::string, ZM_Base::worker>&,
                      ZM_Aux::QueueThrSave<ZM_Base::task>&);
void sendAllMessToDB(ZM_DB::DbProvider& db);
void checkStatusWorkers(const ZM_Base::scheduler&,
                        unordered_map<std::string, ZM_Base::worker>&,
                        ZM_Aux::QueueThrSave<ZM_DB::messSchedr>&);
void getPrevTaskFromDB(ZM_DB::DbProvider& db, ZM_Base::scheduler&,  ZM_Aux::QueueThrSave<ZM_Base::task>&);
void getPrevWorkersFromDB(ZM_DB::DbProvider& db, ZM_Base::scheduler&, unordered_map<std::string, ZM_Base::worker>&);

unordered_map<std::string, ZM_Base::worker> _workers;   // key - connectPnt
ZM_Aux::QueueThrSave<ZM_Base::task> _tasks;
ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
unique_ptr<ZM_Aux::Logger> _pLog = nullptr;
ZM_Base::scheduler _schedr;
bool _fClose = false;

struct params{
  bool logEna = false;
  int capasityTask = 10000;
  int sendAllMessTOutMS = 500;
  int checkWorkerTOutSec = 120; 
  string connectPnt = "localhost:4145";
  string dbServer;
  string dbName;  
};
params _prms;

void statusMess(const string& mess){
  cout << ZM_Aux::currDateTime() << " " << mess << std::endl;
  if (_pLog)
    _pLog->writeMess(mess);
}

void parseArgs(int argc, char* argv[], params& outPrms){
  
  string sargs;
  for (int i = 1; i < argc; ++i){
    sargs += argv[i];
  }
  map<string, string> sprms;
  auto argPair = ZM_Aux::split(sargs, "-");
  size_t asz = argPair.size();
  for (size_t i = 0; i < asz; ++i){
    auto pm = ZM_Aux::split(argPair[i], "=");
    if (pm.size() <= 1){
      sprms[argPair[i]] = "";
    }else{
      sprms[pm[0]] = pm[1];
    }
  }
  if (sprms.find("logEna") != sprms.end()){
    outPrms.logEna = true;
  }
  if (sprms.find("connectPnt") != sprms.end()){
    outPrms.connectPnt = sprms["connectPnt"];
  }  
  if (sprms.find("dbServer") != sprms.end()){
    outPrms.dbServer = sprms["dbServer"];
  }
  if (sprms.find("dbName") != sprms.end()){
    outPrms.dbName = sprms["dbName"];
  }
  if (sprms.find("capasityTask") != sprms.end() && ZM_Aux::isNumber(sprms["capasityTask"])){
    outPrms.capasityTask = stoi(sprms["capasityTask"]);
  }
  if (sprms.find("sendAllMessTOutMS") != sprms.end() && ZM_Aux::isNumber(sprms["sendAllMessTOutMS"])){
    outPrms.sendAllMessTOutMS = stoi(sprms["sendAllMessTOutMS"]);
  } 
  if (sprms.find("checkWorkerTOutSec") != sprms.end() && ZM_Aux::isNumber(sprms["checkWorkerTOutSec"])){
    outPrms.checkWorkerTOutSec = stoi(sprms["checkWorkerTOutSec"]);
  }    
}

void closeHandler(int sig){
  _fClose = true;
}

int main(int argc, char* argv[]){

  parseArgs(argc, argv, _prms);
  
  if (_prms.logEna){
    _pLog = unique_ptr<ZM_Aux::Logger>(new ZM_Aux::Logger("zmSchedr.log", ""));
  }    
 // signal(SIGHUP, initHandler);
  signal(SIGINT, closeHandler);
  signal(SIGTERM, closeHandler);

  // TCP server
  string err;
  if (ZM_Tcp::startServer(_prms.connectPnt, err)){
    ZM_Tcp::setReceiveCBack(receiveHandler);
    ZM_Tcp::setStsSendCBack(sendHandler);
    statusMess("Tcp server running: " + _prms.connectPnt);
  }else{
    statusMess("Tcp server error, busy -connectPnt: " + _prms.connectPnt + " " + err);
    return -1;
  }
  ZM_DB::DbProvider db(statusMess);
  if (db.connect(_prms.dbServer, _prms.dbName)){
    statusMess("DB connect success: " + _prms.dbServer + " " + _prms.dbName);
  }else{
    statusMess("DB connect error, not correct params -dbServer or -dbName: " + 
      _prms.dbServer + " " + _prms.dbName + " " + db.getLastError());
    ZM_Tcp::stopServer();
    return -1;
  }
  // schedr from DB
  db.getSchedr(_prms.connectPnt, _schedr);
  if (_schedr.id == 0){
    statusMess("Schedr not found in DB for connectPnt " + _prms.connectPnt);
    ZM_Tcp::stopServer();
    return -1;
  }
  // prev tasks and workers
  getPrevTaskFromDB(db, _schedr, _tasks);
  getPrevWorkersFromDB(db, _schedr, _workers);
  
  future<void> frGetNewTask,
               frSendAllMessToDB; 
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 5;

#define FUTURE_RUN(fut, func)                                                     \
  if(!fut.valid() || (fut.wait_for(chrono::seconds(0)) == future_status::ready)){ \
    fut = async(launch::async, [&db]{                                             \
      func(db);                                                                   \
    });                                                                           \
  }
  // main cycle
  while (!_fClose){
    timer.updateCycTime();   

    // get new tasks from DB
    if((_tasks.size() < _schedr.capasityTask) && (_schedr.ste == ZM_Base::state::run)){
      FUTURE_RUN(frGetNewTask, getNewTaskFromDB);
    }        
    // send task to worker    
    sendTaskToWorker(_workers, _tasks);    

    // send all mess to DB
    if(timer.onDelTmMS(true, _prms.sendAllMessTOutMS, 0) && !_messToDB.empty()){
      timer.onDelTmMS(false, _prms.sendAllMessTOutMS, 0);
      FUTURE_RUN(frSendAllMessToDB, sendAllMessToDB);
    }    
    // check status of workers
    if(timer.onDelTmSec(true, _prms.checkWorkerTOutSec, 1)){
      timer.onDelTmSec(false, _prms.checkWorkerTOutSec, 1);    
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