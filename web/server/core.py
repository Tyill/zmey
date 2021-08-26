import os
from . import zm_client as zm 

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
  print(tId, uId, prevState, newState)