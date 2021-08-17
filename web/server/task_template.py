from typing import List
from .core import zmConn
from flask import g
from . db

class TaskTemplate: 
  """TaskTemplate config""" 
  def __init__(self,
               id : int = 0,
               averDurationSec : int = 1,
               maxDurationSec : int = 1,
               name : str = "",
               description : str = "",
               script: str = ""):
    self.id = id
    self.averDurationSec = averDurationSec
    self.maxDurationSec = maxDurationSec
    self.name = name    
    self.description = description
    self.script = script   

def add(self, iott : TaskTemplate) -> bool:
  if zmConn and ('userId' in g):
    iott.userId = g.userId
    return zmConn.addTaskTemplate(iott)
  return False

def get(self, iott : TaskTemplate) -> bool:
  if zmConn and ('userId' in g):
    iott.userId = g.userId
    return zmConn.getTaskTemplate(iott)
  return False

def change(self, iott : TaskTemplate) -> bool:
  if zmConn and ('userId' in g):
    iott.userId = g.userId
    return zmConn.changeTaskTemplate(iott)
  return False

def delete(self, ttId : int) -> bool:
  if zmConn and ('userId' in g):
    return zmConn.delTaskTemplate(ttId)
  return False

def getAll(self, uId : int) -> List[TaskTemplate]:
  if zmConn and ('userId' in g):
    return zmConn.getAllTaskTemplates(g.userId)
  return False