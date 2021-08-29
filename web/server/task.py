from typing import List
from contextlib import closing
from flask import g

from .core import zmConn, zmTaskWatch
from . import zm_client as zm 

class Task(zm.Task):
  """Task config""" 
  def __init__(self,
               pplTaskId :  int = 0,
               prevPplTaskId :  int = 0,
               ttlId :  int = 0):
    zm.Task.__init__(self, ttlId=ttlId)
    self.pplTaskId = pplTaskId
    self.prevPplTaskId = prevPplTaskId
  def __repr__(self):
      return f"Task: id {self.id} pplTaskId {self.pplTaskId} prevPplTaskId {self.prevPplTaskId} ttlId {self.ttlId} "
  def __str__(self):
    return self.__repr__()

def start(db, userId, iot : Task) -> bool:
  if zmConn and zmConn.startTask(iot):
    try:
      with closing(db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblTask (id, state, pplTaskId, prevPplTaskId, ttlId, script, params) VALUES("
          f'"{iot.id}",'
          f'"{zm.StateType.READY.value}",'
          f'"{iot.pplTaskId}",'
          f'"{iot.prevPplTaskId}",'
          f'"{iot.ttlId}",'
          f'(SELECT script FROM tblTaskTemplate WHERE id = {iot.ttlId}),'
          f'"{iot.params}");'
        )
        db.commit()
      
      zmTaskWatch.addTaskForTracking(iot.id, userId)

      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("Task.start", str(err)))
  return False

def get(db, id : int) -> Task:
  try:
    task = None
    with closing(db.cursor()) as cr:
      cr.execute(
        "SELECT pplTaskId, ttlId "
        "FROM tblTask "
        f"WHERE id = {id};"      
      )
      rows = cr.fetchall()
      for row in rows:
        task = Task()
        task.id = id
        task.pplTaskId = row[0] 
        task.ttlId = row[1] 
    return task
  except Exception as err:
    print("{0} local db query failed: {1}".format("Task.get", str(err)))
  return None

def changeState(db, t : Task) -> bool:
  try:
    with closing(db.cursor()) as cr:
      cr.execute(
        "UPDATE tblTask SET "
        f"state = '{t.state}',"
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

def getState(pplTaskId : int) -> List[Task]:
  if ('db' in g):    
    try:
      ret = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, prevPplTaskId, state, startTime, stopTime, result "
          "FROM tblTask "
          f"WHERE pplTaskId = {pplTaskId} ORDER BY id DESC LIMIT 1000;"
        )
        rows = cr.fetchall()
        for row in rows:
          task = Task(pplTaskId=pplTaskId)
          task.id = row[0]
          task.prevPplTaskId = row[1]
          task.state = row[2]
          task.startTime = row[3]
          task.stopTime = row[4]
          task.result = row[5]
          ret.append(task) 
      return ret
    except Exception as err:
      print("{0} local db query failed: {1}".format("Task.getState", str(err)))
  return []
