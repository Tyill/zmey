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


_lib = None

def loadLib(path : str):
  global _lib
  _lib = ctypes.CDLL(path)
  
#############################################################################
### Common

class StateType(Enum):
  """State type"""
  UNDEFINED      = -1
  READY          = 0
  START          = 1
  RUNNING        = 2
  PAUSE          = 3
  STOP           = 4    
  COMPLETED      = 5
  ERROR          = 6
  CANCEL         = 7
  NOT_RESPONDING = 8

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
               pingTime : str = "",
               name = "",
               description = ""):
    self.id = id
    self.state = state
    self.connectPnt = connectPnt     # remote connection point: IP or DNS:port
    self.capacityTask = capacityTask # permissible simultaneous number of tasks 
    self.activeTask  = activeTask
    self.startTime  = startTime
    self.stopTime  = stopTime
    self.pingTime  = pingTime
    self.name = name
    self.description = description
    def __repr__(self):
      return f"Scheduler: id {self.id} state {self.state} connectPnt {self.connectPnt} \
                       capacityTask {self.capacityTask} activeTask {self.activeTask} pingTime {self.pingTime} \
                       startTime {self.startTime} stopTime {self.stopTime} name {self.name} description {self.description}"
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
               name = "",
               description = ""):
    self.id = id
    self.sId = sId                   # Scheduler id
    self.state = state
    self.connectPnt = connectPnt     # remote connection point: IP or DNS:port
    self.capacityTask = capacityTask # permissible simultaneous number of tasks 
    self.activeTask  = activeTask
    self.load  = load
    self.startTime  = startTime
    self.stopTime  = stopTime
    self.name = name
    self.description = description
    def __repr__(self):
      return f"Worker: id {self.id} sId {self.sId} state {self.state} connectPnt {self.connectPnt} \
                       capacityTask {self.capacityTask} activeTask {self.activeTask} load {self.load} \
                       startTime {self.startTime} stopTime {self.stopTime} name {self.name} description {self.description}"
    def __str__(self):
      return self.__repr__()
class TaskTemplate: 
  """TaskTemplate config""" 
  def __init__(self,
               id : int = 0,
               userId : int = 0,
               averDurationSec : int = 1,
               maxDurationSec : int = 1,
               name : str = "",
               description : str = "",
               script: str = ""):
    self.id = id
    self.userId = userId               
    self.averDurationSec = averDurationSec
    self.maxDurationSec = maxDurationSec
    self.name = name    
    self.description = description
    self.script = script   
  def __repr__(self):
      return f"TaskTemplate: name {self.name} description {self.description} script {self.script} id {self.id} userId {self.userId} averDurationSec {self.averDurationSec} maxDurationSec {self.maxDurationSec}   "
  def __str__(self):
      return self.__repr__()

class Task:
  """Task config""" 
  def __init__(self,
               id : int = 0,         
               ttlId : int = 0,         # Task template id    
               state : int = 0,
               progress : int = 0,
               result : str = "",
               params : str = "",
               createTime : str = "",
               takeInWorkTime : str = "",
               startTime : str = "",
               stopTime : str = ""):
    self.id = id
    self.ttlId = ttlId
    self.state = state
    self.progress = progress
    self.result = result
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
  _fields_ = [('capacityTask', ctypes.c_uint32),
              ('connectPnt', ctypes.c_char * 256),
              ('name', ctypes.c_char * 256),
              ('description', ctypes.c_char_p)]
class _SchedrState_C(ctypes.Structure):
  _fields_ = [('state', ctypes.c_uint32),
              ('activeTask', ctypes.c_uint32),
              ('startTime', ctypes.c_char * 32),
              ('stopTime', ctypes.c_char * 32),
              ('pingTime', ctypes.c_char * 32)]
class _WorkerCng_C(ctypes.Structure):
  _fields_ = [('schedrId', ctypes.c_uint64),
              ('capacityTask', ctypes.c_uint32),
              ('connectPnt', ctypes.c_char * 256),
              ('name', ctypes.c_char * 256),
              ('description', ctypes.c_char_p)]
class _WorkerState_C(ctypes.Structure):
  _fields_ = [('state', ctypes.c_uint32),
              ('activeTask', ctypes.c_uint32),
              ('load', ctypes.c_uint32),
              ('startTime', ctypes.c_char * 32),
              ('stopTime', ctypes.c_char * 32)]
