
#include <string>
#include <map>
#include "zmBase/structurs.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"

using namespace std;


void sendTaskToWorker(const ZM_Base::worker& wr, const ZM_Base::task& t){
  map<string, string> sendData{
    make_pair("command", "newTask"),
    make_pair("taskId", to_string(t.id)),
    make_pair("params", t.params), 
    make_pair("script", t.script),
    make_pair("executor", to_string(int(t.exrType))),
    make_pair("meanDuration", t.meanDuration), 
    make_pair("maxDuration", t.maxDuration)
  };
  ZM_Tcp::sendData(wr.connectPnt, ZM_Aux::serialn(sendData));
}