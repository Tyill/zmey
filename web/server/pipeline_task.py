from typing import List

class PipelineTask: 
  """PipelineTask config""" 
  def __init__(self,
               id : int = 0,
               pplId : int = 0,
               ttId : int = 0,
               nextTasksId : List[int] = 0,
               params : List[str] = [],
               name = "",
               description = ""):
    self.id = id
    self.pplId = pplId                 # Pipeline id    
    self.nextTasksId = nextTasksId     # Next pipeline tasks id
    self.ttId = ttId                   # TaskTemplate id
    self.params = params               # Task params
    self.name = name    
    self.description = description
 
def add(self, iot : PipelineTask) -> bool:
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

def get(self, iot : PipelineTask) -> bool:
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

def change(self, iot : PipelineTask) -> bool:
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

def delele(self, ptId : int) -> bool:
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

def getAll(self, pplId : int) -> List[PipelineTask]:
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
