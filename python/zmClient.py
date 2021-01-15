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
from enum import Enum

_LIB = None

def loadLib(path : str):
  global _LIB
  _LIB = ctypes.CDLL(path)
  
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
class User: 
  """User config""" 
  def __init__(self, 
               id : int = 0,
               name : str = "",
               passw : str = "", 
               description : str = ""):
    self.id = id
    self.name = name
    self.passw = passw
    self.description = description
class Schedr: 
  """Schedr config""" 
  def __init__(self,
               id : int = 0, 
               state : StateType = StateType.READY,
               connectPnt : str = "",
               capacityTask : int = 10000):
    self.id = id
    self.state = state
    self.connectPnt = connectPnt     # remote connection point: IP or DNS:port
    self.capacityTask = capacityTask # permissible simultaneous number of tasks 
class Worker: 
  """Worker config""" 
  def __init__(self,
               id : int = 0,
               sId : int = 0, 
               state : StateType = StateType.READY,
               connectPnt : str = "",
               capacityTask : int = 10000):
    self.id = id
    self.sId = sId                   # Schedr id
    self.state = state
    self.connectPnt = connectPnt     # remote connection point: IP or DNS:port
    self.capacityTask = capacityTask # permissible simultaneous number of tasks 
class Pipeline: 
  """Pipeline config""" 
  def __init__(self,
               id : int = 0,
               uId : int = 0,
               name : str = "",
               description : str = ""):
    self.id = id
    self.uId = uId                   # User id
    self.name = name    
    self.description = description 
class Group: 
  """Group config""" 
  def __init__(self,
               id : int = 0,
               pplId : int = 0,
               name : str = "",
               description : str = ""):
    self.id = id
    self.pplId = pplId               # Pipeline id
    self.name = name    
    self.description = description 
class TaskTemplate: 
  """TaskTemplate config""" 
  def __init__(self,
               id : int = 0,
               uId : int = 0,
               averDurationSec : int = 1,
               maxDurationSec : int = 1,
               name : str = "",
               description : str = "",
               script: str = ""):
    self.id = id
    self.uId = uId                   # User id
    self.averDurationSec = averDurationSec
    self.maxDurationSec = maxDurationSec
    self.name = name    
    self.description = description
    self.script = script 
class Task: 
  """Task config""" 
  def __init__(self,
               id : int = 0,
               pplId : int = 0,
               ttId : int = 0,
               gId : int = 0,
               priority : int = 1,
               prevTasksId : [int] = [],
               nextTasksId : [int] = [],
               params : [str] = [],
               state : StateType = StateType.READY, 
               progress : int = 0,
               result : str = "",
               createTime : str = "",
               takeInWorkTime : str = "",
               startTime : str = "",
               stopTime : str = ""):
    self.id = id
    self.pplId = pplId                 # Pipeline id    
    self.ttId = ttId                   # TaskTemplate id
    self.gId = gId                     # taskGroup id
    self.priority = priority           # [1..3]
    self.prevTasksId = prevTasksId     # Pipeline Task id of previous tasks to be COMPLETED: [qtId,..] 
    self.nextTasksId = nextTasksId     # Pipeline Task id of next tasks : [qtId,..] 
    self.params = params               # CLI params for script: ['param1','param2'..]
    self.state = state
    self.progress = progress
    self.result = result
    self.createTime = createTime
    self.takeInWorkTime = takeInWorkTime
    self.startTime = startTime
    self.stopTime = stopTime
class InternError: 
  """Internal ERROR""" 
  def __init__(self,
               sId : int = 0,
               wId : int = 0,
               createTime : str = 0,
               message : str = ""):
    self.sId = sId                 # Schedr id    
    self.wId = wId                 # Worker id
    self.createTime = createTime   
    self.message = message          
  
class ConnectCng_C(ctypes.Structure):
  _fields_ = [('connStr', ctypes.c_char_p)]
