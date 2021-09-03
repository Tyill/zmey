from typing import List, Dict
from contextlib import closing
from flask import g

class Event: 
  """Event config""" 
  def __init__(self,
               id : int = 0,
               isEnabled : int = 1,
               nextTasksId : List[int] = 0,
               timeStartOnceOfDay : List[str] = 0,
               timeStartEverySec : int = 0,
               tasksForStart : List[Dict] = 0,
               name : str = "",
               description : str = ""):
    self.id = id
    self.isEnabled = isEnabled
    self.timeStartOnceOfDay = timeStartOnceOfDay
    self.timeStartEverySec = timeStartEverySec
    self.tasksForStart = tasksForStart
    self.nextTasksId = nextTasksId     # Next pipeline tasks id
    self.name = name    
    self.description = description
  def __repr__(self):
      return f"Event: id {self.id} isEnabled {self.isEnabled} \
               timeStartOnceOfDay {self.timeStartOnceOfDay} timeStartEverySec {self.timeStartEverySec} tasksForStart {self.tasksForStart} \
               name {self.name} description {self.description}"
  def __str__(self):
    return self.__repr__()

def add(ev : Event) -> bool:
  if 'db' in g:
    try:
      tasksForStart = ','.join([str(v['pplId']) + " " + str(v['taskId']) for v in ev.tasksForStart if len(v)])
      timeStartOnceOfDay = ','.join([v for v in ev.timeStartOnceOfDay if len(v)])

      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblEvent (isEnabled, timeStartOnceOfDay, timeStartEverySec,"
          "tasksForStart, name, description) VALUES("
          f"'{ev.isEnabled}',"
          f"'{timeStartOnceOfDay}',"
          f"'{ev.timeStartEverySec}',"
          f"'{tasksForStart}',"
          f"'{ev.name}',"
          f"'{ev.description}');"
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
      tasksForStart = ','.join([str(v['pplId']) + " " + str(v['taskId']) for v in ev.tasksForStart if len(v)])
      timeStartOnceOfDay = ','.join([v for v in ev.timeStartOnceOfDay if len(v)])
      
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblEvent SET "
          f"isEnabled = '{ev.isEnabled}',"
          f"timeStartOnceOfDay = '{timeStartOnceOfDay}',"
          f"timeStartEverySec = '{ev.timeStartEverySec}',"
          f"tasksForStart = '{tasksForStart}',"
          f"name = '{ev.name}',"
          f"description = '{ev.description}' "
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

def all() -> List[Event]:
  if 'db' in g:
    try:
      evs = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, isEnabled, timeStartEverySec, timeStartOnceOfDay, tasksForStart,"
          "name, description "
          "FROM tblEvent "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          timeStartOnceOfDay = [v for v in row[3].split(',') if len(v)]
          tasksForStart = [{"pplId" : int(v.split(' ')[0]), "taskId" : int(v.split(' ')[1])} for v in row[4].split(',') if len(v)]
          
          evs.append(Event(id=row[0],
                           isEnabled=row[1],
                           timeStartEverySec=row[2],
                           timeStartOnceOfDay=timeStartOnceOfDay,                           
                           tasksForStart=tasksForStart,
                           name=row[5], description=row[6]))       
      return evs  
    except Exception as err:
      print("{0} local db query failed: {1}".format("Event.all", str(err)))
  return []