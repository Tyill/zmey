from typing import List
from contextlib import closing
from flask import g

from .core import zmConn
from . import zm_client as zm 

class TaskTemplate():
  def __init__(self,
               pplTaskId :  int = 0,
               starterPplTaskId :  int = None,
               starterEventId :  int = None):    
    self.pplTaskId = pplTaskId
    self.starterPplTaskId = starterPplTaskId
    self.starterEventId = starterEventId
  def __repr__(self):
      return f"TaskTemplate: id {self.id} pplTaskId {self.pplTaskId} starterPplTaskId {self.starterPplTaskId} starterEventId {self.starterEventId} ttlId {self.ttlId} "
  def __str__(self):
    return self.__repr__()

def add(iott : TaskTemplate) -> bool:
  if zmConn and g.userId and ('db' in g):
    iott.userId = g.userId
    name = iott.name
    iott.name = iott.name.replace("'", "''")
    description = iott.description
    iott.description = iott.description.replace("'", "''")
    script = iott.script
    iott.script = iott.script.replace("'", "''")
    if zmConn.addTaskTemplate(iott):
      try:
        with closing(g.db.cursor()) as cr:
          cr.execute(
            "INSERT INTO tblTaskTemplate (id, name, description, script, averDurationSec, maxDurationSec) VALUES("
            f"'{iott.id}',"
            f"'{iott.name}',"
            f"'{iott.description}',"
            f"'{iott.script}',"
            f"'{iott.averDurationSec}',"
            f"'{iott.maxDurationSec}');"
          )
          g.db.commit()
          iott.name = name
          iott.description = description
          iott.script = script
        return True
      except Exception as err:
        print("{0} local db query failed: {1}".format("TaskTemplate.add", str(err)))
  return False

def change(iott : TaskTemplate) -> bool:
  if zmConn and g.userId and ('db' in g):
    iott.userId = g.userId
    name = iott.name
    iott.name = iott.name.replace("'", "''")
    description = iott.description
    iott.description = iott.description.replace("'", "''")
    script = iott.script
    iott.script = iott.script.replace("'", "''")
    if zmConn.changeTaskTemplate(iott):
      try:
        with closing(g.db.cursor()) as cr:
          cr.execute(
            "UPDATE tblTaskTemplate SET "
            f"name = '{iott.name}',"
            f"description = '{iott.description}',"
            f"script = '{iott.script}',"
            f"averDurationSec = '{iott.averDurationSec}',"
            f"maxDurationSec = '{iott.maxDurationSec}' "
            f"WHERE id = {iott.id};"
          )
          g.db.commit()
          iott.name = name
          iott.description = description
          iott.script = script
        return True
      except Exception as err:
        print("{0} local db query failed: {1}".format("TaskTemplate.change", str(err)))
  return False

def delete(ttId : int) -> bool:
  if zmConn and ('db' in g):
    if zmConn.delTaskTemplate(ttId):
      try:
        with closing(g.db.cursor()) as cr:
          cr.execute(
            "UPDATE tblTaskTemplate SET "
            "isDeleted = TRUE "
            f"WHERE id = {ttId};" 
          )
          g.db.commit()
        return True
      except Exception as err:
        print("{0} local db query failed: {1}".format("TaskTemplate.delete", str(err)))
  return False

def all() -> List[TaskTemplate]:
  if 'db' in g:
    try:
      ttls = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, name, description, scriptPath, averDurationSec, maxDurationSec "
          "FROM tblTaskTemplate "
          "WHERE isDeleted = FALSE;"
        )
        rows = cr.fetchall()
        for row in rows:
          ttls.append(TaskTemplate(id=row[0], name=row[1], description=row[2], script=row[3], averDurationSec=row[4], maxDurationSec=row[5]))       
      return ttls  
    except Exception as err:
      print("{0} local db query failed: {1}".format("TaskTemplate.all", str(err)))
  return []