class UserCng_C(ctypes.Structure):
  _fields_ = [('name', ctypes.c_char * 256),
              ('passw', ctypes.c_char * 256),
              ('description', ctypes.c_char_p)]
class SchedrCng_C(ctypes.Structure):
  _fields_ = [('capacityTask', ctypes.c_uint32),
              ('connectPnt', ctypes.c_char * 256)]
class WorkerCng_C(ctypes.Structure):
  _fields_ = [('schedrId', ctypes.c_uint64),
              ('capacityTask', ctypes.c_uint32),
              ('connectPnt', ctypes.c_char * 256)]
class PipelineCng_C(ctypes.Structure):
  _fields_ = [('userId', ctypes.c_uint64),
              ('name', ctypes.c_char * 256),
              ('description', ctypes.c_char_p)]
class GroupCng_C(ctypes.Structure):
  _fields_ = [('pplId', ctypes.c_uint64),
              ('name', ctypes.c_char * 256),
              ('description', ctypes.c_char_p)]
class TaskTemplCng_C(ctypes.Structure):
  _fields_ = [('userId', ctypes.c_uint64),
              ('averDurationSec', ctypes.c_uint32),
              ('maxDurationSec', ctypes.c_uint32),
              ('name', ctypes.c_char * 256),
              ('description', ctypes.c_char_p),
              ('script', ctypes.c_char_p)]
class TaskCng_C(ctypes.Structure):
  _fields_ = [('pplId', ctypes.c_uint64),
              ('gId', ctypes.c_uint64),
              ('ttId', ctypes.c_uint64),
              ('priority', ctypes.c_uint32),
              ('prevTasksId', ctypes.c_char_p),
              ('nextTasksId', ctypes.c_char_p),
              ('params', ctypes.c_char_p)]
class TaskState_C(ctypes.Structure):
  _fields_ = [('progress', ctypes.c_uint32),
              ('state', ctypes.c_int32)]
class TaskTime_C(ctypes.Structure):
  _fields_ = [('createTime', ctypes.c_char * 32),
              ('takeInWorkTime', ctypes.c_char * 32),
              ('startTime', ctypes.c_char * 32),
              ('stopTime', ctypes.c_char * 32)]
class InternError_C(ctypes.Structure):
  _fields_ = [('schedrId', ctypes.c_uint64),
              ('workerId', ctypes.c_uint64),
              ('createTime', ctypes.c_char * 32),
              ('message', ctypes.c_char * 256)]

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

