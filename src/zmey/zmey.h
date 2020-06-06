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

#include <cstdint>

#if defined(__cplusplus)
extern "C" {
namespace zmey{
#endif /* __cplusplus */

///////////////////////////////////////////////////////////////////////////////
/// Common

/// internal connection object
typedef void* zmObj;

/// state
enum zmStateType{
  undefined       = -1,
  zmReady         = 0,
  zmStart         = 1,
  zmRunning       = 2,
  zmPause         = 3,
  zmStop          = 4,    
  zmCompleted     = 5,
  zmError         = 6,
  zmNotResponding = 7,
};

/// executor type
enum zmExecutorType{
  zmBash =   0,
  zmCmd =    1,
  zmPython = 2,
};

/// database type
enum zmDbType{
  zmPostgreSQL = 0,
};

/// version lib
/// @param[out] outVersion. The memory is allocated by the user
ZMEY_API void zmVersionLib(char* outVersion /*sz 8*/);

///////////////////////////////////////////////////////////////////////////////
/// Connection with DB

/// connection config
struct zmConnectCng{
  zmDbType dbType;          ///< db type
  char connectPnt[255];     ///< local connection point: IP or DNS:port
  char dbServer[255];       ///< PG: server IP or DNS
  char dbName[255];         ///< PG: if the database does not exist - will be created
  char dbUser[255];         ///< db user name
  char dbPassw[255];        ///< db user password
};

/// create connection
/// @param[in] zmConnectCng - connection config
/// @param[out] err - error string. The memory is allocated by the user
/// @return object connect
ZMEY_API zmObj zmCreateConnection(zmConnectCng, char* err /*sz 256*/);

/// disconnect !!! zmObj after the call will be deleted !!! 
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

///////////////////////////////////////////////////////////////////////////////
/// User

/// user config
struct zmUserCng{  
  char name[255];    ///< unique name
  char passw[255];   ///< password  
  char* description;  ///< the memory is allocated by the user   
};

/// add new user
/// @param[in] zmObj - object connect
/// @param[in] newUserCng - new user config
/// @param[out] outUserId - new user id
/// @return true - ok
ZMEY_API bool zmAddUser(zmObj, zmUserCng newUserCng, uint64_t* outUserId);

/// get exist user id
/// @param[in] zmObj - object connect
/// @param[in] cng - user config
/// @param[out] outUserId - user id
/// @return true - ok
ZMEY_API bool zmGetUserId(zmObj, zmUserCng cng, uint64_t* outUserId);

/// get user config
/// @param[in] zmObj - object connect
/// @param[in] userId - user id
/// @param[out] outCng - user config
/// @return true - ok
ZMEY_API bool zmGetUserCng(zmObj, uint64_t userId, zmUserCng* outCng);

/// change user
/// @param[in] zmObj - object connect
/// @param[in] userId - user id
/// @param[in] newCng - new user cng
/// @return true - ok
ZMEY_API bool zmChangeUser(zmObj, uint64_t userId, zmUserCng newCng);

/// delete user
/// @param[in] zmObj - object connect
/// @param[in] userId - user id
/// @return true - ok
ZMEY_API bool zmDelUser(zmObj, uint64_t userId);

/// get all users
/// @param[in] zmObj - object connect
/// @param[out] outUserId - users id
/// @return count of users
ZMEY_API uint32_t zmGetAllUsers(zmObj, uint64_t** outUserId);

///////////////////////////////////////////////////////////////////////////////
/// Group users

/// group config
struct zmGroupCng{  
  char name[255];    ///< unique name
  char passw[255];   ///< password  
  char* description;  ///< the memory is allocated by the user
};
/// TODO

///////////////////////////////////////////////////////////////////////////////
/// Scheduler

/// scheduler config
struct zmSchedrCng{
  char connectPnt[255];          ///< remote connection point: IP or DNS:port
  uint32_t capasityTask = 10000; ///< permissible simultaneous number of tasks 
};
/// add new scheduler
/// @param[in] zmObj - object connect
/// @param[in] cng - scheduler config
/// @param[out] outSchId - new scheduler id
/// @return true - ok
ZMEY_API bool zmAddScheduler(zmObj, zmSchedrCng cng, uint64_t* outSchId);

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
/// @param[in] state - choose with current state. If the state is 'undefined', select all
/// @param[out] outSchId - schedulers id
/// @return count of schedulers
ZMEY_API uint32_t zmGetAllSchedulers(zmObj, zmStateType state, uint64_t** outSchId);

///////////////////////////////////////////////////////////////////////////////
/// Worker

/// worker config
struct zmWorkerCng{
  uint64_t schId;             ///< scheduler id 
  zmExecutorType exr;         ///< executor type
  uint32_t capasityTask = 10; ///< permissible simultaneous number of tasks
  char connectPnt[255];       ///< remote connection point: IP or DNS:port   
};
  
/// add new worker
/// @param[in] zmObj - object connect
/// @param[in] cng - worker config
/// @param[out] outWId - new worker id
/// @return true - ok
ZMEY_API bool zmAddWorker(zmObj, zmWorkerCng cng, uint64_t* outWId);

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
/// @param[in] state - choose with current state. If the state is 'undefined', select all
/// @param[out] outWId - worker id 
/// @return count of schedulers
ZMEY_API uint32_t zmGetAllWorkers(zmObj, uint64_t schId, zmStateType state, uint64_t** outWId);

///////////////////////////////////////////////////////////////////////////////
/// Pipeline of tasks

/// pipeline config
struct zmPipelineCng{
  uint64_t userId;         ///< user id
  char name[255];          ///< pipeline name
  char* description;       ///< description of pipeline. The memory is allocated by the user
};

/// add pipeline
/// @param[in] zmObj - object connect
/// @param[in] cng - pipeline config
/// @param[out] outPPLId - pipeline id
/// @return true - ok
ZMEY_API bool zmAddPipeline(zmObj, zmPipelineCng cng, uint64_t* outPPLId);

/// get pipeline config
/// @param[in] zmObj - object connect
/// @param[in] pplId - pipeline id
/// @param[out] outCng - pipeline config
/// @return true - ok
ZMEY_API bool zmGetPipelineCng(zmObj, uint64_t pplId, zmPipelineCng* outCng);

/// change pipeline config
/// @param[in] zmObj - object connect
/// @param[in] pplId - pipeline id
/// @param[in] newCng - pipeline config
/// @return true - ok
ZMEY_API bool zmChangePipelineCng(zmObj, uint64_t pplId, zmPipelineCng newCng);

/// delete pipeline
/// @param[in] zmObj - object connect
/// @param[in] pplId - pipeline
/// @return true - ok
ZMEY_API bool zmDelPipeline(zmObj, uint64_t pplId);

/// get all pipelines
/// @param[in] zmObj - object connect
/// @param[in] userId - user id
/// @param[out] outPPLId - pipeline id 
/// @return count of pipelines
ZMEY_API uint32_t zmGetAllPipelines(zmObj, uint64_t userId, uint64_t** outPPLId);

///////////////////////////////////////////////////////////////////////////////
/// Task template 

/// task template config
struct zmTaskTemplateCng{
  uint64_t parent;          ///< user id
  zmExecutorType exr;       ///< executor type
  uint32_t averDurationSec; ///< estimated lead time 
  uint32_t maxDurationSec;  ///< maximum lead time
  char name[255];           ///< task template name
  char* description;        ///< description of task. The memory is allocated by the user
  char* script;             ///< script on bash, python or cmd. The memory is allocated by the user
};
/// add new task template
/// @param[in] zmObj - object connect
/// @param[in] cng - task template config
/// @param[out] outTId - new task id
/// @return true - ok
ZMEY_API bool zmAddTaskTemplate(zmObj, zmTaskTemplateCng cng, uint64_t* outTId);

/// get task template cng
/// @param[in] zmObj - object connect
/// @param[in] tId - task id
/// @param[out] outTCng - task config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmGetTaskTemplateCng(zmObj, uint64_t tId, zmTaskTemplateCng* outTCng);

/// change task template cng
/// A new record is created for each change, the old one is not deleted.
/// @param[in] zmObj - object connect
/// @param[in] tId - task id
/// @param[in] newTCng - new task config
/// @param[out] outTId - new task id
/// @return true - ok
ZMEY_API bool zmChangeTaskTemplateCng(zmObj, uint64_t tId, zmTaskTemplateCng newTCng, uint64_t* outTId);

/// delete task template
/// The record is marked, but not deleted.
/// @param[in] zmObj - object connect
/// @param[in] tId - task id
/// @return true - ok
ZMEY_API bool zmDelTaskTemplate(zmObj, uint64_t tId);

/// get all tasks templates
/// @param[in] zmObj - object connect
/// @param[in] parent - user id
/// @param[out] outTId - task id
/// @return count of tasks
ZMEY_API uint32_t zmGetAllTaskTemplates(zmObj, uint64_t parent, uint64_t** outTId);

///////////////////////////////////////////////////////////////////////////////
/// Task of pipeline

struct zmScreenRect{
  uint32_t x, y, w, h;
};

/// pipeline task config
struct zmTaskCng{
  uint64_t pplId;          ///< pipeline id
  uint64_t tId;            ///< task template id
  uint64_t* prevTasksId;   ///< pipeline task id of previous tasks to be completed
  uint64_t* nextTasksId;   ///< pipeline task id of next tasks
  uint32_t prevTasksCnt;   ///< pipeline task previous count
  uint32_t nextTasksCnt;   ///< pipeline task next count
  uint32_t priority;       ///< [1..3]
  char* params;            ///< params of script: -key=value
  zmScreenRect screenRect; ///< screenRect
};

/// add pipeline task
/// @param[in] zmObj - object connect
/// @param[in] cng - pipeline task config
/// @param[out] outTaskId - pipeline task id
/// @return true - ok
ZMEY_API bool zmAddTask(zmObj, zmTaskCng cng, uint64_t* outTaskId);

/// get pipeline task config
/// @param[in] zmObj - object connect
/// @param[in] tId - pipeline task id
/// @param[out] outTCng - pipeline task config
/// @return true - ok
ZMEY_API bool zmGetTaskCng(zmObj, uint64_t tId, zmTaskCng* outTCng);

/// change pipeline task config
/// @param[in] zmObj - object connect
/// @param[in] tId - pipeline task id
/// @param[in] newCng - pipeline task config
/// @return true - ok
ZMEY_API bool zmChangeTaskCng(zmObj, uint64_t tId, zmTaskCng newCng);

/// delete pipeline task
/// @param[in] zmObj - object connect
/// @param[in] tId - pipeline task id
/// @return true - ok
ZMEY_API bool zmDelTask(zmObj, uint64_t tId);

/// start pipeline task
/// @param[in] zmObj - object connect
/// @param[in] tId - pipeline task id
/// @return true - ok
ZMEY_API bool zmStartTask(zmObj, uint64_t tId);

/// stop pipeline task
/// @param[in] zmObj - object connect
/// @param[in] tId - pipeline task id
/// @return true - ok
ZMEY_API bool zmStopTask(zmObj, uint64_t tId);

/// pause pipeline task
/// @param[in] zmObj - object connect
/// @param[in] tId - pipeline task id
/// @return true - ok
ZMEY_API bool zmPauseTask(zmObj, uint64_t tId);

/// pipeline task state
struct zmTaskState{
  uint32_t progress;      ///< [0..100]
  zmStateType state;  
  char* result;           ///< string result
};
/// get pipeline task state
/// @param[in] zmObj - object connect
/// @param[in] tId - pipeline task id
/// @param[out] outTState - pipeline task state
/// @return true - ok
ZMEY_API bool zmGetTaskState(zmObj, uint64_t tId, zmTaskState* outTState);

/// get all pipeline tasks
/// @param[in] zmObj - object connect
/// @param[in] tId - pipeline id
/// @param[in] state - choose with current state. If the state is 'undefined', select all
/// @param[out] outTId - pipeline task id 
/// @return count of pipeline tasks
ZMEY_API uint32_t zmGetAllTasks(zmObj, uint64_t pplId, zmStateType state, uint64_t** outTId);



///////////////////////////////////////////////////////////////////////////////
/// free resouces
ZMEY_API void zmFreeResouces(uint64_t*, char*);

#if defined(__cplusplus)
}}
#endif /* __cplusplus */

#endif /* ZMEY_C_API_H_ */
