

import sys

import clearDB

sys.path.append('c:/cpp/other/zmey/web/server/')
import zm_client as zm

#clearDB.clearDB()

zm.loadLib('c:/cpp/other/zmey/build/Release/zmclient.dll')
zo = zm.Connection("host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10")

zo.setErrorCBack(lambda err: print(err))

zo.createTables()

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
