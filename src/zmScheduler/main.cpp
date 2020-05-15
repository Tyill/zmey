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
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"
#include "zmCommon/timerDelay.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "dbProvider.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void getNewTaskFromDB(DbProvider& db);
void getAvailableWorkers(DbProvider& db);
bool getWorker(ZM_Base::worker&);
void sendTaskToWorker(const ZM_Base::worker& wr, const ZM_Base::task& t);
void sendAllMessToDB(DbProvider& db);
void getAvailableManagers(DbProvider& db);
void sendStatusToManagers();
void checkStatusWorkers();
void getPrevTaskFromDB(ZM_Aux::QueueThrSave<ZM_Base::task>&);
void getPrevWorkers(vector<ZM_Base::worker>&);

vector<ZM_Base::worker> _workers;
vector<ZM_Base::manager> _managers;
ZM_Aux::QueueThrSave<ZM_Base::task> _tasks;
ZM_Aux::QueueThrSave<messageToDB> _messToDB;

int main(int argc, char* argv[]){   

  ZM_Tcp::setReceiveCBack(receiveHandler);
  ZM_Tcp::setStsSendCBack(sendHandler);

  std::string connPnt = "localhost:4145", err;
  if (ZM_Tcp::startServer(connPnt, err)){
  //  cout << "Tcp server running: " + connPnt;
  }
  else{
  //  cout << "Tcp server error: " + connPnt + " " + err;
    return -1;
  }
    
  DbProvider db("localhost", "pgdb");
   
  future<void> frGetTaskFromDB,
               frGetWorkersFromDB,
               frGetManagersFromDB,
               frSendAllMessToDB; 
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 5;

#define FUTURE_RUN(fut, func)                                                     \
  if(!fut.valid() || (fut.wait_for(chrono::seconds(0)) == future_status::ready)){ \
    fut = async(launch::async, [&db]{                                             \
      func(db);                                                                   \
    });                                                                           \
  }
  // prev tasks and workers
  getPrevTaskFromDB(_tasks);
  getPrevWorkers(_workers);

  // main cycle
  while (true){
    timer.updateCycTime();   

    // get new tasks from DB
    if(_tasks.size() < 10000){
      FUTURE_RUN(frGetTaskFromDB, getNewTaskFromDB);
    }
    // get workers from DB
    if(timer.onDelTmSec(true, 10, 0) || _workers.empty()){  
      timer.onDelTmSec(false, 10, 0);    
      FUTURE_RUN(frGetWorkersFromDB, getAvailableWorkers);
    }       
    // send task to worker    
    ZM_Base::worker wr;
    ZM_Base::task t;
    while (getWorker(wr) && _tasks.tryPop(t)){
      sendTaskToWorker(wr, t);
    }
    // send all mess to DB
    if(timer.onDelTmMS(true, 1000, 0) && !_messToDB.empty()){
      timer.onDelTmMS(false, 1000, 0);
      FUTURE_RUN(frSendAllMessToDB, sendAllMessToDB);
    }
    // get managers from DB
    if(timer.onDelTmSec(true, 30, 1)){
      timer.onDelTmSec(false, 30, 1); 
      FUTURE_RUN(frGetManagersFromDB, getAvailableManagers);
    }
    // send status to managers
    if(timer.onDelTmSec(true, 60, 2) && !_managers.empty()){      
      timer.onDelTmSec(false, 60, 2);
      sendStatusToManagers();
    }
    // check status of workers
    if(timer.onDelTmSec(true, 120, 3)){
      timer.onDelTmSec(false, 120, 3);    
      checkStatusWorkers();
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
void receiveHandler(const string& cp, const string& data){


}
void sendHandler(const string& cp, const string& data, const std::error_code& ec){


}
void getNewTaskFromDB(DbProvider&){
 
}
void sendTaskToWorker(const ZM_Base::worker& wr, const ZM_Base::task& t){
  map<string, string> sendData{
    make_pair("command", "newTask"),
    make_pair("taskId", to_string(t.id)),
    make_pair("params", t.params), 
    make_pair("script", t.script),
    make_pair("executor", to_string(int(t.exrType))),
    make_pair("meanDuration", t.meanDuration), 
    make_pair("maxDuration", t.maxDuration)
  };
  ZM_Tcp::sendData(wr.connectPnt, ZM_Aux::serialn(sendData));
}
void sendAllMessToDB(DbProvider& db){


}
void getAvailableWorkers(DbProvider& db){


}
bool getWorker(ZM_Base::worker&){


  return true;
}
void getAvailableManagers(DbProvider& db){


}
void sendStatusToManagers(){
  map<string, string> sendData{
    make_pair("command", "statusScheduler")
  };      
  for (auto& m : _managers){
    if (m.ste == ZM_Base::state::run){
      ZM_Tcp::sendData(m.connectPnt, ZM_Aux::serialn(sendData));
    }
  }
}
void checkStatusWorkers(){

}
void getPrevTaskFromDB(ZM_Aux::QueueThrSave<ZM_Base::task>&){

}
void getPrevWorkers(vector<ZM_Base::worker>&){
}