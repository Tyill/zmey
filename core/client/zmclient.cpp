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
#include "common/misc.h"
#include "common/tcp.h"
#include "base/messages.h"
#include "db_provider/db_provider.h"

#include <cstring>
#include <algorithm>
#include <mutex>

using namespace std;

struct AllocResource{
  vector<int*> id;
  vector<char*> str;
};
static map<zmConn, AllocResource> m_resources;
static mutex m_mtxResources;

const char* zmVersionLib(){
  static const char* ZM_VERSION = "1.1.0";
  return ZM_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
/// Connection with db

zmConn zmCreateConnection(zmConfig cng, char* err/*sz 256*/){
  
  db::ConnectCng connCng{ cng.connectStr};
  db::DbProvider* pDb = new db::DbProvider(connCng);

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
    delete static_cast<db::DbProvider*>(zo);
  }
}
bool zmGetLastError(zmConn zo, char* err/*sz 256*/){
  if (zo && err){
    strncpy(err, static_cast<db::DbProvider*>(zo)->getLastError().c_str(), 255);
    return true;
  }else{
    return false;
  }
}
bool zmSetErrorCBack(zmConn zo, zmErrorCBack ecb, zmUData ud){
  if (zo && ecb){
    static_cast<db::DbProvider*>(zo)->setErrorCBack(ecb, ud);
    return true;
  }
  return false;
}
///////////////////////////////////////////////////////////////////////////////
/// Scheduler

bool zmAddScheduler(zmConn zo, zmSchedr cng, int* outSchId){
  if (!zo) return false;
  
  if (!outSchId){
    return false;
  }
  base::Scheduler scng;
  scng.sActiveTaskCount= 0;
  scng.sCapacityTaskCount = cng.capacityTask;
  scng.sConnectPnt = cng.connectPnt;
  scng.sState = base::StateType::STOP;

  return static_cast<db::DbProvider*>(zo)->addSchedr(scng, *outSchId);
}
bool zmGetScheduler(zmConn zo, int sId, zmSchedr* outCng){
  if (!zo) return false; 

  if (!outCng){
    return false;
  }
  base::Scheduler scng;
  if (static_cast<db::DbProvider*>(zo)->getSchedr(sId, scng)){    
    outCng->capacityTask = scng.sCapacityTaskCount;
    strcpy(outCng->connectPnt, scng.sConnectPnt.c_str());
    return true;
  }
  return false;
}
bool zmChangeScheduler(zmConn zo, int sId, zmSchedr newCng){
  if (!zo) return false; 
  
  base::Scheduler scng;
  scng.sCapacityTaskCount = newCng.capacityTask;
  scng.sConnectPnt = newCng.connectPnt;

  return static_cast<db::DbProvider*>(zo)->changeSchedr(sId, scng);
}
bool zmDelScheduler(zmConn zo, int sId){
  if (!zo) return false;
 
  return static_cast<db::DbProvider*>(zo)->delSchedr(sId);
}
bool zmStartScheduler(zmConn zo, int sId){
  if (!zo) return false; 
    
  auto connCng = static_cast<db::DbProvider*>(zo)->getConnectCng();  
  mess::InfoMess mess(mess::MessType::START_AFTER_PAUSE_SCHEDR, connCng.connectStr);
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && misc::syncSendData(cng.connectPnt, mess.serialn());
}
bool zmPauseScheduler(zmConn zo, int sId){
  if (!zo) return false; 
  
  auto connCng = static_cast<db::DbProvider*>(zo)->getConnectCng();  
  mess::InfoMess mess(mess::MessType::PAUSE_SCHEDR, connCng.connectStr);
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && misc::syncSendData(cng.connectPnt, mess.serialn());
}
bool zmPingScheduler(zmConn zo, int sId){
  if (!zo) return false; 
  
  auto connCng = static_cast<db::DbProvider*>(zo)->getConnectCng();  
  mess::InfoMess mess(mess::MessType::PING_SCHEDR, connCng.connectStr);
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && misc::syncSendData(cng.connectPnt, mess.serialn());
}
bool zmStateOfScheduler(zmConn zo, int sId, zmSchedulerState* outState){
  if (!zo) return false; 

  if (!outState){
    return false;
  }
  db::SchedulerState state;
  if (static_cast<db::DbProvider*>(zo)->schedrState(sId, state)){ 
    outState->state = (zmStateType)state.state;
    outState->activeTaskCount = state.activeTaskCount;
    strncpy(outState->startTime, state.startTime.c_str(), 31);
    strncpy(outState->stopTime, state.stopTime.c_str(), 31);
    strncpy(outState->pingTime, state.pingTime.c_str(), 31);
    return true;
  }
  return false;
}
int zmGetAllSchedulers(zmConn zo, zmStateType state, int** outSchId){
  if (!zo) return 0; 

  auto schedrs = static_cast<db::DbProvider*>(zo)->getAllSchedrs((base::StateType)state);
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
  return (int)ssz;
}

