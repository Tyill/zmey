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
#include "zmClient.h"
#include "zmCommon/auxFunc.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"
#include "zmDbProvider/dbProvider.h"

#define ZM_VERSION "1.0.0"

using namespace std;

namespace zmey{

void zmVersionLib(char* outVersion /*sz 8*/){
  if (outVersion){
    strcpy(outVersion, ZM_VERSION);
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Connection with DB

zmConn zmCreateConnection(zmConnect cng, char* err/*sz 256*/){
  
  ZM_DB::connectCng connCng{ (ZM_DB::dbType)cng.dbType,
                             cng.connectStr};
  ZM_DB::DbProvider* pDb = ZM_DB::makeDbProvider(connCng);

  if (pDb){
    auto serr = pDb->getLastError();
    if (!serr.empty()){
      if (err){
        strncpy(err, serr.c_str(), 256);
      }
      delete pDb;
      return nullptr;
    }
  }else{
    strcpy(err, ("zmCreateConnection error: not support dbType " + dbTypeToStr(connCng.selType)).c_str());
  }
  return pDb;
}
void zmDisconnect(zmConn zo){
  if (zo){
    delete static_cast<ZM_DB::DbProvider*>(zo);
  }
}
bool zmCreateTables(zmConn zo){
  if (!zo) return false;

  static_cast<ZM_DB::DbProvider*>(zo)->createTables();
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
  ZM_Base::user us;
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
  ZM_Base::user ur;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getUserCng(userId, ur)){    
    strcpy(outUserCng->name, ur.name.c_str());
    outUserCng->description = (char*)realloc(outUserCng->description, ur.description.size() + 1);
    strcpy(outUserCng->description, ur.description.c_str());   
    return true;
  }
  return false;
}
bool zmChangeUser(zmConn zo, uint64_t userId, zmUser newCng){
  if (!zo) return false;
     
  ZM_Base::user us;
  us.name = newCng.name;
  us.passw = newCng.passw;
  us.description = newCng.description;

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
  ZM_Base::scheduler scng;
  scng.activeTask = 0;
  scng.capacityTask = cng.capacityTask;
  scng.connectPnt = cng.connectPnt;
  scng.state = ZM_Base::stateType::ready;

  return static_cast<ZM_DB::DbProvider*>(zo)->addSchedr(scng, *outSchId);
}
bool zmGetScheduler(zmConn zo, uint64_t sId, zmSchedr* outCng){
  if (!zo) return false; 

  if (!outCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetSchedulerCng error: !outCng");
     return false;
  }
  ZM_Base::scheduler scng;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getSchedr(sId, scng)){    
    outCng->capacityTask = scng.capacityTask;
    strcpy(outCng->connectPnt, scng.connectPnt.c_str());
    return true;
  }
  return false;
}
bool zmChangeScheduler(zmConn zo, uint64_t sId, zmSchedr newCng){
  if (!zo) return false; 
  
  ZM_Base::scheduler scng;
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
            make_pair("command", to_string((int)ZM_Base::messType::startSchedr)),
            make_pair("connectPnt", connCng.connectStr)
          };
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && ZM_Tcp::synchSendData(cng.connectPnt, ZM_Aux::serialn(data));
}
bool zmPauseScheduler(zmConn zo, uint64_t sId){
  if (!zo) return false; 
  
  auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            make_pair("command", to_string((int)ZM_Base::messType::pauseSchedr)),
            make_pair("connectPnt", connCng.connectStr)
          };
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && ZM_Tcp::synchSendData(cng.connectPnt, ZM_Aux::serialn(data));
}
bool zmPingScheduler(zmConn zo, uint64_t sId){
  if (!zo) return false; 
  
  auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            make_pair("command", to_string((int)ZM_Base::messType::pingSchedr)),
            make_pair("connectPnt", connCng.connectStr)
          };
  zmSchedr cng;  
  return zmGetScheduler(zo, sId, &cng) && ZM_Tcp::synchSendData(cng.connectPnt, ZM_Aux::serialn(data));
}
bool zmSchedulerState(zmConn zo, uint64_t sId, zmStateType* outState){
  if (!zo) return false; 

  if (!outState){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmSchedulerState error: !outState");
     return false;
  }
  ZM_Base::stateType state;
  if (static_cast<ZM_DB::DbProvider*>(zo)->schedrState(sId, state)){    
    *outState = (zmey::zmStateType)state;
    return true;
  }
  return false;
}
uint32_t zmGetAllSchedulers(zmConn zo, zmStateType state, uint64_t** outSchId){
  if (!zo) return 0; 

  auto schedrs = static_cast<ZM_DB::DbProvider*>(zo)->getAllSchedrs((ZM_Base::stateType)state);
  size_t ssz = schedrs.size();
  if (ssz > 0){
    *outSchId = (uint64_t*)realloc(*outSchId, ssz * sizeof(uint64_t));
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
  ZM_Base::worker wcng;
  wcng.capacityTask = cng.capacityTask;
  wcng.connectPnt = cng.connectPnt;
  wcng.state = ZM_Base::stateType::ready;
  wcng.sId = cng.sId;

  return static_cast<ZM_DB::DbProvider*>(zo)->addWorker(wcng, *outWId);
}
bool zmGetWorker(zmConn zo, uint64_t wId, zmWorker* outWCng){
  if (!zo) return false; 

  if (!outWCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetWorkerCng error: !outWCng");
     return false;
  }
  ZM_Base::worker wcng;
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

  ZM_Base::worker wcng;
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
            make_pair("command", to_string((int)ZM_Base::messType::startWorker)),
            make_pair("connectPnt", connCng.connectStr),
            make_pair("workerConnPnt", wcng.connectPnt)
          };
    return ZM_Tcp::synchSendData(scng.connectPnt, ZM_Aux::serialn(data));
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
            make_pair("command", to_string((int)ZM_Base::messType::pauseWorker)),
            make_pair("connectPnt", connCng.connectStr),
            make_pair("workerConnPnt", wcng.connectPnt)
          };
    return ZM_Tcp::synchSendData(scng.connectPnt, ZM_Aux::serialn(data));
  }else{
    return false;
  } 
}
bool zmPingWorker(zmConn zo, uint64_t wId){
  if (!zo) return false; 
  
  auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
  map<string, string> data{
            make_pair("command", to_string((int)ZM_Base::messType::pingWorker)),
            make_pair("connectPnt", connCng.connectStr)
          };
  zmWorker cng;  
  return zmGetWorker(zo, wId, &cng) && ZM_Tcp::synchSendData(cng.connectPnt, ZM_Aux::serialn(data));
}
bool zmWorkerState(zmConn zo, uint64_t* pWId, uint32_t wCnt, zmStateType* outState){
  if (!zo) return false; 

  if (!outState){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmWorkerState error: !outState");
     return false;
  }
  vector<uint64_t> wId(wCnt);
  memcpy(wId.data(), pWId, wCnt * sizeof(uint64_t));
  vector<ZM_Base::stateType> state;
  if (static_cast<ZM_DB::DbProvider*>(zo)->workerState(wId, state)){    
    memcpy(outState, state.data(), wCnt * sizeof(ZM_Base::stateType));
    return true;
  }
  return false;
}
uint32_t zmGetAllWorkers(zmConn zo, uint64_t sId, zmStateType state, uint64_t** outWId){
  if (!zo) return 0; 

  auto workers = static_cast<ZM_DB::DbProvider*>(zo)->getAllWorkers(sId, (ZM_Base::stateType)state);
  size_t wsz = workers.size();
  if (wsz > 0){ 
    *outWId = (uint64_t*)realloc(*outWId, wsz * sizeof(uint64_t));
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
  ZM_Base::uPipeline pp;
  pp.uId = cng.userId;
  pp.name = cng.name;
  pp.description = cng.description ? cng.description : "";
  pp.isShared = cng.isShared;
  
  return static_cast<ZM_DB::DbProvider*>(zo)->addPipeline(pp, *outPPLId);
}
bool zmGetPipeline(zmConn zo, uint64_t pplId, zmPipeline* outPPLCng){
  if (!zo) return false; 

  if (!outPPLCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetPipelineCng error: !outPPLCng");
     return false;
  }
  ZM_Base::uPipeline pp;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getPipeline(pplId, pp)){    
    outPPLCng->userId = pp.uId;
    strcpy(outPPLCng->name, pp.name.c_str());
    outPPLCng->description = (char*)realloc(outPPLCng->description, pp.description.size() + 1);
    strcpy(outPPLCng->description, pp.description.c_str());  
    outPPLCng->isShared = pp.isShared; 
    return true;
  }
  return false;
}
bool zmChangePipeline(zmConn zo, uint64_t pplId, zmPipeline newCng){
  if (!zo) return false;
     
  ZM_Base::uPipeline pp;
  pp.uId = newCng.userId;
  pp.name = newCng.name;
  pp.description = newCng.description;
  pp.isShared = newCng.isShared;

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
    memcpy(*outPPLId, ppls.data(), psz * sizeof(uint64_t));
  }else{
    *outPPLId = nullptr;
  }
  return (uint32_t)psz;
}

