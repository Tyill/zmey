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
#include "zmclient.h"
#include "common/aux_func.h"
#include "common/tcp.h"
#include "common/serial.h"
#include "db_provider/db_provider.h"

#include <cstring>
#include <algorithm>
#include <mutex>

#define ZM_VERSION "1.0.0"

using namespace std;

namespace zmey{

struct AllocResource{
  vector<int*> id;
  vector<char*> str;
};
static map<zmConn, AllocResource> m_resources;
static mutex m_mtxResources;


void zmVersionLib(char* outVersion /*sz 8*/){
  if (outVersion){
    strcpy(outVersion, ZM_VERSION);
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Connection with DB

zmConn zmCreateConnection(zmConfig cng, char* err/*sz 256*/){
  
  DB::ConnectCng connCng{ cng.connectStr};
  DB::DbProvider* pDb = new DB::DbProvider(connCng);

  auto serr = pDb->getLastError();
  if (!serr.empty()){
    if (err){
      strncpy(err, serr.c_str(), 255);
    }
    delete pDb;
    return nullptr;
  }
  {lock_guard<mutex> lk(m_mtxResources);
    m_resources[pDb] = AllocResource();
  }
  return pDb;
}
void zmDisconnect(zmConn zo){
  if (zo){
    zmFreeResources(zo);
    delete static_cast<DB::DbProvider*>(zo);
  }
}
bool zmCreateTables(zmConn zo){
  if (!zo) return false;

  return static_cast<DB::DbProvider*>(zo)->createTables();
}
bool zmSetErrorCBack(zmConn zo, zmErrorCBack ecb, zmUData ud){
  if (!zo) return false;

  static_cast<DB::DbProvider*>(zo)->setErrorCBack(ecb, ud);
  return true;
}
bool zmGetLastError(zmConn zo, char* err/*sz 256*/){
  if (zo && err){
    strncpy(err, static_cast<DB::DbProvider*>(zo)->getLastError().c_str(), 255);
    return true;
  }else{
    return false;
  }
}
///////////////////////////////////////////////////////////////////////////////
/// Scheduler

bool zmAddScheduler(zmConn zo, zmSchedr cng, int* outSchId){
  if (!zo) return false;
  
  if (!outSchId){
     static_cast<DB::DbProvider*>(zo)->errorMess("zmAddScheduler error: !outSchId");
     return false;
  }
  base::Scheduler scng;
  scng.activeTask = 0;
  scng.capacityTask = cng.capacityTask;
  scng.connectPnt = cng.connectPnt;
  scng.state = base::StateType::STOP;
  scng.name = cng.name;
  scng.description = cng.description ? cng.description : "";

  return static_cast<DB::DbProvider*>(zo)->addSchedr(scng, *outSchId);
}
bool zmGetScheduler(zmConn zo, int sId, zmSchedr* outCng){
  if (!zo) return false; 

  if (!outCng){
     static_cast<DB::DbProvider*>(zo)->errorMess("zmGetSchedulerCng error: !outCng");
     return false;
  }
  base::Scheduler scng;
  if (static_cast<DB::DbProvider*>(zo)->getSchedr(sId, scng)){    
    outCng->capacityTask = scng.capacityTask;
    strcpy(outCng->connectPnt, scng.connectPnt.c_str());
    strncpy(outCng->name, scng.name.c_str(), 255);
    if (!scng.description.empty()){
      outCng->description = (char*)realloc(outCng->description, scng.description.size() + 1);
      {lock_guard<mutex> lk(m_mtxResources);
        m_resources[zo].str.push_back(outCng->description);
      }
      strcpy(outCng->description, scng.description.c_str());
    }else{
      outCng->description = nullptr;
    }
    return true;
  }
  return false;
}
bool zmChangeScheduler(zmConn zo, int sId, zmSchedr newCng){
  if (!zo) return false; 
  
  base::Scheduler scng;
  scng.capacityTask = newCng.capacityTask;
  scng.connectPnt = newCng.connectPnt;
  scng.name = newCng.name;
  scng.description = newCng.description ? newCng.description : "";

  return static_cast<DB::DbProvider*>(zo)->changeSchedr(sId, scng);
}
bool zmDelScheduler(zmConn zo, int sId){
  if (!zo) return false;
 
  return static_cast<DB::DbProvider*>(zo)->delSchedr(sId);
}
bool zmStartScheduler(zmConn zo, int sId){
  if (!zo) return false; 
    
  auto connCng = static_cast<DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            {"command", to_string((int)base::MessType::START_AFTER_PAUSE_SCHEDR)},
            {"connectPnt", connCng.connectStr}
          };
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && Tcp::syncSendData(cng.connectPnt, Aux::serialn(data));
}
bool zmPauseScheduler(zmConn zo, int sId){
  if (!zo) return false; 
  
  auto connCng = static_cast<DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            {"command", to_string((int)base::MessType::PAUSE_SCHEDR)},
            {"connectPnt", connCng.connectStr}
          };
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && Tcp::syncSendData(cng.connectPnt, Aux::serialn(data));
}
bool zmPingScheduler(zmConn zo, int sId){
  if (!zo) return false; 
  
  auto connCng = static_cast<DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            {"command", to_string((int)base::MessType::PING_SCHEDR)},
            {"connectPnt", connCng.connectStr}
          };
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && Tcp::syncSendData(cng.connectPnt, Aux::serialn(data));
}
bool zmStateOfScheduler(zmConn zo, int sId, zmSchedulerState* outState){
  if (!zo) return false; 

  if (!outState){
     static_cast<DB::DbProvider*>(zo)->errorMess("zmStateOfScheduler error: !outState");
     return false;
  }
  DB::SchedulerState state;
  if (static_cast<DB::DbProvider*>(zo)->schedrState(sId, state)){ 
    outState->state = (zmStateType)state.state;
    outState->activeTask = state.activeTask;
    strncpy(outState->startTime, state.startTime.c_str(), 31);
    strncpy(outState->stopTime, state.stopTime.c_str(), 31);
    strncpy(outState->pingTime, state.pingTime.c_str(), 31);
    return true;
  }
  return false;
}
uint32_t zmGetAllSchedulers(zmConn zo, zmStateType state, int** outSchId){
  if (!zo) return 0; 

  auto schedrs = static_cast<DB::DbProvider*>(zo)->getAllSchedrs((base::StateType)state);
  size_t ssz = schedrs.size();
  if (ssz > 0){
    *outSchId = (int*)realloc(*outSchId, ssz * sizeof(int));
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].id.push_back(*outSchId);
    }
    memcpy(*outSchId, schedrs.data(), ssz * sizeof(int));
  }else{
    *outSchId = nullptr;
  }
  return (uint32_t)ssz;
}

