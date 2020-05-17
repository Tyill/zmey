#include <algorithm>
#include "zmBase/structurs.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "stdafx.h"

using namespace std;

ZM_Aux::CounterTick ctick;

std::string getExecutorStr(ZM_Base::executorType et){
  switch (et){
    case ZM_Base::executorType::cmd: return "CMD";
    case ZM_Base::executorType::bash: return "BASH";
    case ZM_Base::executorType::python: return "PYTHON";
    default: return "";
  }
}

void sendTaskToWorker(const map<string, ZM_Base::worker>& workers,
                      ZM_Aux::QueueThrSave<ZM_Base::task>& tasks){
  
  vector<ZM_Base::task> buffTask; 
  ZM_Base::task t;
  while (tasks.tryPop(t)){
    auto iWkr = find_if(workers.begin(), workers.end(),
      [&t](const pair<string, ZM_Base::worker>& w){
        return (w.second.exrType == t.exrType) && (w.second.ste == ZM_Base::state::run);
      }); 
    if(iWkr != workers.end()){
      map<string, string> data{
        make_pair("command", to_string((int)ZM_Base::messType::newTask)),
        make_pair("taskId", to_string(t.id)),
        make_pair("params", t.params), 
        make_pair("script", t.script),
        make_pair("executor", to_string(int(t.exrType))),
        make_pair("meanDuration", to_string(t.averDurationSec)), 
        make_pair("maxDuration", to_string(t.maxDurationSec))
      };      
      ZM_Tcp::sendData(iWkr->first, ZM_Aux::serialn(data));
    }else{
      buffTask.push_back(t);
    }
  }
  if (!buffTask.empty()){ 
    // every 1000 cycle
    if (ctick(1000)){
      statusMess("Not found available worker for task executor = " + 
        getExecutorStr(buffTask[0].exrType));
    }
    for(auto& t : buffTask){
      tasks.push(move(t));
    }   
  }
}