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

///////////////////////////////////////////////////////////////////////////////
/// Common

/// internal connection object
typedef void* zmConn;

/// state
enum zmStateType{
  zmSTATE_UNDEFINED = -1,
  zmSTATE_READY,
  zmSTATE_START,
  zmSTATE_RUNNING,
  zmSTATE_PAUSE,
  zmSTATE_STOP,    
  zmSTATE_COMPLETED,
  zmSTATE_ERROR,
  zmSTATE_CANCEL,
  zmSTATE_NOT_RESPONDING,
};

/// version lib
/// @param[out] outVersion. The memory is allocated by the user
ZMEY_API void zmVersionLib(char* outVersion /*sz 8*/);

///////////////////////////////////////////////////////////////////////////////
/// Connection with DB

/// connection config
struct zmConfig{
  char* connectStr;         ///< connection string
};

/// create connection
/// @param[in] zmConfig - connection config
/// @param[out] err - error string. The memory is allocated by the user
/// @return object connect
ZMEY_API zmConn zmCreateConnection(zmConfig, char* err /*sz 256*/);

/// disconnect !!! zmConn after the call will be deleted !!! 
/// @param[in] zmConn - object connect
ZMEY_API void zmDisconnect(zmConn);

/// create tables, will be created if not exist
/// @param[in] zmConn - object connect
/// @return true - ok
ZMEY_API bool zmCreateTables(zmConn);

typedef void* zmUData;                                     ///< user data    
typedef void(*zmErrorCBack)(const char* mess, zmUData);    ///< error callback

/// set error callback
/// @param[in] zmConn - object connect
/// @param[in] zmErrorCBack - error callback
/// @param[in] zmUData - user data   
/// @return true - ok 
ZMEY_API bool zmSetErrorCBack(zmConn, zmErrorCBack, zmUData);

/// last error str
/// @param[in] zmConn - object connect
/// @param[out] err - error string. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmGetLastError(zmConn, char* err/*sz 256*/);

///////////////////////////////////////////////////////////////////////////////
/// Scheduler

/// scheduler config
struct zmSchedr{
  uint32_t capacityTask = 10000; ///< permissible simultaneous number of tasks 
  char connectPnt[256];          ///< remote connection point: IP or DNS:port  
  char name[256];                ///< scheduler name. Optional.
  char* description;             ///< description of schedr. The memory is allocated by the user. May be NULL 
};
/// add new scheduler
/// @param[in] zmConn - object connect
/// @param[in] cng - scheduler config
/// @param[out] outSchId - new scheduler id
/// @return true - ok
ZMEY_API bool zmAddScheduler(zmConn, zmSchedr cng, uint64_t* outSchId);

/// scheduler cng
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id 
/// @param[out] outCng - scheduler config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmGetScheduler(zmConn, uint64_t sId, zmSchedr* outCng);

/// change scheduler cng
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id 
/// @param[in] newCng - scheduler config
/// @return true - ok
ZMEY_API bool zmChangeScheduler(zmConn, uint64_t sId, zmSchedr newCng);

/// delete scheduler
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id
/// @return true - ok
ZMEY_API bool zmDelScheduler(zmConn, uint64_t sId);

/// start scheduler
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id
/// @return true - ok
ZMEY_API bool zmStartScheduler(zmConn, uint64_t sId);

/// pause scheduler
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id
/// @return true - ok
ZMEY_API bool zmPauseScheduler(zmConn, uint64_t sId);

/// ping scheduler
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id
/// @return true - ok
ZMEY_API bool zmPingScheduler(zmConn, uint64_t sId);

/// scheduler state
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id 
/// @param[out] outState - scheduler state
/// @return true - ok
ZMEY_API bool zmSchedulerState(zmConn, uint64_t sId, zmStateType* outState);

/// get all schedulers
/// @param[in] zmConn - object connect
/// @param[in] state - choose with current state. If the state is 'UNDEFINED', select all
/// @param[out] outSchId - schedulers id. Pass NULL, no need to free memory
/// @return count of schedulers
ZMEY_API uint32_t zmGetAllSchedulers(zmConn, zmStateType state, uint64_t** outSchId);

///////////////////////////////////////////////////////////////////////////////
/// Worker

/// worker config
struct zmWorker{
  uint64_t sId;               ///< scheduler id 
  uint32_t capacityTask = 10; ///< permissible simultaneous number of tasks
  char connectPnt[256];       ///< remote connection point: IP or DNS:port  
  char name[256];             ///< worker name. Optional.
  char* description;          ///< description of worker. The memory is allocated by the user. May be NULL 
};
  
/// add new worker
/// @param[in] zmConn - object connect
/// @param[in] cng - worker config
/// @param[out] outWId - new worker id
/// @return true - ok
ZMEY_API bool zmAddWorker(zmConn, zmWorker cng, uint64_t* outWId);

/// worker cng
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @param[out] outCng - worker config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmGetWorker(zmConn, uint64_t wId, zmWorker* outCng);

/// change worker cng
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @param[in] newCng - worker config
/// @return true - ok
ZMEY_API bool zmChangeWorker(zmConn, uint64_t wId, zmWorker newCng);

/// delete worker
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @return true - ok
ZMEY_API bool zmDelWorker(zmConn, uint64_t wId);

/// start worker
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @return true - ok
ZMEY_API bool zmStartWorker(zmConn, uint64_t wId);

/// pause worker
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @return true - ok
ZMEY_API bool zmPauseWorker(zmConn, uint64_t wId);

/// ping worker
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @return true - ok
ZMEY_API bool zmPingWorker(zmConn, uint64_t wId);

/// worker state
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id, order by id.
/// @param[in] wCnt - worker count
/// @param[out] outState - worker state. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmWorkerState(zmConn, uint64_t* wId, uint32_t wCnt, zmStateType* outState);

