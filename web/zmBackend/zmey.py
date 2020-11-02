import zmBackend.zmClient as zm
from zmBackend.zmClient import User

_zmCommon = None
_zmTaskWatch = None

def initApp(connStr : str):
  global _zmCommon
  _zmCommon = zm.Connection(connStr)
  global _zmTaskWatch 
  _zmTaskWatch = zm.Connection(connStr)  
  _zmCommon.createTables()
  
def lastError() -> str:
  _zmCommon.getLastError()

def isConnection() -> bool:
  return _zmCommon.isOK()


def getUser(uname : str, passw : str) -> User:  
  usr = User(0, uname, passw)
  return usr if _zmCommon.getUserId(usr) else None

def addUser(usr : User) -> bool:
  return _zmCommon.addUser(usr)

def delUser(usr : User) -> bool:
  return _zmCommon.delUser(usr.id)

def allUsers() -> [User]:
  return _zmCommon.getAllUsers()

def hChangeTask(tId : int, prevState : zm.StateType, newState : zm.StateType):
  pass

def startTask(tId : int):
  _zmCommon.startTask(tId)
  _zmTaskWatch.setChangeTaskStateCBack(tId, hChangeTask)