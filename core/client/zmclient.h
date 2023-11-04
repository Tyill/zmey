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

#if defined(__cplusplus)
extern "C" {
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
/// @return outVersion
ZMEY_API const char* zmVersionLib();

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
            
/// last error str
/// @param[in] zmConn - object connect
/// @param[out] err - error string. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmGetLastError(zmConn, char* err/*sz 256*/);

///////////////////////////////////////////////////////////////////////////////
/// Scheduler

/// scheduler config
struct zmSchedr{
  int capacityTask = 10000;      ///< permissible simultaneous number of tasks 
  char connectPnt[256];          ///< remote connection point: IP or DNS:port  
};
/// add new scheduler
/// @param[in] zmConn - object connect
/// @param[in] cng - scheduler config
/// @param[out] outSchId - new scheduler id
/// @return true - ok
ZMEY_API bool zmAddScheduler(zmConn, zmSchedr cng, int* outSchId);

/// scheduler cng
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id 
/// @param[out] outCng - scheduler config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmGetScheduler(zmConn, int sId, zmSchedr* outCng);

/// change scheduler cng
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id 
/// @param[in] newCng - scheduler config
/// @return true - ok
ZMEY_API bool zmChangeScheduler(zmConn, int sId, zmSchedr newCng);

/// delete scheduler
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id
/// @return true - ok
ZMEY_API bool zmDelScheduler(zmConn, int sId);

/// start scheduler
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id
/// @return true - ok
ZMEY_API bool zmStartScheduler(zmConn, int sId);

/// pause scheduler
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id
/// @return true - ok
ZMEY_API bool zmPauseScheduler(zmConn, int sId);

/// ping scheduler
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id
/// @return true - ok
ZMEY_API bool zmPingScheduler(zmConn, int sId);

struct zmSchedulerState{
  zmStateType state;
  int activeTask;
  char startTime[32];
  char stopTime[32];
  char pingTime[32];
};

/// scheduler state
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id 
/// @param[out] outState - scheduler state
/// @return true - ok
ZMEY_API bool zmStateOfScheduler(zmConn, int sId, zmSchedulerState* outState);

/// get all schedulers
/// @param[in] zmConn - object connect
/// @param[in] state - choose with current state. If the state is 'UNDEFINED', select all
/// @param[out] outSchId - schedulers id. Pass NULL, no need to free memory
/// @return count of schedulers
ZMEY_API int zmGetAllSchedulers(zmConn, zmStateType state, int** outSchId);

///////////////////////////////////////////////////////////////////////////////
/// Worker

/// worker config
struct zmWorker{
  int sId;                    ///< scheduler id 
  int capacityTask = 10;      ///< permissible simultaneous number of tasks
  char connectPnt[256];       ///< remote connection point: IP or DNS:port
};
  
/// add new worker
/// @param[in] zmConn - object connect
/// @param[in] cng - worker config
/// @param[out] outWId - new worker id
/// @return true - ok
ZMEY_API bool zmAddWorker(zmConn, zmWorker cng, int* outWId);

/// worker cng
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @param[out] outCng - worker config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmGetWorker(zmConn, int wId, zmWorker* outCng);

/// change worker cng
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @param[in] newCng - worker config
/// @return true - ok
ZMEY_API bool zmChangeWorker(zmConn, int wId, zmWorker newCng);

/// delete worker
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @return true - ok
ZMEY_API bool zmDelWorker(zmConn, int wId);

/// start worker
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @return true - ok
ZMEY_API bool zmStartWorker(zmConn, int wId);

/// pause worker
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @return true - ok
ZMEY_API bool zmPauseWorker(zmConn, int wId);

/// ping worker
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id
/// @return true - ok
ZMEY_API bool zmPingWorker(zmConn, int wId);

struct zmWorkerState{
  zmStateType state;
  int activeTask;
  int load;
  char startTime[32];
  char stopTime[32];
  char pingTime[32];
};

/// worker state
/// @param[in] zmConn - object connect
/// @param[in] wId - worker id, order by id.
/// @param[in] wCnt - worker count
/// @param[out] outState - worker state. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmStateOfWorker(zmConn, int* wId, int wCnt, zmWorkerState* outState);

/// get all workers
/// @param[in] zmConn - object connect
/// @param[in] sId - scheduler id 
/// @param[in] state - choose with current state. If the state is 'UNDEFINED', select all
/// @param[out] outWId - worker id. Pass NULL, no need to free memory
/// @return count of schedulers
ZMEY_API int zmGetAllWorkers(zmConn, int sId, zmStateType state, int** outWId);

///////////////////////////////////////////////////////////////////////////////
/// Task object

/// task config
struct zmTask{
  int schedrPresetId;       ///< schedr preset id. Default 0
  int workerPresetId;       ///< worker preset id. Default 0
  int averDurationSec;      ///< estimated lead time, sec. Default 0
  int maxDurationSec;       ///< maximum lead time, sec. Default 0
  char* params;             ///< CLI params for script. May be NULL
  char* scriptPath;         ///< Script path. Necessarily
  char* resultPath;         ///< Result path. Necessarily
};

/// start task
/// @param[in] zmConn - object connect
/// @param[in] tcng - task config
/// @param[out] outTId - task object id
/// @return true - ok
ZMEY_API bool zmStartTask(zmConn, zmTask tcng, int* outTId);

/// stop task
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @return true - ok
ZMEY_API bool zmStopTask(zmConn, int tId);

/// cancel task (when not yet taken to work)
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @return true - ok
ZMEY_API bool zmCancelTask(zmConn, int tId);

/// pause task
/// @param[in] zmConn - object connect
/// @param[in] tId - tId - task object id
/// @return true - ok
ZMEY_API bool zmPauseTask(zmConn, int tId);

/// continue task
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @return true - ok
ZMEY_API bool zmContinueTask(zmConn, int tId);

/// task state
struct zmTaskState{
  int progress;      ///< [0..100]
  zmStateType state;
};

/// get task state
/// @param[in] zmConn - object connect
/// @param[in] tId - task id, order by tId
/// @param[in] tCnt - task id count
/// @param[out] outTState - task state. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool zmStateOfTask(zmConn, int* tId, int tCnt, zmTaskState* outTState);

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
ZMEY_API bool zmTimeOfTask(zmConn, int tId, zmTaskTime* outTTime);

/// task state callback
typedef void* zmUData;         
typedef void(*zmChangeTaskStateCBack)(int tId, int progress, zmStateType prevState, zmStateType newState, zmUData);

/// set change task state callback
/// @param[in] zmConn - object connect
/// @param[in] tId - task object id
/// @param[in] cback
/// @param[in] userData
/// @return true - ok
ZMEY_API bool zmSetChangeTaskStateCBack(zmConn, int tId, zmChangeTaskStateCBack cback, zmUData);

///////////////////////////////////////////////////////////////////////////////
/// Internal errors

/// error
struct zmInternError{
  int sId;        ///< scheduler id 
  int wId;        ///< worker id 
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
ZMEY_API int zmGetInternErrors(zmConn, int sId, int wId, int mCnt, zmInternError* outErrors);

///////////////////////////////////////////////////////////////////////////////
/// free resouces
/// @param[in] zmConn - object connect
ZMEY_API void zmFreeResources(zmConn);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* ZMEY_C_API_H_ */
