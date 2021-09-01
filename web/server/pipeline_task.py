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
               prevTasksId : List[int] = 0,
               isStartNext : List[int] = 0,
               isSendResultToNext : List[int] = 0,
               params : str = "",
               name : str = "",
               description : str = "",
               setts : str = "{}"):
    self.id = id
    self.pplId = pplId                 # Pipeline id
    self.isEnabled = isEnabled
    self.setts = setts
    self.ttId = ttId                   # TaskTemplate id
    self.nextTasksId = nextTasksId     # Next pipeline tasks id
    self.prevTasksId = prevTasksId     # Prev pipeline tasks id
    self.isStartNext = isStartNext     # Is start next task?
    self.isSendResultToNext = isSendResultToNext # Is send result to next task?
    self.params = params               # Task params
    self.name = name    
    self.description = description
  def __repr__(self):
      return f"PipelineTask: id {self.id} pplId {self.pplId} ttId {self.ttId} params {self.params} \
               isEnabled {self.isEnabled} nextTasksId {self.nextTasksId} prevTasksId {self.prevTasksId} \
               name {self.name} description {self.description} setts {self.setts}"
  def __str__(self):
    return self.__repr__()

def add(pt : PipelineTask) -> bool:
  if 'db' in g:
    try:

      nextTasksId = ','.join([str(v) for v in pt.nextTasksId])
      prevTasksId = ','.join([str(v) for v in pt.prevTasksId])
      isStartNext = ','.join([str(v) for v in pt.isStartNext])
      isSendResultToNext = ','.join([str(v) for v in pt.isSendResultToNext])

      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblPipelineTask (pplId, ttId, params, isEnabled, setts,"
          "nextTasksId, prevTasksId, isStartNext, isSendResultToNext, name, description) VALUES("
          f'"{pt.pplId}",'
          f'"{pt.ttId}",'
          f'"{pt.params}",'
          f'"{pt.isEnabled}",'
          f'"{pt.setts}",'
          f'"{nextTasksId}",'
          f'"{prevTasksId}",'
          f'"{isStartNext}",'
          f'"{isSendResultToNext}",'
          f'"{pt.name}",'
          f'"{pt.description}");'
        )
        pt.id = cr.lastrowid
        g.db.commit()
      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("PipelineTask.add", str(err)))
  return False

def get(db, id : int) -> PipelineTask:
  try:
    task = None
    with closing(db.cursor()) as cr:
      cr.execute(
        "SELECT pplId, ttId, params, isEnabled "
        "FROM tblPipelineTask "
        f"WHERE id = {id};"      
      )
      rows = cr.fetchall()
      for row in rows:
        task = PipelineTask(id=id, pplId=row[0], ttId=row[1], params=row[2], isEnabled=row[3]) 
    return task
  except Exception as err:
    print("{0} local db query failed: {1}".format("PipelineTask.get", str(err)))
  return None

def getNextTasks(db, id : int) -> List[List[int]]:
  try:
    with closing(db.cursor()) as cr:
      cr.execute(
        "SELECT nextTasksId, isStartNext, isSendResultToNext "
        "FROM tblPipelineTask "
        f"WHERE id = {id};"      
      )
      rows = cr.fetchall()
      for row in rows:
        nextTasksId = [int(v) for v in row[0].split(',') if len(v)] 
        isStartNext = [int(v) for v in row[1].split(',') if len(v)] 
        isSendResultToNext = [int(v) for v in row[2].split(',') if len(v)] 
        return [nextTasksId, isStartNext, isSendResultToNext]
  except Exception as err:
    print("{0} local db query failed: {1}".format("PipelineTask.getNextTasks", str(err)))
  return None

def change(pt : PipelineTask) -> bool:
  if 'db' in g:
    try:
      nextTasksId = ','.join([str(v) for v in pt.nextTasksId])
      prevTasksId = ','.join([str(v) for v in pt.prevTasksId])
      isStartNext = ','.join([str(v) for v in pt.isStartNext])
      isSendResultToNext = ','.join([str(v) for v in pt.isSendResultToNext])
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblPipelineTask SET "
          f'pplId = "{pt.pplId}",'
          f'ttId = "{pt.ttId}",'
          f'isEnabled = "{pt.isEnabled}",'
          f'setts = "{pt.setts}",'
          f'params = "{pt.params}",'
          f'nextTasksId = "{nextTasksId}",'
          f'prevTasksId = "{prevTasksId}",'
          f'isStartNext = "{isStartNext}",'
          f'isSendResultToNext = "{isSendResultToNext}",'
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
          "nextTasksId, prevTasksId, isStartNext, isSendResultToNext, name, description "
          "FROM tblPipelineTask "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          nextTasksId = [int(v) for v in row[6].split(',') if len(v)]         
          prevTasksId = [int(v) for v in row[7].split(',') if len(v)]
          isStartNext = [int(v) for v in row[8].split(',') if len(v)]
          isSendResultToNext = [int(v) for v in row[9].split(',') if len(v)]
          pts.append(PipelineTask(id=row[0], pplId=row[1], ttId=row[2], params=row[3],
                                  isEnabled=row[4], setts=row[5],
                                  nextTasksId=nextTasksId, prevTasksId=prevTasksId,
                                  isStartNext=isStartNext, isSendResultToNext=isSendResultToNext,
                                  name=row[10], description=row[11]))       
      return pts  
    except Exception as err:
      print("{0} local db query failed: {1}".format("PipelineTask.all", str(err)))
  return []