///////////////////////////////////////////////////////////////////////////////
/// Worker

bool zmAddWorker(zmConn zo, zmWorker cng, int* outWId){
  if (!zo) return false;

  if (!outWId){
     static_cast<DB::DbProvider*>(zo)->errorMess("zmAddWorker error: !outWId");
     return false;
  }
  base::Worker wcng;
  wcng.capacityTask = cng.capacityTask;
  wcng.connectPnt = cng.connectPnt;
  wcng.state = base::StateType::STOP;
  wcng.sId = cng.sId;
  wcng.name = cng.name;
  wcng.description = cng.description ? cng.description : "";

  return static_cast<DB::DbProvider*>(zo)->addWorker(wcng, *outWId);
}
bool zmGetWorker(zmConn zo, int wId, zmWorker* outWCng){
  if (!zo) return false; 

  if (!outWCng){
     static_cast<DB::DbProvider*>(zo)->errorMess("zmGetWorkerCng error: !outWCng");
     return false;
  }
  base::Worker wcng;
  if (static_cast<DB::DbProvider*>(zo)->getWorker(wId, wcng)){    
    outWCng->sId = wcng.sId;
    outWCng->capacityTask = wcng.capacityTask;
    strncpy(outWCng->connectPnt, wcng.connectPnt.c_str(), 255);
    strncpy(outWCng->name, wcng.name.c_str(), 255);
    if (!wcng.description.empty()){
      outWCng->description = (char*)realloc(outWCng->description, wcng.description.size() + 1);
      {lock_guard<mutex> lk(m_mtxResources);
        m_resources[zo].str.push_back(outWCng->description);
      }
      strcpy(outWCng->description, wcng.description.c_str());
    }else{
      outWCng->description = nullptr;
    }
    return true;
  }
  return false;
}
bool zmChangeWorker(zmConn zo, int wId, zmWorker newCng){
  if (!zo) return false; 

  base::Worker wcng;
  wcng.sId = newCng.sId;
  wcng.capacityTask = newCng.capacityTask;
  wcng.connectPnt = newCng.connectPnt;
  wcng.name = newCng.name;
  wcng.description = newCng.description ? newCng.description : "";
  
  return static_cast<DB::DbProvider*>(zo)->changeWorker(wId, wcng);
}
bool zmDelWorker(zmConn zo, int wId){
  if (!zo) return false;
 
  return static_cast<DB::DbProvider*>(zo)->delWorker(wId);
}
bool zmStartWorker(zmConn zo, int wId){
  if (!zo) return false; 
  
  int sId = 0; 
  zmWorker wcng;  
  zmSchedr scng; 
  auto connCng = static_cast<DB::DbProvider*>(zo)->getConnectCng();  
  if (zmGetWorker(zo, wId, &wcng) && zmGetScheduler(zo, wcng.sId, &scng)){
    map<string, string> data{
            {"command", to_string((int)base::MessType::START_AFTER_PAUSE_WORKER)},
            {"connectPnt", connCng.connectStr},
            {"workerConnPnt", wcng.connectPnt}
          };
    return Tcp::syncSendData(scng.connectPnt, Aux::serialn(data));
  }else{
    return false;
  } 
}
bool zmPauseWorker(zmConn zo, int wId){
  if (!zo) return false; 
  
  int sId = 0; 
  zmWorker wcng;  
  zmSchedr scng; 
  auto connCng = static_cast<DB::DbProvider*>(zo)->getConnectCng();  
  if (zmGetWorker(zo, wId, &wcng) && zmGetScheduler(zo, wcng.sId, &scng)){
    map<string, string> data{
            {"command", to_string((int)base::MessType::PAUSE_WORKER)},
            {"connectPnt", connCng.connectStr},
            {"workerConnPnt", wcng.connectPnt}
          };
    return Tcp::syncSendData(scng.connectPnt, Aux::serialn(data));
  }else{
    return false;
  } 
}
bool zmPingWorker(zmConn zo, int wId){
  if (!zo) return false; 
  
  auto connCng = static_cast<DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            {"command", to_string((int)base::MessType::PING_WORKER)},
            {"connectPnt", connCng.connectStr}
          };
  zmWorker cng;  
  return zmGetWorker(zo, wId, &cng) && Tcp::syncSendData(cng.connectPnt, Aux::serialn(data));
}
bool zmStateOfWorker(zmConn zo, int* pWId, uint32_t wCnt, zmWorkerState* outState){
  if (!zo) return false; 

  if (!outState){
     static_cast<DB::DbProvider*>(zo)->errorMess("zmWorkerState error: !outState");
     return false;
  }
  vector<int> wId(wCnt);
  memcpy(wId.data(), pWId, wCnt * sizeof(int));
  vector<DB::WorkerState> state;
  if (static_cast<DB::DbProvider*>(zo)->workerState(wId, state)){
    for (size_t i = 0; i < state.size(); ++i){
      outState[i].state = (zmStateType)state[i].state;
      outState[i].activeTask = state[i].activeTask;
      outState[i].load = state[i].load;
      strncpy(outState[i].startTime, state[i].startTime.c_str(), 31);
      strncpy(outState[i].stopTime, state[i].stopTime.c_str(), 31);
      strncpy(outState[i].pingTime, state[i].pingTime.c_str(), 31);
    }
    return true;
  }
  return false;
}
uint32_t zmGetAllWorkers(zmConn zo, int sId, zmStateType state, int** outWId){
  if (!zo) return 0; 

  auto workers = static_cast<DB::DbProvider*>(zo)->getAllWorkers(sId, (base::StateType)state);
  size_t wsz = workers.size();
  if (wsz > 0){ 
    *outWId = (int*)realloc(*outWId, wsz * sizeof(int));
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].id.push_back(*outWId);
    }
    memcpy(*outWId, workers.data(), wsz * sizeof(int));
  }else{
    *outWId = nullptr;
  }
  return (uint32_t)wsz;
}

