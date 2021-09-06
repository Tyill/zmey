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
#include "zmey_client.h"
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
  vector<uint64_t*> id;
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
  
  ZM_DB::ConnectCng connCng{ cng.connectStr};
  ZM_DB::DbProvider* pDb = new ZM_DB::DbProvider(connCng);

  auto serr = pDb->getLastError();
  if (!serr.empty()){
    if (err){
      strncpy(err, serr.c_str(), 256);
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
    delete static_cast<ZM_DB::DbProvider*>(zo);
  }
}
bool zmCreateTables(zmConn zo){
  if (!zo) return false;

  return static_cast<ZM_DB::DbProvider*>(zo)->createTables();
}
bool zmSetErrorCBack(zmConn zo, zmErrorCBack ecb, zmUData ud){
  if (!zo) return false;

  static_cast<ZM_DB::DbProvider*>(zo)->setErrorCBack(ecb, ud);
  return true;
}
bool zmGetLastError(zmConn zo, char* err/*sz 256*/){
  if (zo && err){
    strncpy(err, static_cast<ZM_DB::DbProvider*>(zo)->getLastError().c_str(), 256);
    return true;
  }else{
    return false;
  }
}
///////////////////////////////////////////////////////////////////////////////
/// Scheduler

bool zmAddScheduler(zmConn zo, zmSchedr cng, uint64_t* outSchId){
  if (!zo) return false;
  
  if (!outSchId){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddScheduler error: !outSchId");
     return false;
  }
  ZM_Base::Scheduler scng;
  scng.activeTask = 0;
  scng.capacityTask = cng.capacityTask;
  scng.connectPnt = cng.connectPnt;
  scng.state = ZM_Base::StateType::STOP;
  scng.name = cng.name;
  scng.description = cng.description ? cng.description : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->addSchedr(scng, *outSchId);
}
bool zmGetScheduler(zmConn zo, uint64_t sId, zmSchedr* outCng){
  if (!zo) return false; 

  if (!outCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetSchedulerCng error: !outCng");
     return false;
  }
  ZM_Base::Scheduler scng;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getSchedr(sId, scng)){    
    outCng->capacityTask = scng.capacityTask;
    strcpy(outCng->connectPnt, scng.connectPnt.c_str());
    strncpy(outCng->name, scng.name.c_str(), 256);
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
bool zmChangeScheduler(zmConn zo, uint64_t sId, zmSchedr newCng){
  if (!zo) return false; 
  
  ZM_Base::Scheduler scng;
  scng.capacityTask = newCng.capacityTask;
  scng.connectPnt = newCng.connectPnt;
  scng.name = newCng.name;
  scng.description = newCng.description ? newCng.description : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->changeSchedr(sId, scng);
}
bool zmDelScheduler(zmConn zo, uint64_t sId){
  if (!zo) return false;
 
  return static_cast<ZM_DB::DbProvider*>(zo)->delSchedr(sId);
}
bool zmStartScheduler(zmConn zo, uint64_t sId){
  if (!zo) return false; 
    
  auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            {"command", to_string((int)ZM_Base::MessType::START_AFTER_PAUSE_SCHEDR)},
            {"connectPnt", connCng.connectStr}
          };
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && ZM_Tcp::syncSendData(cng.connectPnt, ZM_Aux::serialn(data));
}
bool zmPauseScheduler(zmConn zo, uint64_t sId){
  if (!zo) return false; 
  
  auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            {"command", to_string((int)ZM_Base::MessType::PAUSE_SCHEDR)},
            {"connectPnt", connCng.connectStr}
          };
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && ZM_Tcp::syncSendData(cng.connectPnt, ZM_Aux::serialn(data));
}
bool zmPingScheduler(zmConn zo, uint64_t sId){
  if (!zo) return false; 
  
  auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            {"command", to_string((int)ZM_Base::MessType::PING_SCHEDR)},
            {"connectPnt", connCng.connectStr}
          };
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && ZM_Tcp::syncSendData(cng.connectPnt, ZM_Aux::serialn(data));
}
bool zmSchedulerState(zmConn zo, uint64_t sId, zmStateType* outState){
  if (!zo) return false; 

  if (!outState){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmSchedulerState error: !outState");
     return false;
  }
  ZM_Base::StateType state;
  if (static_cast<ZM_DB::DbProvider*>(zo)->schedrState(sId, state)){    
    *outState = (zmey::zmStateType)state;
    return true;
  }
  return false;
}
uint32_t zmGetAllSchedulers(zmConn zo, zmStateType state, uint64_t** outSchId){
  if (!zo) return 0; 

  auto schedrs = static_cast<ZM_DB::DbProvider*>(zo)->getAllSchedrs((ZM_Base::StateType)state);
  size_t ssz = schedrs.size();
  if (ssz > 0){
    *outSchId = (uint64_t*)realloc(*outSchId, ssz * sizeof(uint64_t));
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].id.push_back(*outSchId);
    }
    memcpy(*outSchId, schedrs.data(), ssz * sizeof(uint64_t));
  }else{
    *outSchId = nullptr;
  }
  return (uint32_t)ssz;
}

