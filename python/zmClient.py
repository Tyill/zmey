#
# zmey Project
# Copyright (C) 2018 by Contributors <https://github.com/Tyill/zmey>
#
# This code is licensed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# coding: utf-8

from __future__ import absolute_import
import os
import sys
import ctypes
from enum import Enum

_LIB = ctypes.CDLL(os.path.expanduser("~") + '/cpp/zmey/build/Release/libzmClient.so')

#####################################################################
### Common

class stateType(Enum):
  """State type"""
  zmUndefined     = -1
  zmReady         = 0
  zmStart         = 1
  zmRunning       = 2
  zmPause         = 3
  zmStop          = 4    
  zmCompleted     = 5
  zmError         = 6
  zmNotResponding = 7

class dbType(Enum):
  """Database type"""
  PostgreSQL = 0

class user: 
  """User config""" 
  id : int
  name : str
  passw: str
  description : str 

def _c_str(string : str) -> ctypes.c_char_p:
    """Create ctypes char * from a Python string."""
    if sys.version_info[0] > 2:
      py_str = lambda x: x.encode('utf-8')
    else:
      py_str = lambda x: x
    return ctypes.c_char_p(py_str(string)) 


_uint64_p = lambda x : ctypes.cast(x, ctypes.POINTER(ctypes.c_uint64))

class _connectCng(ctypes.Structure):
  _fields_ = [('dbType', ctypes.c_int32),
              ('connStr', ctypes.c_char_p)]

class _userCng(ctypes.Structure):
  _fields_ = [('name', ctypes.c_char * 256),
              ('passw', ctypes.c_char * 256),
              ('description', ctypes.c_char_p)]

_errCBackType = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p)      

def version() -> str:
  """
  Version library
  :return: version
  """
  pfun = _LIB.zmVersionLib
  pfun.restype = None
  pfun.argtypes = (ctypes.c_char_p,)

  ver = ctypes.create_string_buffer(32)
  pfun(ver)
  return ver.value.decode("utf-8")

