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
#include "zmManager.h"

#define ZM_VERSION "1.0.0"

using namespace std;

namespace zmey{

void zmVersionLib(char* outVersion /*sz 8*/){
  if (outVersion){
    strcpy(outVersion, ZM_VERSION);
  }
}
zmObj zmCreateConnection(const char* localPnt, const char* dbServer, const char* dbName, char* err){  
  ZManager* mr = new ZManager(localPnt, dbServer, dbName);
  auto serr = mr->getLastError();
  if (!serr.empty()){
    strcpy(err, serr.c_str());
    delete mr;
    return nullptr;
  }
  return mr;
}
void zmDisconnect(zmObj zo){
  if (zo){
    delete static_cast<ZManager*>(zo);
  }
}
void zmSetErrorCBack(zmObj zo, zmErrorCBack ecb, zmUData ud){
  if (!zo) return;

  static_cast<ZManager*>(zo)->setErrorCBack(ecb, ud);
}
void zmGetLastError(zmObj zo, char* err/*sz 256*/){
  if (zo && err){
    strcpy(err, static_cast<ZManager*>(zo)->getLastError().c_str());
  }
}
bool zmCreateDB(zmObj zo, const char* dbName){
  if (!zo) return false;

  return static_cast<ZManager*>(zo)->createDB(dbName);
}
bool zmAddScheduler(zmObj zo, zmSchedrCng cng, uint64_t* outSchId){
  if (!zo) return false;
  
  if (!outSchId){
     static_cast<ZManager*>(zo)->errorMess("error !outSchId");
     return false;
  }
  ZM_Base::scheduler schedr;
  schedr.capasityTask = cng.capasityTask;
  schedr.connectPnt = cng.connectPnt;

  return static_cast<ZManager*>(zo)->addScheduler(schedr, *outSchId);
}
bool zmSchedulerState(zmObj zo, uint64_t schId, zmStateType* outState, zmSchedrCng* outSchCng){
  if (!zo) return false; 

  if (!outState){
     static_cast<ZManager*>(zo)->errorMess("error !outState");
     return false;
  }
  ZM_Base::scheduler schedr;
  if (static_cast<ZManager*>(zo)->schedulerState(schId, schedr)){    
    *outState = (zmey::zmStateType)schedr.state;
    if (outSchCng){
      outSchCng->capasityTask = schedr.capasityTask;
      strcpy(outSchCng->connectPnt, schedr.connectPnt.c_str());
    }
    return true;
  }
  return false;
}
uint32_t zmGetAllSchedulers(zmObj zo, uint64_t** outSchId){
  if (!zo) return false; 

  auto schedrs = static_cast<ZManager*>(zo)->getAllSchedulers();
  size_t ssz = schedrs.size();
  if (ssz > 0){
    *outSchId = (uint64_t*)realloc(*outSchId, ssz * sizeof(uint64_t));
    memcpy(*outSchId, schedrs.data(), ssz * sizeof(uint64_t));
  }else{
    *outSchId = nullptr;
  }
  return (uint32_t)ssz;
}
bool zmAddWorker(zmObj zo, uint64_t schId, zmWorkerCng cng, uint64_t* outWId){
  if (!zo) return false;

  if (!outWId){
     static_cast<ZManager*>(zo)->errorMess("error !outWId");
     return false;
  }
  ZM_Base::worker worker;
  worker.capasityTask = cng.capasityTask;
  worker.connectPnt = cng.connectPnt;

  return static_cast<ZManager*>(zo)->addWorker(schId, worker, *outWId);
}
bool zmWorkerState(zmObj zo, uint64_t wId, zmStateType* outState, zmWorkerCng* outWCng){
  if (!zo) return false; 

  if (!outState){
     static_cast<ZManager*>(zo)->errorMess("error !outState");
     return false;
  }
  ZM_Base::worker worker;
  if (static_cast<ZManager*>(zo)->workerState(wId, worker)){    
    *outState = (zmey::zmStateType)worker.state;
    if (outWCng){
      outWCng->exr = (zmey::zmExecutorType)worker.exrType;
      outWCng->capasityTask = worker.capasityTask;
      strcpy(outWCng->connectPnt, worker.connectPnt.c_str());
    }
    return true;
  }
  return false;
}
uint32_t zmGetAllWorkers(zmObj zo, uint64_t schId, uint64_t** outWId){
  if (!zo) return false; 

  auto workers = static_cast<ZManager*>(zo)->getAllSchedulers();
  size_t wsz = workers.size();
  if (wsz > 0){ 
    *outWId = (uint64_t*)realloc(*outWId, wsz * sizeof(uint64_t));
    memcpy(*outWId, workers.data(), wsz * sizeof(uint64_t));
  }else{
    *outWId = nullptr;
  }
  return (uint32_t)wsz;
}
bool zmAddTask(zmObj zo, zmTaskCng cng, uint64_t* outTId){
  if (!zo) return false;

  if (!outTId || !cng.script){
     static_cast<ZManager*>(zo)->errorMess("error !outTId || !cng.script");
     return false;
  }
  ZM_Base::task task;
  task.averDurationSec = cng.averDurationSec;
  task.maxDurationSec = cng.maxDurationSec;
  task.exr = (ZM_Base::executorType)cng.exr;
  task.script = cng.script;

  return static_cast<ZManager*>(zo)->addTask(task, *outTId);
}
bool zmGetTaskCng(zmObj zo, uint64_t tId, zmTaskCng* outTCng){
   if (!zo) return false; 

  if (!outTCng){
     static_cast<ZManager*>(zo)->errorMess("error !outTCng");
     return false;
  }
  ZM_Base::task task;
  if (static_cast<ZManager*>(zo)->getTaskCng(tId, task)){    
    outTCng->exr = (zmey::zmExecutorType)task.exr;
    outTCng->averDurationSec = task.averDurationSec;
    outTCng->maxDurationSec = task.maxDurationSec;
   
    outTCng->script = (char*)realloc(outTCng->script, task.script.size());
    strcpy(outTCng->script, task.script.c_str());
    return true;
  }
  return false;
}
uint32_t zmGetAllTasks(zmObj zo, uint64_t** outTId){
  if (!zo) return false; 

  auto tasks = static_cast<ZManager*>(zo)->getAllTasks();
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
    static_cast<ZManager*>(zo)->errorMess("error !outQTId");
    return false;
  }
  if ((cng.prevTasksCnt > 0) && !cng.prevTasksQId){
    static_cast<ZManager*>(zo)->errorMess("error !cng.prevTasksQId");
    return false;
  } 

