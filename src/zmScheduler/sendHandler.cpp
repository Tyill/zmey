
#include <string>
#include <system_error>
#include "zmCommon/serial.h"

using namespace std;

void sendHandler(const string& cp, const string& data, const std::error_code& ec){

  if (ec){
    auto mess = ZM_Aux::deserialn(data);
    
    
  }
}