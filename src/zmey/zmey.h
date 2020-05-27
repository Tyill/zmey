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

/// internal connection object
typedef void* zmObj;

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

/// last error str
/// @param[in] zmObj - object connect
/// @param[out] err - error string. The memory is allocated by the user
ZMEY_API bool zmGetLastError(zmObj, char* err/*sz 256*/);

/// create a database if it does not exist
/// @param[in] zmObj - object connect
/// @param[in] dbName - database name (for sqlite - path to file db, for db on files - path to dir)
/// @return true - ok
ZMEY_API bool zmCreateDB(zmObj, const char* dbName);

//////////////////////////////////////////////////////////////////////////
///*** Scheduler ***//////////////////////////////////////////////////////

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

/// get scheduler state and cng
/// @param[in] zmObj - object connect
/// @param[in] schId - scheduler id 
/// @param[out] outState - scheduler state
/// @param[out] outSchCng - scheduler config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool 
zmGetScheduler(zmObj, uint64_t schId, zmStateType* outState, zmSchedrCng* outSchCng = nullptr);

/// get all schedulers
/// @param[in] zmObj - object connect
/// @param[out] outSchId - schedulers id. First pass NULL, then pass it to the same 
/// @return count of schedulers
ZMEY_API uint32_t zmGetAllSchedulers(zmObj, uint64_t** outSchId);

//////////////////////////////////////////////////////////////////////////
///*** Worker ***/////////////////////////////////////////////////////////

/// executor type
enum zmExecutorType{
  bash = 0,
  cmd = 1,
  python = 2,
};
/// worker config
struct zmWorkerCng{
  zmExecutorType exr;         ///< executor type
  char connectPnt[255];       ///< IP or DNS:port
  uint32_t capasityTask = 10; ///< permissible simultaneous number of tasks 
};
/// add new worker
/// @param[in] zmObj - object connect
/// @param[in] schId - scheduler id
/// @param[out] outWId - new worker id
/// @return true - ok
ZMEY_API bool zmAddWorker(zmObj, uint64_t schId, zmWorkerCng, uint64_t* outWId);

/// get worker state and cng
/// @param[in] zmObj - object connect
/// @param[in] wId - worker id
/// @param[out] outState - worker state
/// @param[out] outWCng - worker config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool
zmGetWorker(zmObj, uint64_t wId, zmStateType* outState, zmWorkerCng* outWCng = nullptr);

/// get all workers
/// @param[in] zmObj - object connect
/// @param[in] schId - scheduler id
/// @param[out] outWId - worker id. First pass NULL, then pass it to the same 
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

/// get task state and cng
/// @param[in] zmObj - object connect
/// @param[in] tId - task id
/// @param[out] outState - task state
/// @param[out] outTCng - task config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool
zmGetTask(zmObj, uint64_t tId, zmStateType* outState, zmTaskCng* outTCng = nullptr);

/// get all tasks
/// @param[in] zmObj - object connect
/// @param[out] outTId - task id. First pass NULL, then pass it to the same 
/// @return count of tasks
ZMEY_API uint32_t zmGetAllTasks(zmObj, uint64_t** outTId);

//////////////////////////////////////////////////////////////////////////
///*** Queue task ***/////////////////////////////////////////////////////

/// push task to queue
/// @param[in] zmObj - object connect
/// @param[in] tId - task id
/// @param[in] priority
/// @param[in] params of script: -key=value
/// @param[out] outQTId - queue task id
/// @return true - ok
ZMEY_API bool 
zmPushTaskToQueue(zmObj, uint64_t tId, uint32_t priority, const char* params, uint64_t* outQTId);

/// queue task state
struct zmQueueTaskState{
  uint32_t progress; ///< [0..100]
  uint32_t priority;
  zmStateType state;
  char* result;
};
/// get queue task state
/// @param[in] zmObj - object connect
/// @param[in] qtId - queue task id
/// @param[out] outState - task state
/// @return true - ok
ZMEY_API bool
zmGetQueueTaskState(zmObj, uint64_t qtId, zmQueueTaskState* outState);

/// get all queue tasks
/// @param[in] zmObj - object connect
/// @param[out] outQTId - task id. First pass NULL, then pass it to the same 
/// @return count of queue tasks
ZMEY_API uint32_t zmGetAllQueueTasks(zmObj, uint64_t** outQTId);


#if defined(__cplusplus)
}}
#endif /* __cplusplus */

#endif /* ZMEY_C_API_H_ */
