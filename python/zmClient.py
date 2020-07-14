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

#############################################################################
### Common

class stateType(Enum):
  """State type"""
  undefined     = -1
  ready         = 0
  start         = 1
  running       = 2
  pause         = 3
  stop          = 4    
  completed     = 5
  error         = 6
  cancel        = 7
  notResponding = 8
class dbType(Enum):
  """Database type"""
  PostgreSQL = 0
class user: 
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
class schedr: 
  """Schedr config""" 
  def __init__(self,
               id : int = 0, 
               state : stateType = stateType.ready,
               connectPnt : str = "",
               capacityTask : int = 10000):
    self.id = id
    self.state = state
    self.connectPnt = connectPnt     # remote connection point: IP or DNS:port
    self.capacityTask = capacityTask # permissible simultaneous number of tasks 
class worker: 
  """Worker config""" 
  def __init__(self,
               id : int = 0,
               sId : int = 0, 
               state : stateType = stateType.ready,
               connectPnt : str = "",
               capacityTask : int = 10000):
    self.id = id
    self.sId = sId                   # schedr id
    self.state = state
    self.connectPnt = connectPnt     # remote connection point: IP or DNS:port
    self.capacityTask = capacityTask # permissible simultaneous number of tasks 
class pipeline: 
  """Pipeline config""" 
  def __init__(self,
               id : int = 0,
               uId : int = 0,
               isShared : int = 0,
               name : str = "",
               description : str = ""):
    self.id = id
    self.uId = uId                   # user id
    self.isShared = isShared         # [0..1]
    self.name = name    
    self.description = description 
class taskTemplate: 
  """TaskTemplate config""" 
  def __init__(self,
               id : int = 0,
               uId : int = 0,
               averDurationSec : int = 1,
               maxDurationSec : int = 1,
               isShared : int = 0,
               name : str = "",
               description : str = "",
               script: str = ""):
    self.id = id
    self.uId = uId                   # user id
    self.averDurationSec = averDurationSec
    self.maxDurationSec = maxDurationSec
    self.isShared = isShared         # [0..1]
    self.name = name    
    self.description = description
    self.script = script 
class task: 
  """Task config""" 
  def __init__(self,
               id : int = 0,
               pplId : int = 0,
               ttId : int = 0,
               priority : int = 1,
               prevTasksId = [],
               nextTasksId = [],
               params = [],
               screenRect : str = "",
               state : stateType = stateType.ready, 
               progress : int = 0,
               result : str = "",
               createTime : str = "",
               takeInWorkTime : str = "",
               startTime : str = "",
               stopTime : str = ""):
    self.id = id
    self.pplId = pplId                 # pipeline id    
    self.ttId = ttId                   # taskTemplate id
    self.priority = priority           # [1..3]
    self.prevTasksId = prevTasksId     # pipeline task id of previous tasks to be completed: [qtId,..] 
    self.nextTasksId = nextTasksId     # pipeline task id of next tasks: : [qtId,..]
    self.params = params               # CLI params for script: ['param1','param2'..]
    self.screenRect = screenRect       # screenRect on UI: x y w h
    self.state = state
    self.progress = progress
    self.result = result
    self.createTime = createTime
    self.takeInWorkTime = takeInWorkTime
    self.startTime = startTime
    self.stopTime = stopTime
class internError: 
  """Internal error""" 
  def __init__(self,
               sId : int = 0,
               wId : int = 0,
               createTime : str = 0,
               message : str = ""):
    self.sId = sId                 # schedr id    
    self.wId = wId                 # worker id
    self.createTime = createTime   
    self.message = message          
  
class _connectCng(ctypes.Structure):
  _fields_ = [('dbType', ctypes.c_int32),
              ('connStr', ctypes.c_char_p)]
class _userCng(ctypes.Structure):
  _fields_ = [('name', ctypes.c_char * 255),
              ('passw', ctypes.c_char * 255),
              ('description', ctypes.c_char_p)]
class _schedrCng(ctypes.Structure):
  _fields_ = [('capacityTask', ctypes.c_uint32),
              ('connectPnt', ctypes.c_char * 255)]
