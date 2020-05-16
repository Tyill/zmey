#include "zmDbProvider/dbProvider.h"
#include "zmBase/structurs.h"

using namespace std; 

extern ZM_Base::scheduler _schedr;

void getPrevWorkersFromDB(ZM_DB::DbProvider& db, map<std::string, ZM_Base::worker>& outWorkers){
  
  vector<ZM_Base::worker> workers; 
  if (db.getPrevWorkersForSchedr(_schedr.id, workers)){
    for(auto& w : workers){
      outWorkers[w.connectPnt] = w;
    }
  }
}