class Connection:
  """Connection object"""
  
  _zmConn = None
  _userErrCBack : ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p) = None
  _changeTaskStateCBack : ctypes.CFUNCTYPE(None, ctypes.c_uint64, ctypes.c_int32, ctypes.c_int32) = None
  _cerr : str = ""
  
  def __init__(self, connStr : str):
    if not _LIB:
      raise Exception('lib not load')
    
    cng = ConnectCng_C()
    cng.connStr = connStr.encode("utf-8")
    
    pfun = _LIB.zmCreateConnection
    pfun.argtypes = (ConnectCng_C, ctypes.c_char_p)
    pfun.restype = ctypes.c_void_p
    err = ctypes.create_string_buffer(256)
    self._zmConn = pfun(cng, err)
    self._cerr = err.value.decode('utf-8')
  def __enter__(self):
    return self
  def __exit__(self, exc_type, exc_value, traceback):
    if (self._zmConn):
      pfun = _LIB.zmDisconnect
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

      pfun = _LIB.zmSetErrorCBack
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, errCBackType, ctypes.c_void_p)
      return pfun(self._zmConn, self._userErrCBack, 0)
  def getLastError(self) -> str:
    """
    Last ERROR strind
    :return: errStr
    """
    if (self._zmConn):
      pfun = _LIB.zmGetLastError
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
      pfun = _LIB.zmCreateTables
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p,)
      return pfun(self._zmConn)
    return False

  #############################################################################
  ### User
  
  def addUser(self, iousr : User) -> bool:
    """
    Add new User
    :param iousr: new User config
    :return: True - ok
    """
    if (self._zmConn):
      ucng = UserCng_C()
      ucng.name = iousr.name.encode('utf-8')
      ucng.passw = iousr.passw.encode('utf-8')
      ucng.description = iousr.description.encode('utf-8')

      uid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddUser
      pfun.argtypes = (ctypes.c_void_p, UserCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, ucng, ctypes.byref(uid))):
        iousr.id = uid.value
        return True
    return False
  def getUserId(self, iousr : User)-> bool:
    """
    Get exist User id
    :param iousr: User config
    :return: True - ok
    """
    if (self._zmConn):
      ucng = UserCng_C()
      ucng.name = iousr.name.encode('utf-8')
      ucng.passw = iousr.passw.encode('utf-8')
      ucng.description = iousr.description.encode('utf-8')

      uid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmGetUserId
      pfun.argtypes = (ctypes.c_void_p, UserCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, ucng, ctypes.byref(uid))):
        iousr.id = uid.value
        return True
    return False
  def getUserCng(self, iousr : User) -> bool:
    """
    Get User config by ID
    :param iousr: User config
    :return: True - ok
    """
    if (self._zmConn):
      ucng = UserCng_C()

      uid = ctypes.c_uint64(iousr.id)
      
      pfun = _LIB.zmGetUserCng
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(UserCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, uid, ctypes.byref(ucng))):      
        iousr.name = ucng.name.decode('utf-8')
        iousr.passw = ucng.passw.decode('utf-8')
        if ucng.description:
          iousr.description = ucng.description.decode('utf-8')
          self._freeResources(None, ucng.description)      
        return True
    return False
  def changeUser(self, iusr : User) -> bool:
    """
    Change User config
    :param iusr: new User config
    :return: True - ok
    """
    if (self._zmConn):
      uid = ctypes.c_uint64(iusr.id)
      ucng = UserCng_C()
      ucng.name = iusr.name.encode('utf-8')
      ucng.passw = iusr.passw.encode('utf-8')
      ucng.description = iusr.description.encode('utf-8')
      
      pfun = _LIB.zmChangeUser
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, UserCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, uid, ucng)
    return False
  def delUser(self, usrId) -> bool:
    """
    Delete User
    :param usrId: User id
    :return: True - ok
    """
    if (self._zmConn):
      uid = ctypes.c_uint64(usrId)
            
      pfun = _LIB.zmDelUser
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, uid)
    return False
  def getAllUsers(self) -> [User]:
    """
    Get all users
    :return: list of users
    """
    if (self._zmConn):
      pfun = _LIB.zmGetAllUsers
      pfun.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, ctypes.byref(dbuffer))
                  
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources(dbuffer, ctypes.c_char_p(0))

        ousr = []
        for i in range(osz):
          u = User(oid[i])
          self.getUserCng(u)
          ousr.append(u)
        return ousr
    return []

  #############################################################################
  ### Scheduler
  
  def addScheduler(self, iosch : Schedr) -> bool:
    """
    Add new scheduler
    :param iosch: new Schedr config
    :return: True - ok
    """
    if (self._zmConn):
      scng = SchedrCng_C()
      scng.connectPnt = iosch.connectPnt.encode('utf-8')
      scng.capacityTask = iosch.capacityTask
      
      sid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddScheduler
      pfun.argtypes = (ctypes.c_void_p, SchedrCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, scng, ctypes.byref(sid))):
        iosch.id = sid.value
        return True
    return False
  def getScheduler(self, iosch : Schedr) -> bool:
    """
    Get Schedr config by ID
    :param iosch: Schedr config
    :return: True - ok
    """
    if (self._zmConn):
      scng = SchedrCng_C()

      sid = ctypes.c_uint64(iosch.id)
      
      pfun = _LIB.zmGetScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(SchedrCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, sid, ctypes.byref(scng))):      
        iosch.connectPnt = scng.connectPnt.decode('utf-8')
        iosch.capacityTask = scng.capacityTask
      return True
    return False
  def changeScheduler(self, isch : Schedr) -> bool:
    """
    Change Schedr config
    :param isch: new Schedr config
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(isch.id)
      scng = SchedrCng_C()
      scng.connectPnt = isch.connectPnt.encode('utf-8')
      scng.capacityTask = isch.capacityTask
      
      pfun = _LIB.zmChangeScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, SchedrCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid, scng)
    return False
  def delScheduler(self, schId) -> bool:
    """
    Delete scheduler
    :param schId: Schedr id
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(schId)
            
      pfun = _LIB.zmDelScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid)
    return False
  def startScheduler(self, schId) -> bool:
    """
    Start scheduler
    :param schId: Schedr id
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(schId)
            
      pfun = _LIB.zmStartScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid)
    return False
  def pauseScheduler(self, schId) -> bool:
    """
    Pause scheduler
    :param schId: Schedr id
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(schId)
            
      pfun = _LIB.zmPauseScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid)
    return False
  def pingScheduler(self, schId) -> bool:
    """
    Ping scheduler
    :param schId: Schedr id
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(schId)
            
      pfun = _LIB.zmPingScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid)
    return False
  def schedulerState(self, iosch : Schedr) -> bool:
    """
    Schedr state
    :param iosch: Schedr config
    :return: True - ok
    """
    if (self._zmConn):
      scng = SchedrCng_C()

      sid = ctypes.c_uint64(iosch.id)
      sstate = ctypes.c_int32(0)
      
      pfun = _LIB.zmSchedulerState
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.c_int32))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, sid, ctypes.byref(sstate))):      
        iosch.state = StateType(sstate.value) 
        return True
    return False
  def getAllSchedulers(self, state : StateType=StateType.UNDEFINED) -> [Schedr]:
    """
    Get all schedrs
    :param state: choose with current state. If the state is 'UNDEFINED', select all
    :return: list of Schedr
    """
    if (self._zmConn):
      sstate = ctypes.c_int32(state.value)

      pfun = _LIB.zmGetAllSchedulers
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, sstate, ctypes.byref(dbuffer))
      
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources(dbuffer, ctypes.c_char_p(0))

        osch = []
        for i in range(osz):
          s = Schedr(oid[i])
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
      wcng = WorkerCng_C()
      wcng.schedrId = iowkr.sId
      wcng.connectPnt = iowkr.connectPnt.encode('utf-8')
      wcng.capacityTask = iowkr.capacityTask
      
      wid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddWorker
      pfun.argtypes = (ctypes.c_void_p, WorkerCng_C, ctypes.POINTER(ctypes.c_uint64))
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
      wcng = WorkerCng_C()

      wid = ctypes.c_uint64(iowkr.id)
      
      pfun = _LIB.zmGetWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(WorkerCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, wid, ctypes.byref(wcng))):
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
    if (self._zmConn):
      wid = ctypes.c_uint64(iwkr.id)
      wcng = WorkerCng_C()
      wcng.schedrId = iwkr.sId
      wcng.connectPnt = iwkr.connectPnt.encode('utf-8')
      wcng.capacityTask = iwkr.capacityTask
      
      pfun = _LIB.zmChangeWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, WorkerCng_C)
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
            
      pfun = _LIB.zmDelWorker
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
            
      pfun = _LIB.zmStartWorker
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
            
      pfun = _LIB.zmPauseWorker
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
            
      pfun = _LIB.zmPingWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, wid)
    return False
  def workerState(self, iowkrs : [Worker]) -> bool:
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
      stateBuffer = (ctypes.c_int32 * wsz)()

      pfun = _LIB.zmWorkerState
      pfun.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint64), ctypes.c_uint32, ctypes.POINTER(ctypes.c_int32))
      pfun.restype = ctypes.c_bool

      if (pfun(self._zmConn, idBuffer, cwsz, stateBuffer)):      
        for i in range(wsz):
          iowkrs[i].state = StateType(stateBuffer[i])
        return True
    return False
  def getAllWorkers(self, sId : int, state : StateType=StateType.UNDEFINED) -> [Worker]:
    """
    Get all workers
    :param sId: Schedr id
    :param state: choose with current state. If the state is 'UNDEFINED', select all
    :return: list of Worker
    """
    if (self._zmConn):
      schId = ctypes.c_uint64(sId)
      sstate = ctypes.c_int32(state.value)

      pfun = _LIB.zmGetAllWorkers
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, schId, sstate, ctypes.byref(dbuffer))
      
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources(dbuffer, ctypes.c_char_p(0))

        owkr = []
        for i in range(osz):
          w = Worker(oid[i])
          self.getWorker(w)
          owkr.append(w)
        return owkr
    return []
  
  #############################################################################
  ### Pipeline of tasks
  
  def addPipeline(self, ioppl : Pipeline) -> bool:
    """
    Add new Pipeline
    :param ioppl: new Pipeline config
    :return: True - ok
    """
    if (self._zmConn):
      pcng = PipelineCng_C()
      pcng.userId = ioppl.uId
      pcng.name = ioppl.name.encode('utf-8')
      pcng.description = ioppl.description.encode('utf-8')
      
      pplid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddPipeline
      pfun.argtypes = (ctypes.c_void_p, PipelineCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, pcng, ctypes.byref(pplid))):
        ioppl.id = pplid.value
        return True
    return False
  def getPipeline(self, ioppl : Pipeline) -> bool:
    """
    Get Pipeline config by ID
    :param ioppl: Pipeline config
    :return: True - ok
    """
    if (self._zmConn):
      pcng = PipelineCng_C()

      pplid = ctypes.c_uint64(ioppl.id)
      
      pfun = _LIB.zmGetPipeline
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(PipelineCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, pplid, ctypes.byref(pcng))):      
        ioppl.uId = pcng.userId
        ioppl.name = pcng.name.decode('utf-8')
        if pcng.description:
          ioppl.description = pcng.description.decode('utf-8')
          self._freeResources(None, pcng.description)
        return True
    return False
  def changePipeline(self, ippl : User) -> bool:
    """
    Change Pipeline config
    :param ippl: new Pipeline config
    :return: True - ok
    """
    if (self._zmConn):
      pplid = ctypes.c_uint64(ippl.id)
      pcng = PipelineCng_C()
      pcng.userId = ippl.uId
      pcng.name = ippl.name.encode('utf-8')
      pcng.description = ippl.description.encode('utf-8')
      
      pfun = _LIB.zmChangePipeline
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, PipelineCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, pplid, pcng)
    return False
  def delPipeline(self, pplId : int) -> bool:
    """
    Delete Pipeline
    :param pplId: Pipeline id
    :return: True - ok
    """
    if (self._zmConn):
      pid = ctypes.c_uint64(pplId)
            
      pfun = _LIB.zmDelPipeline
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, pid)
    return False
  def getAllPipelines(self, uId : int) -> [Pipeline]:
    """
    Get all pipelines
    :param uId: User id
    :return: list of Pipeline
    """
    if (self._zmConn):
      userId = ctypes.c_uint64(uId)

      pfun = _LIB.zmGetAllPipelines
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, userId, ctypes.byref(dbuffer))
      
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources(dbuffer, ctypes.c_char_p(0))

        oppl = []
        for i in range(osz):
          p = Pipeline(oid[i])
          self.getPipeline(p)
          oppl.append(p)
        return oppl
    return []

  #############################################################################
  ### Group of tasks
  
  def addGroup(self, iogrp : Group) -> bool:
    """
    Add new Group
    :param iogrp: new Group config
    :return: True - ok
    """
    if (self._zmConn):
      gcng = GroupCng_C()
      gcng.pplId = iogrp.pplId
      gcng.name = iogrp.name.encode('utf-8')
      gcng.description = iogrp.description.encode('utf-8')
      
      gid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddGroup
      pfun.argtypes = (ctypes.c_void_p, GroupCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, pcng, ctypes.byref(gid))):
        iogrp.id = gid.value
        return True
    return False
  def getGroup(self, iogrp : Group) -> bool:
    """
    Get Group config by ID
    :param iogrp: Group config
    :return: True - ok
    """
    if (self._zmConn):
      gcng = GroupCng_C()

      gid = ctypes.c_uint64(iogrp.id)
      
      pfun = _LIB.zmGetGroup
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(GroupCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, gid, ctypes.byref(gcng))):      
        iogrp.pplId = gcng.pplId
        iogrp.name = gcng.name.decode('utf-8')
        if gcng.description:
          iogrp.description = gcng.description.decode('utf-8')
          self._freeResources(None, gcng.description)
        return True
    return False
  def changeGroup(self, igrp : User) -> bool:
    """
    Change Group config
    :param igrp: new Group config
    :return: True - ok
    """
    if (self._zmConn):
      gid = ctypes.c_uint64(igrp.id)
      gcng = GroupCng_C()
      gcng.pplId = igrp.pplId
      gcng.name = igrp.name.encode('utf-8')
      gcng.description = igrp.description.encode('utf-8')
      
      pfun = _LIB.zmChangeGroup
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, GroupCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, gid, gcng)
    return False
  def delGroup(self, grpId : int) -> bool:
    """
    Delete Group
    :param grpId: Group id
    :return: True - ok
    """
    if (self._zmConn):
      gid = ctypes.c_uint64(grpId)
            
      pfun = _LIB.zmDelGroup
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, gid)
    return False
  def getAllGroups(self, pplId : int) -> [Pipeline]:
    """
    Get all groups
    :param pplId: Pipeline id
    :return: list of groups
    """
    if (self._zmConn):
      plineId = ctypes.c_uint64(pplId)

      pfun = _LIB.zmGetAllGroups
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, plineId, ctypes.byref(dbuffer))
      
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources(dbuffer, ctypes.c_char_p(0))

        ogrp = []
        for i in range(osz):
          g = Group(oid[i])
          self.getGroup(g)
          ogrp.append(g)
        return ogrp
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
      tcng = TaskTemplCng_C()
      tcng.userId = iott.uId
      tcng.averDurationSec = iott.averDurationSec
      tcng.maxDurationSec = iott.maxDurationSec
      tcng.name = iott.name.encode('utf-8')
      tcng.description = iott.description.encode('utf-8')
      tcng.script = iott.script.encode('utf-8')
      
      ttid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, TaskTemplCng_C, ctypes.POINTER(ctypes.c_uint64))
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
      tcng = TaskTemplCng_C()

      ttid = ctypes.c_uint64(iott.id)
      
      pfun = _LIB.zmGetTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(TaskTemplCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, ttid, ctypes.byref(tcng))):      
        iott.uId = tcng.userId
        iott.averDurationSec = tcng.averDurationSec
        iott.maxDurationSec = tcng.maxDurationSec
        iott.name = tcng.name.decode('utf-8')
        if tcng.description:
          iott.description = tcng.description.decode('utf-8')
          self._freeResources(None, tcng.description)
        if tcng.script:
          iott.script = tcng.script.decode('utf-8')
          self._freeResources(None, tcng.script)
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
      tcng = TaskTemplCng_C()
      tcng.userId = iott.uId
      tcng.averDurationSec = iott.averDurationSec
      tcng.maxDurationSec = iott.maxDurationSec            
      tcng.name = iott.name.encode('utf-8')
      tcng.description = iott.description.encode('utf-8')
      tcng.script = iott.script.encode('utf-8')
            
      pfun = _LIB.zmChangeTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, TaskTemplCng_C)
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
            
      pfun = _LIB.zmDelTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def getAllTaskTemplates(self, uId : int) -> [TaskTemplate]:
    """
    Get all taskTemplates
    :param uId: User id
    :return: list of TaskTemplate
    """
    if (self._zmConn):
      userId = ctypes.c_uint64(uId)

      pfun = _LIB.zmGetAllTaskTemplates
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, userId, ctypes.byref(dbuffer))

      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources(dbuffer, ctypes.c_char_p(0))

        ott = []
        for i in range(osz):
          t = TaskTemplate(oid[i])
          self.getTaskTemplate(t)
          ott.append(t)
        return ott
    return []

  #############################################################################
  ### Task of Pipeline
 
  def addTask(self, iot : Task) -> bool:
    """
    Add new Task
    :param iot: new Task config
    :return: True - ok
    """
    if (self._zmConn):
      tcng = TaskCng_C()
      tcng.pplId = iot.pplId
      tcng.ttId = iot.ttId
      tcng.gId = iot.gId
      tcng.priority = iot.priority
      tcng.prevTasksId = ','.join(str(i) for i in iot.prevTasksId).encode('utf-8')
      tcng.nextTasksId = ','.join(str(i) for i in iot.nextTasksId).encode('utf-8')
      tcng.params = ','.join(iot.params).encode('utf-8')
      
      tid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddTask
      pfun.argtypes = (ctypes.c_void_p, TaskCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tcng, ctypes.byref(tid))):
        iot.id = tid.value
        return True
    return False
  def getTask(self, iot : Task) -> bool:
    """
    Get Task config by ID
    :param iot: Task config
    :return: True - ok
    """
    if (self._zmConn):
      tcng = TaskCng_C()

      tid = ctypes.c_uint64(iot.id)
      
      pfun = _LIB.zmGetTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(TaskCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tid, ctypes.byref(tcng))):      
        iot.pplId = tcng.pplId
        iot.ttId = tcng.ttId
        iot.gId = tcng.gId
        iot.priority = tcng.priority
        if tcng.prevTasksId:
          iot.prevTasksId = [int(i) for i in tcng.prevTasksId.decode('utf-8').split(',')]
          self._freeResources(None, tcng.prevTasksId)
        if tcng.nextTasksId:
          iot.nextTasksId = [int(i) for i in tcng.nextTasksId.decode('utf-8').split(',')]
          self._freeResources(None, tcng.nextTasksId)
        if tcng.params:
          iot.params = tcng.params.decode('utf-8').split(',')
          self._freeResources(None, tcng.params)
        return True
    return False
  def changeTask(self, iot : Task) -> bool:
    """
    Change Task config
    :param iot: new Task config
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(iot.id)
      tcng = TaskCng_C()
      tcng.pplId = iot.pplId
      tcng.ttId = iot.ttId
      tcng.gId = iot.gId
      tcng.priority = iot.priority
      tcng.prevTasksId = ','.join(str(i) for i in iot.prevTasksId).encode('utf-8')
      tcng.nextTasksId = ','.join(str(i) for i in iot.nextTasksId).encode('utf-8')
      tcng.params = ','.join(iot.params).encode('utf-8')
            
      pfun = _LIB.zmChangeTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, TaskCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid, tcng)
    return False
  def delTask(self, tId : int) -> bool:
    """
    Delete Task
    :param tId: Task id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(tId)
            
      pfun = _LIB.zmDelTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def startTask(self, tId) -> bool:
    """
    Start Task
    :param tId: Task id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(tId)
            
      pfun = _LIB.zmStartTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def stopTask(self, tId) -> bool:
    """
    Stop Task
    :param tId: Task id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(tId)
            
      pfun = _LIB.zmStopTask
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
            
      pfun = _LIB.zmPauseTask
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
            
      pfun = _LIB.zmCancelTask
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
            
      pfun = _LIB.zmContinueTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def taskState(self, iot : [Task]) -> bool:
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
      stateBuffer = (TaskState_C * tsz)()
      
      pfun = _LIB.zmTaskState
      pfun.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint64), ctypes.c_uint32, ctypes.POINTER(TaskState_C))
      pfun.restype = ctypes.c_bool

      if (pfun(self._zmConn, idBuffer, ctsz, stateBuffer)):      
        for i in range(tsz):
          iot[i].state = StateType(stateBuffer[i].state)
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
      
      pfun = _LIB.zmTaskResult
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.c_char_p))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tid, ctypes.byref(tresult))):      
        if tresult:
          iot.result = tresult.decode("utf-8")  
          self._freeResources(None, tresult)
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
      ttime = TaskTime_C()
      
      pfun = _LIB.zmTaskTime
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(TaskTime_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tid, ctypes.byref(ttime))):      
        iot.createTime = ttime.createTime.decode('utf-8')
        iot.takeInWorkTime = ttime.takeInWorkTime.decode('utf-8')
        iot.startTime = ttime.startTime.decode('utf-8')
        iot.stopTime = ttime.stopTime.decode('utf-8')
        return True
    return False
  def getAllTasks(self, pplId : int, state : StateType=StateType.UNDEFINED) -> [Task]:
    """
    Get all tasks
    :param pplId: Pipeline id
    :param state: state type
    :return: list of Task
    """
    if (self._zmConn):
      cpplId = ctypes.c_uint64(pplId)
      cstate = ctypes.c_int32(state.value)

      pfun = _LIB.zmGetAllTasks
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, cpplId, cstate, ctypes.byref(dbuffer))
            
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources(dbuffer, ctypes.c_char_p(0))
      
        ott = []
        for i in range(osz):
          t = Task(oid[i])
          self.getTask(t)
          ott.append(t)
        return ott
    return []
  def setChangeTaskStateCBack(self, tId : int, ucb):
    """
    Set change Task state callback
    :param ucb: def func(tId : uint64, prevState : StateType, newState : StateType)
    """
    if (self._zmConn):   
      c_tid = ctypes.c_uint64(tId)
      def c_ucb(tId: ctypes.c_uint64, prevState: ctypes.c_int32, newState: ctypes.c_int32):
        ucb(tId, prevState, newState)
      
      taskStateCBackType : ctypes.CFUNCTYPE(None, ctypes.c_uint64, ctypes.c_int32, ctypes.c_int32)
      self._changeTaskStateCBack = taskStateCBackType(c_ucb)

      pfun = _LIB.zmSetChangeTaskStateCBack
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, taskStateCBackType)
      return pfun(self._zmConn, c_tid, self._changeTaskStateCBack)
      
  #############################################################################
  ### Internal errors
 
  def getInternErrors(self, sId : int, wId : int, mCnt : int) -> [InternError]:
    """
    Get internal errors
    :param sId: Schedr id
    :param wId: Worker id
    :param mCnt: max mess count
    :return: list of errors
    """
    if (self._zmConn):
      csId = ctypes.c_uint64(sId)
      cwId = ctypes.c_uint64(wId)
      cmCnt = ctypes.c_uint32(mCnt)

      pfun = _LIB.zmGetInternErrors
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.c_uint64, ctypes.c_uint32, ctypes.POINTER(InternError_C))
      pfun.restype = ctypes.c_uint32
      dbuffer = (InternError_C * mCnt)()
      osz = pfun(self._zmConn, csId, cwId, cmCnt, dbuffer)
      
      oerr = []
      for i in range(osz):
        oerr[i].sId = dbuffer[i].schedrId
        oerr[i].wId = dbuffer[i].workerId
        oerr[i].createTime = dbuffer[i].createTime
        oerr[i].message = dbuffer[i].message
      return oerr
    return []

  def _freeResources(self, puint64 : ctypes.POINTER(ctypes.c_uint64), pchar : ctypes.c_char_p):
    pfun = _LIB.zmFreeResources
    pfun.restype = None
    pfun.argtypes = (ctypes.POINTER(ctypes.c_uint64), ctypes.c_char_p)
    pfun(puint64, pchar)