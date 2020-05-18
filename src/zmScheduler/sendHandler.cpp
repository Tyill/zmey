#include <unordered_map>
#include <system_error>
#include "zmCommon/serial.h"
#include "zmCommon/tcp.h"
#include "zmBase/structurs.h"
#include "zmCommon/queue.h"
#include "zmDbProvider/dbProvider.h"
#include "stdafx.h"

using namespace std;

extern ZM_Aux::QueueThrSave<ZM_Base::task> _tasks;
extern unordered_map<std::string, ZM_Base::worker> _workers;
extern ZM_Base::scheduler _schedr;

void sendHandler(const string& cp, const string& data, const std::error_code& ec){
  
  auto mess = ZM_Aux::deserialn(data);
  ZM_Base::messType mtype = ZM_Base::messType(stoi(mess["command"]));

  // error from worker
  if (ec && (_workers.find(cp) != _workers.end())){    
    switch (mtype){
      case ZM_Base::messType::newTask:{
        ZM_Base::task t;
        t.id = stoi(mess["taskId"]);
        t.params = stoi(mess["params"]);
        t.script = stoi(mess["script"]);
        t.exrType = (ZM_Base::executorType)stoi(mess["exrType"]);
        t.averDurationSec = stoi(mess["averDurationSec"]);
        t.maxDurationSec = stoi(mess["maxDurationSec"]);
        _tasks.push(move(t));
        statusMess("sendHandler worker not response, cp: " + cp);
        }
        break;       
      case ZM_Base::messType::taskPause:
      case ZM_Base::messType::taskStart:
      case ZM_Base::messType::taskStop:{
        map<string, string> data{
          make_pair("command", to_string((int)mtype)),
          make_pair("taskId", mess["taskId"]),
          make_pair("schedrId", to_string(_schedr.id)),
          make_pair("workerId", to_string(_workers[cp].id)),
          make_pair("state", to_string((int)ZM_Base::messType::workerNotResponding)),
        };  
        ZM_Tcp::sendData(mess["managerConnPnt"], ZM_Aux::serialn(data));
        statusMess("sendHandler worker not response, cp: " + cp);
        }
        break;
      default: // I'm OK
        break;
    } 
  }
  // error from manager
  else{
    statusMess("sendHandler manager not response, cp: " + cp);
  }  
}