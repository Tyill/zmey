from typing import List
from contextlib import closing
from flask import g


class Event: 
  """Event config""" 
  def __init__(self,
               id : int = 0,
               isEnabled : int = 1,
               nextTasksId : List[int] = 0,
               name = "",
               description = ""):
    self.id = id
    self.isEnabled = isEnabled
    self.nextTasksId = nextTasksId     # Next pipeline tasks id
    self.name = name    
    self.description = description
  def __repr__(self):
      return f"Event: id {self.id} isEnabled {self.isEnabled} \
               nextTasksId {self.nextTasksId} name {self.name} description {self.description}"
  def __str__(self):
    return self.__repr__()

def add(ev : Event) -> bool:
  if 'db' in g:
    try:
      nextTasksId = ','.join([str(v) for v in ev.nextTasksId])

      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblEvent (isEnabled, "
          "nextTasksId, name, description) VALUES("
          f'"{ev.isEnabled}",'
          f'"{nextTasksId}",'
          f'"{ev.name}",'
          f'"{ev.description}");'
        )
        ev.id = cr.lastrowid
        g.db.commit()
      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("Event.add", str(err)))
  return False

def change(ev : Event) -> bool:
  if 'db' in g:
    try:
      nextTasksId = ','.join([str(v) for v in ev.nextTasksId])

      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblEvent SET "
          f'isEnabled = "{ev.isEnabled}",'
          f'nextTasksId = "{nextTasksId}",'
          f'name = "{ev.name}",'
          f'description = "{ev.description}" '
          f'WHERE id = {ev.id};'  
        )
        g.db.commit()
      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("Event.change", str(err)))
  return False

def delete(evId) -> bool:
  if 'db' in g:    
    try:
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblEvent SET "
          "isDelete = 1 "
          f"WHERE id = {evId};" 
        )
        g.db.commit()
      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("Event.delete", str(err)))
  return False

def all() -> List[Event]:
  if 'db' in g:
    try:
      evs = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, isEnabled, nextTasksId, name, description "
          "FROM tblEvent "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          nextTasksId = [int(v) for v in row[2].split(',') if len(v)]
          evs.append(Event(id=row[0],
                           isEnabled=row[1],
                           nextTasksId=nextTasksId,
                           name=row[3], description=row[4]))       
      return evs  
    except Exception as err:
      print("{0} local db query failed: {1}".format("Task.Event.all", str(err)))
  return []