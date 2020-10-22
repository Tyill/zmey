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
#include <unistd.h>
#include <map>
#include <algorithm>
#include <iostream>
#include <condition_variable>
#include <list>
#include <mutex>
#include "zmCommon/tcp.h"
#include "zmCommon/timerDelay.h"
#include "zmCommon/auxFunc.h"
#include "zmCommon/queue.h"
#include "zmCommon/serial.h"
#include "structurs.h"
#include "process.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void sendMessToSchedr(const ZM_Base::Worker&, const std::string& schedrConnPnt, const Mess2schedr&);
void progressToSchedr(const ZM_Base::Worker&, const std::string& schedrConnPnt, list<Process>&);
void pingToSchedr(const ZM_Base::Worker&, const std::string& schedrConnPnt);
void errorToSchedr(const ZM_Base::Worker&, const std::string& schedrConnPnt, ZM_Aux::QueueThrSave<string>& );
void updateListTasks(ZM_Base::Worker& iow, ZM_Aux::QueueThrSave<WTask>& newTasks, list<Process>& procs);
void waitProcess(ZM_Base::Worker&, list<Process>& procs, ZM_Aux::QueueThrSave<Mess2schedr>& messForSchedr);

ZM_Aux::QueueThrSave<Mess2schedr> _messForSchedr;
ZM_Aux::QueueThrSave<WTask> _newTasks;
ZM_Aux::QueueThrSave<string> _errMess;
list<Process> _procs;
mutex _mtxPrc, _mtxSts;
std::condition_variable _cv;
volatile bool _isSendAck = true,
              _isMainCycleRun = false;

struct Config{
  int progressTasksTOutSec = 30;
  int pingSchedrTOutSec = 20; 
  const int sendAckTOutSec = 1; 
  std::string connectPnt;
  std::string schedrConnPnt;
};

void mainCycleNotify(){
  if (!_isMainCycleRun){
    _isMainCycleRun = true;
    _cv.notify_one();
  }
}

void statusMess(const string& mess){
  lock_guard<std::mutex> lock(_mtxSts);
  cout << ZM_Aux::currDateTimeMs() << " " << mess << std::endl;
}

void parseArgs(int argc, char* argv[], Config& outCng){
  
  string sargs;
  for (int i = 1; i < argc; ++i){
    sargs += argv[i];
  }
  map<string, string> sprms;
  auto argPair = ZM_Aux::split(sargs, '-');
  for (auto& arg : argPair){
    arg = ZM_Aux::trim(arg);
    size_t sp = min(arg.find_first_of("="), arg.find_first_of(" "));
    if (sp != std::string::npos){
      sprms[ZM_Aux::trim(arg.substr(0, sp))] = ZM_Aux::trim(arg.substr(sp + 1));
    }else{
      sprms[ZM_Aux::trim(arg)] = "";
    }
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
  SET_PARAM_NUM(prg, progressTasksTOutSec);
  SET_PARAM_NUM(png, pingSchedrTOutSec);

#undef SET_PARAM
#undef SET_PARAM_NUM
}

#define CHECK(fun, mess) \
  if (fun){              \
    statusMess(mess);    \
    return -1;           \
  }

int main(int argc, char* argv[]){

  Config cng;
  parseArgs(argc, argv, cng); 

  CHECK(cng.connectPnt.empty(), "Not set param '-cp' - worker connection point: IP or DNS:port");
  CHECK(cng.schedrConnPnt.empty(), "Not set param '-scp' - scheduler connection point: IP or DNS:port");
    
  signal(SIGPIPE, SIG_IGN);

  // on start
  _messForSchedr.push(Mess2schedr{0, ZM_Base::MessType::JUST_START_WORKER});

  // TCP server
  ZM_Tcp::setReceiveCBack(receiveHandler);
  ZM_Tcp::setStsSendCBack(sendHandler);
  string err;
  CHECK(!ZM_Tcp::startServer(cng.connectPnt, err, 1), "Worker error: " + cng.connectPnt + " " + err);
  statusMess("Worker running: " + cng.connectPnt);
  
  ///////////////////////////////////////////////////////
  
  ZM_Base::Worker worker;
  worker.connectPnt = cng.connectPnt;
    
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 10;
  std::mutex mtxPause;
   
  // main cycle
  while (1){
    timer.updateCycTime();   

    // send mess to schedr (send constantly until it receives)
    if (_isSendAck && !_messForSchedr.empty()){ 
      _isSendAck = false;
      sendMessToSchedr(worker, cng.schedrConnPnt, _messForSchedr.front());
    }
    else if (!_isSendAck && timer.onDelaySec(true, cng.sendAckTOutSec, 0)){
      _isSendAck = true;
    } 
    // update list of tasks
    updateListTasks(worker, _newTasks, _procs);
    
    // progress of tasks
    if(timer.onDelaySec(true, cng.progressTasksTOutSec, 1)){
      timer.onDelaySec(false, cng.progressTasksTOutSec, 1);
      progressToSchedr(worker, cng.schedrConnPnt, _procs);
    }
    // ping to schedr
    if(timer.onDelaySec(true, cng.pingSchedrTOutSec, 2)){
      timer.onDelaySec(false, cng.pingSchedrTOutSec, 2);
      pingToSchedr(worker, cng.schedrConnPnt);
    }
    // check child process
    waitProcess(worker, _procs, _messForSchedr);
    
    // errors
    if (!_errMess.empty()){ 
      errorToSchedr(worker, cng.schedrConnPnt, _errMess);
    }    
    // added delay
    _isMainCycleRun = false;
    std::unique_lock<std::mutex> lck(mtxPause);
    _cv.wait_for(lck, std::chrono::milliseconds(minCycleTimeMS)); 
    _isMainCycleRun = true;      
  }
  return 0;
}