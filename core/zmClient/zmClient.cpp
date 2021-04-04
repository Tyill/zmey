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
#include <cstring>
#include <algorithm>

#include "zmClient.h"
#include "zmCommon/aux_func.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"
#include "zmDbProvider/db_provider.h"

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
/// User

bool zmAddUser(zmConn zo, zmUser newUserCng, uint64_t* outUserId){
  if (!zo) return false;
  
  if (!outUserId){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddUser error: !outUserId");
     return false;
  }
  ZM_Base::User us;
  us.name = newUserCng.name;
  us.passw = newUserCng.passw;
  us.description = newUserCng.description ? newUserCng.description : "";
  
  return static_cast<ZM_DB::DbProvider*>(zo)->addUser(us, *outUserId);
}
bool zmGetUserId(zmConn zo, zmUser cng, uint64_t* outUserId){
  if (!zo) return false;
  
  if (!outUserId){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetUserId error: !outUserId");
     return false;
  }   
  return static_cast<ZM_DB::DbProvider*>(zo)->getUserId(string(cng.name), string(cng.passw), *outUserId);
}
bool zmGetUserCng(zmConn zo, uint64_t userId, zmUser* outUserCng){
  if (!zo) return false; 

  if (!outUserCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetUserCng error: !outUserCng");
     return false;
  }
  ZM_Base::User ur;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getUserCng(userId, ur)){    
    strcpy(outUserCng->name, ur.name.c_str());
    if (!ur.description.empty()){ 
      outUserCng->description = (char*)realloc(outUserCng->description, ur.description.size() + 1);
      {lock_guard<mutex> lk(m_mtxResources);
        m_resources[zo].str.push_back(outUserCng->description);
      }
      strcpy(outUserCng->description, ur.description.c_str());
    }else{
      outUserCng->description = nullptr;
    }  
    return true;
  }
  return false;
}
bool zmChangeUser(zmConn zo, uint64_t userId, zmUser newCng){
  if (!zo) return false;
     
  ZM_Base::User us;
  us.name = newCng.name;
  us.passw = newCng.passw;
  us.description = newCng.description ? newCng.description : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->changeUser(userId, us);
}
bool zmDelUser(zmConn zo, uint64_t userId){
  if (!zo) return false;
 
  return static_cast<ZM_DB::DbProvider*>(zo)->delUser(userId);
}
uint32_t zmGetAllUsers(zmConn zo, uint64_t** outUserId){
  if (!zo) return 0; 

  auto users = static_cast<ZM_DB::DbProvider*>(zo)->getAllUsers();
  size_t usz = users.size();
  if (usz > 0){
    *outUserId = (uint64_t*)realloc(*outUserId, usz * sizeof(uint64_t));
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].id.push_back(*outUserId);
    }
    memcpy(*outUserId, users.data(), usz * sizeof(uint64_t));
  }else{
    *outUserId = nullptr;
  }
  return (uint32_t)usz;
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
  scng.state = ZM_Base::StateType::READY;

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
    return true;
  }
  return false;
}
bool zmChangeScheduler(zmConn zo, uint64_t sId, zmSchedr newCng){
  if (!zo) return false; 
  
  ZM_Base::Scheduler scng;
  scng.capacityTask = newCng.capacityTask;
  scng.connectPnt = newCng.connectPnt;

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
  wcng.state = ZM_Base::StateType::READY;
  wcng.sId = cng.sId;

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
    strcpy(outWCng->connectPnt, wcng.connectPnt.c_str());
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
/// Pipeline of tasks

bool zmAddPipeline(zmConn zo, zmPipeline cng, uint64_t* outPPLId){
  if (!zo) return false;
  
  if (!outPPLId){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddPipeline error: !outPPLId");
     return false;
  }
  ZM_Base::UPipeline pp;
  pp.uId = cng.userId;
  pp.name = cng.name;
  pp.description = cng.description ? cng.description : "";
  
  return static_cast<ZM_DB::DbProvider*>(zo)->addPipeline(pp, *outPPLId);
}
bool zmGetPipeline(zmConn zo, uint64_t pplId, zmPipeline* outPPLCng){
  if (!zo) return false; 

  if (!outPPLCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetPipelineCng error: !outPPLCng");
     return false;
  }
  ZM_Base::UPipeline pp;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getPipeline(pplId, pp)){    
    outPPLCng->userId = pp.uId;
    strcpy(outPPLCng->name, pp.name.c_str());
    if (!pp.description.empty()){
      outPPLCng->description = (char*)realloc(outPPLCng->description, pp.description.size() + 1);
      {lock_guard<mutex> lk(m_mtxResources);
        m_resources[zo].str.push_back(outPPLCng->description);
      }
      strcpy(outPPLCng->description, pp.description.c_str());  
    }else{
      outPPLCng->description = nullptr;
    }
    return true;
  }
  return false;
}
bool zmChangePipeline(zmConn zo, uint64_t pplId, zmPipeline newCng){
  if (!zo) return false;
     
  ZM_Base::UPipeline pp;
  pp.uId = newCng.userId;
  pp.name = newCng.name;
  pp.description = newCng.description ? newCng.description : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->changePipeline(pplId, pp);
}
bool zmDelPipeline(zmConn zo, uint64_t pplId){
  if (!zo) return false;
 
  return static_cast<ZM_DB::DbProvider*>(zo)->delPipeline(pplId);
}
uint32_t zmGetAllPipelines(zmConn zo, uint64_t userId, uint64_t** outPPLId){
  if (!zo) return 0; 

  auto ppls = static_cast<ZM_DB::DbProvider*>(zo)->getAllPipelines(userId);
  size_t psz = ppls.size();
  if (psz > 0){
    *outPPLId = (uint64_t*)realloc(*outPPLId, psz * sizeof(uint64_t));
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].id.push_back(*outPPLId);
    }
    memcpy(*outPPLId, ppls.data(), psz * sizeof(uint64_t));
  }else{
    *outPPLId = nullptr;
  }
  return (uint32_t)psz;
}

