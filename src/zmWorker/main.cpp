//
// zmey Project
// Copyright (C) 2018 by Contributors <https://github.com/Tyill/zmey>
//
// This code is licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include <signal.h>
#include <map>
#include <algorithm>
#include <iostream>
#include <list>
#include <mutex> 
#include "zmCommon/tcp.h"
#include "zmCommon/timerDelay.h"
#include "zmCommon/auxFunc.h"
#include "zmCommon/logger.h"
#include "zmCommon/queue.h"
#include "structurs.h"
#include "process.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void sendMessToSchedr(const ZM_Base::worker&, const std::string& schedrConnPnt, const mess2schedr&);
void progressToSchedr(const ZM_Base::worker&, const std::string& schedrConnPnt, const list<Process>&);
void pingToSchedr(const ZM_Base::worker&, const std::string& schedrConnPnt);
void updateListTasks(ZM_Aux::QueueThrSave<wTask>& newTasks, list<Process>& procs);
void waitProcess(list<Process>& procs, ZM_Aux::QueueThrSave<mess2schedr>& messForSchedr);

unique_ptr<ZM_Aux::Logger> _pLog = nullptr;
ZM_Aux::QueueThrSave<mess2schedr> _messForSchedr;
ZM_Aux::QueueThrSave<wTask> _newTasks;
list<Process> _procs;
bool _fClose = false,
     _isSendAck = true;
mutex _mtx;

struct config{
  bool logEna = false;
  int capacityTask = 10;
  int checkTasksTOutSec = 120;
  int progressTasksTOutSec = 30;
  int pingSchedrTOutSec = 20; 
  std::string connectPnt = "localhost:4146";
  std::string schedrConnPnt;
};

void statusMess(const string& mess){
  cout << ZM_Aux::currDateTime() << " " << mess << std::endl;
  if (_pLog){
    _pLog->writeMess(mess);
  }
}

void parseArgs(int argc, char* argv[], config& outCng){
  
  string sargs;
  for (int i = 1; i < argc; ++i){
    sargs += argv[i];
  }
  map<string, string> sprms;
  auto argPair = ZM_Aux::split(sargs, "-");
  for (auto& arg : argPair){
    size_t sp = arg.find_first_of("=");
    if (sp != std::string::npos){
      sprms[ZM_Aux::trim(arg.substr(0, sp))] = ZM_Aux::trim(arg.substr(sp + 1));
    }else{
      sprms[ZM_Aux::trim(arg)] = "";
    }
  }
  if (sprms.find("log") != sprms.end()){
    outCng.logEna = true;
  }  
#define SET_PARAM(nm, prm) \
  if (sprms.find(#nm) != sprms.end()){ \
    outCng.prm = sprms[#nm]; \
  }
  SET_PARAM(cp,  connectPnt);
  SET_PARAM(scp, schedrConnPnt);

#define SET_PARAM_NUM(nm, prm) \
  if (sprms.find(#nm) != sprms.end() && ZM_Aux::isNumber(sprms[#nm])){ \
    outCng.prm = stoi(sprms[#nm]); \
  }
  SET_PARAM_NUM(ctk, capacityTask);
  SET_PARAM_NUM(cht, checkTasksTOutSec);
  SET_PARAM_NUM(prg, progressTasksTOutSec);
  SET_PARAM_NUM(png, pingSchedrTOutSec);

#undef SET_PARAM
#undef SET_PARAM_NUM
}

void closeHandler(int sig){
  _fClose = true;
}

int main(int argc, char* argv[]){

  config cng;
  parseArgs(argc, argv, cng); 
  if (cng.logEna){
    _pLog = unique_ptr<ZM_Aux::Logger>(new ZM_Aux::Logger("zmWorker.log", ""));
  }    
  if (cng.schedrConnPnt.empty()){
    statusMess("Not set param '-scp' - scheduler connPnt");
    return -1;
  }  
  
  // signal(SIGHUP, initHandler);
  signal(SIGINT, closeHandler);
  signal(SIGTERM, closeHandler);
  signal(SIGQUIT, closeHandler);

  // on start
  _messForSchedr.push(mess2schedr{0, ZM_Base::messType::justStartWorker});

  // TCP server
  string err;
  if (ZM_Tcp::startServer(cng.connectPnt, err, 1)){
    ZM_Tcp::setReceiveCBack(receiveHandler);
    ZM_Tcp::setStsSendCBack(sendHandler);
    statusMess("Tcp server running: " + cng.connectPnt);
  }else{
    statusMess("Tcp server error, busy: " + cng.connectPnt + " " + err);
    return -1;
  }
  ZM_Base::worker worker;
  worker.connectPnt = cng.connectPnt;
    
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 10;
   
  // main cycle
  while (!_fClose){
    timer.updateCycTime();   

    // send mess to schedr (send constantly until it receives)
    if (_isSendAck && !_messForSchedr.empty()){ 
      _isSendAck = false;
      sendMessToSchedr(worker, cng.schedrConnPnt, _messForSchedr.front());
    }
    // update list of tasks
    updateListTasks(_newTasks, _procs);
    worker.activeTask = _procs.size();

    // progress of tasks
    if(timer.onDelTmSec(true, cng.progressTasksTOutSec, 0)){
      timer.onDelTmSec(false, cng.progressTasksTOutSec, 0);
      progressToSchedr(worker, cng.schedrConnPnt, _procs);
    }
    // ping to schedr
    if(timer.onDelTmSec(true, cng.pingSchedrTOutSec, 1)){
      timer.onDelTmSec(false, cng.pingSchedrTOutSec, 1);
      pingToSchedr(worker, cng.schedrConnPnt);
    }
    // check child process
    waitProcess(_procs, _messForSchedr);
    
    // added delay
    if (timer.getDeltaTimeMS() < minCycleTimeMS){
      ZM_Aux::sleepMs(minCycleTimeMS - timer.getDeltaTimeMS());
    }    
  } 
  ZM_Tcp::stopServer();
  return 0;
}