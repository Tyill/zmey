#include "zmBase/structurs.h"
#include "zmCommon/serial.h"
#include "zmCommon/queue.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/tcp.h"
#include "zmCommon/auxFunc.h"
#include "stdafx.h"

using namespace std;

extern ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
extern map<std::string, ZM_Base::worker> _workers;
extern ZM_Base::scheduler _schedr;

void receiveHandler(const string& cp, const string& data){
 
  auto mess = ZM_Aux::deserialn(data);
  if (mess.empty()){
    statusMess("receiveHandler Error deserialn data from: " + cp);
    return;
  }

#define checkFieldNum(field) \
  if ((mess.find("field") == mess.end()) || !ZM_Aux::isNumber(mess["field"])){  \
    statusMess("receiveHandler Error mess.find(field) == mess.end()) || !ZM_Aux::isNumber(mess[field]) from: " + cp);  \
    return;  \
  } 
#define checkField(field) \
  if (mess.find("field") == mess.end()){  \
    statusMess("receiveHandler Error mess.find(field) == mess.end() from: " + cp);  \
    return;  \
  } 

  checkFieldNum(command);
  ZM_Base::messType mtype = ZM_Base::messType(stoi(mess["command"]));
  // from worker
  if(_workers.find(cp) != _workers.end()){
    switch (mtype){
      case ZM_Base::messType::taskRunning:
      case ZM_Base::messType::taskError:
      case ZM_Base::messType::taskSuccess:      
      case ZM_Base::messType::taskPause:
      case ZM_Base::messType::taskStart:
      case ZM_Base::messType::taskStop:
        _messToDB.push(ZM_DB::messSchedr{mtype, _workers[cp].id});
        break;
      case ZM_Base::messType::justStartWorker:
        _messToDB.push(ZM_DB::messSchedr{mtype, _workers[cp].id});
        _workers[cp].ste = ZM_Base::state::run;
        break;
      case ZM_Base::messType::progress:
        checkFieldNum(progress);
        _messToDB.push(ZM_DB::messSchedr{ZM_Base::messType::progress,
                                         _workers[cp].id,
                                         stoi(mess["progress"])});
        break;
      case ZM_Base::messType::pingWorker:
        _workers[cp].isActive = true;
        break;
      default: statusMess("receiveHandler unknown command: " + mess["command"]);
        break;
    }    
    _workers[cp].isActive = true;
  }
  // from manager
  else{
    switch (mtype){
      case ZM_Base::messType::taskPause:
      case ZM_Base::messType::taskStart:
      case ZM_Base::messType::taskStop:{
        checkFieldNum(taskId);
        checkField(workerCP);
        map<string, string> data{
          make_pair("command", to_string((int)mtype)),
          make_pair("taskId", mess["taskId"])
        };      
        ZM_Tcp::sendData(mess["workerCP"], ZM_Aux::serialn(data));
        }
        break;
      case ZM_Base::messType::pingSchedr:{
        checkFieldNum(schedId);
        map<string, string> data{
          make_pair("command", to_string((int)mtype)),
          make_pair("schedId", to_string(_schedr.id))
        };      
        ZM_Tcp::sendData(cp, ZM_Aux::serialn(data));
        }
        break;
      case ZM_Base::messType::pauseSchedr:
        _schedr.ste = ZM_Base::state::pause;
        break;
      case ZM_Base::messType::pauseWorker:
        checkField(workerCP);
        _workers[mess["workerCP"]].ste = ZM_Base::state::pause;
        break;
      case ZM_Base::messType::startSchedr:
        _schedr.ste = ZM_Base::state::run;
        break;
      case ZM_Base::messType::startWorker:
        checkField(workerCP);
        _workers[mess["workerCP"]].ste = ZM_Base::state::run;  
        break;
      default: statusMess("receiveHandler unknown command: " + mess["command"]);
        break;
    }
  }
}