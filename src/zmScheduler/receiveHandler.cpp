
#include <string>
#include <vector>
#include "zmBase/structurs.h"
#include "zmCommon/serial.h"
#include "zmCommon/queue.h"
#include "zmDbProvider/dbProvider.h"
#include "shared.h"

using namespace std;

extern ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
extern map<std::string, ZM_Base::worker> _workers;
extern ZM_Base::scheduler _schedr;

void receiveHandler(const string& cp, const string& data){
 
  auto mess = ZM_Aux::deserialn(data);
  if (mess.empty()){
    statusMess("Error deserialn data from: " + cp);
    return;
  }  
  if(_workers.find(cp) != _workers.end()){
    ZM_Base::messType mtype = ZM_Base::messType(stoi(mess["command"]));
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
        break;    
      default:
        break;
    }
  }
}