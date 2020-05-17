#include "zmDbProvider/dbProvider.h"
#include "zmCommon/queue.h"

using namespace std;

extern ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
extern ZM_Base::scheduler _schedr;

void sendAllMessToDB(ZM_DB::DbProvider& db){

  vector<ZM_DB::messSchedr> mess;
  ZM_DB::messSchedr m;
  while(_messToDB.tryPop(m)){
    mess.push_back(m);
  }
  
  db.sendAllMessFromSchedr(_schedr.id, mess);
}