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
#include <cmath>

#include "zmCommon/tcp.h"
#include "zmCommon/timer_delay.h"
#include "zmCommon/aux_func.h"
#include "zmCommon/queue.h"
#include "zmCommon/serial.h"
#include "structurs.h"
#include "process.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void messageToSchedr(const ZM_Base::Worker&, const string& schedrConnPnt, ZM_Aux::Queue<MessForSchedr>& listMessForSchedr);
void progressToSchedr(const ZM_Base::Worker&, const string& schedrConnPnt, list<Process>&);
void pingToSchedr(const ZM_Base::Worker&, const string& schedrConnPnt);
void errorToSchedr(const ZM_Base::Worker&, const string& schedrConnPnt, ZM_Aux::Queue<string>& );
void updateListTasks(ZM_Aux::Queue<WTask>& newTasks, list<Process>& procs, ZM_Aux::Queue<MessForSchedr>& listMessForSchedr);
void waitProcess(ZM_Base::Worker&, list<Process>& procs, ZM_Aux::Queue<MessForSchedr>& listMessForSchedr);

ZM_Aux::Queue<MessForSchedr> g_listMessForSchedr;
ZM_Aux::Queue<WTask> g_newTasks;
ZM_Aux::Queue<string> g_errMess;
list<Process> g_procs;
mutex g_mtxProc;
static mutex m_mtxSts, m_mtxNotify;
static condition_variable m_cvStandUp;
static volatile bool m_fClose = false;

struct Config{
  int progressTasksTOutSec = 10;
  int pingSchedrTOutSec = 20; 
  const int checkLoadTOutSec = 1; 
  string localConnPnt;
  string remoteConnPnt;
  string schedrConnPnt;
};

void mainCycleNotify(int sig){
  m_cvStandUp.notify_one();
}

void mainCycleSleep(int delayMS){
  unique_lock<mutex> lck(m_mtxNotify);
  m_cvStandUp.wait_for(lck, chrono::milliseconds(delayMS)); 
}

void closeHandler(int sig){
  m_fClose = true;
}

void statusMess(const string& mess){
  lock_guard<mutex> lock(m_mtxSts);
  cout << ZM_Aux::currDateTimeMs() << " " << mess << std::endl;
}

void parseArgs(int argc, char* argv[], Config& outCng){
  
  map<string, string> sprms = ZM_Aux::parseCMDArgs(argc, argv);
  
  if (sprms.empty() || (sprms.cbegin()->first == "help")){
    cout << "Usage: --localAddr[-la] worker local connection point: IP or DNS:port. Required\n"
         << "       --remoteAddr[-ra] worker remote connection point (if from NAT): IP or DNS:port. Optional\n"
         << "       --schedrAddr[-sa] schedr remote connection point: IP or DNS:port. Required\n"
         << "       --progressTOut[-pt] send progress of tasks to schedr, sec. Default 10 sec\n"
         << "       --pingSchedrTOut[-st] send ping to schedr, sec. Default 20 sec\n";
    exit(0);  
  }

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
  signal(SIGCHLD, mainCycleNotify);
  signal(SIGTERM, closeHandler);
  signal(SIGHUP, closeHandler);
  signal(SIGQUIT, closeHandler);

  // on start
  g_listMessForSchedr.push(MessForSchedr{0, ZM_Base::MessType::JUST_START_WORKER});

  // TCP server
  ZM_Tcp::setReceiveCBack(receiveHandler);
  ZM_Tcp::setSendStatusCBack(sendHandler);
  ZM_Tcp::addPreConnectPnt(cng.schedrConnPnt);
  string err;
  CHECK(!ZM_Tcp::startServer(cng.localConnPnt, err, 1), "Worker error: " + cng.localConnPnt + " " + err);
  statusMess("Worker running: " + cng.localConnPnt);
  
  ///////////////////////////////////////////////////////
  ZM_Base::Worker worker;
  worker.connectPnt = cng.remoteConnPnt;
    
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 10;
   
  ZM_Aux::CPUData cpu;

  while (!m_fClose){
    timer.updateCycTime();   

    // check child process
    waitProcess(worker, g_procs, g_listMessForSchedr);

    updateListTasks(g_newTasks, g_procs, g_listMessForSchedr);

    if (!g_listMessForSchedr.empty()){ 
      worker.activeTask = g_newTasks.size() + g_procs.size();
      messageToSchedr(worker, cng.schedrConnPnt, g_listMessForSchedr);
    }

    if(timer.onDelayOncSec(true, cng.progressTasksTOutSec, 0)){
      progressToSchedr(worker, cng.schedrConnPnt, g_procs);
    }
    
    if(timer.onDelayOncSec(true, cng.checkLoadTOutSec, 1)){
      worker.load = cpu.load();
    } 
    
    if(timer.onDelayOncSec(true, cng.pingSchedrTOutSec, 2)){
      worker.activeTask = g_newTasks.size() + g_procs.size();
      pingToSchedr(worker, cng.schedrConnPnt);
    } 
         
    if (!g_errMess.empty()){ 
      errorToSchedr(worker, cng.schedrConnPnt, g_errMess);
    }    
    
    if (g_newTasks.empty()){
      mainCycleSleep(minCycleTimeMS);     
    }
  }
  ZM_Tcp::stopServer();
  return 0;
}