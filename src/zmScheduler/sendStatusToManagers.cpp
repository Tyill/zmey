
#include <map>
#include <string>
#include <vector>
#include "zmBase/structurs.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"

using namespace std;

extern vector<ZM_Base::manager> _managers;

void sendStatusToManagers(){
  map<string, string> sendData{
    make_pair("command", "statusScheduler")
  };      
  for (auto& m : _managers){
    if (m.ste == ZM_Base::state::run){
      ZM_Tcp::sendData(m.connectPnt, ZM_Aux::serialn(sendData));
    }
  }
}