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
#ifdef __linux__
#include <unistd.h>
#endif
#include <future>
#include <condition_variable>
#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>

#include "zmCommon/tcp.h"
#include "zmCommon/timer_delay.h"
#include "zmCommon/queue.h"
#include "zmCommon/aux_func.h"
#include "zmDbProvider/db_provider.h"
#include "structurs.h"

using namespace std;

void receiveHandler(const string& cp, const string& data);
void sendHandler(const string& cp, const string& data, const std::error_code& ec);
void getNewTaskFromDB(ZM_DB::DbProvider& db);
bool sendTaskToWorker(const ZM_Base::Scheduler&, map<string, SWorker>&, ZM_Aux::Queue<ZM_Base::Task>&, ZM_Aux::Queue<ZM_DB::MessSchedr>& messToDB);
void sendAllMessToDB(ZM_DB::DbProvider& db);
void checkStatusWorkers(const ZM_Base::Scheduler&, map<string, SWorker>&, ZM_Aux::Queue<ZM_DB::MessSchedr>&);
void getPrevTaskFromDB(ZM_DB::DbProvider& db, const ZM_Base::Scheduler&,  ZM_Aux::Queue<ZM_Base::Task>&);
void getPrevWorkersFromDB(ZM_DB::DbProvider& db, const ZM_Base::Scheduler&, map<string, SWorker>&);

map<string, SWorker> g_workers;   // key - connectPnt
ZM_Aux::Queue<ZM_Base::Task> g_tasks;
ZM_Aux::Queue<ZM_DB::MessSchedr> g_messToDB;
ZM_Base::Scheduler g_schedr;
static mutex m_mtxSts, m_mtxNotify;
static condition_variable m_cvStandUp;
static volatile bool m_fClose = false;

struct Config{
  int checkWorkerTOutSec = 120;
  string localConnPnt;
  string remoteConnPnt;
  ZM_DB::ConnectCng dbConnCng;
};

void statusMess(const string& mess){
  lock_guard<mutex> lock(m_mtxSts);
  cout << ZM_Aux::currDateTimeMs() << " " << mess << std::endl;
}