  ZM_Base::task task;
  task.tId = cng.tId;
  if (cng.params){
    task.params = cng.params;
  }
  task.priority = cng.priority;
 
  vector<uint64_t> prevTasks;
  for (int i = 0; i < cng.prevTasksCnt; ++i){
    prevTasks.push_back(cng.prevTasksQId[i]);
  }  
  return static_cast<ZManager*>(zo)->pushTaskToQueue(task, prevTasks, *outQTId);
}
bool zmGetQueueTaskCng(zmObj zo, uint64_t qtId, zmQueueTaskCng* outCng){
  if (!zo) return false;
  
  ZM_Base::task task;
  vector<uint64_t> prevTasks;
  if (static_cast<ZManager*>(zo)->getQueueTaskCng(qtId, task, prevTasks)){
    outCng->tId = task.tId;
    outCng->priority = task.priority;
    
    if (!task.params.empty()){ 
      outCng->params = (char*)realloc(outCng->params, task.params.size());
      memcpy(outCng->params, task.params.data(), task.params.size());
    }else{
      outCng->params = nullptr;
    }
    if (!prevTasks.empty()){ 
      outCng->prevTasksCnt = prevTasks.size();
      outCng->prevTasksQId = (uint64_t*)realloc(outCng->prevTasksQId, outCng->prevTasksCnt * sizeof(uint64_t));
      memcpy(outCng->prevTasksQId, prevTasks.data(), outCng->prevTasksCnt * sizeof(uint64_t));
    }else{
      outCng->prevTasksCnt = 0;
      outCng->prevTasksQId = nullptr;
    }
    return true;
  }
  return false;
}
bool zmGetQueueTaskState(zmObj zo, uint64_t qtId, zmQueueTaskState* outState){
  if (!zo) return false;
  
  ZM_Base::task task;
  if (static_cast<ZManager*>(zo)->getQueueTaskState(qtId, task)){
    outState->progress = task.progress;
    outState->state = (zmey::zmStateType)task.state;
    if (!task.result.empty()){ 
      outState->result = (char*)realloc(outState->result, task.result.size());
    }else{
      outState->result = nullptr;
    }
    return true;
  }
  return false;
}
uint32_t zmGetAllQueueTasks(zmObj zo, uint64_t** outQTId){
  if (!zo) return false; 

  auto tasks = static_cast<ZManager*>(zo)->getAllQueueTasks();
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