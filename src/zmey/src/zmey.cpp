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
#include "manager.h"

#define ZM_VERSION "1.0.0"

using namespace std;

namespace zmey{

void zmVersionLib(char* outVersion /*sz 8*/){
  if (outVersion){
    strcpy(outVersion, ZM_VERSION);
  }
}

/////////////////////////////////////////////////////////////////////
///*** Connection with DB ***////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////
///*** User ***//////////////////////////////////////////////////////

bool zmAddUser(zmObj zo, zmUserCng newUserCng, uint64_t* outUserId){
  if (!zo) return false;
  
  if (!outUserId){
     static_cast<Manager*>(zo)->errorMess("error !outUserId");
     return false;
  }
  ZM_Base::user us;
  us.name = newUserCng.name;
  us.passw = newUserCng.passw;
  us.decription = newUserCng.decription;
  
  return static_cast<Manager*>(zo)->addUser(us, *outUserId);
}
bool zmGetUserId(zmObj zo, zmUserCng cng, uint64_t* outUserId){
  if (!zo) return false;
  
  if (!outUserId){
     static_cast<Manager*>(zo)->errorMess("error !outUserId");
     return false;
  }   
  return static_cast<Manager*>(zo)->getUser(string(cng.name), string(cng.passw), *outUserId);
}
bool zmGetUserCng(zmObj zo, uint64_t userId, zmUserCng* outUserCng){
  if (!zo) return false; 

  if (!outUserCng){
     static_cast<Manager*>(zo)->errorMess("error !outUserCng");
     return false;
  }
  ZM_Base::user ur;
  if (static_cast<Manager*>(zo)->getUser(userId, ur)){    
    strcpy(outUserCng->name, ur.name.c_str());
    outUserCng->decription = (char*)realloc(outUserCng->decription, ur.decription.size() + 1);
    strcpy(outUserCng->decription, ur.decription.c_str());   
    return true;
  }
  return false;
}
bool zmChangeUser(zmObj zo, uint64_t userId, zmUserCng newCng){
  if (!zo) return false;
     
  ZM_Base::user us;
  us.name = newCng.name;
  us.passw = newCng.passw;
  us.decription = newCng.decription;

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

//////////////////////////////////////////////////////////////////////////
///*** Scheduler ***//////////////////////////////////////////////////////

bool zmAddScheduler(zmObj zo, zmSchedrCng cng, uint64_t* outSchId){
  if (!zo) return false;
  
  if (!outSchId){
     static_cast<Manager*>(zo)->errorMess("error !outSchId");
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
     static_cast<Manager*>(zo)->errorMess("error !outState");
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

//////////////////////////////////////////////////////////////////////////
///*** Worker ***/////////////////////////////////////////////////////////

bool zmAddWorker(zmObj zo, zmWorkerCng cng, uint64_t* outWId){
  if (!zo) return false;

  if (!outWId){
     static_cast<Manager*>(zo)->errorMess("error !outWId");
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
     static_cast<Manager*>(zo)->errorMess("error !outState");
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

/////////////////////////////////////////////////////////////////////////////////
///*** Pipeline of tasks ***/////////////////////////////////////////////////////

bool zmAddPipeline(zmObj zo, zmPipelineCng cng, uint64_t* outPPLId){
  if (!zo) return false;
  
  if (!outPPLId){
     static_cast<Manager*>(zo)->errorMess("error !outPPLId");
     return false;
  }
  ZM_Base::pipeline pp;
  pp.uId = cng.userId;
  pp.name = cng.name;
  pp.decription = cng.decription;
  
  return static_cast<Manager*>(zo)->addPipeline(pp, *outPPLId);
}
bool zmGetPipelineCng(zmObj zo, uint64_t pplId, zmPipelineCng* outPPLCng){
  if (!zo) return false; 

  if (!outPPLCng){
     static_cast<Manager*>(zo)->errorMess("error !outPPLCng");
     return false;
  }
  ZM_Base::pipeline pp;
  if (static_cast<Manager*>(zo)->getPipeline(pplId, pp)){    
    outPPLCng->userId = pp.uId;
    strcpy(outPPLCng->name, pp.name.c_str());
    outPPLCng->decription = (char*)realloc(outPPLCng->decription, pp.decription.size() + 1);
    strcpy(outPPLCng->decription, pp.decription.c_str());   
    return true;
  }
  return false;
}
bool zmChangePipelineCng(zmObj zo, uint64_t pplId, zmPipelineCng newCng){
  if (!zo) return false;
     
  ZM_Base::pipeline pp;
  pp.uId = newCng.userId;
  pp.name = newCng.name;
  pp.decription = newCng.decription;
  
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

///////////////////////////////////////////////////////////////////////////////////
///*** Task template ***///////////////////////////////////////////////////////////

bool zmAddTask(zmObj zo, zmTaskCng cng, uint64_t* outTId){
  if (!zo) return false;

  if (!outTId || !cng.script){
     static_cast<Manager*>(zo)->errorMess("error !outTId || !cng.script");
     return false;
  }
  ZM_Base::task task;
  task.averDurationSec = cng.averDurationSec;
  task.maxDurationSec = cng.maxDurationSec;
  task.exr = (ZM_Base::executorType)cng.exr;
  task.script = cng.script;

  return static_cast<Manager*>(zo)->addTask(task, *outTId);
}
bool zmGetTaskCng(zmObj zo, uint64_t tId, zmTaskCng* outTCng){
  if (!zo) return false; 

  if (!outTCng){
     static_cast<Manager*>(zo)->errorMess("error !outTCng");
     return false;
  }
  ZM_Base::task task;
  if (static_cast<Manager*>(zo)->getTaskCng(tId, task)){    
    outTCng->exr = (zmey::zmExecutorType)task.exr;
    outTCng->averDurationSec = task.averDurationSec;
    outTCng->maxDurationSec = task.maxDurationSec;
   
    outTCng->script = (char*)realloc(outTCng->script, task.script.size() + 1);
    strcpy(outTCng->script, task.script.c_str());
    return true;
  }
  return false;
}
uint32_t zmGetAllTasks(zmObj zo, uint64_t** outTId){
  if (!zo) return false; 

  auto tasks = static_cast<Manager*>(zo)->getAllTasks();
  size_t tsz = tasks.size();
  if (tsz > 0){
    *outTId = (uint64_t*)realloc(*outTId, tsz * sizeof(uint64_t));
    memcpy(*outTId, tasks.data(), tsz * sizeof(uint64_t));
  }else{
    *outTId = nullptr;
  }
  return (uint32_t)tsz;
}

bool zmPushTaskToQueue(zmObj zo, zmQueueTaskCng cng, uint64_t* outQTId){
  if (!zo) return false;

  if (!outQTId){
    static_cast<Manager*>(zo)->errorMess("error !outQTId");
    return false;
  }
  if ((cng.prevTasksCnt > 0) && !cng.prevTasksQId){
    static_cast<Manager*>(zo)->errorMess("error !cng.prevTasksQId");
    return false;
  }
  ZM_Base::queueTask qTask;
  qTask.tId = cng.tId;
  qTask.priority = cng.priority;
  if (cng.params){
    qTask.params = cng.params;
  }
  for (int i = 0; i < cng.prevTasksCnt; ++i){
    qTask.prevTasks.push_back(cng.prevTasksQId[i]);
  }  
  return static_cast<Manager*>(zo)->pushTaskToQueue(qTask, *outQTId);
}
bool zmGetQueueTaskCng(zmObj zo, uint64_t qtId, zmQueueTaskCng* outQCng){
  if (!zo) return false;
  
  if (!outQCng){
     static_cast<Manager*>(zo)->errorMess("error !outQCng");
     return false;
  }
  ZM_Base::queueTask qTask;
  if (static_cast<Manager*>(zo)->getQueueTaskCng(qtId, qTask)){
    outQCng->tId = qTask.tId;
    outQCng->priority = qTask.priority;
    
    auto& params = qTask.params;
    if (!params.empty()){ 
      outQCng->params = (char*)realloc(outQCng->params, params.size() + 1);
      strcpy(outQCng->params, params.c_str());
    }else{
      outQCng->params = nullptr;
    }
    if (!qTask.prevTasks.empty()){ 
      outQCng->prevTasksCnt = qTask.prevTasks.size();
      outQCng->prevTasksQId = (uint64_t*)realloc(outQCng->prevTasksQId, outQCng->prevTasksCnt * sizeof(uint64_t));
      memcpy(outQCng->prevTasksQId, qTask.prevTasks.data(), outQCng->prevTasksCnt * sizeof(uint64_t));
    }else{
      outQCng->prevTasksCnt = 0;
      outQCng->prevTasksQId = nullptr;
    }
    return true;
  }
  return false;
}
bool zmGetQueueTaskState(zmObj zo, uint64_t qtId, zmQueueTaskState* outQState){
  if (!zo) return false;
  
  if (!outQState){
    static_cast<Manager*>(zo)->errorMess("error !outQState");
    return false;
  }
  ZM_Base::queueTask qTask;
  if (static_cast<Manager*>(zo)->getQueueTaskState(qtId, qTask)){
    outQState->progress = qTask.progress;
    outQState->state = (zmey::zmStateType)qTask.state;
    if (!qTask.result.empty()){ 
      outQState->result = (char*)realloc(outQState->result, qTask.result.size() + 1);
      strcpy(outQState->result, qTask.result.c_str());
    }else{
      outQState->result = nullptr;
    }
    return true;
  }
  return false;
}
uint32_t zmGetAllQueueTasks(zmObj zo, zmStateType state, uint64_t** outQTId){
  if (!zo) return false; 

  auto tasks = static_cast<Manager*>(zo)->getAllQueueTasks((ZM_Base::stateType)state);
  size_t tsz = tasks.size();
  if (tsz > 0){
    *outQTId = (uint64_t*)realloc(*outQTId, tsz * sizeof(uint64_t));
    memcpy(*outQTId, tasks.data(), tsz * sizeof(uint64_t));
  }else{
    *outQTId = nullptr;
  }
  return (uint32_t)tsz;
}

uint32_t zmFreeResouces(uint64_t* pUInt, char* pChar){
  if (pUInt){
    free(pUInt);
  } 
  if (pChar){
    free(pChar);
  }
}
}