class _TaskTemplCng_C(ctypes.Structure):
  _fields_ = [('userId', ctypes.c_uint64),
              ('schedrPresetId', ctypes.c_uint64),
              ('workerPresetId', ctypes.c_uint64),
              ('averDurationSec', ctypes.c_uint32),
              ('maxDurationSec', ctypes.c_uint32),
              ('name', ctypes.c_char * 256),
              ('description', ctypes.c_char_p),
              ('script', ctypes.c_char_p)]
class _TaskCng_C(ctypes.Structure):
  _fields_ = [('ttlId', ctypes.c_uint64),
              ('params', ctypes.c_char_p)]
class _TaskState_C(ctypes.Structure):
  _fields_ = [('progress', ctypes.c_uint32),
              ('state', ctypes.c_int32)]
class _TaskTime_C(ctypes.Structure):
  _fields_ = [('createTime', ctypes.c_char * 32),
              ('takeInWorkTime', ctypes.c_char * 32),
              ('startTime', ctypes.c_char * 32),
              ('stopTime', ctypes.c_char * 32)]
class _InternError_C(ctypes.Structure):
  _fields_ = [('schedrId', ctypes.c_uint64),
              ('workerId', ctypes.c_uint64),
              ('createTime', ctypes.c_char * 32),
              ('message', ctypes.c_char * 256)]

def version() -> str:
  """
  Version library
  :return: version
  """
  pfun = _lib.zmVersionLib
  pfun.restype = None
  pfun.argtypes = (ctypes.c_char_p,)

  ver = ctypes.create_string_buffer(32)
  pfun(ver)
  return ver.value.decode("utf-8")

