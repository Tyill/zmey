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
import ctypes
from typing import List
from enum import Enum


lib_ = None

def loadLib(path : str):
  global lib_
  lib_ = ctypes.CDLL(path)
  
#############################################################################
### Common

class StateType(Enum):
  """State type"""
  UNDEFINED      = 0
  READY          = 1
  START          = 2
  RUNNING        = 3
  PAUSE          = 4
  STOP           = 5    
  COMPLETED      = 6
  ERROR          = 7
  CANCEL         = 8
  NOT_RESPONDING = 9

def stateStr(state : int):
  return StateType(state).name 

class Scheduler: 
  """Scheduler config""" 
  def __init__(self,
               id : int = 0, 
               state : int = 0,
               connectPnt : str = "",
               capacityTask : int = 10000,
               activeTask : int = 0,
               startTime : str = "",
               stopTime : str = "",
               pingTime : str = ""):
    self.id = id
    self.state = state
    self.connectPnt = connectPnt     # remote connection point: IP or DNS:port
    self.capacityTask = capacityTask # permissible simultaneous number of tasks 
    self.activeTask  = activeTask
    self.startTime  = startTime
    self.stopTime  = stopTime
    self.pingTime  = pingTime
    def __repr__(self):
      return f"Scheduler: id {self.id} state {self.state} connectPnt {self.connectPnt} \
                       capacityTask {self.capacityTask} activeTask {self.activeTask} pingTime {self.pingTime} \
                       startTime {self.startTime} stopTime {self.stopTime}"
    def __str__(self):
      return self.__repr__()
class Worker: 
  """Worker config""" 
  def __init__(self,
               id : int = 0,
               sId : int = 0, 
               state : int = 0,
               connectPnt : str = "",
               capacityTask : int = 10,
               activeTask : int = 0,
               load : int = 0,
               startTime : str = "",
               stopTime : str = "",
               pingTime : str = ""):
    self.id = id
    self.sId = sId                   # Scheduler id
    self.state = state
    self.connectPnt = connectPnt     # remote connection point: IP or DNS:port
    self.capacityTask = capacityTask # permissible simultaneous number of tasks 
    self.activeTask  = activeTask
    self.load  = load
    self.startTime  = startTime
    self.stopTime  = stopTime
    self.pingTime  = pingTime
    def __repr__(self):
      return f"Worker: id {self.id} sId {self.sId} state {self.state} connectPnt {self.connectPnt} \
                       capacityTask {self.capacityTask} activeTask {self.activeTask} load {self.load} \
                       startTime {self.startTime} stopTime {self.stopTime}"
    def __str__(self):
      return self.__repr__()

class Task:
  """Task config""" 
  def __init__(self,
               id : int = 0,         
               state : int = 0,
               progress : int = 0,
               averDurationSec : int = 0,
               maxDurationSec : int = 0,
               scriptPath : str = "",
               resultPath : str = "",
               params : str = "",
               createTime : str = "",
               takeInWorkTime : str = "",
               startTime : str = "",
               stopTime : str = ""):
    self.id = id
    self.state = state
    self.progress = progress
    self.averDurationSec = averDurationSec
    self.maxDurationSec = maxDurationSec
    self.scriptPath = scriptPath
    self.resultPath = resultPath
    self.params = params               # CLI params for script
    self.createTime = createTime
    self.takeInWorkTime = takeInWorkTime
    self.startTime = startTime
    self.stopTime = stopTime
    def __repr__(self):
      return f"Task: id {self.id} ttlId {self.ttlId} state {self.state} stopTime {self.stopTime} startTime {self.startTime} takeInWorkTime {self.takeInWorkTime} createTime {self.createTime} params {self.params} progress {self.progress} result {self.result}"
    def __str__(self):
      return self.__repr__()
class InternError: 
  """Internal ERROR""" 
  def __init__(self,
               sId : int = 0,
               wId : int = 0,
               createTime : str = 0,
               message : str = ""):
    self.schedrId = sId                 # Scheduler id    
    self.workerId = wId                 # Worker id
    self.createTime = createTime   
    self.message = message          
  
class _ConnectCng_C(ctypes.Structure):
  _fields_ = [('connStr', ctypes.c_char_p)]