///////////////////////////////////////////////////////////////////////////////
/// Worker

bool zmAddWorker(zmConn zo, zmWorker cng, uint64_t* outWId){
  if (!zo) return false;

  if (!outWId){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddWorker error: !outWId");
     return false;
  }
  ZM_Base::Worker wcng;
  wcng.capacityTask = cng.capacityTask;
  wcng.connectPnt = cng.connectPnt;
  wcng.state = ZM_Base::StateType::STOP;
  wcng.sId = cng.sId;
  wcng.name = cng.name;
  wcng.description = cng.description ? cng.description : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->addWorker(wcng, *outWId);
}
bool zmGetWorker(zmConn zo, uint64_t wId, zmWorker* outWCng){
  if (!zo) return false; 

  if (!outWCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetWorkerCng error: !outWCng");
     return false;
  }
  ZM_Base::Worker wcng;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getWorker(wId, wcng)){    
    outWCng->sId = wcng.sId;
    outWCng->capacityTask = wcng.capacityTask;
    strncpy(outWCng->connectPnt, wcng.connectPnt.c_str(), 256);
    strncpy(outWCng->name, wcng.name.c_str(), 256);
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
bool zmChangeWorker(zmConn zo, uint64_t wId, zmWorker newCng){
  if (!zo) return false; 

  ZM_Base::Worker wcng;
  wcng.sId = newCng.sId;
  wcng.capacityTask = newCng.capacityTask;
  wcng.connectPnt = newCng.connectPnt;
  wcng.name = newCng.name;
  wcng.description = newCng.description ? newCng.description : "";
  
  return static_cast<ZM_DB::DbProvider*>(zo)->changeWorker(wId, wcng);
}
bool zmDelWorker(zmConn zo, uint64_t wId){
  if (!zo) return false;
 
  return static_cast<ZM_DB::DbProvider*>(zo)->delWorker(wId);
}
bool zmStartWorker(zmConn zo, uint64_t wId){
  if (!zo) return false; 
  
  uint64_t sId = 0; 
  zmWorker wcng;  
  zmSchedr scng; 
  auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
  if (zmGetWorker(zo, wId, &wcng) && zmGetScheduler(zo, wcng.sId, &scng)){
    map<string, string> data{
            {"command", to_string((int)ZM_Base::MessType::START_AFTER_PAUSE_WORKER)},
            {"connectPnt", connCng.connectStr},
            {"workerConnPnt", wcng.connectPnt}
          };
    return ZM_Tcp::syncSendData(scng.connectPnt, ZM_Aux::serialn(data));
  }else{
    return false;
  } 
}
bool zmPauseWorker(zmConn zo, uint64_t wId){
  if (!zo) return false; 
  
  uint64_t sId = 0; 
  zmWorker wcng;  
  zmSchedr scng; 
  auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
  if (zmGetWorker(zo, wId, &wcng) && zmGetScheduler(zo, wcng.sId, &scng)){
    map<string, string> data{
            {"command", to_string((int)ZM_Base::MessType::PAUSE_WORKER)},
            {"connectPnt", connCng.connectStr},
            {"workerConnPnt", wcng.connectPnt}
          };
    return ZM_Tcp::syncSendData(scng.connectPnt, ZM_Aux::serialn(data));
  }else{
    return false;
  } 
}
bool zmPingWorker(zmConn zo, uint64_t wId){
  if (!zo) return false; 
  
  auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            {"command", to_string((int)ZM_Base::MessType::PING_WORKER)},
            {"connectPnt", connCng.connectStr}
          };
  zmWorker cng;  
  return zmGetWorker(zo, wId, &cng) && ZM_Tcp::syncSendData(cng.connectPnt, ZM_Aux::serialn(data));
}
bool zmWorkerState(zmConn zo, uint64_t* pWId, uint32_t wCnt, zmStateType* outState){
  if (!zo) return false; 

  if (!outState){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmWorkerState error: !outState");
     return false;
  }
  vector<uint64_t> wId(wCnt);
  memcpy(wId.data(), pWId, wCnt * sizeof(uint64_t));
  vector<ZM_Base::StateType> state;
  if (static_cast<ZM_DB::DbProvider*>(zo)->workerState(wId, state)){    
    memcpy(outState, state.data(), wCnt * sizeof(ZM_Base::StateType));
    return true;
  }
  return false;
}
uint32_t zmGetAllWorkers(zmConn zo, uint64_t sId, zmStateType state, uint64_t** outWId){
  if (!zo) return 0; 

  auto workers = static_cast<ZM_DB::DbProvider*>(zo)->getAllWorkers(sId, (ZM_Base::StateType)state);
  size_t wsz = workers.size();
  if (wsz > 0){ 
    *outWId = (uint64_t*)realloc(*outWId, wsz * sizeof(uint64_t));
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].id.push_back(*outWId);
    }
    memcpy(*outWId, workers.data(), wsz * sizeof(uint64_t));
  }else{
    *outWId = nullptr;
  }
  return (uint32_t)wsz;
}