class Connection:
  """Connection object"""
  
  _zmConn = None
  _userErrCBack : ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p) = None
  _changeTaskStateCBack : ctypes.CFUNCTYPE(None, ctypes.c_uint64, ctypes.c_uint64, ctypes.c_int32, ctypes.c_int32, ctypes.c_void_p) = None
  _cerr : str = ""
  
  def __init__(self, connStr : str):
    if not _lib:
      raise Exception('lib not load')
    
    cng = _ConnectCng_C()
    cng.connStr = connStr.encode("utf-8")
    
    pfun = _lib.zmCreateConnection
    pfun.argtypes = (_ConnectCng_C, ctypes.c_char_p)
    pfun.restype = ctypes.c_void_p
    err = ctypes.create_string_buffer(256)
    self._zmConn = pfun(cng, err)
    self._cerr = err.value.decode('utf-8')
  def __enter__(self):
    return self
  def __exit__(self, exc_type, exc_value, traceback):
    if (self._zmConn):
      pfun = _lib.zmDisconnect
      pfun.restype = None
      pfun.argtypes = (ctypes.c_void_p,)
      pfun(self._zmConn)

  #############################################################################
  ### Connection with DB

  def isOK(self) -> bool:
    return self._zmConn is not None
  def setErrorCBack(self, ucb):
    """
    Set ERROR callback
    :param ucb: def func(err : str)
    """
    if (self._zmConn):      
      def c_ecb(err: ctypes.c_char_p, udata: ctypes.c_void_p):
        ucb(err.decode("utf-8"))
      
      errCBackType = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p)    
      self._userErrCBack = errCBackType(c_ecb)

      pfun = _lib.zmSetErrorCBack
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, errCBackType, ctypes.c_void_p)
      return pfun(self._zmConn, self._userErrCBack, 0)
  def getLastError(self) -> str:
    """
    Last ERROR strind
    :return: errStr
    """
    if (self._zmConn):
      pfun = _lib.zmGetLastError
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_char_p)
      err = ctypes.create_string_buffer(256)
      pfun(self._zmConn, err)
      return err.value.decode("utf-8") 
    return self._cerr
  def createTables(self) -> bool:
    """
    Create tables, will be created if not exist
    :return: True ok
    """
    if (self._zmConn):
      pfun = _lib.zmCreateTables
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p,)
      return pfun(self._zmConn)
    return False

  
  #############################################################################
  ### Scheduler
  
  def addScheduler(self, iosch : Scheduler) -> bool:
    """
    Add new scheduler
    :param iosch: new Scheduler config
    :return: True - ok
    """
    if (self._zmConn):
      scng = _SchedrCng_C()
      scng.connectPnt = iosch.connectPnt.encode('utf-8')
      scng.capacityTask = iosch.capacityTask
      scng.name = iosch.name.encode('utf-8')
      scng.description = iosch.description.encode('utf-8')
      
      sid = ctypes.c_uint64(0)
      
      pfun = _lib.zmAddScheduler
      pfun.argtypes = (ctypes.c_void_p, _SchedrCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, scng, ctypes.byref(sid))):
        iosch.id = sid.value
        return True
    return False
  def getScheduler(self, iosch : Scheduler) -> bool:
    """
    Get Scheduler config by ID
    :param iosch: Scheduler config
    :return: True - ok
    """
    if (self._zmConn):
      scng = _SchedrCng_C()

      sid = ctypes.c_uint64(iosch.id)
      
      pfun = _lib.zmGetScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_SchedrCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, sid, ctypes.byref(scng))):      
        iosch.connectPnt = scng.connectPnt.decode('utf-8')
        iosch.capacityTask = scng.capacityTask
        iosch.name = scng.name.decode('utf-8')
        if scng.description:
          iosch.description = scng.description.decode('utf-8')
          self._freeResources()
        return True
    return False
  def changeScheduler(self, isch : Scheduler) -> bool:
    """
    Change Scheduler config
    :param isch: new Scheduler config
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(isch.id)
      scng = _SchedrCng_C()
      scng.connectPnt = isch.connectPnt.encode('utf-8')
      scng.capacityTask = isch.capacityTask
      scng.name = isch.name.encode('utf-8')
      scng.description = isch.description.encode('utf-8')
      
      pfun = _lib.zmChangeScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _SchedrCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid, scng)
    return False
  def delScheduler(self, schId) -> bool:
    """
    Delete scheduler
    :param schId: Scheduler id
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(schId)
            
      pfun = _lib.zmDelScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid)
    return False
  def startScheduler(self, schId) -> bool:
    """
    Start scheduler
    :param schId: Scheduler id
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(schId)
            
      pfun = _lib.zmStartScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid)
    return False
  def pauseScheduler(self, schId) -> bool:
    """
    Pause scheduler
    :param schId: Scheduler id
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(schId)
            
      pfun = _lib.zmPauseScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid)
    return False
  def pingScheduler(self, schId) -> bool:
    """
    Ping scheduler
    :param schId: Scheduler id
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(schId)
            
      pfun = _lib.zmPingScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid)
    return False
  def schedulerState(self, iosch : Scheduler) -> bool:
    """
    Scheduler state
    :param iosch: Scheduler config
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(iosch.id)      
      stateBuffer = _SchedrState_C()     
     
      pfun = _lib.zmStateOfScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_SchedrState_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, sid, stateBuffer)):      
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
    if (self._zmConn):
      sstate = ctypes.c_int32(state.value)

      pfun = _lib.zmGetAllSchedulers
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, sstate, ctypes.byref(dbuffer))
      
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources()

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
    if (self._zmConn):
      wcng = _WorkerCng_C()
      wcng.schedrId = iowkr.sId
      wcng.connectPnt = iowkr.connectPnt.encode('utf-8')
      wcng.capacityTask = iowkr.capacityTask
      wcng.name = iowkr.name.encode('utf-8')
      wcng.description = iowkr.description.encode('utf-8')

      wid = ctypes.c_uint64(0)
      
      pfun = _lib.zmAddWorker
      pfun.argtypes = (ctypes.c_void_p, _WorkerCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, wcng, ctypes.byref(wid))):
        iowkr.id = wid.value
        return True
    return False
  def getWorker(self, iowkr : Worker) -> bool:
    """
    Get Worker config by ID
    :param iowkr: Worker config
    :return: True - ok
    """
    if (self._zmConn):
      wcng = _WorkerCng_C()

      wid = ctypes.c_uint64(iowkr.id)
      
      pfun = _lib.zmGetWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_WorkerCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, wid, ctypes.byref(wcng))):
        iowkr.sId = wcng.schedrId
        iowkr.connectPnt = wcng.connectPnt.decode('utf-8')
        iowkr.capacityTask = wcng.capacityTask
        iowkr.name = wcng.name.decode('utf-8')
        if wcng.description:
          iowkr.description = wcng.description.decode('utf-8')
          self._freeResources()
        return True
    return False
  def changeWorker(self, iwkr : Worker) -> bool:
    """
    Change Worker config
    :param iwkr: new Worker config
    :return: True - ok
    """
    if (self._zmConn):
      wid = ctypes.c_uint64(iwkr.id)
      wcng = _WorkerCng_C()
      wcng.schedrId = iwkr.sId
      wcng.connectPnt = iwkr.connectPnt.encode('utf-8')
      wcng.capacityTask = iwkr.capacityTask
      wcng.name = iwkr.name.encode('utf-8')
      wcng.description = iwkr.description.encode('utf-8')

      pfun = _lib.zmChangeWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _WorkerCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, wid, wcng)
    return False
  def delWorker(self, wkrId) -> bool:
    """
    Delete Worker
    :param wkrId: Worker id
    :return: True - ok
    """
    if (self._zmConn):
      wid = ctypes.c_uint64(wkrId)
            
      pfun = _lib.zmDelWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, wid)
    return False
  def startWorker(self, wkrId) -> bool:
    """
    Start Worker
    :param wkrId: Worker id
    :return: True - ok
    """
    if (self._zmConn):
      wid = ctypes.c_uint64(wkrId)
            
      pfun = _lib.zmStartWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, wid)
    return False
  def pauseWorker(self, wkrId) -> bool:
    """
    Pause Worker
    :param wkrId: Worker id
    :return: True - ok
    """
    if (self._zmConn):
      wid = ctypes.c_uint64(wkrId)
            
      pfun = _lib.zmPauseWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, wid)
    return False
  def pingWorker(self, wkrId) -> bool:
    """
    Ping Worker
    :param wkrId: Worker id
    :return: True - ok
    """
    if (self._zmConn):
      wid = ctypes.c_uint64(wkrId)
            
      pfun = _lib.zmPingWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, wid)
    return False
  def workerState(self, iowkrs : List[Worker]) -> bool:
    """
    Worker state
    :param iowkrs: workers config
    :return: True - ok
    """
    if (self._zmConn):
      wsz = len(iowkrs)
      cwsz = ctypes.c_uint32(wsz)

      iowkrs.sort(key = lambda w: w.id)
          
      idBuffer = (ctypes.c_uint64 * wsz)(*[iowkrs[i].id for i in range(wsz)])
      stateBuffer = (_WorkerState_C * wsz)()

      pfun = _lib.zmStateOfWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint64), ctypes.c_uint32, ctypes.POINTER(_WorkerState_C))
      pfun.restype = ctypes.c_bool

      if (pfun(self._zmConn, idBuffer, cwsz, stateBuffer)):      
        for i in range(wsz):
          iowkrs[i].state = stateBuffer[i].state
          iowkrs[i].activeTask = stateBuffer[i].activeTask
          iowkrs[i].load = stateBuffer[i].load
          iowkrs[i].startTime = stateBuffer[i].startTime.decode('utf-8')
          iowkrs[i].stopTime = stateBuffer[i].stopTime.decode('utf-8')
        return True
    return False
  def getAllWorkers(self, sId : int, state : StateType=StateType.UNDEFINED) -> List[Worker]:
    """
    Get all workers
    :param sId: Scheduler id
    :param state: choose with current state. If the state is 'UNDEFINED', select all
    :return: list of Worker
    """
    if (self._zmConn):
      schId = ctypes.c_uint64(sId)
      sstate = ctypes.c_int32(state.value)

      pfun = _lib.zmGetAllWorkers
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, schId, sstate, ctypes.byref(dbuffer))
      
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources()

        owkr = []
        for i in range(osz):
          w = Worker(oid[i])
          self.getWorker(w)
          owkr.append(w)
        return owkr
    return []
    
  #############################################################################
  ### Task template 

  def addTaskTemplate(self, iott : TaskTemplate) -> bool:
    """
    Add new TaskTemplate
    :param iott: new tasktempl config
    :return: True - ok
    """
    
    if (self._zmConn):
      tcng = _TaskTemplCng_C()
      tcng.userId = iott.userId
      tcng.schedrPresetId = 0       # not used yet
      tcng.workerPresetId = 0
      tcng.averDurationSec = iott.averDurationSec
      tcng.maxDurationSec = iott.maxDurationSec
      tcng.name = iott.name.encode('utf-8')
      tcng.description = iott.description.encode('utf-8')
      tcng.script = iott.script.encode('utf-8')
      ttid = ctypes.c_uint64(0)
      
      pfun = _lib.zmAddTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, _TaskTemplCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tcng, ctypes.byref(ttid))):
        iott.id = ttid.value
        return True
    return False
  def getTaskTemplate(self, iott : TaskTemplate) -> bool:
    """
    Get TaskTemplate config by ID
    :param iott: TaskTemplate config
    :return: True - ok
    """
    if (self._zmConn):
      tcng = _TaskTemplCng_C()
     
      ttid = ctypes.c_uint64(iott.id)
            
      pfun = _lib.zmGetTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_TaskTemplCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, ttid, ctypes.byref(tcng))):      
        iott.userId = tcng.userId
        iott.averDurationSec = tcng.averDurationSec
        iott.maxDurationSec = tcng.maxDurationSec
        iott.name = tcng.name.decode('utf-8')
        if tcng.description:
          iott.description = tcng.description.decode('utf-8')          
        if tcng.script:          
          iott.script = tcng.script.decode('utf-8')
        self._freeResources()  
        return True
    return False
  def changeTaskTemplate(self, iott : TaskTemplate) -> bool:
    """
    Change TaskTemplate config
    :param iott: new TaskTemplate config
    :return: True - ok
    """
    if (self._zmConn):
      ttid = ctypes.c_uint64(iott.id)
      tcng = _TaskTemplCng_C()
      tcng.userId = iott.userId
      tcng.schedrPresetId = 0 # not used yet
      tcng.workerPresetId = 0
      tcng.averDurationSec = iott.averDurationSec
      tcng.maxDurationSec = iott.maxDurationSec            
      tcng.name = iott.name.encode('utf-8')
      tcng.description = iott.description.encode('utf-8')
      tcng.script = iott.script.encode('utf-8')
            
      pfun = _lib.zmChangeTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _TaskTemplCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, ttid, tcng)
    return False
  def delTaskTemplate(self, ttId : int) -> bool:
    """
    Delete taskTempl
    :param ttId: taskTempl id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(ttId)
            
      pfun = _lib.zmDelTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def getAllTaskTemplates(self, uId : int) -> List[TaskTemplate]:
    """
    Get all taskTemplates
    :param uId: User id
    :return: list of TaskTemplate
    """
    if (self._zmConn):
      userId = ctypes.c_uint64(uId)

      pfun = _lib.zmGetAllTaskTemplates
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, userId, ctypes.byref(dbuffer))

      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources()

        ott = []
        for i in range(osz):
          t = TaskTemplate(oid[i])
          self.getTaskTemplate(t)
          ott.append(t)
        return ott
    return []
  
  #############################################################################
  ### Task object

  def startTask(self, iot : Task) -> bool:
    """
    Start Task
    :param iot: task config
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(0)
            
      tcng = _TaskCng_C()
      tcng.ttlId = iot.ttlId      
      tcng.params = iot.params.encode('utf-8')

      pfun = _lib.zmStartTask
      pfun.argtypes = (ctypes.c_void_p, _TaskCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tcng, ctypes.byref(tid))):
        iot.id = tid.value
        return True
    return False
  def stopTask(self, tId) -> bool:
    """
    Stop Task
    :param tId: Task id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(tId)
            
      pfun = _lib.zmStopTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def pauseTask(self, tId) -> bool:
    """
    Pause Task
    :param tId: Task id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(tId)
            
      pfun = _lib.zmPauseTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def cancelTask(self, tId) -> bool:
    """
    Cancel Task
    :param tId: Task id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(tId)
            
      pfun = _lib.zmCancelTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def continueTask(self, tId) -> bool:
    """
    Continue Task
    :param tId: Task id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(tId)
            
      pfun = _lib.zmContinueTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def taskState(self, iot : List[Task]) -> bool:
    """
    Task state
    :param iot: tasks config
    :return: True - ok
    """
    if (self._zmConn):
      tsz = len(iot)
      ctsz = ctypes.c_uint32(tsz)
          
      iot.sort(key = lambda t: t.id)
        
      idBuffer = (ctypes.c_uint64 * tsz)(*[iot[i].id for i in range(tsz)])
      stateBuffer = (_TaskState_C * tsz)()
      
      pfun = _lib.zmStateOfTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint64), ctypes.c_uint32, ctypes.POINTER(_TaskState_C))
      pfun.restype = ctypes.c_bool

      if (pfun(self._zmConn, idBuffer, ctsz, stateBuffer)):      
        for i in range(tsz):
          iot[i].state = stateBuffer[i].state
          iot[i].progress = stateBuffer[i].progress          
        return True
    return False
  def taskResult(self, iot : Task) -> bool:
    """
    Task result
    :param iot: Task config
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(iot.id)
      tresult = ctypes.c_char_p()
      
      pfun = _lib.zmResultOfTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.c_char_p))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tid, ctypes.byref(tresult))):      
        if tresult:
          iot.result = tresult.value.decode("utf-8")
          self._freeResources()
        return True
    return False
  def taskTime(self, iot : Task) -> bool:
    """
    Task time
    :param iot: Task config
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(iot.id)
      ttime = _TaskTime_C()
      
      pfun = _lib.zmTimeOfTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_TaskTime_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tid, ctypes.byref(ttime))):      
        iot.createTime = ttime.createTime.decode('utf-8')
        iot.takeInWorkTime = ttime.takeInWorkTime.decode('utf-8')
        iot.startTime = ttime.startTime.decode('utf-8')
        iot.stopTime = ttime.stopTime.decode('utf-8')
        return True
    return False

  def addTaskForTracking(self, tId : int, uId : int):
    """
    Set change Task state callback
    """
    if (self._zmConn and self._changeTaskStateCBack):   
      c_tid = ctypes.c_uint64(tId)
      c_uid = ctypes.c_uint64(uId)
    
      taskStateCBackType = ctypes.CFUNCTYPE(None, ctypes.c_uint64, ctypes.c_uint64, ctypes.c_int32, ctypes.c_int32, ctypes.c_int32, ctypes.c_void_p)
      
      pfun = _lib.zmSetChangeTaskStateCBack
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.c_uint64, taskStateCBackType, ctypes.c_void_p)
      return pfun(self._zmConn, c_tid, c_uid, self._changeTaskStateCBack, 0)
    return False

  def setChangeTaskStateCBack(self, ucb):
    """
    Set change Task state callback
    :param ucb: def func(tId : uint64, uId : uint64, progress : int, prevState : StateType, newState : StateType)
    """
    if (self._zmConn):   
      def c_ucb(tId: ctypes.c_uint64, uId: ctypes.c_uint64, progress: ctypes.c_int32, prevState: ctypes.c_int32, newState: ctypes.c_int32, udata: ctypes.c_void_p):
        ucb(tId, uId, progress, prevState, newState)
      
      taskStateCBackType = ctypes.CFUNCTYPE(None, ctypes.c_uint64, ctypes.c_uint64, ctypes.c_int32, ctypes.c_int32, ctypes.c_int32, ctypes.c_void_p)
      self._changeTaskStateCBack = taskStateCBackType(c_ucb)
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
    if (self._zmConn):
      csId = ctypes.c_uint64(sId)
      cwId = ctypes.c_uint64(wId)
      cmCnt = ctypes.c_uint32(mCnt)

      pfun = _lib.zmGetInternErrors
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.c_uint64, ctypes.c_uint32, ctypes.POINTER(_InternError_C))
      pfun.restype = ctypes.c_uint32
      dbuffer = (_InternError_C * mCnt)()
      osz = pfun(self._zmConn, csId, cwId, cmCnt, dbuffer)
      
      oerr = []
      for i in range(osz):
        oerr[i].schedrId = dbuffer[i].schedrId
        oerr[i].workerId = dbuffer[i].workerId
        oerr[i].createTime = dbuffer[i].createTime
        oerr[i].message = dbuffer[i].message
      return oerr
    return []

  def _freeResources(self):
    if (self._zmConn):
      pfun = _lib.zmFreeResources
      pfun.restype = None
      pfun.argtypes = (ctypes.c_void_p,)
      pfun(self._zmConn)