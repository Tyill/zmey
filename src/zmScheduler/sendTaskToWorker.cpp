
#include <string>
#include <map>
#include "zmBase/structurs.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"

using namespace std;


void sendTaskToWorker(const ZM_Base::task& t, const ZM_Base::worker& wr){
  map<string, string> sendData{
    make_pair("command", to_string((int)ZM_Base::messType::newTask)),
    make_pair("taskId", to_string(t.id)),
    make_pair("params", t.params), 
    make_pair("script", t.script),
    make_pair("executor", to_string(int(t.exrType))),
    make_pair("meanDuration", to_string(t.averDurationSec)), 
    make_pair("maxDuration", to_string(t.maxDurationSec))
  };
  ZM_Tcp::sendData(wr.connectPnt, ZM_Aux::serialn(sendData));
}