bool zmStartTask(zmConn zo, zmTask cng, int* tId){
  if (!zo || !tId || !cng.scriptPath || !cng.resultPath) return false;

  base::Task task;
  task.wId = cng.workerPresetId;
  task.averDurationSec = cng.averDurationSec;
  task.maxDurationSec = cng.maxDurationSec;
  task.params = cng.params ? cng.params : "";
  task.scriptPath = cng.scriptPath;
  task.resultPath = cng.resultPath;

  return static_cast<DB::DbProvider*>(zo)->startTask(cng.schedrPresetId, task, *tId);
}
bool zmStopTask(zmConn zo, int tId){
  if (!zo) return false;
    
  base::Worker wcng;  
  if (static_cast<DB::DbProvider*>(zo)->getWorkerByTask(tId, wcng)){
    auto connCng = static_cast<DB::DbProvider*>(zo)->getConnectCng();  
    map<string, string> data{
            {"command", to_string((int)base::MessType::TASK_STOP)},
            {"connectPnt", connCng.connectStr},
            {"taskId", to_string(tId)}
          };

    return Tcp::syncSendData(wcng.connectPnt, Aux::serialn(data));
  }
  return false;
}
bool zmCancelTask(zmConn zo, int tId){
  if (!zo) return false;
  
  return static_cast<DB::DbProvider*>(zo)->cancelTask(tId);
}
bool zmPauseTask(zmConn zo, int tId){
  if (!zo) return false;
    
  base::Worker wcng;  
  if (static_cast<DB::DbProvider*>(zo)->getWorkerByTask(tId, wcng)){
    auto connCng = static_cast<DB::DbProvider*>(zo)->getConnectCng();  
    map<string, string> data{
            {"command", to_string((int)base::MessType::TASK_PAUSE)},
            {"connectPnt", connCng.connectStr},
            {"taskId", to_string(tId)}
          };

    return Tcp::syncSendData(wcng.connectPnt, Aux::serialn(data));
  }
  return false;
}
bool zmContinueTask(zmConn zo, int tId){
  if (!zo) return false;
    
  base::Worker wcng;
  if (static_cast<DB::DbProvider*>(zo)->getWorkerByTask(tId, wcng)){
    auto connCng = static_cast<DB::DbProvider*>(zo)->getConnectCng();  
    map<string, string> data{
            {"command", to_string((int)base::MessType::TASK_CONTINUE)},
            {"connectPnt", connCng.connectStr},
            {"taskId", to_string(tId)}
          };

    return Tcp::syncSendData(wcng.connectPnt, Aux::serialn(data));
  }
  return false;
}
bool zmStateOfTask(zmConn zo, int* qtId, uint32_t tCnt, zmTaskState* outQTState){
  if (!zo) return false;
  
  if (!qtId || !outQTState){
    static_cast<DB::DbProvider*>(zo)->errorMess("zmTaskState error: !qtId || !outQTState");
    return false;
  }
  vector<int> qtaskId(tCnt);
  memcpy(qtaskId.data(), qtId, tCnt * sizeof(int));
  vector<DB::TaskState> state;
  if (static_cast<DB::DbProvider*>(zo)->taskState(qtaskId, state)){  
    for (size_t i = 0; i < tCnt; ++i){
      outQTState[i].progress = state[i].progress;
      outQTState[i].state = (zmStateType)state[i].state;
    }    
    return true;
  }
  return false;
}
bool zmTimeOfTask(zmConn zo, int tId, zmTaskTime* outTTime){
  if (!zo) return false;
  
  if (!outTTime){
    static_cast<DB::DbProvider*>(zo)->errorMess("zmTaskTime error: !outTTime");
    return false;
  }
  DB::TaskTime tskTime;
  if (static_cast<DB::DbProvider*>(zo)->taskTime(tId, tskTime)){  
    strncpy(outTTime->createTime, tskTime.createTime.c_str(), 31);
    strncpy(outTTime->takeInWorkTime, tskTime.takeInWorkTime.c_str(), 31);
    strncpy(outTTime->startTime, tskTime.startTime.c_str(), 31);
    strncpy(outTTime->stopTime, tskTime.stopTime.c_str(), 31);
    return true;
  }
  return false;
}