///////////////////////////////////////////////////////////////////////////////
/// Task template

bool zmAddTaskTemplate(zmConn zo, zmTaskTemplate cng, uint64_t* outTId){
  if (!zo) return false;

  if (!outTId || !cng.script){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTaskTemplate error: !outTId || !cng.script");
    return false;
  }
  ZM_Base::TaskTemplate task;
  task.name = cng.name;
  task.description = cng.description ? cng.description : "";
  task.uId = cng.userId;
  task.sId = cng.schedrPresetId;
  task.wId = cng.workerPresetId;
  task.averDurationSec = cng.averDurationSec;
  task.maxDurationSec = cng.maxDurationSec;
  task.script = cng.script;

  return static_cast<ZM_DB::DbProvider*>(zo)->addTaskTemplate(task, *outTId);
}
bool zmGetTaskTemplate(zmConn zo, uint64_t tId, zmTaskTemplate* outTCng){
  if (!zo) return false; 

  if (!outTCng){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetTaskTemplateCng error: !outTCng");
    return false;
  }
  ZM_Base::TaskTemplate task;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getTaskTemplate(tId, task)){  
    strcpy(outTCng->name, task.name.c_str());  
    outTCng->averDurationSec = task.averDurationSec;
    outTCng->maxDurationSec = task.maxDurationSec;
    outTCng->userId = task.uId;
    outTCng->schedrPresetId = task.sId;
    outTCng->workerPresetId = task.wId;
    outTCng->script = (char*)realloc(outTCng->script, task.script.size() + 1);
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].str.push_back(outTCng->script);
    }
    strcpy(outTCng->script, task.script.c_str());
    if (!task.description.empty()){
      outTCng->description = (char*)realloc(outTCng->description, task.description.size() + 1);
      {lock_guard<mutex> lk(m_mtxResources);
        m_resources[zo].str.push_back(outTCng->description);
      }
      strcpy(outTCng->description, task.description.c_str());
    }else{
      outTCng->description = nullptr;
    }
    return true;
  }
  return false;
}
bool zmChangeTaskTemplate(zmConn zo, uint64_t tId, zmTaskTemplate newCng){
  if (!zo) return false; 
  
  ZM_Base::TaskTemplate task;
  task.name = newCng.name;
  task.description = newCng.description ? newCng.description : "";
  task.uId = newCng.userId;
  task.sId = newCng.schedrPresetId;
  task.wId = newCng.workerPresetId;
  task.averDurationSec = newCng.averDurationSec;
  task.maxDurationSec = newCng.maxDurationSec;
  task.script = newCng.script;
  return static_cast<ZM_DB::DbProvider*>(zo)->changeTaskTemplate(tId, task);
}
bool zmDelTaskTemplate(zmConn zo, uint64_t tId){
  if (!zo) return false; 

  return static_cast<ZM_DB::DbProvider*>(zo)->delTaskTemplate(tId);
}
uint32_t zmGetAllTaskTemplates(zmConn zo, uint64_t userId, uint64_t** outTId){
  if (!zo) return 0; 

  auto tasks = static_cast<ZM_DB::DbProvider*>(zo)->getAllTaskTemplates(userId);
  size_t tsz = tasks.size();
  if (tsz > 0){
    *outTId = (uint64_t*)realloc(*outTId, tsz * sizeof(uint64_t));
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].id.push_back(*outTId);
    }
    memcpy(*outTId, tasks.data(), tsz * sizeof(uint64_t));
  }else{
    *outTId = nullptr;
  }
  return (uint32_t)tsz;
}

