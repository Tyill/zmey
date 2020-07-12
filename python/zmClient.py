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
  undefined     = -1
  ready         = 0
  start         = 1
  running       = 2
  pause         = 3
  stop          = 4    
  completed     = 5
  error         = 6
  notResponding = 7
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
    self.isShared = isShared
    self.name = name    
    self.description = description 

def _c_str(string : str) -> ctypes.c_char_p:
    """Create ctypes char * from a Python string."""
    if sys.version_info[0] > 2:
      py_str = lambda x: x.encode('utf-8')
    else:
      py_str = lambda x: x
    return ctypes.c_char_p(py_str(string)) 
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
_userCng_p = lambda x : ctypes.cast(x, ctypes.POINTER(_userCng))
_schedrCng_p = lambda x : ctypes.cast(x, ctypes.POINTER(_schedrCng))  
_workerCng_p = lambda x : ctypes.cast(x, ctypes.POINTER(_workerCng))  
_pipelineCng_p = lambda x : ctypes.cast(x, ctypes.POINTER(_pipelineCng))  
_uint64_p = lambda x : ctypes.cast(x, ctypes.POINTER(ctypes.c_uint64))
_int32_p = lambda x : ctypes.cast(x, ctypes.POINTER(ctypes.c_int32))
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

  #####################################################################
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
      if (pfun(self._zmConn, ucng, _uint64_p(ctypes.addressof(uid)))):
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
      if (pfun(self._zmConn, ucng, _uint64_p(ctypes.addressof(uid)))):
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
      if (pfun(self._zmConn, uid, _userCng_p(ctypes.addressof(ucng)))):      
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

  #####################################################################
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
      if (pfun(self._zmConn, scng, _uint64_p(ctypes.addressof(sid)))):
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
      if (pfun(self._zmConn, sid, _schedrCng_p(ctypes.addressof(scng)))):      
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
      if (pfun(self._zmConn, sid, _int32_p(ctypes.addressof(sstate)))):      
        iosch.state = stateType(sstate.value) 
        return True
    return False
  def getAllSchedulers(self, state : stateType) -> [schedr]:
    """
    Get all schedrs
    :param state: choose with current state. If the state is 'undefined', select all
    :return: list of schedr id
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

  #####################################################################
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
      if (pfun(self._zmConn, wcng, _uint64_p(ctypes.addressof(wid)))):
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
      if (pfun(self._zmConn, wid, _workerCng_p(ctypes.addressof(wcng)))):
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
  def getAllWorkers(self, schId : int, state : stateType) -> [schedr]:
    """
    Get all workers
    :param schId: schedr id
    :param state: choose with current state. If the state is 'undefined', select all
    :return: list of worker id
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
  
  # ///////////////////////////////////////////////////////////////////////////////
  # /// Pipeline of tasks
  
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
      if (pfun(self._zmConn, pcng, _uint64_p(ctypes.addressof(pplid)))):
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
      if (pfun(self._zmConn, pplid, _pipelineCng_p(ctypes.addressof(pcng)))):      
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
    :return: list of pipeline id
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


obj = ZMObj(dbType.PostgreSQL, "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10")

allUsr = obj.getAllUsers()

ok = obj.getAllPipelines(allUsr[0].id)

err = obj.getLastError()

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
