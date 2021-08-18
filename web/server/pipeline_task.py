from typing import List
from contextlib import closing
from flask import g


class PipelineTask: 
  """PipelineTask config""" 
  def __init__(self,
               id : int = 0,
               pplId : int = 0,
               ttId : int = 0,
               nextTasksId : List[int] = 0,
               nextEventsId : List[int] = 0,
               params : List[str] = [],
               name = "",
               description = ""):
    self.id = id
    self.pplId = pplId                 # Pipeline id    
    self.nextTasksId = nextTasksId     # Next pipeline tasks id
    self.nextEventsId = nextEventsId   # Next events id
    self.ttId = ttId                   # TaskTemplate id
    self.params = params               # Task params
    self.name = name    
    self.description = description
  def __repr__(self):
      return f"PipelineTask: id {self.id} pplId {self.pplId} ttId {self.ttId} params {self.params} nextTasksId {self.nextTasksId} nextEventsId {self.nextEventsId} name {self.name} description {self.description}"
  def __str__(self):
    return self.__repr__()

def add(pt : PipelineTask) -> bool:
  if 'db' in g:
    try:
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblPipelineTask (pplId, ttId, params, nextTasksId, nextEventsId, name, description) VALUES("
          f"'{pt.pplId}',"
          f"'{pt.ttId}',"
          f"'{pt.params}',"
          f"'{pt.nextTasksId}',"
          f"'{pt.nextEventsId}',"
          f"'{pt.name}',"
          f"'{pt.description}');"
        )
        pt.id = cr.lastrowid
        g.db.commit()
      return True
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return False

def change(pt : PipelineTask) -> bool:
  if 'db' in g:
    try:
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblPipelineTask SET "
          f"pplId = '{pt.pplId}',"
          f"ttId = '{pt.ttId}',"
          f"params = '{pt.params}',"
          f"nextTasksId = '{pt.nextTasksId}',"
          f"nextEventsId = '{pt.nextEventsId}',"
          f"name = '{pt.name}',"
          f"description = '{pt.description}' "
          f"WHERE id = {pt.id};"  
        )
        g.db.commit()
      return True
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return False

def delete(ptId) -> bool:
  if 'db' in g:    
    try:
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblPipelineTask SET "
          "isDelete = 1 "
          f"WHERE id = {ptId};" 
        )
        g.db.commit()
      return True
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return False

def all() -> List[PipelineTask]:
  if 'db' in g:
    try:
      pts = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, pplId, ttId, params, nextTasksId, nextEventsId, name, description "
          "FROM tblPipelineTask "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          pts.append(PipelineTask(id=row[0], pplId=row[1], ttId=row[2], params=row[3],
                                   nextTasksId=row[4], nextEventsId=row[5], name=row[6], description=row[7]))       
      return pts  
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return []