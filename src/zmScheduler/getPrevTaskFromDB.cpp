#include "zmDbProvider/dbProvider.h"
#include "zmBase/structurs.h"
#include "zmCommon/queue.h"

using namespace std;

void getPrevTaskFromDB(ZM_DB::DbProvider& db, 
                       ZM_Base::scheduler& schedr,
                       ZM_Aux::QueueThrSave<ZM_Base::task>& outTasks){

  vector<ZM_Base::task> tasks;
  if (db.getTasksForSchedr(schedr.id, tasks)){
    for(auto& t : tasks){
      outTasks.push(move(t));
    }
  }
};