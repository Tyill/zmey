import os
from . import zm_client as zm
from . import user
from . import db

zmConn : zm.Connection = None
zmTaskWatch : zm.Connection = None


def init(dbConnStr : str):

  libname = 'libzmClient.so'
  if os.name == 'nt':
    libname = 'zmClient.dll'
      
  zm.loadLib(libname)
  
  global zmConn
  zmConn = zm.Connection(dbConnStr)

  if not zmConn.isOK():
    raise RuntimeError('Error connection with PostgreSQL: ' + zmConn.getLastError())
  
  zmConn.setErrorCBack(lambda err: print(err))  
  
  zmConn.createTables()
   
  global zmTaskWatch 
  zmTaskWatch = zm.Connection(dbConnStr) 

  zmTaskWatch.setErrorCBack(lambda err: print(err))  
      
  zmTaskWatch.setChangeTaskStateCBack(taskChangeCBack)

def taskChangeCBack(tId : int, uId : int, prevState: int, newState: int):
  try:
    zmt = zm.Task(tId)
    zmt.state = newState
    zmTaskWatch.taskTime(zmt)

    tstate = zm.StateType(newState)
    if (tstate == zm.StateType.COMPLETED) or \
       (tstate == zm.StateType.ERROR) or \
       (tstate == zm.StateType.STOP):
      zmTaskWatch.taskResult(zmt)
    
    usr = user.getById(uId)
    
    dbo = db.createDb(usr.name)

    from . import task
    task.changeState(dbo, zmt)

    if tstate == zm.StateType.COMPLETED:    
      from . import pipeline_task as pt
      t = task.get(dbo, tId)
      pplNextTasks = pt.getNextTasks(dbo, t.pplTaskId)

      for pplTId in pplNextTasks.nextTasksId:
        pplTask = pt.get(dbo, pplTId)
        if pplTask.isEnabled:
          newTask = task.Task(pplTId, t.pplTaskId, pplTask.ttId)
          newTask.params = pplTask.params
          task.start(dbo, uId, newTask)
      for evtId in pplNextTasks.nextEventsId:
        None

    db.closeDb(dbo)
  except Exception as err:
    print("taskChangeCBack failed: {0}".format(str(err)))
  
  