#include <map>
#include "zmBase/structurs.h"
#include "zmCommon/auxFunc.h"
#include "shared.h"

using namespace std;

ZM_Aux::CounterTick ctick;

bool getWorker(const ZM_Base::task& t,
               const map<std::string, ZM_Base::worker>& workers,
               std::string& connPnt){
  
  for (auto& w : workers){
    if (w.second.exrType == t.exrType){
      connPnt = w.second.connectPnt;
      return true;
    }
  }  
  // every 1000 cycle
  if (ctick(1000)){
    statusMess("Not found available worker for task executor = " + ZM_Base::getExecutorStr(t.exrType));
  }
  return false;
}