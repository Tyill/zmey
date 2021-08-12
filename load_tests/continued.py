
import os
import sys
import time
import subprocess
import psycopg2
sys.path.append(os.path.expanduser("~") + '/cpp/zmey/python/')
import zmClient as zm

#### 5 schedr, 5 * 20 workers, 1000000 tasks on one machine

# del all tables
with psycopg2.connect(dbname='zmeydb', user='alm', password='123', host='localhost') as pg:
  csr = pg.cursor()
  csr.execute("drop table if exists tblUser cascade;" +
              "drop table if exists tblScheduler cascade;" +
              "drop table if exists tblWorker cascade;" +
              "drop table if exists tblUTaskTemplate cascade;" +
              "drop table if exists tblTask cascade;" +
              "drop table if exists tblUPipeline cascade;" +
              "drop table if exists tblUPipelineTask cascade;" +
              "drop table if exists tblTaskState cascade;" +
              "drop table if exists tblTaskTime cascade;" + 
              "drop table if exists tblTaskResult cascade;" +
              "drop table if exists tblPrevTask cascade;" +
              "drop table if exists tblTaskParam cascade;" +
              "drop table if exists tblTaskQueue cascade;" + 
              "drop table if exists tblInternError cascade;" + 
              "drop table if exists tblConnectPnt cascade;" + 
              "drop table if exists tblUTaskGroup cascade;" + 
              "DROP FUNCTION IF EXISTS funcStartTask;" +
              "DROP FUNCTION IF EXISTS funcTasksOfSchedr;" +
              "DROP FUNCTION IF EXISTS funcNewTasksForSchedr;")
  csr.close()

zm.loadLib(os.path.expanduser("~") + '/cpp/zmey/build/Release/libzmClient.so')
zo = zm.Connection("host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10")

zo.setErrorCBack(lambda err: print(err))

zo.createTables()

# add user
usr = zm.User(name='alm')
if (not zo.addUser(usr)):
  exit(-1)
  
# add taskTemplate
tt = zm.TaskTemplate(name='tt', uId=usr.id, maxDurationSec = 10, script="#! /bin/sh \n sleep 1; echo res ")
if (not zo.addTaskTemplate(tt)):
  exit(-1)
  
# add pipeline
ppl = zm.Pipeline(name='ppl', uId=usr.id)
if (not zo.addPipeline(ppl)):
  exit(-1)

# add and start schedulers and workers
print('Add and start schedulers and workers')  
sCnt = 5
wCnt = 20
wCapty = 5
schPrc = [] 
wkrPrc = []
for i in range(sCnt):
  sch = zm.Schedr(connectPnt='localhost:' + str(4440 + i), capacityTask=wCnt * wCapty)
  if (not zo.addScheduler(sch)):
    exit(-1)
  for j in range(wCnt):
    if (not zo.addWorker(zm.Worker(sId=sch.id, connectPnt='localhost:' + str(4450 + i * wCnt + j), capacityTask=wCapty))):
      exit(-1)
  schPrc.append(subprocess.Popen([os.path.expanduser("~") + '/cpp/zmey/build/Release/zmScheduler',
                                  '-la=localhost:' + str(4440 + i),
                                  "-db=host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10"]))
  time.sleep(3)
  for j in range(wCnt):
    wkrPrc.append(subprocess.Popen([os.path.expanduser("~") + '/cpp/zmey/build/Release/zmWorker',
                                    '-sa=localhost:' + str(4440 + i),
                                    '-la=localhost:' + str(4450 + i * wCnt + j)]))

# add tasks
taskCnt = 10000
for j in range(taskCnt):
  pt = zm.PipelineTask(name="pt", pplId=ppl.id, ttId=tt.id)
  zo.addPipelineTask(pt)

for i in range(100):
  # start tasks
  tasks = []
  tmStartTasks = time.time()   
  for j in range(taskCnt):
    t = zm.Task(ptId=pt.id)
    zo.startTask(t)
    tasks.append(t)
  tmStartTasks = time.time() - tmStartTasks
 
  # wait until the task is completed
  complCnt = 0  
  tmWaitTasks = time.time()
  while complCnt != taskCnt:  
    zo.taskState(tasks)
    complCnt = 0
    for k in range(taskCnt):
      if ((tasks[k].state == zm.StateType.COMPLETED) or 
          (tasks[k].state == zm.StateType.ERROR)):
        complCnt += 1
  tmWaitTasks = time.time() - tmWaitTasks
  
  print('Complete ', taskCnt * (i + 1), ' tasks:'
   ' tmSumm ', round(tmStartTasks + tmWaitTasks, 1),
   ' tmStartTasks ', round(tmStartTasks, 1),
   ' tmWaitTasks ', round(tmWaitTasks, 1))

# stop all schedr and workers
for i in range(len(schPrc)):
  schPrc[i].terminate()

for i in range(len(wkrPrc)):
  wkrPrc[i].terminate()