class _workerCng(ctypes.Structure):
  _fields_ = [('schedrId', ctypes.c_uint64),
              ('capacityTask', ctypes.c_uint32),
              ('connectPnt', ctypes.c_char * 255)]
class _pipelineCng(ctypes.Structure):
  _fields_ = [('userId', ctypes.c_uint64),
              ('isShared', ctypes.c_uint32),
              ('name', ctypes.c_char * 255),
              ('description', ctypes.c_char_p)]
class _taskTemplCng(ctypes.Structure):
  _fields_ = [('userId', ctypes.c_uint64),
              ('averDurationSec', ctypes.c_uint32),
              ('maxDurationSec', ctypes.c_uint32),
              ('isShared', ctypes.c_uint32),
              ('name', ctypes.c_char * 255),
              ('description', ctypes.c_char_p),
              ('script', ctypes.c_char_p)]
class _taskCng(ctypes.Structure):
  _fields_ = [('pplId', ctypes.c_uint64),
              ('ttId', ctypes.c_uint64),
              ('priority', ctypes.c_uint32),
              ('prevTasksId', ctypes.c_char_p),
              ('nextTasksId', ctypes.c_char_p),
              ('params', ctypes.c_char_p),
              ('screenRect', ctypes.c_char_p)]
class _taskState(ctypes.Structure):
  _fields_ = [('progress', ctypes.c_uint32),
              ('state', ctypes.c_int32)]
class _taskTime(ctypes.Structure):
  _fields_ = [('createTime', ctypes.c_char * 32),
              ('takeInWorkTime', ctypes.c_char * 32),
              ('startTime', ctypes.c_char * 32),
              ('stopTime', ctypes.c_char * 32)]
class _internError(ctypes.Structure):
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

