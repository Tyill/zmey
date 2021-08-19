from typing import List
from contextlib import closing
from flask import g


class PipelineTask: 
  """PipelineTask config""" 
  def __init__(self,
               id : int = 0,
               pplId : int = 0,
               ttId : int = 0,
               isEnabled : int = 1,
               isVisible : int = 0,
               positionX : int = 0,
               positionY : int = 0,
               nextTasksId : List[int] = 0,
               nextEventsId : List[int] = 0,
               params : str = [],
               name = "",
               description = ""):
    self.id = id
    self.pplId = pplId                 # Pipeline id
    self.isEnabled = isEnabled
    self.isVisible = isVisible
    self.positionX = positionX
    self.positionY = positionY
    self.ttId = ttId                   # TaskTemplate id
    self.nextTasksId = nextTasksId     # Next pipeline tasks id
    self.nextEventsId = nextEventsId   # Next events id
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

      nextTasksId = ','.join(pt.nextTasksId)
      nextEventsId = ','.join(pt.nextEventsId)

      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblPipelineTask (pplId, ttId, params, isEnabled, isVisible, positionX, positionY,"
          "nextTasksId, nextEventsId, name, description) VALUES("
          f"'{pt.pplId}',"
          f"'{pt.ttId}',"
          f"'{pt.params}',"
          f"'{pt.isEnabled}',"
          f"'{pt.isVisible}',"
          f"'{pt.positionX}',"
          f"'{pt.positionY}',"
          f"'{nextTasksId}',"
          f"'{nextEventsId}',"
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
      nextTasksId = ','.join(pt.nextTasksId)
      nextEventsId = ','.join(pt.nextEventsId)
      
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblPipelineTask SET "
          f"pplId = '{pt.pplId}',"
          f"ttId = '{pt.ttId}',"
          f"isEnabled = '{pt.isEnabled}',"
          f"isVisible = '{pt.isVisible}',"
          f"positionX = '{pt.positionX}',"
          f"positionY = '{pt.positionY}',"
          f"params = '{pt.params}',"
          f"nextTasksId = '{nextTasksId}',"
          f"nextEventsId = '{nextEventsId}',"
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
          "SELECT id, pplId, ttId, params, isEnabled, isVisible, positionX, positionY,"
          "nextTasksId, nextEventsId, name, description "
          "FROM tblPipelineTask "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          nextTasksId = row[8].split(',')
          nextEventsId = row[9].split(',')
          pts.append(PipelineTask(id=row[0], pplId=row[1], ttId=row[2], params=row[3],
                                  isEnabled=row[4], isVisible=row[5], positionX=row[6], positionY=row[7],
                                  nextTasksId=nextTasksId, nextEventsId=nextEventsId, name=row[10], description=row[11]))       
      return pts  
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return []