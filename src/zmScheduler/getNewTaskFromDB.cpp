#include "zmDbProvider/dbProvider.h"
#include "zmBase/structurs.h"
#include "zmCommon/queue.h"

using namespace std;

extern ZM_Base::scheduler _schedr;
extern ZM_Aux::QueueThrSave<ZM_Base::task> _tasks;

void getNewTaskFromDB(ZM_DB::DbProvider& db){

  size_t cSz = _tasks.size(),
         capSz = _schedr.capasityTask;
  vector<ZM_Base::task> tasks;
  if (db.getNewTasks(tasks)){
    for(auto& t : tasks){
      if (cSz < capSz){
        _tasks.push(move(t));
        ++cSz;      
      }else
        break;
    }
    _schedr.activeTask = cSz;
  }
};