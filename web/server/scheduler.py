from typing import List
from flask import g

from .core import zmConn
from . import zm_client as zm 

class Scheduler(zm.Schedr):
  None

def add(shedr : Scheduler) -> bool:
  if zmConn and g.userId:
    
    shedr.name = shedr.name.replace("'", "''")
    shedr.description = shedr.description.replace("'", "''")
    
    return zmConn.addScheduler(shedr)
  return False

def change(shedr : Scheduler) -> bool:
  if zmConn and g.userId:
    shedr.name = shedr.name.replace("'", "''")
    shedr.description = shedr.description.replace("'", "''")
    
    return zmConn.changeScheduler(shedr)
  return False

def delete(shedrId : int) -> bool:
  if zmConn and g.userId:
    
    return zmConn.deleteScheduler(shedrId)
  return False

def all() -> List[Scheduler]:
  if zmConn and g.userId:
    
    return zmConn.getAllSchedulers()
  return []