///////////////////////////////////////////////////////////////////////////////
/// Worker

bool zmAddWorker(zmConn zo, zmWorker cng, int* outWId){
  if (!zo) return false;

  if (!outWId){
    return false;
  }
  base::Worker wcng;
  wcng.wCapacityTaskCount = cng.capacityTask;
  wcng.wConnectPnt = cng.connectPnt;
  wcng.wState = base::StateType::STOP;
  wcng.sId = cng.schedrId;

  return static_cast<db::DbProvider*>(zo)->addWorker(wcng, *outWId);
}
bool zmGetWorker(zmConn zo, int wId, zmWorker* outWCng){
  if (!zo) return false; 

  if (!outWCng){
    return false;
  }
  base::Worker wcng;
  if (static_cast<db::DbProvider*>(zo)->getWorker(wId, wcng)){    
    outWCng->schedrId = wcng.sId;
    outWCng->capacityTask = wcng.wCapacityTaskCount;
    strncpy(outWCng->connectPnt, wcng.wConnectPnt.c_str(), 255);
    return true;
  }
  return false;
}
bool zmChangeWorker(zmConn zo, int wId, zmWorker newCng){
  if (!zo) return false; 

  base::Worker wcng;
  wcng.sId = newCng.schedrId;
  wcng.wCapacityTaskCount= newCng.capacityTask;
  wcng.wConnectPnt = newCng.connectPnt;
  
  return static_cast<db::DbProvider*>(zo)->changeWorker(wId, wcng);
}
bool zmDelWorker(zmConn zo, int wId){
  if (!zo) return false;
 
  return static_cast<db::DbProvider*>(zo)->delWorker(wId);
}
bool zmStartWorker(zmConn zo, int wId){
  if (!zo) return false; 
  
  zmWorker wcng;   
  zmSchedr scng; 
  if (zmGetWorker(zo, wId, &wcng) && zmGetScheduler(zo, wcng.schedrId, &scng)){
    mess::InfoMess mess(mess::MessType::START_AFTER_PAUSE_WORKER, wcng.connectPnt);
    return misc::syncSendData(scng.connectPnt, mess.serialn());
  }else{
    return false;
  } 
}
bool zmPauseWorker(zmConn zo, int wId){
  if (!zo) return false; 
  
  zmWorker wcng;  
  zmSchedr scng;  
  if (zmGetWorker(zo, wId, &wcng) && zmGetScheduler(zo, wcng.schedrId, &scng)){
    mess::InfoMess mess(mess::MessType::PAUSE_WORKER, wcng.connectPnt);
    return misc::syncSendData(scng.connectPnt, mess.serialn());
  }else{
    return false;
  } 
}
bool zmPingWorker(zmConn zo, int wId){
  if (!zo) return false; 
  
  zmWorker wcng;  
  if (zmGetWorker(zo, wId, &wcng)){
    mess::InfoMess mess(mess::MessType::PING_WORKER, wcng.connectPnt);
    return misc::syncSendData(wcng.connectPnt, mess.serialn());
  }else{
    return false;
  }
}
bool zmStateOfWorker(zmConn zo, int* pWId, int wCnt, zmWorkerState* outState){
  if (!zo) return false; 

  if (!outState){
    return false;
  }
  vector<int> wId(wCnt);
  memcpy(wId.data(), pWId, wCnt * sizeof(int));
  vector<db::WorkerState> state;
  if (static_cast<db::DbProvider*>(zo)->workerState(wId, state)){
    for (size_t i = 0; i < state.size(); ++i){
      outState[i].state = (zmStateType)state[i].state;
      outState[i].activeTaskCount = state[i].activeTaskCount;
      outState[i].load = state[i].load;
      strncpy(outState[i].startTime, state[i].startTime.c_str(), 31);
      strncpy(outState[i].stopTime, state[i].stopTime.c_str(), 31);
      strncpy(outState[i].pingTime, state[i].pingTime.c_str(), 31);
    }
    return true;
  }
  return false;
}
int zmGetAllWorkers(zmConn zo, int sId, zmStateType state, int** outWId){
  if (!zo) return 0; 

  auto workers = static_cast<db::DbProvider*>(zo)->getAllWorkers(sId, (base::StateType)state);
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
  return (int)wsz;
}

