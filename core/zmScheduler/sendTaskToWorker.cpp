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
#include <algorithm>
#include <map>
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "zmDbProvider/dbProvider.h"
#include "structurs.h"

using namespace std;

ZM_Aux::CounterTick ctickTW;
vector<ZM_Base::worker> workersCpy;
vector<ZM_Base::worker*> refWorkers;

void sendTaskToWorker(const ZM_Base::scheduler& schedr,
                      map<std::string, sWorker>& workers,
                      ZM_Aux::QueueThrSave<sTask>& tasks, 
                      ZM_Aux::QueueThrSave<ZM_DB::messSchedr>& messToDB){  
#define ERROR_MESS(mess, wId)                                     \
  messToDB.push(ZM_DB::messSchedr{ZM_Base::messType::internError, \
                                  wId,                            \
                                  0,                              \
                                  0,                              \
                                  0,                              \
                                  0,                              \
                                  0,                              \
                                  mess});                         \
  statusMess(mess);
  
  if (workersCpy.empty()){
    for (auto& w : workers){
      workersCpy.push_back(w.second.base);
    }
    for (auto& w : workersCpy){
      refWorkers.push_back(&w);
    }
  } 
  auto iw = workers.cbegin();
  auto iwcp = workersCpy.begin();
  for (; iw != workers.cend(); ++iw, ++iwcp){
    iwcp->activeTask = iw->second.base.activeTask;
    iwcp->rating = iw->second.base.rating;
    iwcp->state = iw->second.base.state;
  }
  
  while (!tasks.empty()){
    sort(refWorkers.begin(), refWorkers.end(), [](const ZM_Base::worker* l, const ZM_Base::worker* r){
      return (float)l->activeTask / l->rating < (float)r->activeTask / r->rating;
    });
    auto iWr = find_if(refWorkers.begin(), refWorkers.end(),
      [](const ZM_Base::worker* w){
        return (w->state == ZM_Base::stateType::running) && 
               (w->activeTask <= w->capacityTask);
      }); 
    if(iWr != refWorkers.end()){
      sTask t;
      tasks.tryPop(t);
      map<string, string> data{
        make_pair("command",         to_string((int)ZM_Base::messType::newTask)),
        make_pair("connectPnt",      schedr.connectPnt),
        make_pair("taskId",          to_string(t.qTaskId)),
        make_pair("params",          t.params), 
        make_pair("script",          t.base.script),
        make_pair("averDurationSec", to_string(t.base.averDurationSec)), 
        make_pair("maxDurationSec",  to_string(t.base.maxDurationSec))
      };
      ++(*iWr)->activeTask;
      workers[(*iWr)->connectPnt].base.activeTask = (*iWr)->activeTask;

      ZM_Tcp::sendData((*iWr)->connectPnt, ZM_Aux::serialn(data));

      messToDB.push(ZM_DB::messSchedr{ZM_Base::messType::taskStart, 
                                      (*iWr)->id,
                                      t.qTaskId});      
      ctickTW.reset();
    }
    else{
      if (ctickTW(1000)){ // every 1000 cycle
        ERROR_MESS("schedr::sendTaskToWorker not found available worker", 0);
      }
      break;
    }
  }
}