import os
from . import zm_client as zm
from . import db

zmConn : zm.Connection = None
zmTaskWatch : zm.Connection = None


def init(dbConnStr : str):
  
  libname = 'libzmclient.so'
  if os.name == 'nt':
    libname = 'zmclient.dll'
      
  zm.loadLib(libname)
  
  global zmConn
  zmConn = zm.Connection(dbConnStr)

  if not zmConn.isOK():
    raise RuntimeError('Error connection with PostgreSQL: ' + zmConn.getLastError())
  
  zmConn.setErrorCBack(lambda err: print(err))  
  
  global zmTaskWatch 
  zmTaskWatch = zm.Connection(dbConnStr) 

  zmTaskWatch.setErrorCBack(lambda err: print(err))  
      
  zmTaskWatch.setChangeTaskStateCBack(taskChangeCBack)

def taskChangeCBack(tId : int, progress : int, prevState: int, newState: int):
  dbo = None
  try:
    zmt = zm.Task(tId)
    zmt.state = newState
    zmt.progress = progress
    zmTaskWatch.taskTime(zmt)

    tstate = zm.StateType(newState)
    if (tstate == zm.StateType.COMPLETED) or \
       (tstate == zm.StateType.ERROR) or \
       (tstate == zm.StateType.STOP):
      zmTaskWatch.taskResult(zmt)
        
    dbo = db.createDb(usr.name)

    from . import task
    task.changeState(dbo, zmt)

    t = task.get(dbo, tId)

    from . import pipeline_task as pt
    pt.setChange(dbo, t.pplTaskId, True)    

    if tstate == zm.StateType.COMPLETED: 

      nextTasks = pt.getNextTasks(dbo, t.pplTaskId)      
      for nextTaskId, isStartNext in nextTasks:
        if not isStartNext:
          continue

        nextTask = pt.get(dbo, nextTaskId)
        if nextTask.isEnabled:
          newTask = task.Task(nextTaskId, starterPplTaskId=t.pplTaskId, ttlId=nextTask.ttId)
          newTask.params = nextTask.params
          task.start(dbo, newTask)    
    
  except Exception as err:
    print("taskChangeCBack failed: {0}".format(str(err)))
  finally:
    if dbo:
      db.closeDb(dbo)