/// get all workers
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id 
/// @param[in] state - choose with current state. If the state is 'UNDEFINED', select all
/// @param[out] outWId - worker id. Pass NULL, no need to free memory
/// @return count of schedulers
ZMEY_API uint32_t zmGetAllWorkers(zmConn, uint64_t sId, zmStateType state, uint64_t** outWId);

///////////////////////////////////////////////////////////////////////////////
/// Task template 

/// task template config
struct zmTaskTemplate{
  uint64_t userId;          ///< user id
  uint64_t schedrPresetId;  ///< schedr preset id. Default 0
  uint64_t workerPresetId;  ///< worker preset id. Default 0
  uint32_t averDurationSec; ///< estimated lead time, sec 
  uint32_t maxDurationSec;  ///< maximum lead time, sec
  char name[256];           ///< task template name. Necessarily
  char* description;        ///< description of task. The memory is allocated by the user. May be NULL
  char* script;             ///< script on bash, python or cmd. The memory is allocated by the user
};

/// add new task template
/// @param[in] zmConn - object connect
/// @param[in] cng - task template config
/// @param[out] outTId - new task template id
/// @return true - ok
ZMEY_API bool zmAddTaskTemplate(zmConn, zmTaskTemplate cng, uint64_t* outTId);

/// get task template cng
/// @param[in] zmConn - object connect
/// @param[in] ttId - task template  id
/// @param[out] outTCng - task template config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmGetTaskTemplate(zmConn, uint64_t ttId, zmTaskTemplate* outTCng);

/// change task template cng
/// A new record is created for each change, the old one is not deleted.
/// @param[in] zmConn - object connect
/// @param[in] ttId - task template  id
/// @param[in] newTCng - new task template config
/// @return true - ok
ZMEY_API bool zmChangeTaskTemplate(zmConn, uint64_t ttId, zmTaskTemplate newTCng);

/// delete task template
/// The record is marked, but not deleted.
/// @param[in] zmConn - object connect
/// @param[in] ttId - task template id
/// @return true - ok
ZMEY_API bool zmDelTaskTemplate(zmConn, uint64_t ttId);

/// get all tasks templates
/// @param[in] zmConn - object connect
/// @param[in] userId - user id
/// @param[out] outTId - task template id. Pass NULL, no need to free memory
/// @return count of tasks
ZMEY_API uint32_t zmGetAllTaskTemplates(zmConn, uint64_t userId, uint64_t** outTId);

///////////////////////////////////////////////////////////////////////////////
/// Task object

/// task config
struct zmTask{
  uint64_t ttlId;          ///< task template id
  char* params;            ///< CLI params for script. May be NULL
};

/// start task
/// @param[in] zmConn - object connect
/// @param[in] tcng - task config
/// @param[out] outTId - task object id
/// @return true - ok
ZMEY_API bool zmStartTask(zmConn, zmTask tcng, uint64_t* outTId);

/// stop task
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @return true - ok
ZMEY_API bool zmStopTask(zmConn, uint64_t tId);

/// cancel task (when not yet taken to work)
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @return true - ok
ZMEY_API bool zmCancelTask(zmConn, uint64_t tId);

/// pause task
/// @param[in] zmConn - object connect
/// @param[in] tId - tId - task object id
/// @return true - ok
ZMEY_API bool zmPauseTask(zmConn, uint64_t tId);

/// continue task
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @return true - ok
ZMEY_API bool zmContinueTask(zmConn, uint64_t tId);

/// task state
struct zmTaskState{
  uint32_t progress;      ///< [0..100]
  zmStateType state;
};

/// get task state
/// @param[in] zmConn - object connect
/// @param[in] tId - task id, order by tId
/// @param[in] tCnt - task id count
/// @param[out] outTState - task state. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmStateOfTask(zmConn, uint64_t* tId, uint32_t tCnt, zmTaskState* outTState);

/// get task result
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @param[out] outTResult - task object result. Pass NULL, no need to free memory
/// @return true - ok
ZMEY_API bool zmResultOfTask(zmConn, uint64_t tId, char** outTResult);

/// task time
struct zmTaskTime{
  char createTime[32];
  char takeInWorkTime[32];        
  char startTime[32]; 
  char stopTime[32]; 
};
/// get task time
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @param[out] outTTime - task time. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmTimeOfTask(zmConn, uint64_t tId, zmTaskTime* outTTime);

/// task state callback
typedef void(*zmChangeTaskStateCBack)(uint64_t tId, uint64_t userId, zmStateType prevState, zmStateType newState, zmUData);

/// set change task state callback
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @param[in] cback
/// @param[in] userData
/// @return true - ok
ZMEY_API bool zmSetChangeTaskStateCBack(zmConn, uint64_t tId, uint64_t userId, zmChangeTaskStateCBack cback, zmUData);

///////////////////////////////////////////////////////////////////////////////
/// Internal errors

/// error
struct zmInternError{
  uint64_t sId;        ///< scheduler id 
  uint64_t wId;        ///< worker id 
  char createTime[32];
  char message[256];
};

/// get errors
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id. If '0' then all
/// @param[in] wId - worker id. If '0' then all
/// @param[in] mCnt - last mess max count
/// @param[out] outErrors. The memory is allocated by the user
/// @return count of errors
ZMEY_API uint32_t zmGetInternErrors(zmConn, uint64_t sId, uint64_t wId, uint32_t mCnt, zmInternError* outErrors);

///////////////////////////////////////////////////////////////////////////////
/// free resouces
/// @param[in] zmConn - object connect
ZMEY_API void zmFreeResources(zmConn);

#if defined(__cplusplus)
}}
#endif /* __cplusplus */

#endif /* ZMEY_C_API_H_ */
