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

vector<ZM_Base::worker> _workers;

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

  // концепт 
  //   рабочий:
  //    -запускает и наблюдает за задачами
  //    -для каждой задачи свой поток, чтобы ждать. 
  //    -в основном потоке таймеры и обработка сообщений от/для шедулеров
  //    -общается только с шедулерами, БД не знает.
  //    -адресов шедулеров тоже не знает, ждет когда к нему обратятся
  //    -от шедулера может прийти только сообщение с задачей, больше никаких сообщений
  //    -как только запустил задачу (или нет) на выполнение, сказать шедулеру об этом 
  //    -если задача вылетела или выполняется больше макс времени 
  //     остановить задачу и уведомить шедулера об этом
  //    -когда задача выполнилась, уведомить шедулера и послать ему последнее слово от задачи
  //    -после запуска ничего не предпринимать, ждать когда шедулер пришлет задачу
  //    -как только каждый шедулер обращается с задачей первый раз, сказать ему что только запустился, 
  //     чтобы он снял прошлые задачи, если висят
  //    -каждую 1 мин слать каждому известному шедулеру сообщение "работаю"
  //    -если шедулер не отвечает или сеть пропала, всеравно делать свое дело 
  //     и пытаться отправлять сообщения о том что в работе и статус текущих задач
  //    -при отключении завершить все текущие задачи принудительно 
  //     и сказать всем известным шедулерам что отключаюсь, чтобы сняли задачи 
  //   шедулер:
  //    -наблюдает за рабочими, рабочему может только слать новую задачу, больше ничего не говорит
  //    -если рабочий давно (5 мин) не говорил, что работает, и не присылал сообщения о задачах,
  //     его задачи надо перекинуть на другого рабочего
  //    -если рабочий загружен по максимуму или отдыхает, не слать ему задачи 
  //    -если рабочий "проснулся" и говорит о задачах, которые уже назначены другим, игнорировать эти сообщения 
  //    -если рабочий говорит, что задача запустилась, выполнилась или зависла, пометить задачу в БД
  //    -если рабочий говорит, что только запустился, снять с него все его прошлые задачи, которые еще в работе
  //     кроме последней отправленной, на которую он ответил только что 
  //    -из БД пытаться взять столько задач, сколько еще помещается в карман
  //    -за новыми задачами в БД лазить максимально часто на сколько возможно, если карман позволяет
  //    -помечать (выполнена/в отказе/запущена) задачи как накопится несколько штук, но не реже 1 мин
  //    -задачи выполнять в соответствии с приоритетом 
  //     и смотреть условие - выполнились ли прошлые задачи для этой задачи
  //    -если сам не в сети, значит надо перестать следить за рабочими (по времени), ничего не трогать,
  //     новые задачи из БД не брать, ждать пока появится сеть
  //    -когда только запустился выгребсти из БД все свои задачи и своих рабочих
  //    -при отключении все что есть на данный момент записать в БД      
  //   менеджер:
  //     

  // main cycle
  while (true){
    
    vector<ZM_Base::task> task = getAvailableTask();

    map<string, string> prms;
    for (auto& t : task){

      if (t.ste != ZM_Base::state::taskTakeByScheduler) continue;
      
      ZM_Base::worker wr;
      if (getAvailableWorker(wr)){   

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

      // check of workers
      for (auto& w : _workers){
        if (w.ste == ZM_Base::state::workerSilentLongTime){


        }
        w.ste = ZM_Base::state::workerSilentLongTime;
      }
    }
  }
 
  return 0;
}