
import os
import sys
sys.path.append(os.path.expanduser("~") + '/projects/zmey/web/server/')
import zm_client as zm

zm.loadLib(os.path.expanduser("~") + '/projects/zmey/build/Debug/libzmclient.so')
zo = zm.Connection("host=localhost port=5432 user=postgres dbname=zmeydb connect_timeout=10")

sCnt = 1
wCnt = 1
wCapty = 10
for i in range(sCnt):
  sch = zm.Scheduler(connectPnt='localhost:' + str(4440 + i), capacityTask=wCnt * wCapty)
  if (not zo.addScheduler(sch)):
    exit(-1)
  for j in range(wCnt):
    if (not zo.addWorker(zm.Worker(sId=sch.id, connectPnt='localhost:' + str(4450 + i * wCnt + j), capacityTask=wCapty))):
      exit(-1)
