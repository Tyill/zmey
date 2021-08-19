from typing import List
from contextlib import closing
from flask import g


class Event: 
  """Event config""" 
  def __init__(self,
               id : int = 0,
               isEnabled : int = 1,
               isVisible : int = 0,
               positionX : int = 0,
               positionY : int = 0,
               nextTasksId : List[int] = 0,
               nextEventsId : List[int] = 0,
               params : List[str] = [],
               name = "",
               description = ""):
    self.id = id
    self.isEnabled = isEnabled
    self.isVisible = isVisible
    self.positionX = positionX
    self.positionY = positionY
    self.nextTasksId = nextTasksId     # Next pipeline tasks id
    self.nextEventsId = nextEventsId   # Next events id
    self.params = params               # Task params
    self.name = name    
    self.description = description
  def __repr__(self):
      return f"Event: id {self.id} params {self.params} nextTasksId {self.nextTasksId} nextEventsId {self.nextEventsId} name {self.name} description {self.description}"
  def __str__(self):
    return self.__repr__()

def add(ev : Event) -> bool:
  if 'db' in g:
    try:
      nextTasksId = ','.join(ev.nextTasksId)
      nextEventsId = ','.join(ev.nextEventsId)

      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblEvent (params, , isEnabled, isVisible, positionX, positionY,"
          "nextTasksId, nextEventsId, name, description) VALUES("
          f"'{ev.params}',"
          f"'{ev.isEnabled}',"
          f"'{ev.isVisible}',"
          f"'{ev.positionX}',"
          f"'{ev.positionY}',"
          f"'{nextTasksId}',"
          f"'{nextEventsId}',"
          f"'{ev.name}',"
          f"'{ev.description}');"
        )
        ev.id = cr.lastrowid
        g.db.commit()
      return True
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return False

def change(ev : Event) -> bool:
  if 'db' in g:
    try:
      nextTasksId = ','.join(ev.nextTasksId)
      nextEventsId = ','.join(ev.nextEventsId)

      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblEvent SET "
          f"params = '{ev.params}',"
          f"isEnabled = '{ev.isEnabled}',"
          f"isVisible = '{ev.isVisible}',"
          f"positionX = '{ev.positionX}',"
          f"positionY = '{ev.positionY}',"
          f"nextTasksId = '{nextTasksId}',"
          f"nextEventsId = '{nextEventsId}',"
          f"name = '{ev.name}',"
          f"description = '{ev.description}' "
          f"WHERE id = {ev.id};"  
        )
        g.db.commit()
      return True
    except Exception as err:
      print('Local db query failed: %s' % str(err))
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
      print('Local db query failed: %s' % str(err))
  return False

def all() -> List[Event]:
  if 'db' in g:
    try:
      evs = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, params, isEnabled, isVisible, positionX, positionY,"
          "nextTasksId, nextEventsId, name, description "
          "FROM tblEvent "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          nextTasksId = row[6].split(',')
          nextEventsId = row[7].split(',')
          evs.append(Event(id=row[0], params=row[1],
                           isEnabled=row[2], isVisible=row[3], positionX=row[4], positionY=row[5],
                           nextTasksId=nextTasksId, nextEventsId=nextEventsId, name=row[8], description=row[9]))       
      return evs  
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return []