bool zmStartTask(zmConn zo, zmTask cng, uint64_t* tId){
  if (!zo || !tId) return false;

  string params = cng.params ? cng.params : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->startTask(cng.ttlId, params, *tId);
}
bool zmStopTask(zmConn zo, uint64_t tId){
  if (!zo) return false;
    
  ZM_Base::Worker wcng;  
  if (static_cast<ZM_DB::DbProvider*>(zo)->getWorkerByTask(tId, wcng)){
    auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
    map<string, string> data{
            {"command", to_string((int)ZM_Base::MessType::TASK_STOP)},
            {"connectPnt", connCng.connectStr},
            {"taskId", to_string(tId)}
          };

    return ZM_Tcp::syncSendData(wcng.connectPnt, ZM_Aux::serialn(data));
  }
  return false;
}
bool zmCancelTask(zmConn zo, uint64_t tId){
  if (!zo) return false;
  
  return static_cast<ZM_DB::DbProvider*>(zo)->cancelTask(tId);
}
bool zmPauseTask(zmConn zo, uint64_t tId){
  if (!zo) return false;
    
  ZM_Base::Worker wcng;  
  if (static_cast<ZM_DB::DbProvider*>(zo)->getWorkerByTask(tId, wcng)){
    auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
    map<string, string> data{
            {"command", to_string((int)ZM_Base::MessType::TASK_PAUSE)},
            {"connectPnt", connCng.connectStr},
            {"taskId", to_string(tId)}
          };

    return ZM_Tcp::syncSendData(wcng.connectPnt, ZM_Aux::serialn(data));
  }
  return false;
}
bool zmContinueTask(zmConn zo, uint64_t tId){
  if (!zo) return false;
    
  ZM_Base::Worker wcng;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getWorkerByTask(tId, wcng)){
    auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
    map<string, string> data{
            {"command", to_string((int)ZM_Base::MessType::TASK_CONTINUE)},
            {"connectPnt", connCng.connectStr},
            {"taskId", to_string(tId)}
          };

    return ZM_Tcp::syncSendData(wcng.connectPnt, ZM_Aux::serialn(data));
  }
  return false;
}
bool zmStateOfTask(zmConn zo, uint64_t* qtId, uint32_t tCnt, zmTaskState* outQTState){
  if (!zo) return false;
  
  if (!qtId || !outQTState){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmTaskState error: !qtId || !outQTState");
    return false;
  }
  vector<uint64_t> qtaskId(tCnt);
  memcpy(qtaskId.data(), qtId, tCnt * sizeof(uint64_t));
  vector<ZM_DB::TaskState> state;
  if (static_cast<ZM_DB::DbProvider*>(zo)->taskState(qtaskId, state)){  
    for (size_t i = 0; i < tCnt; ++i){
      outQTState[i].progress = state[i].progress;
      outQTState[i].state = (zmStateType)state[i].state;
    }    
    return true;
  }
  return false;
}
bool zmResultOfTask(zmConn zo, uint64_t tId, char** outTResult){
  if (!zo) return false;
  
  if (!outTResult){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmTaskResult error: !outTResult");
    return false;
  }
  string result;
  if (static_cast<ZM_DB::DbProvider*>(zo)->taskResult(tId, result)){  
    if (!result.empty()){ 
      *outTResult = (char*)realloc(*outTResult, result.size() + 1); 
      {lock_guard<mutex> lk(m_mtxResources);
        m_resources[zo].str.push_back(*outTResult);
      }
      strcpy(*outTResult, result.c_str());
    }else{
      *outTResult = nullptr;
    }
    return true;
  }
  return false;
}
bool zmTimeOfTask(zmConn zo, uint64_t tId, zmTaskTime* outTTime){
  if (!zo) return false;
  
  if (!outTTime){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmTaskTime error: !outTTime");
    return false;
  }
  ZM_DB::TaskTime tskTime;
  if (static_cast<ZM_DB::DbProvider*>(zo)->taskTime(tId, tskTime)){  
    strncpy(outTTime->createTime, tskTime.createTime.c_str(), 32);
    strncpy(outTTime->takeInWorkTime, tskTime.takeInWorkTime.c_str(), 32);
    strncpy(outTTime->startTime, tskTime.startTime.c_str(), 32);
    strncpy(outTTime->stopTime, tskTime.stopTime.c_str(), 32);
    return true;
  }
  return false;
}

bool zmSetChangeTaskStateCBack(zmConn zo, uint64_t tId, uint64_t userId, zmChangeTaskStateCBack cback, void* userData){
  if (!zo) return false; 

  return static_cast<ZM_DB::DbProvider*>(zo)->setChangeTaskStateCBack(tId, userId, (ZM_DB::ChangeTaskStateCBack)cback, userData);
}

///////////////////////////////////////////////////////////////////////////////
/// Internal errors

uint32_t zmGetInternErrors(zmConn zo, uint64_t sId, uint64_t wId, uint32_t mCnt, zmInternError* outErrors){
  if (!zo) return 0; 

  auto errs = static_cast<ZM_DB::DbProvider*>(zo)->getInternErrors(sId, wId, mCnt);
  size_t esz = errs.size();
  if ((esz > 0) && (esz <= mCnt)){
    for(int i = 0; i < esz; ++i){
      outErrors[i].sId = sId;
      outErrors[i].wId = wId;
      strncpy(outErrors[i].createTime, errs[i].createTime.c_str(), 32);
      strncpy(outErrors[i].message, errs[i].message.c_str(), 256);
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