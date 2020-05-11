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


#include <iostream>
#include <thread>
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"
#include "dbProvider.h"

using namespace std;

void receiveHandler(const string& cp, const string& data){


}

void errSendHandler(const string& cp, const string& data, const std::error_code& ec){


}

vector<ZM_Base::task> getAvailableTask(){
  return vector<ZM_Base::task>();
} 

bool getAvailableWorker(ZM_Base::worker&){
  return true;
} 

int main(int argc, char* argv[]){   

  ZM_Tcp::setReceiveCBack(receiveHandler);
  ZM_Tcp::setErrorSendCBack(errSendHandler);

  std::string connPnt = "localhost:4145",
              err;
  if (ZM_Tcp::startServer(connPnt, err)){
    cout << "Tcp server running: " + connPnt;
  }
  else{
    cout << "Tcp server error: " + connPnt + " " + err;
    return -1;
  }
  
  DbProvider db("localhost", "pgdb");

  // концепт: рабочий наблюдает за задачей:
  //           если задача вылетела или выполняется больше макс времени - уведомить шедулера об этом
  //          шедулер наблюдает за рабочим:
  //           если рабочий откинулся - его задачи надо перекинуть на другого рабочего,
  //          менеджер наблюдает за шедулером:
  //           если шедулер откинулся - его задачи надо пометить, что они свободны для выполнения,
  //           рабочий видит что шедулера нет, бросает все свои задачи на самотек

  // main cycle
  while (true){
    
    vector<ZM_Base::task> task = getAvailableTask();

    map<string, string> prms;
    for (auto& t : task){
      
      ZM_Base::worker wr;
      while (!getAvailableWorker(wr)){
        std::this_thread::yield();          // !!!  Надо в отдельном потоке наблюдать за прошлыми рабочими
      }
      prms["command"] = "newTask";
      prms["params"] = t.params; 
      prms["script"] = t.script;
      prms["executor"] = to_string(int(t.exrType));
      prms["meanDuration"] = t.meanDuration; 
      prms["maxDuration"] = t.maxDuration;          
    
      ZM_Tcp::sendData(wr.connectPnt, ZM_Aux::serialn(prms));
    }
  }
 
  return 0;
}