class _SchedrCng_C(ctypes.Structure):
  _fields_ = [('capacityTask', ctypes.c_int32),
              ('connectPnt', ctypes.c_char * 256)]
class _SchedrState_C(ctypes.Structure):
  _fields_ = [('state', ctypes.c_int32),
              ('activeTaskCount', ctypes.c_int32),
              ('startTime', ctypes.c_char * 32),
              ('stopTime', ctypes.c_char * 32),
              ('pingTime', ctypes.c_char * 32)]
class _WorkerCng_C(ctypes.Structure):
  _fields_ = [('schedrId', ctypes.c_int32),
              ('capacityTask', ctypes.c_int32),
              ('connectPnt', ctypes.c_char * 256)]
class _WorkerState_C(ctypes.Structure):
  _fields_ = [('state', ctypes.c_int32),
              ('activeTaskCount', ctypes.c_int32),
              ('load', ctypes.c_int32),
              ('startTime', ctypes.c_char * 32),
              ('stopTime', ctypes.c_char * 32),
              ('pingTime', ctypes.c_char * 32)]
class _TaskCng_C(ctypes.Structure):
  _fields_ = [('schedrPresetId', ctypes.c_int32),
              ('workerPresetId', ctypes.c_int32),
              ('averDurationSec', ctypes.c_int32),
              ('maxDurationSec', ctypes.c_int32),
              ('params', ctypes.c_char_p),
              ('scriptPath', ctypes.c_char_p),
              ('resultPath', ctypes.c_char_p)]
class _TaskState_C(ctypes.Structure):
  _fields_ = [('progress', ctypes.c_int32),
              ('state', ctypes.c_int32)]
class _TaskTime_C(ctypes.Structure):
  _fields_ = [('createTime', ctypes.c_char * 32),
              ('takeInWorkTime', ctypes.c_char * 32),
              ('startTime', ctypes.c_char * 32),
              ('stopTime', ctypes.c_char * 32)]
class _InternError_C(ctypes.Structure):
  _fields_ = [('schedrId', ctypes.c_int32),
              ('workerId', ctypes.c_int32),
              ('createTime', ctypes.c_char * 32),
              ('message', ctypes.c_char * 256)]

def version() -> str:
  """
  Version library
  :return: version
  """
  pfun = lib_.zmVersionLib
  pfun.restype = None
  pfun.argtypes = (ctypes.c_char_p,)

  ver = ctypes.create_string_buffer(32)
  pfun(ver)
  return ver.value.decode("utf-8")

