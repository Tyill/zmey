from . import user
from . import db

import multiprocessing as mp
import threading as thr
import time

def init(instance_path):
  queue = mp.Queue()  
  p = mp.Process(target=checkEvents, args=(instance_path, queue,))
  p.daemon = True
  p.start()

  t = thr.Thread(target=checkQueue, args=(queue,))
  t.daemon = True
  t.start()

def checkQueue(queue : mp.Queue):
  while(True):
    dbo = None
    try:
      (uId, newTask) = queue.get()
      
      usr = user.getById(uId)    
      dbo = db.createDb(usr.name)

      from . import task
      task.start(dbo, uId, newTask)

      from . import pipeline_task as pt
      pt.setChange(dbo, newTask.pplTaskId, True)

    except Exception as err:
      print("startNewTask failed: {0}".format(str(err)))
    finally:
      if dbo:
        db.closeDb(dbo)  

def checkEvents(instance_path : str,  queue : mp.Queue):
  startTime = time.time()
  while(True):
    deltaTimeSec = time.time() - startTime
    startTime = time.time()
    currTimeStr = time.strftime('%H:%M', time.localtime())
    currDateTimeStr = time.strftime('%Y-%m-%d %H:%M', time.localtime())
    for usr in user.allWithPath(instance_path):
      dbo = None      
      try:
        dbo = db.createDbWithPath(instance_path, usr.name)
        if (dbo is None):
          continue

        from . import event
        evts = event.allWithDB(dbo)
        for evt in evts:
          if (not evt.isEnabled):
            continue
    
          if evt.timeStartEverySec > 0:
            evt.timeLastStartEverySec += deltaTimeSec
            if evt.timeLastStartEverySec > evt.timeStartEverySec:
              startTask(evt, dbo, usr, queue)
              evt.timeLastStartEverySec = 0.0
          
          for stTime in evt.timeStartOnceOfDay:           
            if (currTimeStr == stTime) and (evt.timeLastStartOnceOfDay != currTimeStr):
              startTask(evt, dbo, usr, queue)
              evt.timeLastStartOnceOfDay = currTimeStr
            elif (currDateTimeStr == stTime) and (evt.timeLastStartOnceOfDay != currDateTimeStr):
              startTask(evt, dbo, usr, queue)
              evt.timeLastStartOnceOfDay = currDateTimeStr
              
          event.updateLastStartTime(dbo, evt)

      except Exception as err:
        print("checkEvents failed: {0}".format(str(err)))
      finally:
        if dbo:
          db.closeDb(dbo)

    deltaTimeSec = time.time() - startTime
    maxElapseTimeSec = 1.0
    if (maxElapseTimeSec - deltaTimeSec) > 0.0:
      time.sleep(maxElapseTimeSec - deltaTimeSec)
    
def startTask(evt, dbo, usr, queue):
  for tId in evt.tasksForStart:
    from . import pipeline_task as pt
    pplTask = pt.get(dbo, tId)
    if pplTask.isEnabled:
      from . import task
      newTask = task.Task(tId, starterEventId=evt.id, ttlId=pplTask.ttId)
      newTask.params = pplTask.params                             
      queue.put((usr.id, newTask,))

