#include <unordered_map>
#include <system_error>
#include "zmCommon/serial.h"
#include "zmBase/structurs.h"
#include "zmCommon/queue.h"
#include "zmDbProvider/dbProvider.h"
#include "stdafx.h"

using namespace std;

extern ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
extern unordered_map<std::string, ZM_Base::worker> _workers;
extern ZM_Base::scheduler _schedr;

void sendHandler(const string& cp, const string& data, const std::error_code& ec){
  
  // error from worker
  if (ec && (_workers.find(cp) != _workers.end())){
       
    _messToDB.push(ZM_DB::messSchedr{ZM_Base::messType::workerNotResponding, 
                                     _workers[cp].id});
    _workers[cp].ste = ZM_Base::state::notResponding;
    _workers[cp].activeTask = 0;
  }
}