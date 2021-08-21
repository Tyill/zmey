from typing import List
from contextlib import closing
from flask import g

from .core import zmConn
from . import zm_client as zm 

class TaskTemplate(zm.TaskTemplate):
  None

def add(iott : TaskTemplate) -> bool:
  if zmConn and g.userId and ('db' in g):
    iott.userId = g.userId
    if zmConn.addTaskTemplate(iott):
      try:
        with closing(g.db.cursor()) as cr:
          cr.execute(
            "INSERT INTO tblTaskTemplate (id, name, description, script, averDurationSec, maxDurationSec) VALUES("
            f'"{iott.id}",'
            f'"{iott.name}",'
            f'"{iott.description}",'
            f'"{iott.script}",'
            f'"{iott.averDurationSec}",'
            f'"{iott.maxDurationSec}");'
          )
          g.db.commit()
        return True
      except Exception as err:
        print('Local db query failed: %s' % str(err))
  return False

def change(iott : TaskTemplate) -> bool:
  if zmConn and g.userId and ('db' in g):
    iott.userId = g.userId
    if zmConn.changeTaskTemplate(iott):
      try:
        with closing(g.db.cursor()) as cr:
          cr.execute(
            "UPDATE tblTaskTemplate SET "
            f'name = "{iott.name}",'
            f'description = "{iott.description}",'
            f'script = "{iott.script}",'
            f'averDurationSec = "{iott.averDurationSec}",'
            f'maxDurationSec = "{iott.maxDurationSec}" '
            f'WHERE id = {iott.id};'  
          )
          g.db.commit()
        return True
      except Exception as err:
        print('Local db query failed: %s' % str(err))
  return False

def delete(ttId : int) -> bool:
  if zmConn and ('db' in g):
    if zmConn.delTaskTemplate(ttId):
      try:
        with closing(g.db.cursor()) as cr:
          cr.execute(
            "UPDATE tblTaskTemplate SET "
            "isDelete = 1 "
            f"WHERE id = {ttId};" 
          )
          g.db.commit()
        return True
      except Exception as err:
        print('Local db query failed: %s' % str(err))
  return False

def all() -> List[TaskTemplate]:
  if 'db' in g:
    try:
      ttls = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, name, description, script, averDurationSec, maxDurationSec "
          "FROM tblTaskTemplate "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          ttls.append(TaskTemplate(id=row[0], name=row[1], description=row[2], script=row[3], averDurationSec=row[4], maxDurationSec=row[5]))       
      return ttls  
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return []