///////////////////////////////////////////////////////////////////////////////
/// Group of tasks
bool zmAddGroup(zmConn zo, zmGroup cng, uint64_t* outGId){
  if (!zo) return false;
  
  if (!outGId){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddGroup error: !outGId");
     return false;
  }
  ZM_Base::UGroup gr;
  gr.pplId = cng.pplId;
  gr.name = cng.name;
  gr.description = cng.description ? cng.description : "";
  
  return static_cast<ZM_DB::DbProvider*>(zo)->addGroup(gr, *outGId);
}
bool zmGetGroup(zmConn zo, uint64_t gId, zmGroup* outGCng){
  if (!zo) return false; 

  if (!outGCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetGroup error: !outGCng");
     return false;
  }
  ZM_Base::UGroup gr;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getGroup(gId, gr)){    
    outGCng->pplId = gr.pplId;
    strcpy(outGCng->name, gr.name.c_str());
    if (!gr.description.empty()){
      outGCng->description = (char*)realloc(outGCng->description, gr.description.size() + 1);
      {lock_guard<mutex> lk(m_mtxResources);
        m_resources[zo].str.push_back(outGCng->description);
      }
      strcpy(outGCng->description, gr.description.c_str()); 
    }else{
      outGCng->description = nullptr;
    }
    return true;
  }
  return false;
}
bool zmChangeGroup(zmConn zo, uint64_t gId, zmGroup newCng){
  if (!zo) return false;
     
  ZM_Base::UGroup gr;
  gr.pplId = newCng.pplId;
  gr.name = newCng.name;
  gr.description = newCng.description ? newCng.description : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->changeGroup(gId, gr);
}
bool zmDelGroup(zmConn zo, uint64_t gId){
  if (!zo) return false;
 
  return static_cast<ZM_DB::DbProvider*>(zo)->delGroup(gId);
}
uint32_t zmGetAllGroups(zmConn zo, uint64_t pplId, uint64_t** outGId){
  if (!zo) return 0; 

  auto groups = static_cast<ZM_DB::DbProvider*>(zo)->getAllGroups(pplId);
  size_t gsz = groups.size();
  if (gsz > 0){
    *outGId = (uint64_t*)realloc(*outGId, gsz * sizeof(uint64_t));
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].id.push_back(*outGId);
    }
    memcpy(*outGId, groups.data(), gsz * sizeof(uint64_t));
  }else{
    *outGId = nullptr;
  }
  return (uint32_t)gsz;
}

///////////////////////////////////////////////////////////////////////////////
/// Task template

