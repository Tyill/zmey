#include "zmDbProvider/dbProvider.h"
#include "zmCommon/queue.h"

using namespace std;

extern ZM_Aux::QueueThrSave<ZM_DB::message> _messToDB;
extern ZM_Base::scheduler _schedr;

void sendAllMessToDB(ZM_DB::DbProvider& db){

  vector<ZM_DB::message> mess;
  ZM_DB::message m;
  while(_messToDB.tryPop(m)){
    mess.push_back(m);
  }
  
  db.sendAllMessFromSchedr(_schedr.id, mess);
}