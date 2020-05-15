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
#include "dbProvider.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void getNewTaskFromDB(DbProvider& db, ZM_Aux::QueueThrSave<ZM_Base::task>&);
void sendAllMessToDB(DbProvider& db, ZM_Aux::QueueThrSave<messageToDB>&);
bool getAvailableWorker(DbProvider& db, vector<ZM_Base::worker>& workers, ZM_Base::worker&);

int main(int argc, char* argv[]){   

  ZM_Tcp::setReceiveCBack(receiveHandler);
  ZM_Tcp::setStsSendCBack(sendHandler);

  std::string connPnt = "localhost:4145",
              err;
  if (ZM_Tcp::startServer(connPnt, err)){
  //  cout << "Tcp server running: " + connPnt;
  }
  else{
  //  cout << "Tcp server error: " + connPnt + " " + err;
    return -1;
  }
    
  DbProvider db("localhost", "pgdb");
   
  future<void> frGetTaskFromDB,
               frSendToDB;
  vector<ZM_Base::worker> workers;
  ZM_Aux::QueueThrSave<ZM_Base::task> tasks;
  ZM_Aux::QueueThrSave<messageToDB> messToDB;
  ZM_Aux::TimerDelay timer;


  // main cycle
  while (true){
    timer.updateCycTime();
    
    // get new task from DB
    if(timer.onDelTmMS(true, 10, 0) && (tasks.size() < 10000)){
      timer.onDelTmMS(false, 10, 0);
      if (!frGetTaskFromDB.valid() || (frGetTaskFromDB.wait_for(chrono::seconds(0)) == future_status::ready)){
        frGetTaskFromDB = async(
          launch::async, [&db, &tasks]{
            getNewTaskFromDB(db, tasks);
          });
      }
    }

    // send task to worker
    map<string, string> prms;
    ZM_Base::task t;
    while (tasks.tryPop(t)){

      if (t.ste != ZM_Base::state::taskTakeByScheduler) continue;
      
      ZM_Base::worker wr;
      if (getAvailableWorker(db, workers, wr)){   

        prms["command"] = "newTask";
        prms["taskId"] = t.id;
        prms["params"] = t.params; 
        prms["script"] = t.script;
        prms["executor"] = to_string(int(t.exrType));
        prms["meanDuration"] = t.meanDuration; 
        prms["maxDuration"] = t.maxDuration;          
      
        ZM_Tcp::sendData(wr.connectPnt, ZM_Aux::serialn(prms));
        t.ste = ZM_Base::state::taskSendToWorker;
      }     
    }

    // send all mess to DB
    if(timer.onDelTmMS(true, 1000, 1) && !messToDB.empty()){
      timer.onDelTmMS(false, 1000, 1);
      if (!frSendToDB.valid() || (frSendToDB.wait_for(chrono::seconds(0)) == future_status::ready)){
        frSendToDB = async(
          launch::async, [&db, &messToDB]{
            sendAllMessToDB(db, messToDB);
          });
      }
    }     
  }
  
  ZM_Tcp::stopServer();
  return 0;
}


void receiveHandler(const string& cp, const string& data){


}

void sendHandler(const string& cp, const string& data, const std::error_code& ec){


}

void getNewTaskFromDB(DbProvider&, ZM_Aux::QueueThrSave<ZM_Base::task>&){
 
}

void sendAllMessToDB(DbProvider& db, ZM_Aux::QueueThrSave<messageToDB>&){


}

bool getAvailableWorker(DbProvider& db, vector<ZM_Base::worker>& workers, ZM_Base::worker&){


  return true;
}