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
  std::string localConnPnt;
  std::string remoteConnPnt;
  std::string schedrConnPnt;
};

void mainCycleNotify(){
  if (!_isMainCycleRun){
    _isMainCycleRun = true;
    _cv.notify_one();
  }
}

void mainCycleSleep(int delayMS){
  _isMainCycleRun = false;
  std::mutex mtx;
  {std::unique_lock<std::mutex> lck(mtx);
    _cv.wait_for(lck, std::chrono::milliseconds(delayMS)); 
  }
  _isMainCycleRun = true;
}

void statusMess(const string& mess){
  lock_guard<std::mutex> lock(_mtxSts);
  cout << ZM_Aux::currDateTimeMs() << " " << mess << std::endl;
}

void parseArgs(int argc, char* argv[], Config& outCng){
  
  map<string, string> sprms = ZM_Aux::parseCMDArgs(argc, argv);
  
#define SET_PARAM(shortName, longName, prm)        \
  if (sprms.find(#longName) != sprms.end()){       \
    outCng.prm = sprms[#longName];                 \
  }                                                \
  else if (sprms.find(#shortName) != sprms.end()){ \
    outCng.prm = sprms[#shortName];                \
  }

  SET_PARAM(la, localAddr, localConnPnt); 
  SET_PARAM(ra, remoteAddr, remoteConnPnt);
  SET_PARAM(sa, schedrAddr, schedrConnPnt);

#define SET_PARAM_NUM(shortName, longName, prm)                                           \
  if (sprms.find(#longName) != sprms.end() && ZM_Aux::isNumber(sprms[#longName])){        \
    outCng.prm = stoi(sprms[#longName]);                                                  \
  }                                                                                       \
  else if (sprms.find(#shortName) != sprms.end() && ZM_Aux::isNumber(sprms[#shortName])){ \
    outCng.prm = stoi(sprms[#shortName]);                                                 \
  }

  SET_PARAM_NUM(pt, progressTOut, progressTasksTOutSec);
  SET_PARAM_NUM(st, pingSchedrTOut, pingSchedrTOutSec);
}

#define CHECK(fun, mess) \
  if (fun){              \
    statusMess(mess);    \
    return -1;           \
  }

int main(int argc, char* argv[]){

  Config cng;
  parseArgs(argc, argv, cng); 

  if (cng.remoteConnPnt.empty()){  // when without NAT
    cng.remoteConnPnt = cng.localConnPnt;
  } 

  CHECK(cng.localConnPnt.empty() || (ZM_Aux::split(cng.localConnPnt, ':').size() != 2), "Not set param '--localAddr[-la]' - worker local connection point: IP or DNS:port");
  CHECK(cng.remoteConnPnt.empty() || (ZM_Aux::split(cng.remoteConnPnt, ':').size() != 2), "Not set param '--remoteAddr[-ra]' - worker remote connection point: IP or DNS:port");
  CHECK(cng.schedrConnPnt.empty() || (ZM_Aux::split(cng.schedrConnPnt, ':').size() != 2), "Not set param '--schedrAddr[-sa]' - scheduler connection point: IP or DNS:port");
    
  signal(SIGPIPE, SIG_IGN);

  // on start
  _messForSchedr.push(Mess2schedr{0, ZM_Base::MessType::JUST_START_WORKER});

  // TCP server
  ZM_Tcp::setReceiveCBack(receiveHandler);
  ZM_Tcp::setStsSendCBack(sendHandler);
  string err;
  CHECK(!ZM_Tcp::startServer(cng.localConnPnt, err, 1), "Worker error: " + cng.localConnPnt + " " + err);
  statusMess("Worker running: " + cng.localConnPnt);
  
  ///////////////////////////////////////////////////////
  
  ZM_Base::Worker worker;
  worker.connectPnt = cng.remoteConnPnt;
    
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 10;
   
  ZM_Aux::CPUData cpu;

  // main cycle
  while (1){
    timer.updateCycTime();   

    // update list of tasks
    updateListTasks(worker, _newTasks, _procs);

    // check child process
    waitProcess(worker, _procs, _messForSchedr);

    // send mess to schedr (send constantly until it receives)
    Mess2schedr mess;
    if (_isSendAck && _messForSchedr.front(mess)){ 
      _isSendAck = false;
      sendMessToSchedr(worker, cng.schedrConnPnt, mess);
    }
    else if (!_isSendAck && timer.onDelaySec(true, cng.sendAckTOutSec, 0)){
      _isSendAck = true;
    }

    // progress of tasks
    if(timer.onDelayOncSec(true, cng.progressTasksTOutSec, 1)){
      progressToSchedr(worker, cng.schedrConnPnt, _procs);
    }
    // load CPU
    if(timer.onDelayOncSec(true, 1, 2)){
      worker.load = cpu.load();
    } 
    // ping to schedr
    if(timer.onDelayOncSec(true, cng.pingSchedrTOutSec, 3)){
      pingToSchedr(worker, cng.schedrConnPnt);
    }        
    // errors
    if (!_errMess.empty()){ 
      errorToSchedr(worker, cng.schedrConnPnt, _errMess);
    }    
    // added delay
    if (_newTasks.empty()){
      mainCycleSleep(minCycleTimeMS);     
    }
  }
  return 0;
}