bool zmSetChangeTaskStateCBack(zmConn zo, int tId, zmChangeTaskStateCBack cback, void* userData){
  if (!zo) return false; 

  return static_cast<DB::DbProvider*>(zo)->setChangeTaskStateCBack(tId, (DB::ChangeTaskStateCBack)cback, userData);
}

///////////////////////////////////////////////////////////////////////////////
/// Internal errors

uint32_t zmGetInternErrors(zmConn zo, int sId, int wId, uint32_t mCnt, zmInternError* outErrors){
  if (!zo) return 0; 

  auto errs = static_cast<DB::DbProvider*>(zo)->getInternErrors(sId, wId, mCnt);
  size_t esz = errs.size();
  if ((esz > 0) && (esz <= mCnt)){
    for(int i = 0; i < esz; ++i){
      outErrors[i].sId = sId;
      outErrors[i].wId = wId;
      strncpy(outErrors[i].createTime, errs[i].createTime.c_str(), 31);
      strncpy(outErrors[i].message, errs[i].message.c_str(), 255);
    }    
  }
  return (uint32_t)esz;
}

///////////////////////////////////////////////////////////////////////////////
/// free resouces
void zmFreeResources(zmConn zo){
  if (!zo) return;
  
  {lock_guard<mutex> lk(m_mtxResources);
    auto& idRes = m_resources[zo].id;
    if (!idRes.empty()){
      std::sort(idRes.begin(), idRes.end());
      idRes.resize(std::distance(idRes.begin(), std::unique(idRes.begin(), idRes.end())));
      for (auto pId : idRes){ 
        free(pId);
      }
      idRes.clear();
    }
    
    auto& strRes = m_resources[zo].str;
    if (!strRes.empty()){
      std::sort(strRes.begin(), strRes.end());
      strRes.resize(std::distance(strRes.begin(), std::unique(strRes.begin(), strRes.end())));
      for (auto pStr : strRes){
        free(pStr);   
      }
      strRes.clear(); 
    }
  }  
}
}