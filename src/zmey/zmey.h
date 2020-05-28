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

#ifndef ZMEY_C_API_H_
#define ZMEY_C_API_H_

#ifdef _WIN32
#ifdef ZMEYDLL_EXPORTS
#define ZMEY_API __declspec(dllexport)
#else
#define ZMEY_API __declspec(dllimport)
#endif
#else
#define ZMEY_API
#endif

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
namespace zmey{
#endif /* __cplusplus */

//////////////////////////////////////////////////////////////////////////
///*** Common ***/////////////////////////////////////////////////////////

/// internal connection object
typedef void* zmObj;

/// state
enum zmStateType{
  ready         = 0,
  start         = 1,
  running       = 2,
  pause         = 3,
  stop          = 4,    
  completed     = 5,
  error         = 6,
  notResponding = 7,
};

/// executor type
enum zmExecutorType{
  bash =   0,
  cmd =    1,
  python = 2,
};

/// version lib
/// @param[out] outVersion. The memory is allocated by the user
ZMEY_API void zmVersionLib(char* outVersion /*sz 8*/);

/// create connection
/// @param[in] localPnt - IP or DNS:port
/// @param[in] dbServer - database server IP or DNS (for sqlite - path to file db, for db on files - path to dir)
/// @param[in] dbName - database name (for sqlite or files - empty) 
/// @param[out] err - error string. The memory is allocated by the user
/// @return object connect
ZMEY_API zmObj 
zmCreateConnection(const char* localPnt, const char* dbServer, const char* dbName, char* err /*sz 256*/);

/// disconnect
/// @param[in] zmObj - object connect
ZMEY_API void zmDisconnect(zmObj);

typedef void* zmUData;                                     ///< user data    
typedef void(*zmErrorCBack)(const char* mess, zmUData);    ///< error callback

/// set error callback
/// @param[in] zmObj - object connect
/// @param[in] zmErrorCBack - error callback
/// @param[in] zmUData - user data    
ZMEY_API void zmSetErrorCBack(zmObj, zmErrorCBack, zmUData);

/// last error str
/// @param[in] zmObj - object connect
/// @param[out] err - error string. The memory is allocated by the user
ZMEY_API void zmGetLastError(zmObj, char* err/*sz 256*/);

/// create a database if it does not exist
/// @param[in] zmObj - object connect
/// @param[in] dbName - database name (for sqlite - path to file db, for db on files - path to dir)
/// @return true - ok
ZMEY_API bool zmCreateDB(zmObj, const char* dbName);

//////////////////////////////////////////////////////////////////////////
///*** Scheduler ***//////////////////////////////////////////////////////

/// scheduler config
struct zmSchedrCng{
  char connectPnt[255];          ///< IP or DNS:port
  uint32_t capasityTask = 10000; ///< permissible simultaneous number of tasks 
};
/// add new scheduler
/// @param[in] zmObj - object connect
/// @param[in] zmSchedr - scheduler config
/// @param[out] outSchId - new scheduler id
/// @return true - ok
ZMEY_API bool zmAddScheduler(zmObj, zmSchedrCng, uint64_t* outSchId);

/// scheduler state
/// @param[in] zmObj - object connect
/// @param[in] schId - scheduler id 
/// @param[out] outState - scheduler state
/// @param[out] outSchCng - scheduler config. Not necessary
/// @return true - ok
ZMEY_API bool 
zmSchedulerState(zmObj, uint64_t schId, zmStateType* outState, zmSchedrCng* outSchCng = nullptr);

/// get all schedulers
/// @param[in] zmObj - object connect
/// @param[out] outSchId - schedulers id
/// @return count of schedulers
ZMEY_API uint32_t zmGetAllSchedulers(zmObj, uint64_t** outSchId);

//////////////////////////////////////////////////////////////////////////
///*** Worker ***/////////////////////////////////////////////////////////

/// worker config
struct zmWorkerCng{
  zmExecutorType exr;         ///< executor type
  char connectPnt[255];       ///< IP or DNS:port
  uint32_t capasityTask = 10; ///< permissible simultaneous number of tasks 
};
  
/// add new worker
/// @param[in] zmObj - object connect
/// @param[in] schId - scheduler id
/// @param[in] zmWorkerCng - worker config
/// @param[out] outWId - new worker id
/// @return true - ok
ZMEY_API bool zmAddWorker(zmObj, uint64_t schId, zmWorkerCng, uint64_t* outWId);

/// worker state
/// @param[in] zmObj - object connect
/// @param[in] wId - worker id
/// @param[out] outState - worker state
/// @param[out] outWCng - worker config. Not necessary
/// @return true - ok
ZMEY_API bool
zmWorkerState(zmObj, uint64_t wId, zmStateType* outState, zmWorkerCng* outWCng = nullptr);

/// get all workers
/// @param[in] zmObj - object connect
/// @param[in] schId - scheduler id
/// @param[out] outWId - worker id 
/// @return count of schedulers
ZMEY_API uint32_t zmGetAllWorkers(zmObj, uint64_t schId, uint64_t** outWId);

//////////////////////////////////////////////////////////////////////////
///*** Task ***///////////////////////////////////////////////////////////

/// task config
struct zmTaskCng{
  zmExecutorType exr;       ///< executor type
  uint32_t averDurationSec; ///< estimated lead time 
  uint32_t maxDurationSec;  ///< maximum lead time
  char* script;             ///< script on bash, python or cmd. The memory is allocated by the user
};
/// add new task
/// @param[in] zmObj - object connect
/// @param[in] zmTaskCng - task config
/// @param[out] outTId - new task id
/// @return true - ok
ZMEY_API bool zmAddTask(zmObj, zmTaskCng, uint64_t* outTId);

/// get task cng
/// @param[in] zmObj - object connect
/// @param[in] tId - task id
/// @param[out] outTCng - task config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool
zmGetTaskCng(zmObj, uint64_t tId, zmTaskCng* outTCng);

/// get all tasks
/// @param[in] zmObj - object connect
/// @param[out] outTId - task id
/// @return count of tasks
ZMEY_API uint32_t zmGetAllTasks(zmObj, uint64_t** outTId);

//////////////////////////////////////////////////////////////////////////
///*** Queue task ***/////////////////////////////////////////////////////

/// queue task config
struct zmQueueTaskCng{
  uint64_t tId;           ///< task id
  uint64_t* prevTasksQId; ///< queue task id of previous tasks to be completed
  uint32_t prevTasksCnt;  ///< queue task count
  uint32_t priority;      ///< [1..3]
  char* params;           ///< params of script: -key=value
};

/// push task to queue
/// @param[in] zmObj - object connect
/// @param[in] zmQueueTaskCng - queue task config
/// @param[out] outQTId - queue task id
/// @return true - ok
ZMEY_API bool 
zmPushTaskToQueue(zmObj, zmQueueTaskCng, uint64_t* outQTId);

/// get queue task config
/// @param[in] zmObj - object connect
/// @param[in] qtId - queue task id
/// @param[out] outQCng - queue task config
/// @return true - ok
ZMEY_API bool
zmGetQueueTaskCng(zmObj, uint64_t qtId, zmQueueTaskCng* outQCng);

/// queue task state
struct zmQueueTaskState{
  uint32_t progress;      ///< [0..100]
  zmStateType state;  
  char* result;           ///< string result
};
/// get queue task state
/// @param[in] zmObj - object connect
/// @param[in] qtId - queue task id
/// @param[out] outQState - queue task state
/// @return true - ok
ZMEY_API bool
zmGetQueueTaskState(zmObj, uint64_t qtId, zmQueueTaskState* outQState);

/// get all queue tasks
/// @param[in] zmObj - object connect
/// @param[out] outQTId - task id 
/// @return count of queue tasks
ZMEY_API uint32_t zmGetAllQueueTasks(zmObj, uint64_t** outQTId);

/// free resouces
ZMEY_API uint32_t zmFreeResouces(uint64_t*, char*);

#if defined(__cplusplus)
}}
#endif /* __cplusplus */

#endif /* ZMEY_C_API_H_ */