class ZMObj:
  """Connection object"""
  
  _zmConn = 0
  _userErrCBack : _errCBackType = 0
  _cerr : str = ""
     
  #####################################################################
  ### Connection with DB

  def __init__(self, db : dbType, connStr : str):
    cng = _connectCng()
    cng.dbType = db.value
    cng.connStr = _c_str(connStr)

    pfun = _LIB.zmCreateConnection
    pfun.argtypes = (_connectCng, ctypes.c_char_p)
    pfun.restype = ctypes.c_void_p
    err = ctypes.create_string_buffer(256)
    self._zmConn = pfun(cng, err)
  
  def __enter__(self):
    return self

  def __exit__(self, exc_type, exc_value, traceback):
    if (self._zmConn):
      pfun = _LIB.zmDisconnect
      pfun.restype = None
      pfun.argtypes = (ctypes.c_void_p,)
      pfun(self._zmConn)
 
  def setErrorCBack(self, ucb):
    """
    Set error callback
    :param ucb: def func(err : str)
    """
    if (self._zmConn):      
      def c_ecb(err: ctypes.c_char_p, udata: ctypes.c_void_p):
        ucb(err.decode("utf-8"))
      
      self._userErrCBack = _errCBackType(c_ecb)

      pfun = _LIB.zmSetErrorCBack
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, _errCBackType, ctypes.c_void_p)
      return pfun(self._zmConn, self._userErrCBack, 0)
 
  def getLastError(self) ->str:
    """
    Last error strind
    :return: errStr
    """
    if (self._zmConn):
      pfun = _LIB.zmGetLastError
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_char_p)
      err = ctypes.create_string_buffer(256)
      pfun(self._zmConn, err)
      return err.value
    return "no connection with DB" 

  def createTables(self) -> bool:
    """
    Create tables, will be created if not exist
    :return: True ok
    """
    if (self._zmConn):
      pfun = _LIB.zmCreateTables
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p,)
      return pfun(self._zmConn)
    return False

  # ///////////////////////////////////////////////////////////////////////////////
  # /// User
  
  def addUser(self, iou : user) ->bool:
    """
    Add new user
    :param user: new user config
    :return: True - ok
    """
    if (self._zmConn):
      ucng = _userCng()
      ucng.name = iou.name.encode('utf-8')
      ucng.passw = iou.passw.encode('utf-8')
      ucng.description = iou.description.encode('utf-8')

      uid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddUser
      pfun.argtypes = (ctypes.c_void_p, _userCng, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      ok = pfun(self._zmConn, ucng, _uint64_p(ctypes.addressof(uid)))
      iou.id = uid.value
      return ok
    return False

obj = ZMObj(dbType.PostgreSQL, "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10")

usr = user
usr.id = 0
usr.name = "alm"
usr.passw = "123"
usr.description = "" 

ok = obj.addUser(usr)   

err = obj.getLastError();

ok = False

  # /// get exist user id
  # /// @param[in] zmConn - object connect
  # /// @param[in] cng - user config
  # /// @param[out] outUserId - user id
  # /// @return true - ok
  # ZMEY_API bool zmGetUserId(zmConn, zmUser cng, uint64_t* outUserId);

  # /// get user config
  # /// @param[in] zmConn - object connect
  # /// @param[in] userId - user id
  # /// @param[out] outCng - user config
  # /// @return true - ok
  # ZMEY_API bool zmGetUserCng(zmConn, uint64_t userId, zmUser* outCng);

  # /// change user config
  # /// @param[in] zmConn - object connect
  # /// @param[in] userId - user id
  # /// @param[in] newCng - new user cng
  # /// @return true - ok
  # ZMEY_API bool zmChangeUser(zmConn, uint64_t userId, zmUser newCng);

  # /// delete user
  # /// @param[in] zmConn - object connect
  # /// @param[in] userId - user id
  # /// @return true - ok
  # ZMEY_API bool zmDelUser(zmConn, uint64_t userId);

  # /// get all users
  # /// @param[in] zmConn - object connect
  # /// @param[out] outUserId - users id
  # /// @return count of users
  # ZMEY_API uint32_t zmGetAllUsers(zmConn, uint64_t** outUserId);

  # ///////////////////////////////////////////////////////////////////////////////
  # /// Scheduler

  # /// scheduler config
  # struct zmSchedr{
  #   char connectPnt[255];          ///< remote connection point: IP or DNS:port
  #   uint32_t capacityTask = 10000; ///< permissible simultaneous number of tasks 
  # };
  # /// add new scheduler
  # /// @param[in] zmConn - object connect
  # /// @param[in] cng - scheduler config
  # /// @param[out] outSchId - new scheduler id
  # /// @return true - ok
  # ZMEY_API bool zmAddScheduler(zmConn, zmSchedr cng, uint64_t* outSchId);

  # /// scheduler cng
  # /// @param[in] zmConn - object connect
  # /// @param[in] sId - scheduler id 
  # /// @param[out] outCng - scheduler config
  # /// @return true - ok
  # ZMEY_API bool zmGetScheduler(zmConn, uint64_t sId, zmSchedr* outCng);

  # /// change scheduler cng
  # /// @param[in] zmConn - object connect
  # /// @param[in] sId - scheduler id 
  # /// @param[in] newCng - scheduler config
  # /// @return true - ok
  # ZMEY_API bool zmChangeScheduler(zmConn, uint64_t sId, zmSchedr newCng);

  # /// delete scheduler
  # /// @param[in] zmConn - object connect
  # /// @param[in] sId - scheduler id
  # /// @return true - ok
  # ZMEY_API bool zmDelScheduler(zmConn, uint64_t sId);

  # /// start scheduler
  # /// @param[in] zmConn - object connect
  # /// @param[in] sId - scheduler id
  # /// @return true - ok
  # ZMEY_API bool zmStartScheduler(zmConn, uint64_t sId);

  # /// pause scheduler
  # /// @param[in] zmConn - object connect
  # /// @param[in] sId - scheduler id
  # /// @return true - ok
  # ZMEY_API bool zmPauseScheduler(zmConn, uint64_t sId);

  # /// ping scheduler
  # /// @param[in] zmConn - object connect
  # /// @param[in] sId - scheduler id
  # /// @return true - ok
  # ZMEY_API bool zmPingScheduler(zmConn, uint64_t sId);

  # /// scheduler state
  # /// @param[in] zmConn - object connect
  # /// @param[in] sId - scheduler id 
  # /// @param[out] outState - scheduler state
  # /// @return true - ok
  # ZMEY_API bool zmSchedulerState(zmConn, uint64_t sId, zmStateType* outState);

  # /// get all schedulers
  # /// @param[in] zmConn - object connect
  # /// @param[in] state - choose with current state. If the state is 'undefined', select all
  # /// @param[out] outSchId - schedulers id
  # /// @return count of schedulers
  # ZMEY_API uint32_t zmGetAllSchedulers(zmConn, zmStateType state, uint64_t** outSchId);

  # ///////////////////////////////////////////////////////////////////////////////
  # /// Worker

  # /// worker config
  # struct zmWorker{
  #   uint64_t sId;               ///< scheduler id 
  #   uint32_t capacityTask = 10; ///< permissible simultaneous number of tasks
  #   char connectPnt[255];       ///< remote connection point: IP or DNS:port   
  # };
    
  # /// add new worker
  # /// @param[in] zmConn - object connect
  # /// @param[in] cng - worker config
  # /// @param[out] outWId - new worker id
  # /// @return true - ok
  # ZMEY_API bool zmAddWorker(zmConn, zmWorker cng, uint64_t* outWId);

  # /// worker cng
  # /// @param[in] zmConn - object connect
  # /// @param[in] wId - worker id
  # /// @param[out] outCng - worker config
  # /// @return true - ok
  # ZMEY_API bool zmGetWorker(zmConn, uint64_t wId, zmWorker* outCng);

  # /// change worker cng
  # /// @param[in] zmConn - object connect
  # /// @param[in] wId - worker id
  # /// @param[in] newCng - worker config
  # /// @return true - ok
  # ZMEY_API bool zmChangeWorker(zmConn, uint64_t wId, zmWorker newCng);

  # /// delete worker
  # /// @param[in] zmConn - object connect
  # /// @param[in] wId - worker id
  # /// @return true - ok
  # ZMEY_API bool zmDelWorker(zmConn, uint64_t wId);

  # /// start worker
  # /// @param[in] zmConn - object connect
  # /// @param[in] wId - worker id
  # /// @return true - ok
  # ZMEY_API bool zmStartWorker(zmConn, uint64_t wId);

  # /// pause worker
  # /// @param[in] zmConn - object connect
  # /// @param[in] wId - worker id
  # /// @return true - ok
  # ZMEY_API bool zmPauseWorker(zmConn, uint64_t wId);

  # /// ping worker
  # /// @param[in] zmConn - object connect
  # /// @param[in] wId - worker id
  # /// @return true - ok
  # ZMEY_API bool zmPingWorker(zmConn, uint64_t wId);

  # /// worker state
  # /// @param[in] zmConn - object connect
  # /// @param[in] wId - worker id, order by id.
  # /// @param[in] wCnt - worker count
  # /// @param[out] outState - worker state. The memory is allocated by the user
  # /// @return true - ok
  # ZMEY_API bool zmWorkerState(zmConn, uint64_t* wId, uint32_t wCnt, zmStateType* outState);

  # /// get all workers
  # /// @param[in] zmConn - object connect
  # /// @param[in] sId - scheduler id 
  # /// @param[in] state - choose with current state. If the state is 'undefined', select all
  # /// @param[out] outWId - worker id 
  # /// @return count of schedulers
  # ZMEY_API uint32_t zmGetAllWorkers(zmConn, uint64_t sId, zmStateType state, uint64_t** outWId);

  # ///////////////////////////////////////////////////////////////////////////////
  # /// Pipeline of tasks

  # /// pipeline config
  # struct zmPipeline{
  #   uint64_t userId;         ///< user id
  #   uint32_t isShared;       ///< may be shared [0..1]   
  #   char name[255];          ///< pipeline name
  #   char* description;       ///< description of pipeline. The memory is allocated by the user. May be NULL
  # };

  # /// add pipeline
  # /// @param[in] zmConn - object connect
  # /// @param[in] cng - pipeline config
  # /// @param[out] outPPLId - pipeline id
  # /// @return true - ok
  # ZMEY_API bool zmAddPipeline(zmConn, zmPipeline cng, uint64_t* outPPLId);

  # /// get pipeline config
  # /// @param[in] zmConn - object connect
  # /// @param[in] pplId - pipeline id
  # /// @param[out] outCng - pipeline config
  # /// @return true - ok
  # ZMEY_API bool zmGetPipeline(zmConn, uint64_t pplId, zmPipeline* outCng);

  # /// change pipeline config
  # /// @param[in] zmConn - object connect
  # /// @param[in] pplId - pipeline id
  # /// @param[in] newCng - pipeline config
  # /// @return true - ok
  # ZMEY_API bool zmChangePipeline(zmConn, uint64_t pplId, zmPipeline newCng);

  # /// delete pipeline
  # /// @param[in] zmConn - object connect
  # /// @param[in] pplId - pipeline
  # /// @return true - ok
  # ZMEY_API bool zmDelPipeline(zmConn, uint64_t pplId);

  # /// get all pipelines
  # /// @param[in] zmConn - object connect
  # /// @param[in] userId - user id
  # /// @param[out] outPPLId - pipeline id 
  # /// @return count of pipelines
  # ZMEY_API uint32_t zmGetAllPipelines(zmConn, uint64_t userId, uint64_t** outPPLId);

  # ///////////////////////////////////////////////////////////////////////////////
  # /// Task template 

  # /// task template config
  # struct zmTaskTemplate{
  #   uint64_t userId;          ///< user id
  #   uint32_t averDurationSec; ///< estimated lead time 
  #   uint32_t maxDurationSec;  ///< maximum lead time
  #   uint32_t isShared;        ///< may be shared [0..1]   
  #   char name[255];           ///< task template name
  #   char* description;        ///< description of task. The memory is allocated by the user. May be NULL
  #   char* script;             ///< script on bash, python or cmd. The memory is allocated by the user
  # };

  # /// add new task template
  # /// @param[in] zmConn - object connect
  # /// @param[in] cng - task template config
  # /// @param[out] outTId - new task id
  # /// @return true - ok
  # ZMEY_API bool zmAddTaskTemplate(zmConn, zmTaskTemplate cng, uint64_t* outTId);

  # /// get task template cng
  # /// @param[in] zmConn - object connect
  # /// @param[in] tId - task id
  # /// @param[out] outTCng - task config. The memory is allocated by the user
  # /// @return true - ok
  # ZMEY_API bool zmGetTaskTemplate(zmConn, uint64_t tId, zmTaskTemplate* outTCng);

  # /// change task template cng
  # /// A new record is created for each change, the old one is not deleted.
  # /// @param[in] zmConn - object connect
  # /// @param[in] tId - task id
  # /// @param[in] newTCng - new task config
  # /// @param[out] outTId - new task id
  # /// @return true - ok
  # ZMEY_API bool zmChangeTaskTemplate(zmConn, uint64_t tId, zmTaskTemplate newTCng, uint64_t* outTId);

  # /// delete task template
  # /// The record is marked, but not deleted.
  # /// @param[in] zmConn - object connect
  # /// @param[in] tId - task id
  # /// @return true - ok
  # ZMEY_API bool zmDelTaskTemplate(zmConn, uint64_t tId);

  # /// get all tasks templates
  # /// @param[in] zmConn - object connect
  # /// @param[in] parent - user id
  # /// @param[out] outTId - task id
  # /// @return count of tasks
  # ZMEY_API uint32_t zmGetAllTaskTemplates(zmConn, uint64_t parent, uint64_t** outTId);

  # ///////////////////////////////////////////////////////////////////////////////
  # /// Task of pipeline

  # /// pipeline task config
  # struct zmTask{
  #   uint64_t pplId;          ///< pipeline id
  #   uint64_t ttId;           ///< task template id
  #   uint32_t priority;       ///< [1..3]
  #   char* prevTasksId;       ///< pipeline task id of previous tasks to be completed: [qtId,..]. May be NULL 
  #   char* nextTasksId;       ///< pipeline task id of next tasks: : [qtId,..]. May be NULL
  #   char* params;            ///< CLI params for script: ['param1','param2'..]. May be NULL
  #   char* screenRect;        ///< screenRect on UI: x y w h. May be NULL
  # };

  # /// add pipeline task
  # /// @param[in] zmConn - object connect
  # /// @param[in] cng - pipeline task config
  # /// @param[out] outQTaskId - pipeline task id
  # /// @return true - ok
  # ZMEY_API bool zmAddTask(zmConn, zmTask cng, uint64_t* outQTaskId);

  # /// get pipeline task config
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id
  # /// @param[out] outTCng - pipeline task config
  # /// @return true - ok
  # ZMEY_API bool zmGetTask(zmConn, uint64_t qtId, zmTask* outTCng);

  # /// change pipeline task config
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id
  # /// @param[in] newCng - pipeline task config
  # /// @return true - ok
  # ZMEY_API bool zmChangeTask(zmConn, uint64_t qtId, zmTask newCng);

  # /// delete pipeline task
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id
  # /// @return true - ok
  # ZMEY_API bool zmDelTask(zmConn, uint64_t qtId);

  # /// start pipeline task
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id
  # /// @return true - ok
  # ZMEY_API bool zmStartTask(zmConn, uint64_t qtId);

  # /// stop pipeline task
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id
  # /// @return true - ok
  # ZMEY_API bool zmStopTask(zmConn, uint64_t qtId);

  # /// pause pipeline task
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id
  # /// @return true - ok
  # ZMEY_API bool zmPauseTask(zmConn, uint64_t qtId);

  # /// continue pipeline task
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id
  # /// @return true - ok
  # ZMEY_API bool zmContinueTask(zmConn, uint64_t qtId);

  # /// pipeline task state
  # struct zmTskState{
  #   uint32_t progress;      ///< [0..100]
  #   zmStateType state;
  # };
  # /// get pipeline task state
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id, order by tId
  # /// @param[in] tCnt - pipeline task id count
  # /// @param[out] outTState - pipeline task state. The memory is allocated by the user
  # /// @return true - ok
  # ZMEY_API bool zmTaskState(zmConn, uint64_t* qtId, uint32_t tCnt, zmTskState* outTState);

  # /// get pipeline task result
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id
  # /// @param[out] outTResult - pipeline task result
  # /// @return true - ok
  # ZMEY_API bool zmTaskResult(zmConn, uint64_t qtId, char** outTResult);

  # /// pipeline task time
  # struct zmTskTime{
  #   char createTime[32];
  #   char takeInWorkTime[32];        
  #   char startTime[32]; 
  #   char stopTime[32]; 
  # };
  # /// get pipeline task time
  # /// @param[in] zmConn - object connect
  # /// @param[in] qtId - pipeline task id
  # /// @param[out] outTTime - pipeline task time
  # /// @return true - ok
  # ZMEY_API bool zmTaskTime(zmConn, uint64_t qtId, zmTskTime* outTTime);

  # /// get all pipeline tasks
  # /// @param[in] zmConn - object connect
  # /// @param[in] pplId - pipeline id
  # /// @param[in] state - choose with current state. If the state is 'undefined', select all
  # /// @param[out] outQTId - pipeline task id 
  # /// @return count of pipeline tasks
  # ZMEY_API uint32_t zmGetAllTasks(zmConn, uint64_t pplId, zmStateType state, uint64_t** outQTId);

  # ///////////////////////////////////////////////////////////////////////////////
  # /// Internal errors

  # /// error
  # struct zmInternError{
  #   uint64_t sId;        ///< scheduler id 
  #   uint64_t wId;        ///< worker id 
  #   char createTime[32];
  #   char message[256];
  # };

  # /// get errors
  # /// @param[in] zmConn - object connect
  # /// @param[in] sId - scheduler id. If '0' then all
  # /// @param[in] wId - worker id. If '0' then all
  # /// @param[in] mCnt - last mess max count. If '0' then all 
  # /// @param[out] outErrors
  # /// @return count of errors
  # ZMEY_API uint32_t zmGetInternErrors(zmConn, uint64_t sId, uint64_t wId, uint32_t mCnt, zmInternError** outErrors);

  # ///////////////////////////////////////////////////////////////////////////////
  # /// free resouces
  # ZMEY_API void zmFreeResouces(uint64_t*, char*);