class Connection:
  """Connection object"""
  
  zmConn_ = None
  userErrCBack_ : ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p) = None
  changeTaskStateCBack_ : ctypes.CFUNCTYPE(None, ctypes.c_int32, ctypes.c_int32, ctypes.c_int32, ctypes.c_int32, ctypes.c_void_p) = None
  cerr_ : str = ""
  
  def __init__(self, connStr : str):
    if not lib_:
      raise Exception('lib not load')
    
    cng = _ConnectCng_C()
    cng.connStr = connStr.encode("utf-8")
    
    pfun = lib_.zmCreateConnection
    pfun.argtypes = (_ConnectCng_C, ctypes.c_char_p)
    pfun.restype = ctypes.c_void_p
    err = ctypes.create_string_buffer(256)
    self.zmConn_ = pfun(cng, err)
    self.cerr_ = err.value.decode('utf-8')
  def __enter__(self):
    return self
  def __exit__(self, exc_type, exc_value, traceback):
    if (self.zmConn_):
      pfun = lib_.zmDisconnect
      pfun.restype = None
      pfun.argtypes = (ctypes.c_void_p,)
      pfun(self.zmConn_)

  def getLastError(self) -> str:
    """
    Last ERROR strind
    :return: errStr
    """
    if (self.zmConn_):
      pfun = lib_.zmGetLastError
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_char_p)
      err = ctypes.create_string_buffer(256)
      pfun(self.zmConn_, err)
      return err.value.decode("utf-8") 
    return self.cerr_
  def setErrorCBack(self, ucb):
    """
    Set ERROR callback
    :param ucb: def func(err : str)
    """
    if (self.zmConn_):      
      def c_ecb(err: ctypes.c_char_p, udata: ctypes.c_void_p):
        ucb(err.decode("utf-8"))
      
      errCBackType = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p)    
      self.userErrCBack_ = errCBackType(c_ecb)

      pfun = lib_.zmSetErrorCBack
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, errCBackType, ctypes.c_void_p)
      return pfun(self.zmConn_, self.userErrCBack_, 0)
    
  #############################################################################
  ### Scheduler
  
  def addScheduler(self, iosch : Scheduler) -> bool:
    """
    Add new scheduler
    :param iosch: new Scheduler config
    :return: True - ok
    """
    if (self.zmConn_):
      scng = _SchedrCng_C()
      scng.connectPnt = iosch.connectPnt.encode('utf-8')
      scng.capacityTask = iosch.capacityTask
      
      sid = ctypes.c_int32(0)
      
      pfun = lib_.zmAddScheduler
      pfun.argtypes = (ctypes.c_void_p, _SchedrCng_C, ctypes.POINTER(ctypes.c_int32))
      pfun.restype = ctypes.c_bool
      if (pfun(self.zmConn_, scng, ctypes.byref(sid))):
        iosch.id = sid.value
        return True
    return False
  def getScheduler(self, iosch : Scheduler) -> bool:
    """
    Get Scheduler config by ID
    :param iosch: Scheduler config
    :return: True - ok
    """
    if (self.zmConn_):
      scng = _SchedrCng_C()

      sid = ctypes.c_int32(iosch.id)
      
      pfun = lib_.zmGetScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(_SchedrCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self.zmConn_, sid, ctypes.byref(scng))):      
        iosch.connectPnt = scng.connectPnt.decode('utf-8')
        iosch.capacityTask = scng.capacityTask
        return True
    return False
  def changeScheduler(self, isch : Scheduler) -> bool:
    """
    Change Scheduler config
    :param isch: new Scheduler config
    :return: True - ok
    """
    if (self.zmConn_):
      sid = ctypes.c_int32(isch.id)
      scng = _SchedrCng_C()
      scng.connectPnt = isch.connectPnt.encode('utf-8')
      scng.capacityTask = isch.capacityTask
      
      pfun = lib_.zmChangeScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, _SchedrCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, sid, scng)
    return False
  def delScheduler(self, schId) -> bool:
    """
    Delete scheduler
    :param schId: Scheduler id
    :return: True - ok
    """
    if (self.zmConn_):
      sid = ctypes.c_int32(schId)
            
      pfun = lib_.zmDelScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, sid)
    return False
  def startScheduler(self, schId) -> bool:
    """
    Start scheduler
    :param schId: Scheduler id
    :return: True - ok
    """
    if (self.zmConn_):
      sid = ctypes.c_int32(schId)
            
      pfun = lib_.zmStartScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, sid)
    return False
  def pauseScheduler(self, schId) -> bool:
    """
    Pause scheduler
    :param schId: Scheduler id
    :return: True - ok
    """
    if (self.zmConn_):
      sid = ctypes.c_int32(schId)
            
      pfun = lib_.zmPauseScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, sid)
    return False
  def pingScheduler(self, schId) -> bool:
    """
    Ping scheduler
    :param schId: Scheduler id
    :return: True - ok
    """
    if (self.zmConn_):
      sid = ctypes.c_int32(schId)
            
      pfun = lib_.zmPingScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, sid)
    return False
  def schedulerState(self, iosch : Scheduler) -> bool:
    """
    Scheduler state
    :param iosch: Scheduler config
    :return: True - ok
    """
    if (self.zmConn_):
      sid = ctypes.c_int32(iosch.id)      
      stateBuffer = _SchedrState_C()     
     
      pfun = lib_.zmStateOfScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(_SchedrState_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self.zmConn_, sid, stateBuffer)):      
        iosch.state = stateBuffer.state 
        iosch.activeTask = stateBuffer.activeTask 
        iosch.startTime = stateBuffer.startTime.decode('utf-8')
        iosch.stopTime = stateBuffer.stopTime.decode('utf-8')
        iosch.pingTime = stateBuffer.pingTime.decode('utf-8')
        return True
    return False
  def getAllSchedulers(self, state : StateType=StateType.UNDEFINED) -> List[Scheduler]:
    """
    Get all schedrs
    :param state: choose with current state. If the state is 'UNDEFINED', select all
    :return: list of Scheduler
    """
    if (self.zmConn_):
      sstate = ctypes.c_int32(state.value)

      pfun = lib_.zmGetAllSchedulers
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_int32)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_int32)()
      osz = pfun(self.zmConn_, sstate, ctypes.byref(dbuffer))
      
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self.freeResources_()

        osch = []
        for i in range(osz):
          s = Scheduler(oid[i])
          self.getScheduler(s)
          osch.append(s)
        return osch
    return []

  #############################################################################
  ### Worker

  def addWorker(self, iowkr : Worker) -> bool:
    """
    Add new Worker
    :param iowkr: new Worker config
    :return: True - ok
    """
    if (self.zmConn_):
      wcng = _WorkerCng_C()
      wcng.schedrId = iowkr.sId
      wcng.connectPnt = iowkr.connectPnt.encode('utf-8')
      wcng.capacityTask = iowkr.capacityTask
      
      wid = ctypes.c_int32(0)
      
      pfun = lib_.zmAddWorker
      pfun.argtypes = (ctypes.c_void_p, _WorkerCng_C, ctypes.POINTER(ctypes.c_int32))
      pfun.restype = ctypes.c_bool
      if (pfun(self.zmConn_, wcng, ctypes.byref(wid))):
        iowkr.id = wid.value
        return True
    return False
  def getWorker(self, iowkr : Worker) -> bool:
    """
    Get Worker config by ID
    :param iowkr: Worker config
    :return: True - ok
    """
    if (self.zmConn_):
      wcng = _WorkerCng_C()

      wid = ctypes.c_int32(iowkr.id)
      
      pfun = lib_.zmGetWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(_WorkerCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self.zmConn_, wid, ctypes.byref(wcng))):
        iowkr.sId = wcng.schedrId
        iowkr.connectPnt = wcng.connectPnt.decode('utf-8')
        iowkr.capacityTask = wcng.capacityTask
        return True
    return False
  def changeWorker(self, iwkr : Worker) -> bool:
    """
    Change Worker config
    :param iwkr: new Worker config
    :return: True - ok
    """
    if (self.zmConn_):
      wid = ctypes.c_int32(iwkr.id)
      wcng = _WorkerCng_C()
      wcng.schedrId = iwkr.sId
      wcng.connectPnt = iwkr.connectPnt.encode('utf-8')
      wcng.capacityTask = iwkr.capacityTask
    
      pfun = lib_.zmChangeWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, _WorkerCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, wid, wcng)
    return False
  def delWorker(self, wkrId) -> bool:
    """
    Delete Worker
    :param wkrId: Worker id
    :return: True - ok
    """
    if (self.zmConn_):
      wid = ctypes.c_int32(wkrId)
            
      pfun = lib_.zmDelWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, wid)
    return False
  def startWorker(self, wkrId) -> bool:
    """
    Start Worker
    :param wkrId: Worker id
    :return: True - ok
    """
    if (self.zmConn_):
      wid = ctypes.c_int32(wkrId)
            
      pfun = lib_.zmStartWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, wid)
    return False
  def pauseWorker(self, wkrId) -> bool:
    """
    Pause Worker
    :param wkrId: Worker id
    :return: True - ok
    """
    if (self.zmConn_):
      wid = ctypes.c_int32(wkrId)
            
      pfun = lib_.zmPauseWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, wid)
    return False
  def pingWorker(self, wkrId) -> bool:
    """
    Ping Worker
    :param wkrId: Worker id
    :return: True - ok
    """
    if (self.zmConn_):
      wid = ctypes.c_int32(wkrId)
            
      pfun = lib_.zmPingWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, wid)
    return False
  def workerState(self, iowkrs : List[Worker]) -> bool:
    """
    Worker state
    :param iowkrs: workers config
    :return: True - ok
    """
    if (self.zmConn_):
      wsz = len(iowkrs)
      cwsz = ctypes.c_uint32(wsz)

      iowkrs.sort(key = lambda w: w.id)
          
      idBuffer = (ctypes.c_int32 * wsz)(*[iowkrs[i].id for i in range(wsz)])
      stateBuffer = (_WorkerState_C * wsz)()

      pfun = lib_.zmStateOfWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_int32), ctypes.c_uint32, ctypes.POINTER(_WorkerState_C))
      pfun.restype = ctypes.c_bool

      if (pfun(self.zmConn_, idBuffer, cwsz, stateBuffer)):      
        for i in range(wsz):
          iowkrs[i].state = stateBuffer[i].state
          iowkrs[i].activeTask = stateBuffer[i].activeTask
          iowkrs[i].load = stateBuffer[i].load
          iowkrs[i].startTime = stateBuffer[i].startTime.decode('utf-8')
          iowkrs[i].stopTime = stateBuffer[i].stopTime.decode('utf-8')
          iowkrs[i].pingTime = stateBuffer[i].pingTime.decode('utf-8')
        return True
    return False
  def getAllWorkers(self, sId : int, state : StateType=StateType.UNDEFINED) -> List[Worker]:
    """
    Get all workers
    :param sId: Scheduler id
    :param state: choose with current state. If the state is 'UNDEFINED', select all
    :return: list of Worker
    """
    if (self.zmConn_):
      schId = ctypes.c_int32(sId)
      sstate = ctypes.c_int32(state.value)

      pfun = lib_.zmGetAllWorkers
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_int32)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_int32)()
      osz = pfun(self.zmConn_, schId, sstate, ctypes.byref(dbuffer))
      
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self.freeResources_()

        owkr = []
        for i in range(osz):
          w = Worker(oid[i])
          self.getWorker(w)
          owkr.append(w)
        return owkr
    return []
    
  #############################################################################
  ### Task object

  def startTask(self, iot : Task) -> bool:
    """
    Start Task
    :param iot: task config
    :return: True - ok
    """
    if (self.zmConn_):
      tid = ctypes.c_int32(0)
            
      tcng = _TaskCng_C()
      tcng.schedrPresetId = 0       # not used yet
      tcng.workerPresetId = 0
      tcng.averDurationSec = iot.averDurationSec
      tcng.maxDurationSec = iot.maxDurationSec   
      tcng.params = iot.params.encode('utf-8')
      tcng.scriptPath = iot.scriptPath.encode('utf-8')
      tcng.resultPath = iot.resultPath.encode('utf-8')

      pfun = lib_.zmStartTask
      pfun.argtypes = (ctypes.c_void_p, _TaskCng_C, ctypes.POINTER(ctypes.c_int32))
      pfun.restype = ctypes.c_bool
      if (pfun(self.zmConn_, tcng, ctypes.byref(tid))):
        iot.id = tid.value
        return True
    return False
  def stopTask(self, tId) -> bool:
    """
    Stop Task
    :param tId: Task id
    :return: True - ok
    """
    if (self.zmConn_):
      tid = ctypes.c_int32(tId)
            
      pfun = lib_.zmStopTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, tid)
    return False
  def pauseTask(self, tId) -> bool:
    """
    Pause Task
    :param tId: Task id
    :return: True - ok
    """
    if (self.zmConn_):
      tid = ctypes.c_int32(tId)
            
      pfun = lib_.zmPauseTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, tid)
    return False
  def cancelTask(self, tId) -> bool:
    """
    Cancel Task
    :param tId: Task id
    :return: True - ok
    """
    if (self.zmConn_):
      tid = ctypes.c_int32(tId)
            
      pfun = lib_.zmCancelTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, tid)
    return False
  def continueTask(self, tId) -> bool:
    """
    Continue Task
    :param tId: Task id
    :return: True - ok
    """
    if (self.zmConn_):
      tid = ctypes.c_int32(tId)
            
      pfun = lib_.zmContinueTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32)
      pfun.restype = ctypes.c_bool
      return pfun(self.zmConn_, tid)
    return False
  def taskState(self, iot : List[Task]) -> bool:
    """
    Task state
    :param iot: tasks config
    :return: True - ok
    """
    if (self.zmConn_):
      tsz = len(iot)
      ctsz = ctypes.c_uint32(tsz)
          
      iot.sort(key = lambda t: t.id)
        
      idBuffer = (ctypes.c_int32 * tsz)(*[iot[i].id for i in range(tsz)])
      stateBuffer = (_TaskState_C * tsz)()
      
      pfun = lib_.zmStateOfTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_int32), ctypes.c_uint32, ctypes.POINTER(_TaskState_C))
      pfun.restype = ctypes.c_bool

      if (pfun(self.zmConn_, idBuffer, ctsz, stateBuffer)):      
        for i in range(tsz):
          iot[i].state = stateBuffer[i].state
          iot[i].progress = stateBuffer[i].progress          
        return True
    return False
  def taskTime(self, iot : Task) -> bool:
    """
    Task time
    :param iot: Task config
    :return: True - ok
    """
    if (self.zmConn_):
      tid = ctypes.c_int32(iot.id)
      ttime = _TaskTime_C()
      
      pfun = lib_.zmTimeOfTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(_TaskTime_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self.zmConn_, tid, ctypes.byref(ttime))):      
        iot.createTime = ttime.createTime.decode('utf-8')
        iot.takeInWorkTime = ttime.takeInWorkTime.decode('utf-8')
        iot.startTime = ttime.startTime.decode('utf-8')
        iot.stopTime = ttime.stopTime.decode('utf-8')
        return True
    return False

  def addTaskForTracking(self, tId : int):
    """
    Set change Task state callback
    """
    if (self.zmConn_ and self.changeTaskStateCBack_):   
      c_tid = ctypes.c_int32(tId)
    
      taskStateCBackType = ctypes.CFUNCTYPE(None, ctypes.c_int32, ctypes.c_int32, ctypes.c_int32, ctypes.c_int32, ctypes.c_void_p)
      
      pfun = lib_.zmSetChangeTaskStateCBack
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, taskStateCBackType, ctypes.c_void_p)
      return pfun(self.zmConn_, c_tid, self.changeTaskStateCBack_, 0)
    return False

  def setChangeTaskStateCBack(self, ucb):
    """
    Set change Task state callback
    :param ucb: def func(tId : uint64, progress : int, prevState : StateType, newState : StateType)
    """
    if (self.zmConn_):   
      def c_ucb(tId: ctypes.c_int32, progress: ctypes.c_int32, prevState: ctypes.c_int32, newState: ctypes.c_int32, udata: ctypes.c_void_p):
        ucb(tId, progress, prevState, newState)
      
      taskStateCBackType = ctypes.CFUNCTYPE(None, ctypes.c_int32, ctypes.c_int32, ctypes.c_int32, ctypes.c_int32, ctypes.c_void_p)
      self.changeTaskStateCBack_ = taskStateCBackType(c_ucb)
      return True
    return False

  #############################################################################
  ### Internal errors
 
  def getInternErrors(self, sId : int, wId : int, mCnt : int) -> List[InternError]:
    """
    Get internal errors
    :param sId: Scheduler id
    :param wId: Worker id
    :param mCnt: max mess count
    :return: list of errors
    """
    if (self.zmConn_):
      csId = ctypes.c_int32(sId)
      cwId = ctypes.c_int32(wId)
      cmCnt = ctypes.c_uint32(mCnt)

      pfun = lib_.zmGetInternErrors
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.c_int32, ctypes.c_uint32, ctypes.POINTER(_InternError_C))
      pfun.restype = ctypes.c_uint32
      dbuffer = (_InternError_C * mCnt)()
      osz = pfun(self.zmConn_, csId, cwId, cmCnt, dbuffer)
      
      oerr = []
      for i in range(osz):
        oerr[i].schedrId = dbuffer[i].schedrId
        oerr[i].workerId = dbuffer[i].workerId
        oerr[i].createTime = dbuffer[i].createTime
        oerr[i].message = dbuffer[i].message
      return oerr
    return []

  def freeResources_(self):
    if (self.zmConn_):
      pfun = lib_.zmFreeResources
      pfun.restype = None
      pfun.argtypes = (ctypes.c_void_p,)
      pfun(self.zmConn_)