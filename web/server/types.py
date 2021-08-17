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

class Pipeline: 
  """Pipeline config""" 
  def __init__(self,
               id : int = 0,
               uId : int = 0,
               name : str = "",
               description : str = ""):
    self.id = id
    self.userId = uId                # User id
    self.name = name    
    self.description = description 

class TaskParam:
  """Task Parameter""" 
  def __init__(self, 
              enable : bool = True,
              value : str = ""):
    self.enable = enable
    self.value = value

class PipelineTask: 
  """PipelineTask config""" 
  def __init__(self,
               id : int = 0,
               pplId : int = 0,
               ttId : int = 0,
               nextTasksId : List[int] = 0,
               params : List[TaskParam] = [],
               name = "",
               description = ""):
    self.id = id
    self.pplId = pplId                 # Pipeline id    
    self.nextTasksId = nextTasksId     # Next pipeline tasks id
    self.ttId = ttId                   # TaskTemplate id
    self.params = params               # Task params
    self.name = name    
    self.description = description


#############################################################################
  ### User
  
def addUser(self, iousr : User) -> bool:
  """
  Add new User
  :param iousr: new User config
  :return: True - ok
  """
  if (self._zmConn):
    ucng = _UserCng_C()
    ucng.name = iousr.name.encode('utf-8')
    ucng.passw = iousr.passw.encode('utf-8')
    ucng.description = iousr.description.encode('utf-8')

    uid = ctypes.c_uint64(0)
    
    pfun = _lib.zmAddUser
    pfun.argtypes = (ctypes.c_void_p, _UserCng_C, ctypes.POINTER(ctypes.c_uint64))
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
    ucng = _UserCng_C()
    ucng.name = iousr.name.encode('utf-8')
    ucng.passw = iousr.passw.encode('utf-8')
    ucng.description = iousr.description.encode('utf-8')

    uid = ctypes.c_uint64(0)
    
    pfun = _lib.zmGetUserId
    pfun.argtypes = (ctypes.c_void_p, _UserCng_C, ctypes.POINTER(ctypes.c_uint64))
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
    ucng = _UserCng_C()

    uid = ctypes.c_uint64(iousr.id)
    
    pfun = _lib.zmGetUserCng
    pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_UserCng_C))
    pfun.restype = ctypes.c_bool
    if (pfun(self._zmConn, uid, ctypes.byref(ucng))):      
      iousr.name = ucng.name.decode('utf-8')
      iousr.passw = ucng.passw.decode('utf-8')
      if ucng.description:
        iousr.description = ucng.description.decode('utf-8')
        self._freeResources()      
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
    ucng = _UserCng_C()
    ucng.name = iusr.name.encode('utf-8')
    ucng.passw = iusr.passw.encode('utf-8')
    ucng.description = iusr.description.encode('utf-8')
    
    pfun = _lib.zmChangeUser
    pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _UserCng_C)
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
          
    pfun = _lib.zmDelUser
    pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
    pfun.restype = ctypes.c_bool
    return pfun(self._zmConn, uid)
  return False
def getAllUsers(self) -> List[User]:
  """
  Get all users
  :return: list of users
  """
  if (self._zmConn):
    pfun = _lib.zmGetAllUsers
    pfun.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
    pfun.restype = ctypes.c_uint32
    dbuffer = ctypes.POINTER(ctypes.c_uint64)()
    osz = pfun(self._zmConn, ctypes.byref(dbuffer))
                
    if dbuffer and (osz > 0):
      oid = [dbuffer[i] for i in range(osz)]
      self._freeResources()

      ousr = []
      for i in range(osz):
        u = User(oid[i])
        self.getUserCng(u)
        ousr.append(u)
      return ousr
  return []


#############################################################################
  ### Task of Pipeline
 
  def addPipelineTask(self, iot : PipelineTask) -> bool:
    """
    Add new Task
    :param iot: new Task config
    :return: True - ok
    """
    if (self._zmConn):
      tcng = _PipelineTaskCng_C()
      tcng.pplId = iot.pplId
      tcng.ttId = iot.ttId
      tcng.gId = 0 # not used yet
      tcng.name = iot.name.encode('utf-8')
      tcng.description = iot.description.encode('utf-8')

      tid = ctypes.c_uint64(0)
      
      pfun = _lib.zmAddPipelineTask
      pfun.argtypes = (ctypes.c_void_p, _PipelineTaskCng_C, ctypes.POINTER(ctypes.c_uint64))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tcng, ctypes.byref(tid))):
        iot.id = tid.value
        return True
    return False
  def getPipelineTask(self, iot : PipelineTask) -> bool:
    """
    Get pipeline task config by ID
    :param iot: Task config
    :return: True - ok
    """
    if (self._zmConn):
      tcng = _PipelineTaskCng_C()

      tid = ctypes.c_uint64(iot.id)
      
      pfun = _lib.zmGetPipelineTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(_PipelineTaskCng_C))
      pfun.restype = ctypes.c_bool
      if (pfun(self._zmConn, tid, ctypes.byref(tcng))):      
        iot.pplId = tcng.pplId
        iot.ttId = tcng.ttId
        iot.name = tcng.name.decode('utf-8')
        if tcng.description:
          iot.description = tcng.description.decode('utf-8')          
          self._freeResources()  
        return True
    return False
  def changePipelineTask(self, iot : PipelineTask) -> bool:
    """
    Change Task config
    :param iot: new Task config
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(iot.id)
      tcng = _PipelineTaskCng_C()
      tcng.pplId = iot.pplId
      tcng.ttId = iot.ttId
      tcng.gId = 0 # not used yet
      tcng.name = iot.name.encode('utf-8')
      tcng.description = iot.description.encode('utf-8')
      
      pfun = _lib.zmChangePipelineTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, _PipelineTaskCng_C)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid, tcng)
    return False
  def delPipelineTask(self, ptId : int) -> bool:
    """
    Delete pipeline task
    :param tId: Task id
    :return: True - ok
    """
    if (self._zmConn):
      tid = ctypes.c_uint64(ptId)
            
      pfun = _lib.zmDelPipelineTask
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64)
      pfun.restype = ctypes.c_bool
      return pfun(self._zmConn, tid)
    return False
  def getAllPipelineTasks(self, pplId : int) -> List[PipelineTask]:
    """
    Get all tasks of pipeline
    :param pplId: Pipeline id
    :param state: state type
    :return: list of Task
    """
    if (self._zmConn):
      cpplId = ctypes.c_uint64(pplId)

      pfun = _lib.zmGetAllPipelineTasks
      pfun.argtypes = (ctypes.c_void_p, ctypes.c_uint64, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint64)))
      pfun.restype = ctypes.c_uint32
      dbuffer = ctypes.POINTER(ctypes.c_uint64)()
      osz = pfun(self._zmConn, cpplId, ctypes.byref(dbuffer))
            
      if dbuffer and (osz > 0):
        oid = [dbuffer[i] for i in range(osz)]
        self._freeResources()
      
        ott = []
        for i in range(osz):
          t = PipelineTask(oid[i])
          self.getPipelineTask(t)
          ott.append(t)
        return ott
    return []
  