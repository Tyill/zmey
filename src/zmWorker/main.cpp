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
#include <queue>
#include <iostream>
#include "zmCommon/tcp.h"
#include "zmCommon/timerDelay.h"
#include "zmCommon/auxFunc.h"
#include "zmBase/structurs.h"
#include "zmCommon/logger.h"
#include "zmCommon/queue.h"
#include "structurs.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void sendMessToSchedr(const message&);
void checkStatusTasks(const ZM_Base::worker&);

unique_ptr<ZM_Aux::Logger> _pLog = nullptr;
ZM_Aux::QueueThrSave<message> _messToSchedr;
ZM_Base::worker _worker;
bool _fClose = false;

struct params{
  bool logEna = false;
  int capasityTask = 10;
  int checkTasksTOutSec = 120; 
  string connectPnt = "localhost:4146";
  string schedrConnPnt;
};
params _prms;

void statusMess(const string& mess){
  cout << ZM_Aux::currDateTime() << mess << std::endl;
  if (_pLog)
    _pLog->writeMess(mess);
}

void parseArgs(int argc, char* argv[], params& outPrms){
  
  string sargs;
  for (int i = 1; i < argc; ++i){
    sargs += argv[i];
  }
  map<string, string> sprms;
  auto argPair = ZM_Aux::split(sargs, "-");
  size_t asz = argPair.size();
  for (size_t i = 0; i < asz; ++i){
    auto pm = ZM_Aux::split(argPair[i], "=");
    if (pm.size() <= 1){
      sprms[argPair[i]] = "";
    }else{
      sprms[pm[0]] = pm[1];
    }
  }
  if (sprms.find("logEna") != sprms.end()){
    outPrms.logEna = true;
  }
  if (sprms.find("connectPnt") != sprms.end()){
    outPrms.connectPnt = sprms["connectPnt"];
  }  
  if (sprms.find("schedrConnPnt") != sprms.end()){
    outPrms.schedrConnPnt = sprms["schedrConnPnt"];
  }
  if (sprms.find("capasityTask") != sprms.end() && ZM_Aux::isNumber(sprms["capasityTask"])){
    outPrms.capasityTask = stoi(sprms["capasityTask"]);
  }
  if (sprms.find("checkTasksTOutSec") != sprms.end() && ZM_Aux::isNumber(sprms["checkTasksTOutSec"])){
    outPrms.checkTasksTOutSec = stoi(sprms["checkTasksTOutSec"]);
  }    
}

void closeHandler(int sig){
  _fClose = true;
}

int main(int argc, char* argv[]){

  parseArgs(argc, argv, _prms);
  
  if (_prms.logEna){
    _pLog = unique_ptr<ZM_Aux::Logger>(new ZM_Aux::Logger("zmWorker.log", ""));
  }
  // signal(SIGHUP, initHandler);
  signal(SIGINT, closeHandler);
  signal(SIGTERM, closeHandler);

  // TCP server
  string err;
  if (ZM_Tcp::startServer(_prms.connectPnt, err, 1)){
    ZM_Tcp::setReceiveCBack(receiveHandler);
    ZM_Tcp::setStsSendCBack(sendHandler);
    statusMess("Tcp server running: " + _prms.connectPnt);
  }else{
    statusMess("Tcp server error, busy -connectPnt: " + _prms.connectPnt + " " + err);
    return -1;
  }
  
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 5;

  // main cycle
  int messIdMem = -1;
  while (!_fClose){
    timer.updateCycTime();   

    // check status of tasks
    checkStatusTasks(_worker);
    
    // send first mess to schedr (transfer constantly until it receives)
    auto firstMess = !_messToSchedr.empty() ? _messToSchedr.front() : message{messIdMem};
    if ((firstMess.id != messIdMem) || firstMess.isErrorSend){ 
      messIdMem = firstMess.id; 
      sendMessToSchedr(firstMess);
    }
   
    // added delay
    if (timer.getCTime() < minCycleTimeMS){
      ZM_Aux::sleepMs(minCycleTimeMS - timer.getCTime());
    }
  }  
  return 0;
}