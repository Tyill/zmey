#include "zmBase/structurs.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/queue.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"

using namespace std;

void checkStatusWorkers(const ZM_Base::scheduler& schedr,
                        map<std::string, ZM_Base::worker>& workers,
                        ZM_Aux::QueueThrSave<ZM_DB::messSchedr>& messToDB){
   
  for(auto& w : workers){
    if (!w.second.isActive){            
      messToDB.push(ZM_DB::messSchedr{ZM_Base::messType::workerNotResponding,
                                      w.second.id});
    }else{
      w.second.isActive = false;
    }
  }

  // all inactive
  vector<ZM_Base::worker> wrsNotResp;
  for(auto& w : workers){
    if (w.second.ste == ZM_Base::state::notResponding){
      wrsNotResp.push_back(w.second);
    }
  }
  if (wrsNotResp.empty())
    return; 

  // check max10% inactive
  int cnt = 0,
      maxCnt = max(1, int(wrsNotResp.size() * 0.1));
  while(true){   
    int i = rand() % wrsNotResp.size(); 
    map<string, string> sendData{
      make_pair("command", to_string((int)ZM_Base::messType::pingWorker))
    };      
    ZM_Tcp::sendData(wrsNotResp[i].connectPnt, ZM_Aux::serialn(sendData));
    ++cnt;
    if (cnt == maxCnt)
      break;
  }
}