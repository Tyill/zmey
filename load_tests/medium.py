
import os
import sys
import time
import subprocess

sys.path.append(os.path.expanduser("~") + '/projects/zmey/web/server/')
import zm_client as zm

#### 3 schedr, 3 * 30 workers, 3000 tasks on one machine

zm.loadLib(os.path.expanduser("~") + '/projects/zmey/build/Release/libzmclient.so')
zo = zm.Connection("host=localhost port=5432 user=postgres dbname=zmeydb connect_timeout=10")

zo.setErrorCBack(lambda err: print(err))

# add and start schedulers and workers
print('Add and start schedulers and workers')  
sCnt = 3
wCnt = 30
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

# start tasks
taskCnt = 3000
print('Start', taskCnt, 'tasks') 
tstart = time.time()
tasks = []
for j in range(taskCnt):
  t = zm.Task(scriptPath="load_tests/task.sh", resultPath="load_tests/restask/")
  zo.startTask(t)
  tasks.append(t)

# wait until the task is completed
print('Wait until the task is completed')  
complCnt = 0
while complCnt != taskCnt:  
  zo.taskState(tasks)
  complCnt = 0
  for i in range(taskCnt):
    if ((tasks[i].state == zm.StateType.COMPLETED.value) or 
        (tasks[i].state == zm.StateType.ERROR.value)):
      complCnt += 1
    
print('Time to complete all tasks: ', time.time() - tstart)

# stop all schedr and workers
for i in range(len(schPrc)):
  schPrc[i].terminate()

for i in range(len(wkrPrc)):
  wkrPrc[i].terminate()