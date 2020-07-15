
import os
import sys
sys.path.append(os.path.expanduser("~") + '/cpp/zmey/python/')
import subprocess
import zmClient as zm

#### 10 schedr, 100 workers, 10000 tasks on one machine


zo = zm.ZMObj(zm.dbType.PostgreSQL, "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10")

zo.setErrorCBack(lambda err: print(err))

zo.createTables()

# add user
if (len(zo.getAllUsers()) == 0):
  if (not zo.addUser(zm.user(name='alm'))):
    exit(-1)
usr = zo.getAllUsers()[0]

# add taskTemplate
if (len(zo.getAllTaskTemplates(uId=usr.id)) == 0):
  if (not zo.addTaskTemplate(zm.taskTemplate(name='tt', uId=usr.id, script="#! /bin/sh \n sleep 1"))):
    exit(-1)
tt = zo.getAllTaskTemplates(uId=usr.id)[0]

# add pipeline
if (len(zo.getAllPipelines(uId=usr.id)) == 0):
  if (not zo.addPipeline(zm.pipeline(name='ppl', uId=usr.id))):
    exit(-1)
ppl = zo.getAllPipelines(uId=usr.id)[0]

# add 10 schedulers and 100 workers
if (len(zo.getAllSchedulers()) < 10):
  for i in range(10):
    sch = zm.schedr(connectPnt='localhost:' + str(4440 + i), capacityTask=100)
    if (not zo.addScheduler(sch)):
      exit(-1)
    for j in range(10):
      if (not zo.addWorker(zm.worker(sId=sch.id, connectPnt='localhost:' + str(4450 + i * 10 + j), capacityTask=10))):
        exit(-1)
    
# start 10 schedr and 100 workers
# for i in range(10):
#   subprocess.Popen([os.path.expanduser("~") + '/cpp/zmey/build/Debug/zmScheduler',
#                     '-cp=localhost:' + str(4440 + i),
#                     "-dbtp=PostgreSQL",
#                     "-dbcs=host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10"])
#   for j in range(10):
#     subprocess.Popen([os.path.expanduser("~") + '/cpp/zmey/build/Debug/zmWorker',
#                       '-scp=localhost:' + str(4440 + i),
#                       '-cp=localhost:' + str(4450 + i * 10 + j)])

# add 10000 tasks
#for j in range(10000):
#  zo.addTask(zm.task(pplId=ppl.id, ttId=tt.id))                     

allTasks = zo.getAllTasks(pplId=ppl.id)

# start 10000 tasks
# for j in range(10000):
#   zo.startTask(allTasks[j].id)       

while True:
  zo.taskState(allTasks[0:10])
  zo.taskResult(allTasks[0])
  kk = False

