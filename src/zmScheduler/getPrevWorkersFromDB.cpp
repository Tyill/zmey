#include <unordered_map>
#include "zmDbProvider/dbProvider.h"
#include "zmBase/structurs.h"

using namespace std; 

void getPrevWorkersFromDB(ZM_DB::DbProvider& db, 
                          ZM_Base::scheduler& schedr,
                          unordered_map<std::string, ZM_Base::worker>& outWorkers){
  
  vector<ZM_Base::worker> workers; 
  if (db.getWorkersForSchedr(schedr.id, workers)){
    for(auto& w : workers){
      outWorkers[w.connectPnt] = w;
    }
  }
}