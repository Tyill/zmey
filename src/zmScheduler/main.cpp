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

#include <future>
#include <chrono>
#include <iostream>
#include <memory>
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"
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
bool getWorker(const ZM_Base::task& t, const map<std::string, ZM_Base::worker>&, std::string& connPnt);
void sendTaskToWorker(const ZM_Base::task& t, const ZM_Base::worker& wr);
void sendAllMessToDB(ZM_DB::DbProvider& db);
void getManagersFromDB(ZM_DB::DbProvider& db);
void sendStatusToManagers(const map<std::string, ZM_Base::manager>&);
void checkStatusWorkers(const ZM_Base::scheduler&,
                        map<std::string, ZM_Base::worker>&,
                        ZM_Aux::QueueThrSave<ZM_DB::message>&);
void getPrevTaskFromDB(ZM_DB::DbProvider& db, ZM_Base::scheduler&,  ZM_Aux::QueueThrSave<ZM_Base::task>&);
void getPrevWorkersFromDB(ZM_DB::DbProvider& db, ZM_Base::scheduler&, map<std::string, ZM_Base::worker>&);

map<std::string, ZM_Base::worker> _workers;   // key - connectPnt
map<std::string, ZM_Base::manager> _managers; // key - connectPnt
ZM_Aux::QueueThrSave<ZM_Base::task> _tasks;
ZM_Aux::QueueThrSave<ZM_DB::message> _messToDB;
unique_ptr<ZM_Aux::Logger> _pLog = nullptr;
ZM_Base::scheduler _schedr;

struct params{
  bool logEna = false;
  int capasityTask = 10000;
  int newManagerTOutSec = 30;
  int sendAllMessTOutMS = 500;
  int stsForManagerTOutSec = 20; 
  int checkWorkerTOutSec = 120; 
  string connectPnt = "localhost:4145";
  string dbServer;
  string dbName;  
};
params _prms;

void statusMess(const string& mess){
  cout << ZM_Aux::currDateTime() << mess << std::endl;
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
      sprms["-" + argPair[i]] = "";
    }else{
      sprms["-" + pm[0]] = pm[1];
    }
  }
  if (sprms.find("-logEna") != sprms.end()){
    outPrms.logEna = true;
  }
  if (sprms.find("-connectPnt") != sprms.end()){
    outPrms.connectPnt = sprms["-connectPnt"];
  }  
  if (sprms.find("-dbServer") != sprms.end()){
    outPrms.dbServer = sprms["-dbServer"];
  }
  if (sprms.find("-dbName") != sprms.end()){
    outPrms.dbName = sprms["-dbName"];
  }
  if (sprms.find("-capasityTask") != sprms.end() && ZM_Aux::isNumber(sprms["-capasityTask"])){
    outPrms.capasityTask = stoi(sprms["-capasityTask"]);
  }
  if (sprms.find("-newManagerTOutSec") != sprms.end() && ZM_Aux::isNumber(sprms["-newManagerTOutSec"])){
    outPrms.newManagerTOutSec = stoi(sprms["-newManagerTOutSec"]);
  }
  if (sprms.find("-sendAllMessTOutMS") != sprms.end() && ZM_Aux::isNumber(sprms["-sendAllMessTOutMS"])){
    outPrms.sendAllMessTOutMS = stoi(sprms["-sendAllMessTOutMS"]);
  } 
  if (sprms.find("-stsForManagerTOutSec") != sprms.end() && ZM_Aux::isNumber(sprms["-stsForManagerTOutSec"])){
    outPrms.stsForManagerTOutSec = stoi(sprms["-stsForManagerTOutSec"]);
  } 
  if (sprms.find("-checkWorkerTOutSec") != sprms.end() && ZM_Aux::isNumber(sprms["-checkWorkerTOutSec"])){
    outPrms.checkWorkerTOutSec = stoi(sprms["-checkWorkerTOutSec"]);
  }    
}

int main(int argc, char* argv[]){

  parseArgs(argc, argv, _prms);
  
  if (_prms.logEna){
    _pLog = unique_ptr<ZM_Aux::Logger>(new ZM_Aux::Logger("zmSchedr.log", ""));
  }
  // TCP server
  string err;
  if (ZM_Tcp::startServer(_prms.connectPnt, err)){
    ZM_Tcp::setReceiveCBack(receiveHandler);
    ZM_Tcp::setStsSendCBack(sendHandler);
    statusMess("Tcp server running: " + _prms.connectPnt);
  }else{
    statusMess("Tcp server error: " + _prms.connectPnt + " " + err);
    return -1;
  }
  ZM_DB::DbProvider db(statusMess);
  if (db.connect(_prms.dbServer, _prms.dbName)){
    statusMess("DB connect success: " + _prms.dbServer + " " + _prms.dbName);
  }else{
    statusMess("DB connect error: " + _prms.dbServer + " " + _prms.dbName + " " + db.getLastError());
    return -1;
  }
  // schedr from DB
  db.getSchedr(_prms.connectPnt, _schedr);
  if (_schedr.id == 0){
    statusMess("Schedr not found in DB for connPnt " + _prms.connectPnt);
    return -1;
  }
  // prev tasks and workers
  getPrevTaskFromDB(db, _schedr, _tasks);
  getPrevWorkersFromDB(db, _schedr, _workers);

 future<void> frGetNewTask,
              frGetManagers,
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
  while (true){
    timer.updateCycTime();   

    // get new tasks from DB
    if(_tasks.size() < _schedr.capasityTask){
      FUTURE_RUN(frGetNewTask, getNewTaskFromDB);
    }        
    // send task to worker    
    string wcp;
    ZM_Base::task t;
    while (_tasks.tryPop(t) && getWorker(t, _workers, wcp)){
      sendTaskToWorker(t, _workers[wcp]);
    }
    // send all mess to DB
    if(timer.onDelTmMS(true, _prms.sendAllMessTOutMS, 1) && !_messToDB.empty()){
      timer.onDelTmMS(false, _prms.sendAllMessTOutMS, 1);
      FUTURE_RUN(frSendAllMessToDB, sendAllMessToDB);
    }
    // get new managers from DB
    if(timer.onDelTmSec(true, _prms.newManagerTOutSec, 2)){
      timer.onDelTmSec(false, _prms.newManagerTOutSec, 2); 
      FUTURE_RUN(frGetManagers, getManagersFromDB);
    }
    // send status to managers
    if(timer.onDelTmSec(true, _prms.stsForManagerTOutSec, 3) && !_managers.empty()){      
      timer.onDelTmSec(false, _prms.stsForManagerTOutSec, 3);
      sendStatusToManagers(_managers);
    }
    // check status of workers
    if(timer.onDelTmSec(true, _prms.checkWorkerTOutSec, 4)){
      timer.onDelTmSec(false, _prms.checkWorkerTOutSec, 4);    
      checkStatusWorkers(_schedr, _workers, _messToDB);
    }
    // added delay
    if (timer.getCTime() < minCycleTimeMS){
      ZM_Aux::sleepMs(minCycleTimeMS - timer.getCTime());
    }
  }
  // on end
  sendAllMessToDB(db);
  
  ZM_Tcp::stopServer();
  return 0;
}