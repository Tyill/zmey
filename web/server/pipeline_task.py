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
               nextTasksId : List[int] = 0,
               nextEventsId : List[int] = 0,
               prevTasksId : List[int] = 0,
               prevEventsId : List[int] = 0,
               params = "",
               name = "",
               description = "",
               setts = "{}"):
    self.id = id
    self.pplId = pplId                 # Pipeline id
    self.isEnabled = isEnabled
    self.setts = setts
    self.ttId = ttId                   # TaskTemplate id
    self.nextTasksId = nextTasksId     # Next pipeline tasks id
    self.nextEventsId = nextEventsId   # Next events id
    self.prevTasksId = prevTasksId     # Prev pipeline tasks id
    self.prevEventsId = prevEventsId   # Prev events id
    self.params = params               # Task params
    self.name = name    
    self.description = description
  def __repr__(self):
      return f"PipelineTask: id {self.id} pplId {self.pplId} ttId {self.ttId} params {self.params} \
               isEnabled {self.isEnabled} nextTasksId {self.nextTasksId} nextEventsId {self.nextEventsId} \
               prevTasksId {self.prevTasksId} prevEventsId {self.prevEventsId} \
               name {self.name} description {self.description} setts {self.setts}"
  def __str__(self):
    return self.__repr__()

def add(pt : PipelineTask) -> bool:
  if 'db' in g:
    try:

      nextTasksId = ','.join([str(v) for v in pt.nextTasksId])
      nextEventsId = ','.join([str(v) for v in pt.nextEventsId])
      prevTasksId = ','.join([str(v) for v in pt.prevTasksId])
      prevEventsId = ','.join([str(v) for v in pt.prevEventsId])

      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblPipelineTask (pplId, ttId, params, isEnabled, setts,"
          "nextTasksId, nextEventsId, prevTasksId, prevEventsId, name, description) VALUES("
          f'"{pt.pplId}",'
          f'"{pt.ttId}",'
          f'"{pt.params}",'
          f'"{pt.isEnabled}",'
          f'"{pt.setts}",'
          f'"{nextTasksId}",'
          f'"{nextEventsId}",'
          f'"{prevTasksId}",'
          f'"{prevEventsId}",'
          f'"{pt.name}",'
          f'"{pt.description}");'
        )
        pt.id = cr.lastrowid
        g.db.commit()
      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("PipelineTask.add", str(err)))
  return False

def change(pt : PipelineTask) -> bool:
  if 'db' in g:
    try:
      nextTasksId = ','.join([str(v) for v in pt.nextTasksId])
      nextEventsId = ','.join([str(v) for v in pt.nextEventsId])
      prevTasksId = ','.join([str(v) for v in pt.prevTasksId])
      prevEventsId = ','.join([str(v) for v in pt.prevEventsId])
      print("db ",pt.params)
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblPipelineTask SET "
          f'pplId = "{pt.pplId}",'
          f'ttId = "{pt.ttId}",'
          f'isEnabled = "{pt.isEnabled}",'
          f'setts = "{pt.setts}",'
          f'params = "{pt.params}",'
          f'nextTasksId = "{nextTasksId}",'
          f'nextEventsId = "{nextEventsId}",'
          f'prevTasksId = "{prevTasksId}",'
          f'prevEventsId = "{prevEventsId}",'
          f'name = "{pt.name}",'
          f'description = "{pt.description}" '
          f'WHERE id = {pt.id};' 
        )
        g.db.commit()
      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("PipelineTask.change", str(err)))
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
      print("{0} local db query failed: {1}".format("PipelineTask.delete", str(err)))
  return False

def all() -> List[PipelineTask]:
  if 'db' in g:
    try:
      pts = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, pplId, ttId, params, isEnabled, setts,"
          "nextTasksId, nextEventsId, prevTasksId, prevEventsId, name, description "
          "FROM tblPipelineTask "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          nextTasksId = [int(v) for v in row[6].split(',') if len(v)]
          nextEventsId = [int(v) for v in row[7].split(',') if len(v)]
          prevTasksId = [int(v) for v in row[8].split(',') if len(v)]
          prevEventsId = [int(v) for v in row[9].split(',') if len(v)]
          pts.append(PipelineTask(id=row[0], pplId=row[1], ttId=row[2], params=row[3],
                                  isEnabled=row[4], setts=row[5],
                                  nextTasksId=nextTasksId, nextEventsId=nextEventsId,
                                  prevTasksId=prevTasksId, prevEventsId=prevEventsId,
                                  name=row[10], description=row[11]))       
      return pts  
    except Exception as err:
      print("{0} local db query failed: {1}".format("PipelineTask.all", str(err)))
  return []