
import os
import sys
import time
import subprocess
import numpy as np

import matplotlib
matplotlib.use('tkagg')
import matplotlib.pyplot as plot

sys.path.append(os.path.expanduser("~") + '/projects/zmey/web/server/')
import zm_client as zm


#### 5 schedr, 5 * 20 workers, 1000000 tasks on one machine

zm.loadLib(os.path.expanduser("~") + '/projects/zmey/build/Release/libzmclient.so')
zo = zm.Connection("host=localhost port=5432 user=postgres dbname=zmeydb connect_timeout=10")

zo.setErrorCBack(lambda err: print(err))
  
# add and start schedulers and workers
print('Add and start schedulers and workers')  
sCnt = 5
wCnt = 20
wCapty = 5
schPrc = [] 
wkrPrc = []
for i in range(sCnt):
  sch = zm.Scheduler(connectPnt='localhost:' + str(4440 + i), capacityTask=wCnt * wCapty)
  if (not zo.addScheduler(sch)):
    exit(-1)
  for j in range(wCnt):
    if (not zo.addWorker(zm.Worker(sId=sch.id, connectPnt='localhost:' + str(4450 + i * wCnt + j), capacityTask=wCapty))):
      exit(-1)
  schPrc.append(subprocess.Popen([os.path.expanduser("~") + '/projects/zmey/build/Release/zmscheduler',
                                  '-la=localhost:' + str(4440 + i),
                                  "-db=host=localhost port=5432 user=postgres dbname=zmeydb connect_timeout=10"]))
  time.sleep(3)
  for j in range(wCnt):
    wkrPrc.append(subprocess.Popen([os.path.expanduser("~") + '/projects/zmey/build/Release/zmworker',
                                    '-sa=localhost:' + str(4440 + i),
                                    '-la=localhost:' + str(4450 + i * wCnt + j)]))

taskStat = []
taskCnt = 1000
batch = 1000
for i in range(batch):
  # start tasks
  tasks = []
  tmStartTasks = time.time()   
  for j in range(taskCnt):
    t = zm.Task(scriptPath="load_tests/task.sh", resultPath="load_tests/restask/")
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
      if ((tasks[k].state == zm.StateType.COMPLETED.value) or 
          (tasks[k].state == zm.StateType.ERROR.value)):
        complCnt += 1
  tmWaitTasks = time.time() - tmWaitTasks
  
  taskStat.append(round(tmStartTasks + tmWaitTasks, 1))
  print('Complete ', taskCnt * (i + 1), ' tasks:'
   ' tmSumm ', round(tmStartTasks + tmWaitTasks, 1),
   ' tmStartTasks ', round(tmStartTasks, 1),
   ' tmWaitTasks ', round(tmWaitTasks, 1))

# stop all schedr and workers
for i in range(len(schPrc)):
  schPrc[i].terminate()

for i in range(len(wkrPrc)):
  wkrPrc[i].terminate()

# graph
xp = np.arange(len(taskStat))
 
plot.bar(xp, taskStat, align='center')
plot.xlabel('taskCount, 1000/1')
plot.ylabel('time, sec/1000')
plot.title(f'{sCnt} schedr, {sCnt} * {wCnt} workers, {batch * taskCnt} tasks on one machine')

plot.show()