bool zmAddTaskTemplate(zmConn zo, zmTaskTemplate cng, uint64_t* outTId){
  if (!zo) return false;

  if (!outTId || !cng.script){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTaskTemplate error: !outTId || !cng.script");
    return false;
  }
  ZM_Base::UTaskTemplate task;
  task.name = cng.name;
  task.description = cng.description ? cng.description : "";
  task.uId = cng.userId;
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
  ZM_Base::UTaskTemplate task;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getTaskTemplate(tId, task)){  
    strcpy(outTCng->name, task.name.c_str());  
    outTCng->averDurationSec = task.averDurationSec;
    outTCng->maxDurationSec = task.maxDurationSec;
    outTCng->userId = task.uId;
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
  
  ZM_Base::UTaskTemplate task;
  task.name = newCng.name;
  task.description = newCng.description ? newCng.description : "";
  task.uId = newCng.userId;
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

///////////////////////////////////////////////////////////////////////////////
/// Task of pipeline

bool zmAddTaskPipeline(zmConn zo, zmTaskPipeline cng, uint64_t* outQTId){
  if (!zo) return false;

  if (!outQTId){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTask error: !outQTId");
    return false;
  }
  ZM_Base::UTaskPipeline task;
  task.pplId = cng.pplId;
  task.ttId = cng.ttId;
  task.gId = cng.gId;
  task.params = cng.params ? cng.params : "";
  task.priority = cng.priority;
    
  return static_cast<ZM_DB::DbProvider*>(zo)->addTaskPipeline(task, *outQTId);
}
bool zmGetTaskPipeline(zmConn zo, uint64_t ptId, zmTaskPipeline* outCng){
  if (!zo) return false;
  
  if (!outCng){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetTaskCng error: !outCng");
    return false;
  }
  ZM_Base::UTaskPipeline task;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getTaskPipeline(ptId, task)){
    outCng->pplId = task.pplId;
    outCng->gId = task.gId;
    outCng->ttId = task.ttId;
    outCng->priority = task.priority;    
    if(!task.params.empty()){
      outCng->params = (char*)realloc(outCng->params, task.params.size() + 1);
      {lock_guard<mutex> lk(m_mtxResources);
        m_resources[zo].str.push_back(outCng->params);
      }
      strcpy(outCng->params, task.params.c_str());  
    }else{
      outCng->params = nullptr;
    }
    return true;
  }
  return false;
}
bool zmChangeTaskPipeline(zmConn zo, uint64_t tId, zmTaskPipeline newCng){
  if (!zo) return false;
  
  ZM_Base::UTaskPipeline task;
  task.pplId = newCng.pplId;
  task.gId = newCng.gId;
  task.ttId = newCng.ttId;
  task.priority = newCng.priority;
  task.params = newCng.params ? newCng.params : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->changeTaskPipeline(tId, task);
}
bool zmDelTaskPipeline(zmConn zo, uint64_t tId){
  if (!zo) return false;

  return static_cast<ZM_DB::DbProvider*>(zo)->delTaskPipeline(tId);
}
uint32_t zmGetAllTasksPipeline(zmConn zo, uint64_t pplId, uint64_t** outQTId){
  if (!zo) return 0; 

  auto tasks = static_cast<ZM_DB::DbProvider*>(zo)->getAllTasksPipeline(pplId);
  size_t tsz = tasks.size();
  if (tsz > 0){
    *outQTId = (uint64_t*)realloc(*outQTId, tsz * sizeof(uint64_t));
    {lock_guard<mutex> lk(m_mtxResources);
      m_resources[zo].id.push_back(*outQTId);
    }
    memcpy(*outQTId, tasks.data(), tsz * sizeof(uint64_t));
  }else{
    *outQTId = nullptr;
  }
  return (uint32_t)tsz;
}

bool zmStartTask(zmConn zo, zmTask cng, uint64_t* tId){
  if (!zo || !tId) return false;

  string prTask = cng.prevTId ? cng.prevTId : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->startTask(cng.ptId, prTask, *tId);
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

bool zmSetEndTaskCBack(zmConn zo, uint64_t tId, zmChangeTaskStateCBack cback){
  if (!zo) return false; 

  return static_cast<ZM_DB::DbProvider*>(zo)->setChangeTaskStateCBack(tId, (ZM_DB::changeTaskStateCBack)cback);
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
    std::sort(idRes.begin(), idRes.end());
    idRes.resize(std::distance(idRes.begin(), std::unique(idRes.begin(), idRes.end())));
    for (auto pId : idRes){ 
      free(pId);
    }
    idRes.clear();

    auto& strRes = m_resources[zo].str;
    std::sort(strRes.begin(), strRes.end());
    strRes.resize(std::distance(strRes.begin(), std::unique(strRes.begin(), strRes.end())));
    for (auto pStr : strRes){
      free(pStr);   
    }
    strRes.clear(); 
  }  
}
}