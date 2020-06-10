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
#include "zmey/zmey.h"
#include "zmCommon/auxFunc.h"
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

zmConn zmCreateConnection(zmConnectCng cng, char* err){
  
  ZM_DB::connectCng connCng{ (ZM_DB::dbType)cng.dbType,
                             cng.connectStr};
  ZM_DB::DbProvider* pDb = ZM_DB::makeDbProvider(connCng);

  if (pDb){
    auto serr = pDb->getLastError();
    if (!serr.empty()){
      if (err){
        strcpy(err, serr.c_str());
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
void zmSetErrorCBack(zmConn zo, zmErrorCBack ecb, zmUData ud){
  if (!zo) return;

  static_cast<ZM_DB::DbProvider*>(zo)->setErrorCBack(ecb, ud);
}
void zmGetLastError(zmConn zo, char* err/*sz 256*/){
  if (zo && err){
    strcpy(err, static_cast<ZM_DB::DbProvider*>(zo)->getLastError().c_str());
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
  us.description = newUserCng.description;
  
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
  if (!zo) return false; 

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
  ZM_Base::scheduler schedr;
  schedr.capacityTask = cng.capacityTask;
  schedr.connectPnt = cng.connectPnt;

  return static_cast<ZM_DB::DbProvider*>(zo)->addSchedr(schedr, *outSchId);
}
bool zmGetScheduler(zmConn zo, uint64_t schId, zmSchedr* outCng){
  if (!zo) return false; 

  if (!outCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetSchedulerCng error: !outCng");
     return false;
  }
  ZM_Base::scheduler schedr;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getScheduler(schId, schedr)){    
    outSchCng->capacityTask = schedr.capacityTask;
    strcpy(outSchCng->connectPnt, schedr.connectPnt.c_str());
    return true;
  }
  return false;
}
bool zmChangeScheduler(zmConn zo, uint64_t schId, zmSchedr newCng){
  if (!zo) return false; 
  
  ZM_Base::scheduler schedr;
  schedr.capacityTask = newCng->capacityTask;
  schedr.connectPnt = newCng->connectPnt;

  return static_cast<ZM_DB::DbProvider*>(zo)->changeScheduler(schId, schedr);
}
bool zmDelScheduler(zmConn, uint64_t schId){
  if (!zo) return false;
 
  return static_cast<ZM_DB::DbProvider*>(zo)->delSchedr(schId);
}
bool zmSchedulerState(zmConn zo, uint64_t schId, zmStateType* outState){
  if (!zo) return false; 

  if (!outState){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmSchedulerState error: !outState");
     return false;
  }
  ZM_Base::stateType state;
  if (static_cast<ZM_DB::DbProvider*>(zo)->schedrState(schId, state)){    
    *outState = (zmey::zmStateType)state;
    return true;
  }
  return false;
}
uint32_t zmGetAllSchedulers(zmConn zo, zmStateType state, uint64_t** outSchId){
  if (!zo) return false; 

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
  ZM_Base::worker worker;
  worker.capacityTask = cng.capacityTask;
  worker.connectPnt = cng.connectPnt;

  return static_cast<ZM_DB::DbProvider*>(zo)->addWorker(worker, *outWId);
}
bool zmGetWorker(zmConn zo, uint64_t wId, zmWorker* outWCng){
  if (!zo) return false; 

  if (!outWCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetWorkerCng error: !outWCng");
     return false;
  }
  ZM_Base::worker worker;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getWorker(wId, worker)){    
    outWCng->schId = worker.sId;
    outWCng->exr = (zmey::zmExecutorType)worker.exr;
    outWCng->capacityTask = worker.capacityTask;
    strcpy(outWCng->connectPnt, worker.connectPnt.c_str());
    return true;
  }
  return false;
}
bool zmChangeWorker(zmConn zo, uint64_t wId, zmWorker newCng){
  if (!zo) return false; 

  ZM_Base::worker worker;
  worker.sId = newCng->schId;
  worker.exr = (ZM_Base::executorType)newCng->exr;
  worker.capacityTask = newCng->capacityTask;
  worker.connectPnt = newCng->connectPnt;
  
  return static_cast<ZM_DB::DbProvider*>(zo)->changeWorker(wId, worker);
}
bool zmDelWorker(zmConn, uint64_t wId){
  if (!zo) return false;
 
  return static_cast<ZM_DB::DbProvider*>(zo)->delWorker(wId);
}
bool zmWorkerState(zmConn zo, uint64_t wId, zmStateType* outState){
  if (!zo) return false; 

  if (!outState){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmWorkerState error: !outState");
     return false;
  }
  ZM_Base::stateType state;
  if (static_cast<ZM_DB::DbProvider*>(zo)->workerState(wId, state)){    
    *outState = (zmey::zmStateType)state;
    return true;
  }
  return false;
}
uint32_t zmGetAllWorkers(zmConn zo, uint64_t schId, zmStateType state, uint64_t** outWId){
  if (!zo) return false; 

  auto workers = static_cast<ZM_DB::DbProvider*>(zo)->getAllWorkers(schId, (ZM_Base::stateType)state);
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
  pp.description = cng.description;
  
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
  
  return static_cast<ZM_DB::DbProvider*>(zo)->changePipeline(pplId, pp);
}
bool zmDelPipeline(zmConn zo, uint64_t pplId){
  if (!zo) return false;
 
  return static_cast<ZM_DB::DbProvider*>(zo)->delPipeline(pplId);
}
uint32_t zmGetAllPipelines(zmConn zo, uint64_t userId, uint64_t** outPPLId){
  if (!zo) return false; 

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
  task.description = cng.description;
  task.uId = cng.parent;
  task.base.averDurationSec = cng.averDurationSec;
  task.base.maxDurationSec = cng.maxDurationSec;
  task.base.exr = (ZM_Base::executorType)cng.exr;
  task.base.script = cng.script;
  
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
    outTCng->exr = (zmey::zmExecutorType)task.base.exr;
    outTCng->averDurationSec = task.base.averDurationSec;
    outTCng->maxDurationSec = task.base.maxDurationSec;
    outTCng->parent = task.uId;
    outTCng->script = (char*)realloc(outTCng->script, task.base.script.size() + 1);
    strcpy(outTCng->script, task.base.script.c_str());
    return true;
  }
  return false;
}
bool zmChangeTaskTemplateCng(zmConn zo, uint64_t tId, zmTaskTemplate cng, uint64_t* outTId){
  if (!zo) return false; 

  if (!outTId){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmChangeTaskTemplateCng error: !outTId");
     return false;
  }
  ZM_Base::uTaskTemplate task;
  task.name = cng.name;
  task.description = cng.description;
  task.uId = cng.parent;
  task.base.averDurationSec = cng.averDurationSec;
  task.base.maxDurationSec = cng.maxDurationSec;
  task.base.exr = (ZM_Base::executorType)cng.exr;
  task.base.script = cng.script;
  return static_cast<ZM_DB::DbProvider*>(zo)->changeTaskTemplate(tId, task, *outTId);
}
bool zmDelTaskTemplate(zmConn zo, uint64_t tId){
  if (!zo) return false; 

  return static_cast<ZM_DB::DbProvider*>(zo)->delTaskTemplate(tId);
}
uint32_t zmGetAllTaskTemplates(zmConn zo, uint64_t parent, uint64_t** outTId){
  if (!zo) return false; 

  auto tasks = static_cast<ZM_DB::DbProvider*>(zo)->getAllTaskTemplates(parent);
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
  if ((cng.prevTasksCnt > 0) && !cng.prevTasksId){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTask error: !cng.prevTasksId");
    return false;
  }
  if ((cng.nextTasksCnt > 0) && !cng.nextTasksId){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTask error: !cng.nextTasksId");
    return false;
  }
  ZM_Base::uTask task;
  task.pplId = cng.pplId;
  if (cng.params){
    task.base.params = cng.params;
    auto params = ZM_Aux::split(task.base.params, "-");
    if (params.empty()){
      static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTask error: params not correct, should be -key=value.. ");
      return false;
    }
    for (auto& p : params){
      auto keyVal = ZM_Aux::split(p, "=");
      if (keyVal.size() != 2){ 
        static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTask error: params not correct, should be -key=value.. ");
        return false;
      }
    }
  }
  for (int i = 0; i < cng.prevTasksCnt; ++i){
    task.prevTasks.push_back(cng.prevTasksId[i]);
  }
  for (int i = 0; i < cng.nextTasksCnt; ++i){
    task.nextTasks.push_back(cng.nextTasksId[i]);
  }
  task.base.priority = cng.priority;
  task.rct = ZM_Base::uScreenRect{cng.screenRect.x, cng.screenRect.y, cng.screenRect.w, cng.screenRect.h};
  task.base.tId = cng.tId;
    
  return static_cast<ZM_DB::DbProvider*>(zo)->addTask(task, *outQTId);
}
bool zmGetTask(zmConn zo, uint64_t tId, zmTask* outCng){
  if (!zo) return false;
  
  if (!outCng){
     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetTaskCng error: !outCng");
     return false;
  }
  ZM_Base::uTask task;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getTask(tId, task)){
    outCng->tId = task.id;
    outCng->priority = task.base.priority;
    outCng->screenRect = zmScreenRect{task.rct.x, task.rct.y, task.rct.w, task.rct.h};
    
    auto& params = task.base.params;
    if (!params.empty()){ 
      outCng->params = (char*)realloc(outCng->params, params.size() + 1);
      strcpy(outCng->params, params.c_str());
    }else{
      outCng->params = nullptr;
    }
    if (!task.prevTasks.empty()){ 
      outCng->prevTasksCnt = task.prevTasks.size();
      outCng->prevTasksId = (uint64_t*)realloc(outCng->prevTasksId, outCng->prevTasksCnt * sizeof(uint64_t));
      memcpy(outCng->prevTasksId, task.prevTasks.data(), outCng->prevTasksCnt * sizeof(uint64_t));
    }else{
      outCng->prevTasksCnt = 0;
      outCng->prevTasksId = nullptr;
    }
    return true;
  }
  return false;
}
bool zmChangeTask(zmConn zo, uint64_t qtId, zmTask newQCng){
  if (!zo) return false;

  // task.base.params = newQCng.params;
  // auto params = ZM_Aux::split(task.base.params, "-");
  // if (params.empty()){
  //   static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTask error: params not correct, should be -key=value.. ");
  //   return false;
  // }
  // for (auto& p : params){
  //   auto keyVal = ZM_Aux::split(p, "=");
  //   if (keyVal.size() != 2){ 
  //     static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmAddTask error: params not correct, should be -key=value.. ");
  //     return false;
  //   }
  // }
}
bool zmDelTask(zmConn, uint64_t qtId){
  
}
bool zmStartTask(zmConn, uint64_t qtId){
  
}
bool zmStopTask(zmConn, uint64_t qtId){
  
}
bool zmPauseTask(zmConn, uint64_t qtId){
  
}
bool zmContinueTask(zmConn, uint64_t tId){

}
bool zmGetTaskState(zmConn zo, uint64_t tId, zmTaskState* outQTState){
  if (!zo) return false;
  
  if (!outQTState){
    static_cast<ZM_DB::DbProvider*>(zo)->errorMess("zmGetTaskState error: !outQTState");
    return false;
  }
  ZM_Base::queueTask qTask;
  if (static_cast<ZM_DB::DbProvider*>(zo)->getTaskState(tId, qTask)){
    outQTState->progress = qTask.progress;
    outQTState->state = (zmey::zmStateType)qTask.state;
    if (!qTask.result.empty()){ 
      outQTState->result = (char*)realloc(outQTState->result, qTask.result.size() + 1);
      strcpy(outQTState->result, qTask.result.c_str());
    }else{
      outQTState->result = nullptr;
    }
    return true;
  }
  return false;
}
uint32_t zmGetAllTasks(zmConn zo, uint64_t pplId, zmStateType state, uint64_t** outQTId){
  if (!zo) return false; 

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
/// free resouces
void zmFreeResouces(uint64_t* pUInt, char* pChar){
  if (pUInt){
    free(pUInt);
  } 
  if (pChar){
    free(pChar);
  }
}
}