class ZMObj:
  """Connection object"""
  
  _zmConn = 0
  _userErrCBack : ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p) = 0
  _cerr : str = ""
     
  #############################################################################
  ### Connection with DB

  def __init__(self, db : dbType, connStr : str):
    cng = _connectCng()
    cng.dbType = db.value
    cng.connStr = connStr.encode("utf-8")

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
      
      errCBackType = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_void_p)    
      self._userErrCBack = errCBackType(c_ecb)

      pfun = _LIB.zmSetErrorCBack
      pfun.restype = ctypes.c_bool
      pfun.argtypes = (ctypes.c_void_p, errCBackType, ctypes.c_void_p)
      return pfun(self._zmConn, self._userErrCBack, 0)
  def getLastError(self) -> str:
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

  #############################################################################
  ### User
  
  def addUser(self, iousr : user) -> bool:
    """
    Add new user
    :param iousr: new user config
    :return: True - ok
    """
    if (self._zmConn):
      ucng = _userCng()
      ucng.name = iousr.name.encode('utf-8')
      ucng.passw = iousr.passw.encode('utf-8')
      ucng.description = iousr.description.encode('utf-8')

      uid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddUser
      pfun.argtypes = (ctypes.c_void_p, _userCng, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, ucng, ctypes.byref(uid))):
        iousr.id = uid.value
        return True
    return False
  def getUserId(self, iousr : user)-> bool:
    """
    Get exist user id
    :param iousr: user config
    :return: True - ok
    """
    if (self._zmConn):
      ucng = _userCng()
      ucng.name = iousr.name.encode('utf-8')
      ucng.passw = iousr.passw.encode('utf-8')
      ucng.description = iousr.description.encode('utf-8')

      uid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmGetUserId
      pfun.argtypes = (ctypes.c_void_p, _userCng, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, ucng, ctypes.byref(uid))):
        iousr.id = uid.value
        return True
    return False
  def getUserCng(self, iousr : user) -> bool:
    """
    Get user config by ID
    :param iousr: user config
    :return: True - ok
    """
    if (self._zmConn):
      ucng = _userCng()

      uid = ctypes.c_uint64(iousr.id)
      
      pfun = _LIB.zmGetUserCng
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_userCng))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, uid, ctypes.byref(ucng))):      
        iousr.name = ucng.name.decode('utf-8')
        iousr.passw = ucng.passw.decode('utf-8')
        iousr.description = ucng.description.decode('utf-8')
        return True
    return False
  def changeUser(self, iusr : user) -> bool:
    """
    Change user config
    :param iusr: new user config
    :return: True - ok
    """
    if (self._zmConn):
      uid = ctypes.c_uint64(iusr.id)
      ucng = _userCng()
      ucng.name = iusr.name.encode('utf-8')
      ucng.passw = iusr.passw.encode('utf-8')
      ucng.description = iusr.description.encode('utf-8')
      
      pfun = _LIB.zmChangeUser
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _userCng)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, uid, ucng)
    return False
  def delUser(self, usrId) -> bool:
    """
    Delete user
    :param usrId: user id
    :return: True - ok
    """
    if (self._zmConn):
      uid = ctypes.c_uint64(usrId)
            
      pfun = _LIB.zmDelUser
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, uid)
    return False
  def getAllUsers(self) -> [user]:
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
      oid = [dbuffer[i] for i in range(osz)]
      
      pfun = _LIB.zmFreeResources
      pfun.restype = None
      pfun.argtypes = (ctypes.POINTER(ctypes.c_uint64), ctypes.c_char_p)
      pfun(dbuffer, ctypes.c_char_p(0))

      ousr = []
      for i in range(osz):
        u = user(oid[i])
        self.getUserCng(u)
        ousr.append(u)
      return ousr
    return []

  #############################################################################
  ### Scheduler
  
  def addScheduler(self, iosch : schedr) -> bool:
    """
    Add new scheduler
    :param iosch: new schedr config
    :return: True - ok
    """
    if (self._zmConn):
      scng = _schedrCng()
      scng.connectPnt = iosch.connectPnt.encode('utf-8')
      scng.capacityTask = iosch.capacityTask
      
      sid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddScheduler
      pfun.argtypes = (ctypes.c_void_p, _schedrCng, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, scng, ctypes.byref(sid))):
        iosch.id = sid.value
        return True
    return False
  def getScheduler(self, iosch : schedr) -> bool:
    """
    Get schedr config by ID
    :param iosch: schedr config
    :return: True - ok
    """
    if (self._zmConn):
      scng = _schedrCng()

      sid = ctypes.c_uint64(iosch.id)
      
      pfun = _LIB.zmGetScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_schedrCng))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, sid, ctypes.byref(scng))):      
        iosch.connectPnt = scng.connectPnt.decode('utf-8')
        iosch.capacityTask = scng.capacityTask
      return True
    return False
  def changeScheduler(self, isch : schedr) -> bool:
    """
    Change schedr config
    :param isch: new schedr config
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(isch.id)
      scng = _schedrCng()
      scng.connectPnt = isch.connectPnt.encode('utf-8')
      scng.capacityTask = isch.capacityTask
      
      pfun = _LIB.zmChangeScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _schedrCng)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid, scng)
    return False
  def delScheduler(self, schId) -> bool:
    """
    Delete scheduler
    :param schId: schedr id
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
    :param schId: schedr id
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
    :param schId: schedr id
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
    :param schId: schedr id
    :return: True - ok
    """
    if (self._zmConn):
      sid = ctypes.c_uint64(schId)
            
      pfun = _LIB.zmPingScheduler
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, sid)
    return False
  def schedulerState(self, iosch : schedr) -> bool:
    """
    Schedr state
    :param iosch: schedr config
    :return: True - ok
    """
    if (self._zmConn):
      scng = _schedrCng()

      sid = ctypes.c_uint64(iosch.id)
      sstate = ctypes.c_int32(0)
      
      pfun = _LIB.zmSchedulerState
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.c_int32))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, sid, ctypes.byref(sstate))):      
        iosch.state = stateType(sstate.value) 
        return True
    return False
  def getAllSchedulers(self, state : stateType) -> [schedr]:
    """
    Get all schedrs
    :param state: choose with current state. If the state is 'undefined', select all
    :return: list of schedr
    """
    if (self._zmConn):
      sstate = ctypes.c_int32(state.value)

      pfun = _LIB.zmGetAllSchedulers
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, sstate, ctypes.byref(dbuffer))
      oid = [dbuffer[i] for i in range(osz)]
      
      pfun = _LIB.zmFreeResources
      pfun.restype = None
      pfun.argtypes = (ctypes.POINTER(ctypes.c_uint64), ctypes.c_char_p)
      pfun(dbuffer, ctypes.c_char_p(0))

      osch = []
      for i in range(osz):
        s = schedr(oid[i])
        self.getScheduler(s)
        osch.append(s)
      return osch
    return []

  #############################################################################
  ### Worker

  def addWorker(self, iowkr : worker) -> bool:
    """
    Add new worker
    :param iowkr: new worker config
    :return: True - ok
    """
    if (self._zmConn):
      wcng = _workerCng()
      wcng.schedrId = iowkr.sId
      wcng.connectPnt = iowkr.connectPnt.encode('utf-8')
      wcng.capacityTask = iowkr.capacityTask
      
      wid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddWorker
      pfun.argtypes = (ctypes.c_void_p, _workerCng, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, wcng, ctypes.byref(wid))):
        iowkr.id = wid.value
        return True
    return False
  def getWorker(self, iowkr : worker) -> bool:
    """
    Get worker config by ID
    :param iowkr: worker config
    :return: True - ok
    """
    if (self._zmConn):
      wcng = _workerCng()

      wid = ctypes.c_uint64(iowkr.id)
      
      pfun = _LIB.zmGetWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_workerCng))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, wid, ctypes.byref(wcng))):
        iowkr.sId = wcng.schedrId
        iowkr.connectPnt = wcng.connectPnt.decode('utf-8')
        iowkr.capacityTask = wcng.capacityTask
        return True
    return False
  def changeWorker(self, iwkr : worker) -> bool:
    """
    Change worker config
    :param iwkr: new worker config
    :return: True - ok
    """
    if (self._zmConn):
      wid = ctypes.c_uint64(iwkr.id)
      wcng = _workerCng()
      wcng.schedrId = iwkr.sId
      wcng.connectPnt = iwkr.connectPnt.encode('utf-8')
      wcng.capacityTask = iwkr.capacityTask
      
      pfun = _LIB.zmChangeWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _workerCng)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, wid, wcng)
    return False
  def delWorker(self, wkrId) -> bool:
    """
    Delete worker
    :param wkrId: worker id
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
    Start worker
    :param wkrId: worker id
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
    Pause worker
    :param wkrId: worker id
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
    Ping worker
    :param wkrId: worker id
    :return: True - ok
    """
    if (self._zmConn):
      wid = ctypes.c_uint64(wkrId)
            
      pfun = _LIB.zmPingWorker
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, wid)
    return False
  def workerState(self, iowkrs : [worker]) -> bool:
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
          iowkrs[i].state = stateType(stateBuffer[i])
        return True
    return False
  def getAllWorkers(self, schId : int, state : stateType) -> [worker]:
    """
    Get all workers
    :param schId: schedr id
    :param state: choose with current state. If the state is 'undefined', select all
    :return: list of worker
    """
    if (self._zmConn):
      sId = ctypes.c_uint64(schId)
      sstate = ctypes.c_int32(state.value)

      pfun = _LIB.zmGetAllWorkers
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, sId, sstate, ctypes.byref(dbuffer))
      oid = [dbuffer[i] for i in range(osz)]
      
      pfun = _LIB.zmFreeResources
      pfun.restype = None
      pfun.argtypes = (ctypes.POINTER(ctypes.c_uint64), ctypes.c_char_p)
      pfun(dbuffer, ctypes.c_char_p(0))

      owkr = []
      for i in range(osz):
        w = worker(oid[i])
        self.getWorker(w)
        owkr.append(w)
      return owkr
    return []
  
  #############################################################################
  ### Pipeline of tasks
  
  def addPipeline(self, ioppl : pipeline) -> bool:
    """
    Add new pipeline
    :param ioppl: new pipeline config
    :return: True - ok
    """
    if (self._zmConn):
      pcng = _pipelineCng()
      pcng.userId = ioppl.uId
      pcng.isShared = ioppl.isShared
      pcng.name = ioppl.name.encode('utf-8')
      pcng.description = ioppl.description.encode('utf-8')
      
      pplid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddPipeline
      pfun.argtypes = (ctypes.c_void_p, _pipelineCng, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, pcng, ctypes.byref(pplid))):
        ioppl.id = pplid.value
        return True
    return False
  def getPipeline(self, ioppl : pipeline) -> bool:
    """
    Get pipeline config by ID
    :param ioppl: pipeline config
    :return: True - ok
    """
    if (self._zmConn):
      pcng = _pipelineCng()

      pplid = ctypes.c_uint64(ioppl.id)
      
      pfun = _LIB.zmGetPipeline
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_pipelineCng))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, pplid, ctypes.byref(pcng))):      
        ioppl.uId = pcng.userId
        ioppl.isShared = pcng.isShared
        ioppl.name = pcng.name.decode('utf-8')
        ioppl.description = pcng.description.decode('utf-8')
        return True
    return False
  def changePipeline(self, ippl : user) -> bool:
    """
    Change pipeline config
    :param ippl: new pipeline config
    :return: True - ok
    """
    if (self._zmConn):
      pplid = ctypes.c_uint64(ippl.id)
      pcng = _pipelineCng()
      pcng.userId = ippl.uId
      pcng.isShared = ippl.isShared
      pcng.name = ippl.name.encode('utf-8')
      pcng.description = ippl.description.encode('utf-8')
      
      pfun = _LIB.zmChangePipeline
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _pipelineCng)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, pplid, pcng)
    return False
  def delPipeline(self, pplId : int) -> bool:
    """
    Delete pipeline
    :param pplId: pipeline id
    :return: True - ok
    """
    if (self._zmConn):
      pid = ctypes.c_uint64(pplId)
            
      pfun = _LIB.zmDelPipeline
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, pid)
    return False
  def getAllPipelines(self, userId : int) -> [pipeline]:
    """
    Get all pipelines
    :param userId: user id
    :return: list of pipeline
    """
    if (self._zmConn):
      uId = ctypes.c_uint64(userId)

      pfun = _LIB.zmGetAllPipelines
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, uId, ctypes.byref(dbuffer))
      oid = [dbuffer[i] for i in range(osz)]
      
      pfun = _LIB.zmFreeResources
      pfun.restype = None
      pfun.argtypes = (ctypes.POINTER(ctypes.c_uint64), ctypes.c_char_p)
      pfun(dbuffer, ctypes.c_char_p(0))

      oppl = []
      for i in range(osz):
        p = pipeline(oid[i])
        self.getPipeline(p)
        oppl.append(p)
      return oppl
    return []

  #############################################################################
  ### Task template 

  def addTaskTemplate(self, iott : taskTemplate) -> bool:
    """
    Add new taskTemplate
    :param iott: new tasktempl config
    :return: True - ok
    """
    if (self._zmConn):
      tcng = _taskTemplCng()
      tcng.userId = iott.uId
      tcng.averDurationSec = iott.averDurationSec
      tcng.maxDurationSec = iott.maxDurationSec
      tcng.isShared = iott.isShared
      tcng.name = iott.name.encode('utf-8')
      tcng.description = iott.description.encode('utf-8')
      tcng.script = iott.script.encode('utf-8')
      
      ttid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, _taskTemplCng, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tcng, ctypes.byref(ttid))):
        iott.id = ttid.value
        return True
    return False
  def getTaskTemplate(self, iott : taskTemplate) -> bool:
    """
    Get taskTemplate config by ID
    :param iott: taskTemplate config
    :return: True - ok
    """
    if (self._zmConn):
      tcng = _taskTemplCng()

      ttid = ctypes.c_uint64(iott.id)
      
      pfun = _LIB.zmGetTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_taskTemplCng))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, ttid, ctypes.byref(tcng))):      
        iott.uId = tcng.userId
        iott.averDurationSec = tcng.averDurationSec
        iott.maxDurationSec = tcng.maxDurationSec
        iott.isShared = tcng.isShared
        iott.name = tcng.name.decode('utf-8')
        iott.description = tcng.description.decode('utf-8')
        iott.script = tcng.script.decode('utf-8')
        return True
    return False
  def changeTaskTemplate(self, iott : taskTemplate) -> bool:
    """
    Change taskTemplate config
    :param iott: new taskTemplate config
    :return: True - ok
    """
    if (self._zmConn):
      ttid = ctypes.c_uint64(iott.id)
      tcng = _taskTemplCng()
      tcng.userId = iott.uId
      tcng.averDurationSec = iott.averDurationSec
      tcng.maxDurationSec = iott.maxDurationSec            
      tcng.isShared = iott.isShared
      tcng.name = iott.name.encode('utf-8')
      tcng.description = iott.description.encode('utf-8')
      tcng.script = iott.script.encode('utf-8')
      
      nttid = ctypes.c_uint64(0)

      pfun = _LIB.zmChangeTaskTemplate
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _taskTemplCng, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, ttid, tcng, ctypes.byref(nttid))):
        iott.id = nttid.value
        return True
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
  def getAllTaskTemplates(self, userId : int) -> [taskTemplate]:
    """
    Get all taskTemplates
    :param userId: user id
    :return: list of taskTemplate
    """
    if (self._zmConn):
      uId = ctypes.c_uint64(userId)

      pfun = _LIB.zmGetAllTaskTemplates
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, uId, ctypes.byref(dbuffer))
      oid = [dbuffer[i] for i in range(osz)]
      
      pfun = _LIB.zmFreeResources
      pfun.restype = None
      pfun.argtypes = (ctypes.POINTER(ctypes.c_uint64), ctypes.c_char_p)
      pfun(dbuffer, ctypes.c_char_p(0))

      ott = []
      for i in range(osz):
        t = taskTemplate(oid[i])
        self.getTaskTemplate(t)
        ott.append(t)
      return ott
    return []

  #############################################################################
  ### Task of pipeline
 
  def addTask(self, iot : task) -> bool:
    """
    Add new task
    :param iot: new task config
    :return: True - ok
    """
    if (self._zmConn):
      tcng = _taskCng()
      tcng.pplId = iot.pplId
      tcng.ttId = iot.ttId
      tcng.priority = iot.priority
      tcng.prevTasksId = ','.join(iot.prevTasksId).encode('utf-8')
      tcng.nextTasksId = iot.nextTasksId.encode('utf-8')
      tcng.params = iot.params.encode('utf-8')
      tcng.screenRect = iot.screenRect.encode('utf-8')
      
      tid = ctypes.c_uint64(0)
      
      pfun = _LIB.zmAddTask
      pfun.argtypes = (ctypes.c_void_p, _taskCng, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tcng, ctypes.byref(tid))):
        iot.id = tid.value
        return True
    return False
  def getTask(self, iot : task) -> bool:
    """
    Get task config by ID
    :param iot: task config
    :return: True - ok
    """
    if (self._zmConn):
      tcng = _taskCng()

      tid = ctypes.c_uint64(iot.id)
      
      pfun = _LIB.zmGetTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_taskCng))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tid, ctypes.byref(tcng))):      
        iot.pplId = tcng.pplId
        iot.ttId = tcng.ttId
        iot.priority = tcng.priority
        iot.prevTasksId = tcng.prevTasksId.decode('utf-8')
        iot.nextTasksId = tcng.nextTasksId.decode('utf-8')
        iot.params = tcng.params.decode('utf-8')
        iot.screenRect = tcng.screenRect.decode('utf-8')
        return True
    return False
  def changeTask(self, iot : task) -> bool:
    """
    Change task config
    :param iot: new task config
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(iot.id)
      tcng = _taskCng()
      tcng.pplId = iot.pplId
      tcng.ttId = iot.ttId
      tcng.priority = iot.priority
      tcng.prevTasksId = iot.prevTasksId.encode('utf-8')
      tcng.nextTasksId = iot.nextTasksId.encode('utf-8')
      tcng.params = iot.params.encode('utf-8')
      tcng.screenRect = iot.screenRect.encode('utf-8')
            
      pfun = _LIB.zmChangeTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _taskCng)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid, tcng)
    return False
  def delTask(self, tId : int) -> bool:
    """
    Delete task
    :param tId: task id
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
    Start task
    :param tId: task id
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
    Stop task
    :param tId: task id
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
    Pause task
    :param tId: task id
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
    Cancel task
    :param tId: task id
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
    Continue task
    :param tId: task id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(tId)
            
      pfun = _LIB.zmContinueTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def taskState(self, iot : [task]) -> bool:
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
      stateBuffer = (_taskState * tsz)()
      
      pfun = _LIB.zmTaskState
      pfun.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint64), ctypes.c_uint32, ctypes.POINTER(_taskState))
      pfun.restype = ctypes.c_bool

      if (pfun(self._zmConn, idBuffer, ctsz, stateBuffer)):      
        for i in range(tsz):
          iot[i].state = stateType(stateBuffer[i].state)
          iot[i].progress = stateBuffer[i].progress          
        return True
    return False
  def taskResult(self, iot : task) -> bool:
    """
    Task result
    :param iot: task config
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(iot.id)
      tresult = ctypes.c_char_p()
      
      pfun = _LIB.zmTaskResult
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.c_char_p))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tid, ctypes.byref(tresult))):      
        iot.result = tresult.value
      
        pfun = _LIB.zmFreeResources
        pfun.restype = None
        pfun.argtypes = (ctypes.POINTER(ctypes.c_uint64), ctypes.c_char_p)
        pfun(None, tresult)
        return True
    return False
  def taskTime(self, iot : task) -> bool:
    """
    Task time
    :param iot: task config
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(iot.id)
      ttime = _taskTime()
      
      pfun = _LIB.zmTaskTime
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_taskTime))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tid, ctypes.byref(ttime))):      
        iot.createTime = ttime.createTime.decode('utf-8')
        iot.takeInWorkTime = ttime.takeInWorkTime.decode('utf-8')
        iot.startTime = ttime.startTime.decode('utf-8')
        iot.stopTime = ttime.stopTime.decode('utf-8')
        return True
    return False
  def getAllTasks(self, pplId : int, state : stateType) -> [task]:
    """
    Get all tasks
    :param pplId: pipeline id
    :param state: state type
    :return: list of task
    """
    if (self._zmConn):
      cpplId = ctypes.c_uint64(pplId)
      cstate = ctypes.c_int32(state.value)

      pfun = _LIB.zmGetAllTasks
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.c_int32, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, cpplId, cstate, ctypes.byref(dbuffer))
      oid = [dbuffer[i] for i in range(osz)]
      
      pfun = _LIB.zmFreeResources
      pfun.restype = None
      pfun.argtypes = (ctypes.POINTER(ctypes.c_uint64), ctypes.c_char_p)
      pfun(dbuffer, ctypes.c_char_p(0))

      ott = []
      for i in range(osz):
        t = task(oid[i])
        self.getTask(t)
        ott.append(t)
      return ott
    return []
 
  #############################################################################
  ### Internal errors
 
  def getInternErrors(self, sId : int, wId : int, mCnt : int) -> [internError]:
    """
    Get internal errors
    :param sId: schedr id
    :param wId: worker id
    :param mCnt: max mess count
    :return: list of errors
    """
    if (self._zmConn):
      csId = ctypes.c_uint64(sId)
      cwId = ctypes.c_uint64(wId)
      cmCnt = ctypes.c_uint32(mCnt)

      pfun = _LIB.zmGetInternErrors
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.c_uint64, ctypes.c_uint32, ctypes.POINTER(_internError))
      pfun.restype = ctypes.c_uint32
      dbuffer = (_internError * mCnt)()
      osz = pfun(self._zmConn, csId, cwId, cmCnt, dbuffer)
      
      oerr = []
      for i in range(osz):
        oerr[i].sId = dbuffer[i].schedrId
        oerr[i].wId = dbuffer[i].workerId
        oerr[i].createTime = dbuffer[i].createTime
        oerr[i].message = dbuffer[i].message
      return oerr
    return []


obj = zm.ZMObj(zm.dbType.PostgreSQL, "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10")

u = user(name = "alm")
ok = obj.addUser(u)

tt = taskTemplate(uId = u.id, name = "tt", script = "script")

ppl = pipeline(uId = u.id, name = "ppl")
ok = obj.addPipeline(ppl)

allErr = obj.addTask(task(pplId=ppl.id,  ))

err = obj.getLastError()

err