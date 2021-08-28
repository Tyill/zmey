from typing import List
from contextlib import closing
from flask import g

from .core import zmConn, zmTaskWatch
from . import zm_client as zm 

class Task(zm.Task):
  """Task config""" 
  def __init__(self,
               pplTaskId :  int = 0,
               ttlId :  int = 0):
    zm.Task.__init__(self, ttlId=ttlId)
    self.pplTaskId = pplTaskId
  def __repr__(self):
      return f"Task: id {self.id} pplTaskId {self.pplTaskId} ttlId {self.ttlId}"
  def __str__(self):
    return self.__repr__()

def start(iot : Task) -> bool:
  if zmConn and g.userId and ('db' in g):
    if zmConn.startTask(iot):
      try:
        with closing(g.db.cursor()) as cr:
          cr.execute(
            "INSERT INTO tblTask (id, pplTaskId, ttlId, script, params) VALUES("
            f'"{iot.id}",'
            f'"{iot.pplTaskId}",'
            f'"{iot.ttlId}",'
            f'(SELECT script FROM tblTaskTemplate WHERE id = {iot.ttlId}),'
            f'(SELECT params FROM tblPipelineTask WHERE id = {iot.pplTaskId}));'
          )
          g.db.commit()
        
        zmTaskWatch.addTaskForTracking(iot.id, g.userId)

        return True
      except Exception as err:
        print("{0} local db query failed: {1}".format("Task.start", str(err)))
  return False

def changeState(db, t : Task) -> bool:
  try:
    with closing(db.cursor()) as cr:
      cr.execute(
        "UPDATE tblTask SET "
        f"result = '{t.result}',"
        f'createTime = "{t.createTime}",'
        f'takeInWorkTime = "{t.takeInWorkTime}",'
        f'startTime = "{t.startTime}",'
        f'stopTime = "{t.stopTime}" '
        f'WHERE id = {t.id};'  
      )
      db.commit()
    return True
  except Exception as err:
    print("{0} local db query failed: {1}".format("Task.changeState", str(err)))
  return False
