
import os
import sys
import time
sys.path.append(os.path.expanduser("~") + '/cpp/zmey/python/')
import subprocess
import psycopg2
import zmClient as zm

#### 10 schedr, 100 workers, 10000 tasks on one machine

# del all tables
with psycopg2.connect(dbname='zmeyDb', user='alm', password='123', host='localhost') as pg:
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
              "drop table if exists tblInternError cascade;")
  csr.close()

zo = zm.ZMObj(zm.dbType.PostgreSQL, "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10")

#zo.setErrorCBack(lambda err: print(err))

zo.createTables()

# add user
usr = zm.user(name='alm')
if (not zo.addUser(usr)):
  exit(-1)
  
# add taskTemplate
tt = zm.taskTemplate(name='tt', uId=usr.id, maxDurationSec = 100, script="#! /bin/sh \n sleep 1")
if (not zo.addTaskTemplate(tt)):
  exit(-1)
  
# add pipeline
ppl = zm.pipeline(name='ppl', uId=usr.id)
if (not zo.addPipeline(ppl)):
  exit(-1)

# add and start schedulers and workers
print('Add and start schedulers and workers')  
sCnt = 1
wCnt = 10
schPrc = wkrPrc = []
for i in range(sCnt):
  sch = zm.schedr(connectPnt='localhost:' + str(4440 + i), capacityTask=100)
  if (not zo.addScheduler(sch)):
    exit(-1)
  for j in range(wCnt):
    if (not zo.addWorker(zm.worker(sId=sch.id, connectPnt='localhost:' + str(4450 + i * 10 + j), capacityTask=10))):
      exit(-1)
  schPrc.append(subprocess.Popen([os.path.expanduser("~") + '/cpp/zmey/build/Release/zmScheduler',
                                  '-cp=localhost:' + str(4440 + i),
                                  "-dbtp=PostgreSQL",
                                  "-dbcs=host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10"]))
  for j in range(wCnt):
    wkrPrc.append(subprocess.Popen([os.path.expanduser("~") + '/cpp/zmey/build/Release/zmWorker',
                                    '-scp=localhost:' + str(4440 + i),
                                    '-cp=localhost:' + str(4450 + i * 10 + j)]))
# wait until is running
time.sleep(5)

# pause schedrs
allSch = zo.getAllSchedulers()
for i in range(len(allSch)):
  zo.pauseScheduler(allSch[i].id)

# add and start 10000 tasks
taskCnt = 1000
print('Add and start', taskCnt, 'tasks')  
tasks = []
for j in range(taskCnt):
  t = zm.task(pplId=ppl.id, ttId=tt.id)
  zo.addTask(t)
  zo.startTask(t.id)
  tasks.append(t)

# start schedrs
allSch = zo.getAllSchedulers()
for i in range(len(allSch)):
  zo.startScheduler(allSch[i].id)

# wait until the task is completed
print('Wait until the task is completed')  
complCnt = 0
tstart = time.time()
while complCnt != taskCnt:
  zo.taskState(tasks)
  complCnt = 0
  for i in range(taskCnt):
    if (tasks[i].state == zm.stateType.completed):
      complCnt += 1
    
print('Time to complete all tasks: ', time.time() - tstart)  

time.sleep(45)

# stop all schedr and workers
for i in range(len(schPrc)):
  schPrc[i].terminate()

for i in range(len(wkrPrc)):
  wkrPrc[i].terminate()