#include <map>
#include "zmDbProvider/dbProvider.h"

using namespace std;

extern map<std::string, ZM_Base::manager> _managers;

void getManagersFromDB(ZM_DB::DbProvider& db){

  vector<ZM_Base::manager> manags;
  if (db.getAllManagers(manags)){
    for (auto& m : manags){
      _managers[m.connectPnt] = m; 
    }   
  }
}