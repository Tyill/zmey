from typing import List
from flask import g

from .core import zmConn
from . import zm_client as zm 

class Scheduler(zm.Scheduler):
  None

def add(shedr : Scheduler) -> bool:
  if zmConn and ('userId' in g):
    
    shedr.name = shedr.name.replace("'", "''")
    shedr.description = shedr.description.replace("'", "''")
    
    ok = zmConn.addScheduler(shedr)
    print(shedr)
    return ok
  return False

def change(shedr : Scheduler) -> bool:
  if zmConn and ('userId' in g):
    shedr.name = shedr.name.replace("'", "''")
    shedr.description = shedr.description.replace("'", "''")
    
    return zmConn.changeScheduler(shedr)
  return False

def delete(shedrId : int) -> bool:
  if zmConn and ('userId' in g):
    
    return zmConn.deleteScheduler(shedrId)
  return False

def all() -> List[Scheduler]:
  if zmConn and ('userId' in g):
    
    scheds = zmConn.getAllSchedulers()
    for s in scheds:
      zmConn.schedulerState(s)
      s.state = zm.stateStr(s.state)
    return scheds
  return []