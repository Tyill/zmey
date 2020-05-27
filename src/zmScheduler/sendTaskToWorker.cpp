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
#include "zmBase/structurs.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "stdafx.h"

using namespace std;

ZM_Aux::CounterTick ctick;
vector<ZM_Base::worker*> refWorkers;

void sendTaskToWorker(unordered_map<std::string, ZM_Base::worker>& workers,
                      ZM_Aux::QueueThrSave<ZM_Base::task>& tasks){  
  
  if (refWorkers.empty()){
    for (auto& w : workers){
      refWorkers.push_back(&w.second);
    }
  }
  vector<ZM_Base::task> buffTask; 
  ZM_Base::task t;
  while (tasks.tryPop(t)){
    sort(refWorkers.begin(), refWorkers.end(), [](ZM_Base::worker* l, ZM_Base::worker* r){
      return (float)l->activeTask/max(1, l->rating) < (float)r->activeTask/max(1, r->rating);
    });
    auto iWr = find_if(refWorkers.begin(), refWorkers.end(),
      [&t](const ZM_Base::worker* w){
        return (w->exrType == t.exrType) && 
               (w->ste == ZM_Base::state::running) && 
               (w->activeTask < w->capasityTask);
      }); 
    if(iWr != refWorkers.end()){
      map<string, string> data{
        make_pair("command",         to_string((int)ZM_Base::messType::newTask)),
        make_pair("taskId",          to_string(t.id)),
        make_pair("params",          t.params), 
        make_pair("script",          t.script),
        make_pair("exrType",         to_string(int(t.exrType))),
        make_pair("averDurationSec", to_string(t.averDurationSec)), 
        make_pair("maxDurationSec",  to_string(t.maxDurationSec))
      };      
      ++(*iWr)->activeTask;
      ZM_Tcp::sendData((*iWr)->connectPnt, ZM_Aux::serialn(data));
    }else{
      buffTask.push_back(t);
    }
  }
  if (!buffTask.empty()){ 
    // every 1000 cycle
    if (ctick(1000)){
      string sTskId;
      for(auto& t : buffTask){
        sTskId += to_string(t.id) + " ";
      }
      statusMess("Not found available worker for tasks: " + sTskId);
    }
    for(auto& t : buffTask){
      tasks.push(move(t));
    }   
  }
}