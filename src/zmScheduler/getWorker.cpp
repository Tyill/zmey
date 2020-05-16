#include <map>
#include "zmBase/structurs.h"

using namespace std;

extern map<string, ZM_Base::worker> _workers;

bool getWorker(const ZM_Base::task& t, ZM_Base::worker* pWorker){
  
  for (auto& w : _workers){
    if (w.second.exrType == t.exrType){
      pWorker = &w.second;
      return true;
    }
  }

  return false;
}