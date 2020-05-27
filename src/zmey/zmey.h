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
typedef void* zmConnect;

/// version lib
/// @param[out] outVersion. The memory is allocated by the user
ZMEY_API void zmVersionLib(char* outVersion /*sz 32*/);

/// create connection
/// @param[in] localPnt - IP or DNS:port
/// @param[in] dbServer - database server IP or DNS (for sqlite - path to file db, for db on files - path to dir)
/// @param[in] dbName - database name (for sqlite or files - empty) 
/// @return object connect
ZMEY_API zmConnect createConnection(const char* localPnt, const char* dbServer, const char* dbName);

/// disconnect
/// @param[in] zmConnect - object connect
ZMEY_API void disconnect(zmConnect);

/// create a database if it does not exist
/// @param[in] zmConnect - object connect
/// @param[in] dbName - database name (for sqlite - path to file db, for db on files - path to dir)
/// @return true - ok
ZMEY_API bool createDB(zmConnect, const char* dbName);

/// state
enum stateType{
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
/// @param[in] zmConnect - object connect
/// @param[in] zmSchedr - scheduler config
/// @param[out] outSchId - new scheduler id
/// @return true - ok
ZMEY_API bool addScheduler(zmConnect, zmSchedrCng, uint64_t* outSchId);

/// get scheduler state and cng
/// @param[in] zmConnect - object connect
/// @param[in] schId - scheduler id 
/// @param[out] outState - scheduler state
/// @param[out] outSchCng - scheduler config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool getScheduler(zmConnect, uint64_t schId, stateType* outState, zmSchedrCng* outSchCng);

/// get all schedulers
/// @param[in] zmConnect - object connect
/// @param[out] outSchId - schedulers id. First pass NULL, then pass it to the same 
/// @return count of schedulers
ZMEY_API uint32_t getAllSchedulers(zmConnect, uint64_t** outSchId);

/// executor type
enum executorType{
  bash = 0,
  cmd = 1,
  python = 2,
};
/// worker config
struct zmWorkerCng{
  executorType exr;           ///< executor type
  char connectPnt[255];       ///< IP or DNS:port
  uint32_t capasityTask = 10; ///< permissible simultaneous number of tasks 
};
/// add new worker
/// @param[in] zmConnect - object connect
/// @param[in] schId - scheduler id
/// @param[out] outWId - new worker id
/// @return true - ok
ZMEY_API bool addWorker(zmConnect, uint64_t schId, zmWorkerCng, uint64_t* outWId);

/// get worker state and cng
/// @param[in] zmConnect - object connect
/// @param[in] wId - worker id
/// @param[out] outState - worker state
/// @param[out] outWCng - worker config. The memory is allocated by the user
/// @return true - ok
ZMEY_API bool getWorker(zmConnect, uint64_t wId, stateType* outState, zmWorkerCng* outWCng);

/// get all workers
/// @param[in] zmConnect - object connect
/// @param[in] schId - scheduler id
/// @param[out] outWId - new worker id. First pass NULL, then pass it to the same 
/// @return true - ok
ZMEY_API bool getAllWorkers(zmConnect, uint64_t schId, uint64_t** outWId);

/// task config
struct zmTaskCng{
  executorType exr;         ///< executor type
  uint32_t averDurationSec; ///< estimated lead time 
  uint32_t maxDurationSec;  ///< maximum lead time
  char* script;             ///< script on bash, python or cmd. The memory is allocated by the user
};
/// add new task
/// @param[in] zmConnect - object connect
/// @param[in] zmTaskCng - task config
/// @param[out] outTId - new task id
/// @return true - ok
ZMEY_API bool addTask(zmConnect, zmTaskCng, uint64_t* outTId);

// /// get task id and cng
// /// @param[in] zmConnect - object connect
// /// @param[in] connectPnt - IP or DNS:port
// /// @param[out] outWId - worker id 
// /// @param[out] outWCng - worker config. The memory is allocated by the user
// /// @return true - ok
// ZMEY_API bool getAllTask(zmConnect, uint64_t taskId, zmTask* out);

ZMEY_API bool pushTaskToQueue(zmConnect, uint64_t taskId, uint32_t priority, const char* params);

#if defined(__cplusplus)
}}
#endif /* __cplusplus */

#endif /* ZMEY_C_API_H_ */
