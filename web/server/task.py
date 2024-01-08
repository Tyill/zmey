from typing import List
from contextlib import closing
from flask import g
import psycopg2

from .core import zmConn, zmTaskWatch
from . import zm_client as zm
from . import pipeline_task as pt  

class Task(zm.Task):
  """Task config""" 
  def __init__(self,
               pplTaskId :  int = 0,
               starterPplTaskId :  int = None,
               starterEventId :  int = None,
               ttlId :  int = 0):
    zm.Task.__init__(self, ttlId=ttlId)
    self.pplTaskId = pplTaskId
    self.starterPplTaskId = starterPplTaskId
    self.starterEventId = starterEventId
  def __repr__(self):
      return f"Task: id {self.id} pplTaskId {self.pplTaskId} starterPplTaskId {self.starterPplTaskId} starterEventId {self.starterEventId} ttlId {self.ttlId} "
  def __str__(self):
    return self.__repr__()

def start(db, iot : Task) -> bool:
  params = iot.params
  iot.params = iot.params.replace("'", "''")
  if zmConn and zmConn.startTask(iot):  
    try:
      with psycopg2.connect(dbname='zmeydb', user='postgres', password='postgres', host='localhost') as pg:
        csr = pg.cursor()
        csr.execute(
          "INSERT INTO tblTask (id, state, pplTaskId, starterPplTaskId, starterEventId, ttlId, scriptPath, params) VALUES("
          f"'{iot.id}',"
          f"'{zm.StateType.READY.value}',"
          f"'{iot.pplTaskId}',"
          f"'{iot.starterPplTaskId}',"
          f"'{iot.starterEventId}',"
          f"'{iot.ttlId}');"
        )
        csr.close()
        iot.params = params
      
      zmTaskWatch.addTaskForTracking(iot.id)

      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("Task.start", str(err)))
  return False

def continueTask(id : int) -> bool:
  return True if zmConn and zmConn.continueTask(id) else False

def pause(id : int) -> bool:
  return True if zmConn and zmConn.pauseTask(id) else False

def stop(id : int) -> bool:
  return True if zmConn and zmConn.stopTask(id) else False

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
    result = t.result.replace("'", "''")
    with closing(db.cursor()) as cr:
      cr.execute(
        "UPDATE tblTask SET "
        f"state = '{t.state}',"
        f"progress = '{t.progress}',"
        f"result = '{result}',"
        f"createTime = '{t.createTime}',"
        f"takeInWorkTime = '{t.takeInWorkTime}',"
        f"startTime = '{t.startTime}',"
        f"stopTime = '{t.stopTime}' "
        f"WHERE id = {t.id};"  
      )
      db.commit()
    return True
  except Exception as err:
    print("{0} local db query failed: {1}".format("Task.changeState", str(err)))
  return False

def getState(pplTaskId : int, ifChange : bool) -> List[Task]:
  if ('db' in g):    
    try:
      if ifChange and not pt.hasChange(g.db, pplTaskId):
        return []
      
      pt.setChange(g.db, pplTaskId, False)

      ret = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, starterPplTaskId, starterEventId, state, progress, startTime, stopTime, result "
          "FROM tblTask "
          f"WHERE pplTaskId = {pplTaskId} ORDER BY id DESC LIMIT 1000;"
        )
        rows = cr.fetchall()
        for row in rows:
          task = Task(pplTaskId=pplTaskId)
          task.id = row[0]
          task.starterPplTaskId = row[1]
          task.starterEventId = row[2]
          task.state = row[3]
          task.progress = row[4]
          task.startTime = row[5]
          task.stopTime = row[6]
          task.result = row[7]
          ret.append(task) 
      return ret
    except Exception as err:
      print("{0} local db query failed: {1}".format("Task.getState", str(err)))
  return []
