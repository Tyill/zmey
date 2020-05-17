#include <system_error>
#include "zmCommon/serial.h"
#include "zmBase/structurs.h"
#include "zmCommon/queue.h"
#include "zmDbProvider/dbProvider.h"
#include "stdafx.h"

using namespace std;

extern ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
extern map<std::string, ZM_Base::worker> _workers;
extern ZM_Base::scheduler _schedr;

void sendHandler(const string& cp, const string& data, const std::error_code& ec){
  
  // error from worker
  if (ec && (_workers.find(cp) != _workers.end())){
    auto mess = ZM_Aux::deserialn(data);
    if (mess.empty()){
      statusMess("sendHandler Error deserialn data from: " + cp);
      return;
    }
    switch (ZM_Base::messType(stoi(mess["command"]))){
      case ZM_Base::messType::newTask:
      case ZM_Base::messType::taskPause:
      case ZM_Base::messType::taskStart:
      case ZM_Base::messType::taskStop:
      case ZM_Base::messType::startWorker:
      case ZM_Base::messType::pauseWorker:
        _messToDB.push(ZM_DB::messSchedr{ZM_Base::messType::workerNotResponding, 
                                         _workers[cp].id});
        _workers[cp].ste = ZM_Base::state::notResponding;
        break;      
      default:
        break;
    }
  }
}