void parseArgs(int argc, char* argv[], Config& outCng){ 
  
  map<string, string> sprms = ZM_Aux::parseCMDArgs(argc, argv);

  if (sprms.empty() || (sprms.cbegin()->first == "help")){
    cout << "Usage: --localAddr[-la] schedr local connection point: IP or DNS:port. Required\n"
         << "       --remoteAddr[-ra] schedr remote connection point (if from NAT): IP or DNS:port. Optional\n"
         << "       --dbConnStr[-db] database connection string\n"
         << "       --checkWorkerTOut[-cw] check ping from workers, sec. Default 120 sec\n";
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
  SET_PARAM(db, dbConnStr, dbConnCng.connectStr);
 
#define SET_PARAM_NUM(shortName, longName, prm)                                           \
  if (sprms.find(#longName) != sprms.end() && ZM_Aux::isNumber(sprms[#longName])){        \
    outCng.prm = stoi(sprms[#longName]);                                                  \
  }                                                                                       \
  else if (sprms.find(#shortName) != sprms.end() && ZM_Aux::isNumber(sprms[#shortName])){ \
    outCng.prm = stoi(sprms[#shortName]);                                                 \
  }

  SET_PARAM_NUM(cw, checkWorkerTOut, checkWorkerTOutSec);
}

void mainCycleNotify(){
  m_cvStandUp.notify_one();  
}

void mainCycleSleep(int delayMS){
  unique_lock<mutex> lck(m_mtxNotify);
  m_cvStandUp.wait_for(lck, chrono::milliseconds(delayMS)); 
}

void closeHandler(int sig){
  m_fClose = true;
}

unique_ptr<ZM_DB::DbProvider> 
createDbProvider(const Config& cng, string& err){
  unique_ptr<ZM_DB::DbProvider> db(new ZM_DB::DbProvider(cng.dbConnCng));
  err = db->getLastError();
  if (err.empty()){
    return db;
  } else{    
    return nullptr;
  }
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

  CHECK(cng.localConnPnt.empty() || (ZM_Aux::split(cng.localConnPnt, ':').size() != 2), "Not set param '--localAddr[-la]' - scheduler local connection point: IP or DNS:port");
  CHECK(cng.remoteConnPnt.empty() || (ZM_Aux::split(cng.remoteConnPnt, ':').size() != 2), "Not set param '--remoteAddr[-ra]' - scheduler remote connection point: IP or DNS:port");
  CHECK(cng.dbConnCng.connectStr.empty(), "Not set param '--dbConnStr[-db]' - database connection string");
   
  signal(SIGINT, closeHandler);
  signal(SIGTERM, closeHandler);
#ifdef __linux__
  signal(SIGHUP, closeHandler);
  signal(SIGQUIT, closeHandler);
  signal(SIGPIPE, SIG_IGN);
#endif
  
  // db providers
  string err;
  auto dbNewTask = createDbProvider(cng, err);
  auto dbSendMess = dbNewTask ? createDbProvider(cng, err) : nullptr;
  CHECK(!dbNewTask || !dbSendMess, "Schedr DB connect error " + err + ": " + cng.dbConnCng.connectStr); 
    
  // schedr from DB
  dbNewTask->getSchedr(cng.remoteConnPnt, g_schedr);
  CHECK(g_schedr.id == 0, "Schedr not found in DB for connectPnt " + cng.remoteConnPnt);
      
  // prev tasks and workers
  getPrevTaskFromDB(*dbNewTask, g_schedr, g_tasks);
  getPrevWorkersFromDB(*dbNewTask, g_schedr, g_workers);
 
  // TCP server
  ZM_Tcp::setReceiveCBack(receiveHandler);
  ZM_Tcp::setSendStatusCBack(sendHandler);
  for (auto& w : g_workers){
    ZM_Tcp::addPreConnectPnt(w.first);
  }  
  CHECK(!ZM_Tcp::startServer(cng.localConnPnt, err), "Schedr error: " + cng.localConnPnt + " " + err);
  statusMess("Schedr running: " + cng.localConnPnt);
  
  ///////////////////////////////////////////////////////

  future<void> frGetNewTask,
               frSendAllMessToDB; 
  ZM_Aux::TimerDelay timer;
  const int minCycleTimeMS = 10;

  // on start
  g_messToDB.push(ZM_DB::MessSchedr{ ZM_Base::MessType::START_SCHEDR });
  
  while (!m_fClose){
    timer.updateCycTime();   

    if((g_tasks.size() < g_schedr.capacityTask) && (g_schedr.state != ZM_Base::StateType::PAUSE)){
      if(!frGetNewTask.valid() || (frGetNewTask.wait_for(chrono::seconds(0)) == future_status::ready))
        frGetNewTask = async(launch::async, [&dbNewTask]{ getNewTaskFromDB(*dbNewTask); });                                        
    }        

    bool isAvailableWorkers = sendTaskToWorker(g_schedr, g_workers, g_tasks, g_messToDB);    

    if(!g_messToDB.empty()){   
      if(!frSendAllMessToDB.valid() || (frSendAllMessToDB.wait_for(chrono::seconds(0)) == future_status::ready))
        frSendAllMessToDB = async(launch::async, [&dbSendMess]{ sendAllMessToDB(*dbSendMess); });      
    }

    if(timer.onDelayOncSec(true, cng.checkWorkerTOutSec, 0)){
      checkStatusWorkers(g_schedr, g_workers, g_messToDB);
    }
    
    if (g_messToDB.empty() && (g_tasks.empty() || !isAvailableWorkers)){ 
      mainCycleSleep(minCycleTimeMS);
    }
  }

  ZM_Tcp::stopServer();
  g_messToDB.push(ZM_DB::MessSchedr{ ZM_Base::MessType::STOP_SCHEDR });
  sendAllMessToDB(*dbSendMess);
  return 0;
}

void getPrevTaskFromDB(ZM_DB::DbProvider& db, 
                       const ZM_Base::Scheduler& schedr,
                       ZM_Aux::Queue<ZM_Base::Task>& outTasks){
  vector<ZM_Base::Task> tasks;
  if (db.getTasksOfSchedr(schedr.id, tasks)){
    for(auto& t : tasks){
      outTasks.push(move(t));
    }
  }else{
    statusMess("getPrevTaskFromDB db error: " + db.getLastError());
  }
};

void getPrevWorkersFromDB(ZM_DB::DbProvider& db, 
                          const ZM_Base::Scheduler& schedr,
                          map<string, SWorker>& outWorkers){  
  vector<ZM_Base::Worker> workers; 
  if (db.getWorkersOfSchedr(schedr.id, workers)){
    for(auto& w : workers){
      outWorkers[w.connectPnt] = SWorker{w, w.state, w.state != ZM_Base::StateType::NOT_RESPONDING};
    }
  }else{
    statusMess("getPrevWorkersFromDB db error: " + db.getLastError());
  }
}