///////////////////////////////////////////////////////////////////////////////
/// Task template

bool zmAddTaskTemplate(zmConn zo, zmTaskTemplate cng, uint64_t* outTId){
  if (!zo) return false;

  if (!outTId || !cng.script){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTaskTemplate error: !outTId || !cng.script");
    return false;
  }
  ZM_Base::uTaskTemplate task;
  task.name = cng.name;
  task.description = cng.description ? cng.description : "";
  task.uId = cng.userId;
  task.base.averDurationSec = cng.averDurationSec;
  task.base.maxDurationSec = cng.maxDurationSec;
  task.base.script = cng.script;
  task.isShared = cng.isShared;

  return static_cast<ZM_DB::DbProvider*>(zo)->addTaskTemplate(task, *outTId);
}
bool zmGetTaskTemplate(zmConn zo, uint64_t tId, zmTaskTemplate* outTCng){
  if (!zo) return false; 

  if (!outTCng){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetTaskTemplateCng error: !outTCng");
    return false;
  }
  ZM_Base::uTaskTemplate task;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getTaskTemplate(tId, task)){  
    strcpy(outTCng->name, task.name.c_str());  
    outTCng->description = (char*)realloc(outTCng->description, task.description.size() + 1);
    strcpy(outTCng->description, task.description.c_str());
    outTCng->averDurationSec = task.base.averDurationSec;
    outTCng->maxDurationSec = task.base.maxDurationSec;
    outTCng->isShared = task.isShared;
    outTCng->userId = task.uId;
    outTCng->script = (char*)realloc(outTCng->script, task.base.script.size() + 1);
    strcpy(outTCng->script, task.base.script.c_str());
    return true;
  }
  return false;
}
bool zmChangeTaskTemplate(zmConn zo, uint64_t tId, zmTaskTemplate newCng, uint64_t* outTId){
  if (!zo) return false; 

  if (!outTId){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmChangeTaskTemplateCng error: !outTId");
    return false;
  }
  ZM_Base::uTaskTemplate task;
  task.name = newCng.name;
  task.description = newCng.description;
  task.uId = newCng.userId;
  task.base.averDurationSec = newCng.averDurationSec;
  task.base.maxDurationSec = newCng.maxDurationSec;
  task.isShared = newCng.isShared;
  task.base.script = newCng.script;
  return static_cast<ZM_DB::DbProvider*>(zo)->changeTaskTemplate(tId, task, *outTId);
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
    memcpy(*outTId, tasks.data(), tsz * sizeof(uint64_t));
  }else{
    *outTId = nullptr;
  }
  return (uint32_t)tsz;
}

