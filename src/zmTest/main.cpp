#include <iostream> 
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/auxFunc.h"


using namespace std;

void statusMess(const string& mess){
  cout << ZM_Aux::currDateTime() << " " << mess << std::endl;
}

int main(int argc, char* argv[]){

  ZM_DB::connectCng cng;

  ZM_DB::DbProvider* db = ZM_DB::makeDbProvider(cng);
  if (db && db->getLastError().empty()){
    statusMess(
      "DB connect success ");
  }else{
    statusMess(
      "DB connect error");
    return -1;
  }

  


 return 0;
}