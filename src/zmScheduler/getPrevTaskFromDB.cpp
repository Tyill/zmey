#include "zmDbProvider/dbProvider.h"
#include "zmBase/structurs.h"
#include "zmCommon/queue.h"

using namespace std;

extern ZM_Base::scheduler _schedr;

void getPrevTaskFromDB(ZM_DB::DbProvider& db, ZM_Aux::QueueThrSave<ZM_Base::task>& outTasks){

  vector<ZM_Base::task> tasks;
  if (db.getPrevTasksForSchedr(_schedr.id, tasks)){
    for(auto& t : tasks){
      outTasks.push(move(t));
    }
  }
};