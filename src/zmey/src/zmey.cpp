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

namespace zmey{

void zmVersionLib(char* outVersion /*sz 8*/){
  if (outVersion)
    strcpy(outVersion, ZM_VERSION);
}

zmObj zmCreateConnection(const char* localPnt, const char* dbServer, const char* dbName, char* err){
  
  if (localPnt && dbServer && dbName){
    return new ZManager(localPnt, dbServer, dbName);
  }
  else if (err){
    strcpy(err, "zmCreateConnection error: !localPnt || !dbServer || !dbName");
  } 
  return nullptr;
}

void zmDisconnect(zmObj zo){
  if (zo){
    delete static_cast<ZManager*>(zo);
  }
}

bool zmGetLastError(zmObj zo, char* err/*sz 256*/){
  if (zo && err){
    strcpy(err, static_cast<ZManager*>(zo)->getLastError().c_str());
    return true;
  }else{
    return false;
  } 
}

bool zmCreateDB(zmObj, const char* dbName){

}

bool zmAddScheduler(zmObj, zmSchedrCng, uint64_t* outSchId){

}

bool zmGetScheduler(zmObj, uint64_t schId, zmStateType* outState, zmSchedrCng* outSchCng){

}

uint32_t zmGetAllSchedulers(zmObj, uint64_t** outSchId){

}

bool zmAddWorker(zmObj, uint64_t schId, zmWorkerCng, uint64_t* outWId){

}

bool zmGetWorker(zmObj, uint64_t wId, zmStateType* outState, zmWorkerCng* outWCng){

}

uint32_t zmGetAllWorkers(zmObj, uint64_t schId, uint64_t** outWId){

}

bool zmAddTask(zmObj, zmTaskCng, uint64_t* outTId){

}

bool zmGetTask(zmObj, uint64_t tId, zmStateType* outState, zmTaskCng* outTCng){

}

uint32_t zmGetAllTasks(zmObj, uint64_t** outTId){

}

bool zmPushTaskToQueue(zmObj, uint64_t tId, uint32_t priority, const char* params, uint64_t* outQTId){

}

bool zmGetQueueTaskState(zmObj, uint64_t qtId, zmQueueTaskState* outState){

}

uint32_t zmGetAllQueueTasks(zmObj, uint64_t** outQTId);

}
}