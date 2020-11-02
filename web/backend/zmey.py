import zmClient as zm 

_zmo = 0

def initApp(connStr : str):
  _zmo = zm.ZMObj(connStr)

def hChangeTask(tId : int, prevState : zm.stateType, newState : zm.stateType):
  pass

def startTask(tId : int):
  _zmo.
  _zmo.setChangeTaskStateCBack(tId, hChangeTask)