///////////////////////////////////////////////////////////////////////////////
/// Task of pipeline

bool zmAddTask(zmConn zo, zmTask cng, uint64_t* outQTId){
  if (!zo) return false;

  if (!outQTId){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTask error: !outQTId");
    return false;
  }
  ZM_Base::uTask task;
  task.pplId = cng.pplId;
  task.base.params = cng.params ? cng.params : "";
  task.prevTasks = cng.prevTasksId ? cng.prevTasksId : "";
  task.nextTasks = cng.nextTasksId ? cng.nextTasksId : "";
  task.screenRect = cng.screenRect ? cng.screenRect : "";  
  task.base.priority = cng.priority;
  task.base.tId = cng.ttId;
  
  return static_cast<ZM_DB::DbProvider*>(zo)->addTask(task, *outQTId);
}
bool zmGetTask(zmConn zo, uint64_t qtId, zmTask* outCng){
  if (!zo) return false;
  
  if (!outCng){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetTaskCng error: !outCng");
    return false;
  }
  ZM_Base::uTask task;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getTask(qtId, task)){
    outCng->pplId = task.pplId;
    outCng->ttId = task.base.tId;
    outCng->priority = task.base.priority;

    outCng->prevTasksId = (char*)realloc(outCng->prevTasksId, task.prevTasks.size() + 1);
    strcpy(outCng->prevTasksId, task.prevTasks.c_str());

    outCng->nextTasksId = (char*)realloc(outCng->nextTasksId, task.nextTasks.size() + 1);
    strcpy(outCng->nextTasksId, task.nextTasks.c_str());

    outCng->params = (char*)realloc(outCng->params, task.base.params.size() + 1);
    strcpy(outCng->params, task.base.params.c_str());
    
    outCng->screenRect = (char*)realloc(outCng->screenRect, task.screenRect.size() + 1);
    strcpy(outCng->screenRect, task.screenRect.c_str());   
    return true;
  }
  return false;
}
bool zmChangeTask(zmConn zo, uint64_t qtId, zmTask newCng){
  if (!zo) return false;
  
  ZM_Base::uTask task;
  task.pplId = newCng.pplId;
  task.base.tId = newCng.ttId;
  task.base.priority = newCng.priority;
  task.prevTasks = newCng.prevTasksId ? newCng.prevTasksId : "";
  task.nextTasks = newCng.nextTasksId ? newCng.nextTasksId : "";
  task.base.params = newCng.params ? newCng.params : "";
  task.screenRect = newCng.screenRect ? newCng.screenRect : "";

  return static_cast<ZM_DB::DbProvider*>(zo)->changeTask(qtId, task);
}
bool zmDelTask(zmConn zo, uint64_t qtId){
  if (!zo) return false;

  return static_cast<ZM_DB::DbProvider*>(zo)->delTask(qtId);
}
bool zmStartTask(zmConn zo, uint64_t qtId){
  if (!zo) return false;

  return static_cast<ZM_DB::DbProvider*>(zo)->startTask(qtId);
}
bool zmStopTask(zmConn zo, uint64_t tId){
  if (!zo) return false;
    
  ZM_Base::worker wcng;  
  uint64_t qtId = 0;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getWorkerByTask(tId, qtId, wcng)){
    auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
    map<string, string> data{
            make_pair("command", to_string((int)ZM_Base::messType::taskStop)),
            make_pair("connectPnt", connCng.connectStr),
            make_pair("taskId", to_string(qtId))
          };

    return ZM_Tcp::synchSendData(wcng.connectPnt, ZM_Aux::serialn(data));
  }
  return false;
}
bool zmCancelTask(zmConn zo, uint64_t tId){
  if (!zo) return false;
  
  return static_cast<ZM_DB::DbProvider*>(zo)->cancelTask(tId);
}
bool zmPauseTask(zmConn zo, uint64_t tId){
  if (!zo) return false;
    
  ZM_Base::worker wcng;  
  uint64_t qtId = 0;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getWorkerByTask(tId, qtId, wcng)){
    auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
    map<string, string> data{
            make_pair("command", to_string((int)ZM_Base::messType::taskPause)),
            make_pair("connectPnt", connCng.connectStr),
            make_pair("taskId", to_string(qtId))
          };

    return ZM_Tcp::synchSendData(wcng.connectPnt, ZM_Aux::serialn(data));
  }
  return false;
}
bool zmContinueTask(zmConn zo, uint64_t tId){
  if (!zo) return false;
    
  ZM_Base::worker wcng;  
  uint64_t qtId = 0;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getWorkerByTask(tId, qtId, wcng)){
    auto connCng = static_cast<ZM_DB::DbProvider*>(zo)->getConnectCng();  
    map<string, string> data{
            make_pair("command", to_string((int)ZM_Base::messType::taskContinue)),
            make_pair("connectPnt", connCng.connectStr),
            make_pair("taskId", to_string(qtId))
          };

    return ZM_Tcp::synchSendData(wcng.connectPnt, ZM_Aux::serialn(data));
  }
  return false;
}
bool zmTaskState(zmConn zo, uint64_t* qtId, uint32_t tCnt, zmTskState* outQTState){
  if (!zo) return false;
  
  if (!qtId || !outQTState){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmTaskState error: !qtId || !outQTState");
    return false;
  }
  vector<uint64_t> qtaskId(tCnt);
  memcpy(qtaskId.data(), qtId, tCnt * sizeof(uint64_t));
  vector<ZM_DB::tskState> state;
  if (static_cast<ZM_DB::DbProvider*>(zo)->taskState(qtaskId, state)){  
    for (size_t i = 0; i < tCnt; ++i){
      outQTState[i].progress = state[i].progress;
      outQTState[i].state = (zmStateType)state[i].state;
    }    
    return true;
  }
  return false;
}
bool zmTaskResult(zmConn zo, uint64_t qtId, char** outTResult){
  if (!zo) return false;
  
  if (!outTResult){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmTaskResult error: !outTResult");
    return false;
  }
  string result;
  if (static_cast<ZM_DB::DbProvider*>(zo)->taskResult(qtId, result)){  
    *outTResult = (char*)realloc(*outTResult, result.size() + 1); 
    strcpy(*outTResult, result.c_str());
    return true;
  }
  return false;
}
bool zmTaskTime(zmConn zo, uint64_t qtId, zmTskTime* outTTime){
  if (!zo) return false;
  
  if (!outTTime){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmTaskTime error: !outTTime");
    return false;
  }
  ZM_DB::taskTime tskTime;
  if (static_cast<ZM_DB::DbProvider*>(zo)->taskTime(qtId, tskTime)){  
    strncpy(outTTime->createTime, tskTime.createTime.c_str(), 32);
    strncpy(outTTime->takeInWorkTime, tskTime.takeInWorkTime.c_str(), 32);
    strncpy(outTTime->startTime, tskTime.startTime.c_str(), 32);
    strncpy(outTTime->stopTime, tskTime.stopTime.c_str(), 32);
    return true;
  }
  return false;
}
uint32_t zmGetAllTasks(zmConn zo, uint64_t pplId, zmStateType state, uint64_t** outQTId){
  if (!zo) return 0; 

  auto tasks = static_cast<ZM_DB::DbProvider*>(zo)->getAllTasks(pplId, (ZM_Base::stateType)state);
  size_t tsz = tasks.size();
  if (tsz > 0){
    *outQTId = (uint64_t*)realloc(*outQTId, tsz * sizeof(uint64_t));
    memcpy(*outQTId, tasks.data(), tsz * sizeof(uint64_t));
  }else{
    *outQTId = nullptr;
  }
  return (uint32_t)tsz;
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
void zmFreeResources(uint64_t* pUInt, char* pChar){
  if (pUInt){
    free(pUInt);
  } 
  if (pChar){
    free(pChar);
  }
}
}