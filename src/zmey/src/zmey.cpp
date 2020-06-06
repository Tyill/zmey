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
#include "manager.h"

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

zmObj zmCreateConnection(zmConnectCng cng, char* err){
  
  ZM_DB::connectCng connCng{ (ZM_DB::dbType)cng.dbType,
                             cng.connectPnt,
                             cng.dbServer,
                             cng.dbName,
                             cng.dbUser,
                             cng.dbPassw,
                            };

  Manager* mr = new Manager(connCng);
  auto serr = mr->getLastError();
  if (!serr.empty()){
    if (err){
      strcpy(err, serr.c_str());
    }
    delete mr;
    return nullptr;
  }
  return mr;
}
void zmDisconnect(zmObj zo){
  if (zo){
    delete static_cast<Manager*>(zo);
  }
}
void zmSetErrorCBack(zmObj zo, zmErrorCBack ecb, zmUData ud){
  if (!zo) return;

  static_cast<Manager*>(zo)->setErrorCBack(ecb, ud);
}
void zmGetLastError(zmObj zo, char* err/*sz 256*/){
  if (zo && err){
    strcpy(err, static_cast<Manager*>(zo)->getLastError().c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
/// User

bool zmAddUser(zmObj zo, zmUserCng newUserCng, uint64_t* outUserId){
  if (!zo) return false;
  
  if (!outUserId){
     static_cast<Manager*>(zo)->errorMess("zmAddUser error: !outUserId");
     return false;
  }
  ZM_Base::user us;
  us.name = newUserCng.name;
  us.passw = newUserCng.passw;
  us.description = newUserCng.description;
  
  return static_cast<Manager*>(zo)->addUser(us, *outUserId);
}
bool zmGetUserId(zmObj zo, zmUserCng cng, uint64_t* outUserId){
  if (!zo) return false;
  
  if (!outUserId){
     static_cast<Manager*>(zo)->errorMess("zmGetUserId error: !outUserId");
     return false;
  }   
  return static_cast<Manager*>(zo)->getUser(string(cng.name), string(cng.passw), *outUserId);
}
bool zmGetUserCng(zmObj zo, uint64_t userId, zmUserCng* outUserCng){
  if (!zo) return false; 

  if (!outUserCng){
     static_cast<Manager*>(zo)->errorMess("zmGetUserCng error: !outUserCng");
     return false;
  }
  ZM_Base::user ur;
  if (static_cast<Manager*>(zo)->getUser(userId, ur)){    
    strcpy(outUserCng->name, ur.name.c_str());
    outUserCng->description = (char*)realloc(outUserCng->description, ur.description.size() + 1);
    strcpy(outUserCng->description, ur.description.c_str());   
    return true;
  }
  return false;
}
bool zmChangeUser(zmObj zo, uint64_t userId, zmUserCng newCng){
  if (!zo) return false;
     
  ZM_Base::user us;
  us.name = newCng.name;
  us.passw = newCng.passw;
  us.description = newCng.description;

  return static_cast<Manager*>(zo)->changeUser(userId, us);
}
bool zmDelUser(zmObj zo, uint64_t userId){
  if (!zo) return false;
 
  return static_cast<Manager*>(zo)->delUser(userId);
}
uint32_t zmGetAllUsers(zmObj zo, uint64_t** outUserId){
  if (!zo) return false; 

  auto users = static_cast<Manager*>(zo)->getAllUsers();
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

bool zmAddScheduler(zmObj zo, zmSchedrCng cng, uint64_t* outSchId){
  if (!zo) return false;
  
  if (!outSchId){
     static_cast<Manager*>(zo)->errorMess("zmAddScheduler error: !outSchId");
     return false;
  }
  ZM_Base::scheduler schedr;
  schedr.capasityTask = cng.capasityTask;
  schedr.connectPnt = cng.connectPnt;

  return static_cast<Manager*>(zo)->addScheduler(schedr, *outSchId);
}
bool zmSchedulerState(zmObj zo, uint64_t schId, zmStateType* outState, zmSchedrCng* outSchCng){
  if (!zo) return false; 

  if (!outState){
     static_cast<Manager*>(zo)->errorMess("zmSchedulerState error: !outState");
     return false;
  }
  ZM_Base::scheduler schedr;
  if (static_cast<Manager*>(zo)->schedulerState(schId, schedr)){    
    *outState = (zmey::zmStateType)schedr.state;
    if (outSchCng){
      outSchCng->capasityTask = schedr.capasityTask;
      strcpy(outSchCng->connectPnt, schedr.connectPnt.c_str());
    }
    return true;
  }
  return false;
}
uint32_t zmGetAllSchedulers(zmObj zo, zmStateType state, uint64_t** outSchId){
  if (!zo) return false; 

  auto schedrs = static_cast<Manager*>(zo)->getAllSchedulers((ZM_Base::stateType)state);
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

bool zmAddWorker(zmObj zo, zmWorkerCng cng, uint64_t* outWId){
  if (!zo) return false;

  if (!outWId){
     static_cast<Manager*>(zo)->errorMess("zmAddWorker error: !outWId");
     return false;
  }
  ZM_Base::worker worker;
  worker.capasityTask = cng.capasityTask;
  worker.connectPnt = cng.connectPnt;

  return static_cast<Manager*>(zo)->addWorker(worker, *outWId);
}
bool zmWorkerState(zmObj zo, uint64_t wId, zmStateType* outState, zmWorkerCng* outWCng){
  if (!zo) return false; 

  if (!outState){
     static_cast<Manager*>(zo)->errorMess("zmWorkerState error: !outState");
     return false;
  }
  ZM_Base::worker worker;
  if (static_cast<Manager*>(zo)->workerState(wId, worker)){    
    *outState = (zmey::zmStateType)worker.state;
    if (outWCng){
      outWCng->schId = worker.sId;
      outWCng->exr = (zmey::zmExecutorType)worker.exr;
      outWCng->capasityTask = worker.capasityTask;
      strcpy(outWCng->connectPnt, worker.connectPnt.c_str());
    }
    return true;
  }
  return false;
}
uint32_t zmGetAllWorkers(zmObj zo, uint64_t schId, zmStateType state, uint64_t** outWId){
  if (!zo) return false; 

  auto workers = static_cast<Manager*>(zo)->getAllWorkers(schId, (ZM_Base::stateType)state);
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

bool zmAddPipeline(zmObj zo, zmPipelineCng cng, uint64_t* outPPLId){
  if (!zo) return false;
  
  if (!outPPLId){
     static_cast<Manager*>(zo)->errorMess("zmAddPipeline error: !outPPLId");
     return false;
  }
  ZM_Base::uPipeline pp;
  pp.uId = cng.userId;
  pp.name = cng.name;
  pp.description = cng.description;
  
  return static_cast<Manager*>(zo)->addPipeline(pp, *outPPLId);
}
bool zmGetPipelineCng(zmObj zo, uint64_t pplId, zmPipelineCng* outPPLCng){
  if (!zo) return false; 

  if (!outPPLCng){
     static_cast<Manager*>(zo)->errorMess("zmGetPipelineCng error: !outPPLCng");
     return false;
  }
  ZM_Base::uPipeline pp;
  if (static_cast<Manager*>(zo)->getPipeline(pplId, pp)){    
    outPPLCng->userId = pp.uId;
    strcpy(outPPLCng->name, pp.name.c_str());
    outPPLCng->description = (char*)realloc(outPPLCng->description, pp.description.size() + 1);
    strcpy(outPPLCng->description, pp.description.c_str());   
    return true;
  }
  return false;
}
bool zmChangePipelineCng(zmObj zo, uint64_t pplId, zmPipelineCng newCng){
  if (!zo) return false;
     
  ZM_Base::uPipeline pp;
  pp.uId = newCng.userId;
  pp.name = newCng.name;
  pp.description = newCng.description;
  
  return static_cast<Manager*>(zo)->changePipeline(pplId, pp);
}
bool zmDelPipeline(zmObj zo, uint64_t pplId){
  if (!zo) return false;
 
  return static_cast<Manager*>(zo)->delPipeline(pplId);
}
uint32_t zmGetAllPipelines(zmObj zo, uint64_t userId, uint64_t** outPPLId){
  if (!zo) return false; 

  auto ppls = static_cast<Manager*>(zo)->getAllPipelines(userId);
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

bool zmAddTaskTemplate(zmObj zo, zmTaskTemplateCng cng, uint64_t* outTId){
  if (!zo) return false;

  if (!outTId || !cng.script){
     static_cast<Manager*>(zo)->errorMess("zmAddTaskTemplate error: !outTId || !cng.script");
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
  
  return static_cast<Manager*>(zo)->addTaskTemplate(task, *outTId);
}
bool zmGetTaskTemplateCng(zmObj zo, uint64_t tId, zmTaskTemplateCng* outTCng){
  if (!zo) return false; 

  if (!outTCng){
     static_cast<Manager*>(zo)->errorMess("zmGetTaskTemplateCng error: !outTCng");
     return false;
  }
  ZM_Base::uTaskTemplate task;
  if (static_cast<Manager*>(zo)->getTaskTemplateCng(tId, task)){  
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
bool zmChangeTaskTemplateCng(zmObj zo, uint64_t tId, zmTaskTemplateCng cng, uint64_t* outTId){
  if (!zo) return false; 

  if (!outTId){
     static_cast<Manager*>(zo)->errorMess("zmChangeTaskTemplateCng error: !outTId");
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
  return static_cast<Manager*>(zo)->changeTaskTemplateCng(tId, task, *outTId);
}
bool zmDelTaskTemplate(zmObj zo, uint64_t tId){
  if (!zo) return false; 

  return static_cast<Manager*>(zo)->delTaskTemplate(tId);
}
uint32_t zmGetAllTaskTemplates(zmObj zo, uint64_t parent, uint64_t** outTId){
  if (!zo) return false; 

  auto tasks = static_cast<Manager*>(zo)->getAllTaskTemplates(parent);
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

bool zmAddTask(zmObj zo, zmTaskCng cng, uint64_t* outQTId){
  if (!zo) return false;

  if (!outQTId){
    static_cast<Manager*>(zo)->errorMess("zmAddTask error: !outQTId");
    return false;
  }
  if ((cng.prevTasksCnt > 0) && !cng.prevTasksId){
    static_cast<Manager*>(zo)->errorMess("zmAddTask error: !cng.prevTasksId");
    return false;
  }
  if ((cng.nextTasksCnt > 0) && !cng.nextTasksId){
    static_cast<Manager*>(zo)->errorMess("zmAddTask error: !cng.nextTasksId");
    return false;
  }
  ZM_Base::uTask task;
  task.pplId = cng.pplId;
  if (cng.params){
    task.base.params = cng.params;
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
    
  return static_cast<Manager*>(zo)->addTask(task, *outQTId);
}
bool zmGetTaskCng(zmObj zo, uint64_t tId, zmTaskCng* outCng){
  if (!zo) return false;
  
  if (!outCng){
     static_cast<Manager*>(zo)->errorMess("zmGetTaskCng error: !outCng");
     return false;
  }
  ZM_Base::uTask task;
  if (static_cast<Manager*>(zo)->getTaskCng(tId, task)){
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
bool zmChangeTaskCng(zmObj, uint64_t qtId, zmTaskCng newQCng){
  
}
bool zmDelTask(zmObj, uint64_t qtId){
  
}
bool zmStartTask(zmObj, uint64_t qtId){
  
}
bool zmStopTask(zmObj, uint64_t qtId){
  
}
bool zmPauseTask(zmObj, uint64_t qtId){
  
}
bool zmGetTaskState(zmObj zo, uint64_t tId, zmTaskState* outQTState){
  if (!zo) return false;
  
  if (!outQTState){
    static_cast<Manager*>(zo)->errorMess("zmGetTaskState error: !outQTState");
    return false;
  }
  ZM_Base::queueTask qTask;
  if (static_cast<Manager*>(zo)->getTaskState(tId, qTask)){
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
uint32_t zmGetAllTasks(zmObj zo, uint64_t pplId, zmStateType state, uint64_t** outQTId){
  if (!zo) return false; 

  auto tasks = static_cast<Manager*>(zo)->getAllTasks(pplId, (ZM_Base::stateType)state);
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