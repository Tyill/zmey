from typing import List, Dict
from contextlib import closing
from flask import g

class Event: 
  """Event config""" 
  def __init__(self,
               id : int = 0,
               isEnabled : int = 1,
               timeStartEverySec : int = 0,
               timeLastStartEverySec : float = 0,
               timeStartOnceOfDay : List[str] = 0,               
               timeLastStartOnceOfDay : str = 0,    
               tasksForStart : List[int] = 0,
               name : str = "",
               description : str = ""):
    self.id = id
    self.isEnabled = isEnabled
    self.timeStartOnceOfDay = timeStartOnceOfDay
    self.timeStartEverySec = timeStartEverySec
    self.timeLastStartOnceOfDay = timeLastStartOnceOfDay
    self.timeLastStartEverySec = timeLastStartEverySec
    self.tasksForStart = tasksForStart
    self.name = name    
    self.description = description
  def __repr__(self):
      return f"Event: id {self.id} isEnabled {self.isEnabled} \
               timeStartOnceOfDay {self.timeStartOnceOfDay} timeStartEverySec {self.timeStartEverySec} \
               timeLastStartOnceOfDay {self.timeLastStartOnceOfDay} timeLastStartEverySec {self.timeLastStartEverySec} \
               tasksForStart {self.tasksForStart} name {self.name} description {self.description}"
  def __str__(self):
    return self.__repr__()

def add(ev : Event) -> bool:
  if 'db' in g:
    try:
      tasksForStart = ','.join([str(v) for v in ev.tasksForStart])
      timeStartOnceOfDay = ','.join([v for v in ev.timeStartOnceOfDay if len(v)])

      name = ev.name.replace("'", "''")
      description = ev.description.replace("'", "''")
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblEvent (isEnabled, timeStartOnceOfDay, timeStartEverySec,"
          "tasksForStart, name, description) VALUES("
          f"'{ev.isEnabled}',"
          f"'{timeStartOnceOfDay}',"
          f"'{ev.timeStartEverySec}',"
          f"'{tasksForStart}',"
          f"'{name}',"
          f"'{description}');"
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
      tasksForStart = ','.join([str(v) for v in ev.tasksForStart])
      timeStartOnceOfDay = ','.join([v for v in ev.timeStartOnceOfDay if len(v)])
      name = ev.name.replace("'", "''")
      description = ev.description.replace("'", "''")
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblEvent SET "
          f"isEnabled = '{ev.isEnabled}',"
          f"timeStartOnceOfDay = '{timeStartOnceOfDay}',"
          f"timeStartEverySec = '{ev.timeStartEverySec}',"
          f"tasksForStart = '{tasksForStart}',"
          f"name = '{name}',"
          f"description = '{description}' "
          f"WHERE id = {ev.id};"
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

def updateLastStartTime(db, ev : Event) -> bool:
  try:    
    with closing(db.cursor()) as cr:
      cr.execute(
        "UPDATE tblEvent SET "
        f"timeLastStartEverySec = '{ev.timeLastStartEverySec}',"
        f"timeLastStartOnceOfDay = '{ev.timeLastStartOnceOfDay}' "
        f"WHERE id = {ev.id};"
      )
      db.commit()
    return True
  except Exception as err:
    print("{0} local db query failed: {1}".format("Event.updateStartTime", str(err)))
  return False

def all() -> List[Event]:
  return allWithDB(g.db) if ('db' in g) else []

def allWithDB(db) -> List[Event]:
  try:
    evs = []
    with closing(db.cursor()) as cr:
      cr.execute(
        "SELECT id, isEnabled, timeStartEverySec, timeLastStartEverySec,"
        "timeStartOnceOfDay, timeLastStartOnceOfDay, tasksForStart, name, description "
        "FROM tblEvent "
        "WHERE isDelete = 0;"
      )
      rows = cr.fetchall()
      for row in rows:
        timeStartOnceOfDay = [v for v in row[4].split(',') if len(v)]
        tasksForStart = [int(v) for v in row[6].split(',') if len(v)]
        
        evs.append(Event(id=row[0],
                          isEnabled=row[1],
                          timeStartEverySec=row[2],
                          timeLastStartEverySec=row[3],
                          timeStartOnceOfDay=timeStartOnceOfDay,
                          timeLastStartOnceOfDay=row[5],                           
                          tasksForStart=tasksForStart,
                          name=row[7], description=row[8]))       
    return evs  
  except Exception as err:
    print("{0} local db query failed: {1}".format("Event.all", str(err)))
  return []