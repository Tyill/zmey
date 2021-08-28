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
    
  zmTaskWatch.setChangeTaskStateCBack(taskChangeCBack)

def taskChangeCBack(tId : int, uId: int, prevState: int, newState: int):
  t = zm.Task(tId)
  zmTaskWatch.taskTime(t)

  tstate = zm.StateType(newState)
  if (tstate == zm.StateType.COMPLETED) or \
     (tstate == zm.StateType.ERROR) or \
     (tstate == zm.StateType.STOP):
    zmTaskWatch.taskResult(t)
  
  usr = user.getById(uId)
  
  dbo = db.createDb(usr.name)
   
  from . import task
  if dbo:
    task.changeState(dbo, t)
    db.closeDb(dbo)
  else:
    print('taskChangeCBack: not found user db: %s' % usr.name)
  
  