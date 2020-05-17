
#include <map>
#include <string>
#include <map>
#include "zmBase/structurs.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"

using namespace std;

void sendStatusToManagers(const map<std::string, ZM_Base::manager>& managers){
  map<string, string> sendData{
    make_pair("command", "statusScheduler")
  };      
  for (auto& m : managers){
    if (m.second.ste == ZM_Base::state::run){
      ZM_Tcp::sendData(m.second.connectPnt, ZM_Aux::serialn(sendData));
    }
  }
}