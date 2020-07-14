
import sys
sys.path.append('../python/')

import zmClient as zm

#### 10 schedr, 100 workers, 10000 tasks on one machine


zo = zm.ZMObj(zm.dbType.PostgreSQL, "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10")

# add user
if (len(zo.getAllUsers()) == 0):
  if (!zo.addUser(zm.user(name='alm'))):
    exit(-1)
usr = zo.getAllUsers()[0]

# add taskTemplate
if (len(zo.getAllTaskTemplates()) == 0):
  if (!zo.addTaskTemplate(zm.taskTemplate(name='tt', uId=usr.id, script="#!/bin/sh \n sleep 1"))):
    exit(-1)
tt = zo.getAllTaskTemplates()[0]

# add pipeline
if (len(zo.getAllPipelines()) == 0):
  if (!zo.addPipeline(zm.pipeline(name='ppl', uId=usr.id):
    exit(-1)
tt = zo.getAllPipelines()[0]

# add 10 schedulers and 100 workers
if (zo.getAllSchedulers() != 10): 
  for i in range(10):
    sch = zm.schedr(connectPnt='localhost:' + str(4440 + i), capacityTask=100)
    if (!zo.addScheduler(sch)):
      exit(-1)
    for j in range(10):
      if (!zo.addWorker(zm.worker(sId=sch.id, connectPnt='localhost:' + str(4450 + i * 10 + j), capacityTask=10)
    
# start 100 workers



