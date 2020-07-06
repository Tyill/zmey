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
#include <unordered_map>
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "structurs.h"

using namespace std;

ZM_Aux::CounterTick ctickTW;
vector<sWorker*> refWorkers;

void sendTaskToWorker(const ZM_Base::scheduler& schedr,
                      unordered_map<std::string, sWorker>& workers,
                      ZM_Aux::QueueThrSave<sTask>& tasks){  
  
  if (refWorkers.empty()){
    for (auto& w : workers){
      refWorkers.push_back(&w.second);
    }
  }
  sTask t;
  while (tasks.tryPop(t)){
    sort(refWorkers.begin(), refWorkers.end(), [](sWorker* l, sWorker* r){
      return (float)l->base.activeTask / l->base.rating < (float)r->base.activeTask / r->base.rating;
    });
    auto iWr = find_if(refWorkers.begin(), refWorkers.end(),
      [](const sWorker* w){
        return (w->base.state == ZM_Base::stateType::running) && 
               (w->base.activeTask < w->base.capacityTask);
      }); 
    if(iWr != refWorkers.end()){
      map<string, string> data{
        make_pair("command",         to_string((int)ZM_Base::messType::newTask)),
        make_pair("connectPnt",      schedr.connectPnt),
        make_pair("taskId",          to_string(t.qTaskId)),
        make_pair("params",          t.params), 
        make_pair("script",          t.base.script),
        make_pair("averDurationSec", to_string(t.base.averDurationSec)), 
        make_pair("maxDurationSec",  to_string(t.base.maxDurationSec))
      };      
      ++(*iWr)->base.activeTask;
      ZM_Tcp::sendData((*iWr)->base.connectPnt, ZM_Aux::serialn(data));
    }
    else{      
      tasks.push(move(t));
      if (ctickTW(1000)){ // every 1000 cycle
        statusMess("Not found available worker");
      }
      break;
    }
  }
}