bool zmStartTask(zmConn zo, zmTask cng, int* tId){
  if (!zo || !tId || !cng.scriptPath || !cng.resultPath) return false;

  base::Task task;
  task.tWId= cng.workerPresetId;
  task.tAverDurationSec = cng.averDurationSec;
  task.tMaxDurationSec = cng.maxDurationSec;
  task.tParams = cng.params ? cng.params : "";
  task.tScriptPath = cng.scriptPath;
  task.tResultPath = cng.resultPath;

  return static_cast<db::DbProvider*>(zo)->startTask(cng.schedrPresetId, task, *tId);
}
bool zmStopTask(zmConn zo, int tId){
  if (!zo) return false;
    
  base::Worker wcng;  
  if (static_cast<db::DbProvider*>(zo)->getWorkerByTask(tId, wcng)){
    mess::TaskStatus mess(mess::MessType::TASK_STOP, wcng.wConnectPnt);
    mess.taskId = tId;
    return misc::syncSendData(wcng.wConnectPnt, mess.serialn());
  }
  return false;
}
bool zmCancelTask(zmConn zo, int tId){
  if (!zo) return false;
  
  return static_cast<db::DbProvider*>(zo)->cancelTask(tId);
}
bool zmPauseTask(zmConn zo, int tId){
  if (!zo) return false;
    
  base::Worker wcng;  
  if (static_cast<db::DbProvider*>(zo)->getWorkerByTask(tId, wcng)){
    mess::TaskStatus mess(mess::MessType::TASK_PAUSE, wcng.wConnectPnt);
    mess.taskId = tId;
    return misc::syncSendData(wcng.wConnectPnt, mess.serialn());
  }
  return false;
}
bool zmContinueTask(zmConn zo, int tId){
  if (!zo) return false;
    
  base::Worker wcng;  
  if (static_cast<db::DbProvider*>(zo)->getWorkerByTask(tId, wcng)){
    mess::TaskStatus mess(mess::MessType::TASK_CONTINUE, wcng.wConnectPnt);
    mess.taskId = tId;
    return misc::syncSendData(wcng.wConnectPnt, mess.serialn());
  }
  return false;
}
bool zmStateOfTask(zmConn zo, int* qtId, int tCnt, zmTaskState* outQTState){
  if (!zo) return false;
  
  if (!qtId || !outQTState){
    return false;
  }
  vector<int> qtaskId(tCnt);
  memcpy(qtaskId.data(), qtId, tCnt * sizeof(int));
  vector<db::TaskState> state;
  if (static_cast<db::DbProvider*>(zo)->taskState(qtaskId, state)){  
    for (size_t i = 0; i < tCnt; ++i){
      outQTState[i].state = (zmStateType)state[i].state;
      outQTState[i].progress = state[i].progress;
    }    
    return true;
  }
  return false;
}
bool zmTimeOfTask(zmConn zo, int tId, zmTaskTime* outTTime){
  if (!zo) return false;
  
  if (!outTTime){
    return false;
  }
  db::TaskTime tskTime;
  if (static_cast<db::DbProvider*>(zo)->taskTime(tId, tskTime)){  
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

  return static_cast<db::DbProvider*>(zo)->setChangeTaskStateCBack(tId, (db::ChangeTaskStateCBack)cback, userData);
}

///////////////////////////////////////////////////////////////////////////////
/// Internal errors

int zmGetInternErrors(zmConn zo, int sId, int wId, int mCnt, zmInternError* outErrors){
  if (!zo) return 0; 

  auto errs = static_cast<db::DbProvider*>(zo)->getInternErrors(sId, wId, mCnt);
  size_t esz = errs.size();
  if ((esz > 0) && (esz <= mCnt)){
    for(int i = 0; i < esz; ++i){
      outErrors[i].schedrId = sId;
      outErrors[i].workerId = wId;  
      strncpy(outErrors[i].createTime, errs[i].createTime.c_str(), 31);
      strncpy(outErrors[i].message, errs[i].message.c_str(), 255);
    }    
  }
  return (int)esz;
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