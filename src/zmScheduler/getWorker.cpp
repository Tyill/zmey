#include <map>
#include "zmBase/structurs.h"
#include "zmCommon/auxFunc.h"
#include "shared.h"

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
    statusMess("Not found available worker for task executor = " + getExecutorStr(t.exrType));
  }
  return false;
}