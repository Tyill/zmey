#include <algorithm>
#include <map>
#include "zmDbProvider/dbProvider.h"
#include "zmBase/structurs.h"
#include "zmCommon/queue.h"
#include "zmCommon/tcp.h" 
#include "zmCommon/serial.h" 

using namespace std;

extern ZM_Base::scheduler _schedr;
extern map<std::string, ZM_Base::worker> _workers;

void getNewWorkersFromDB(ZM_DB::DbProvider& db){

  vector<ZM_Base::worker> wrs;
  if (!db.getNewWorkersForSchedr(_schedr.id, wrs))
    return;

  // by reuting
  sort(wrs.begin(), wrs.end(),[](const ZM_Base::worker& l, const ZM_Base::worker& r){
    return (float)l.rating / max(1, l.activeTask) > (float)r.rating / max(1, r.activeTask);
  });

  size_t cwSz = _workers.size(),
         maxSz = _schedr.capasityWorker;
  for(auto& w : wrs){
    if ((w.ste != ZM_Base::state::run) || (w.activeTask >= w.capasityTask))
      continue; 
    if (cwSz < maxSz){
      _workers[w.connectPnt] = w;
      ++cwSz;
    }else
      break;
  }  
  // all inactive
  vector<ZM_Base::worker> wrsNotResp;
  for(auto& w : wrs){
    if (w.ste == ZM_Base::state::notResponding){
      wrsNotResp.push_back(w);
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
      make_pair("command", "ping")
    };      
    ZM_Tcp::sendData(wrsNotResp[i].connectPnt, ZM_Aux::serialn(sendData));
    ++cnt;
    if (cnt == maxCnt)
      break;
  }
}