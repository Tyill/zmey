
#include <map>
#include <string>
#include <map>
#include "zmBase/structurs.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"

using namespace std;

extern map<string, ZM_Base::manager> _managers;

void sendStatusToManagers(){
  map<string, string> sendData{
    make_pair("command", "statusScheduler")
  };      
  for (auto& m : _managers){
    if (m.second.ste == ZM_Base::state::run){
      ZM_Tcp::sendData(m.second.connectPnt, ZM_Aux::serialn(sendData));
    }
  }
}