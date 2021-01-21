
import os
import sys
import time
import json
import subprocess
sys.path.append('C:/cpp/other/zmey/python/')
import zmClient as zm

os.add_dll_directory('c:/Program Files/PostgreSQL/10/bin/')
os.add_dll_directory('c:/cpp/other/zmey/build/Release/')

zm.loadLib('c:/cpp/other/zmey/build/Release/zmClient.dll')
zo = zm.Connection("host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10")

zo.setErrorCBack(lambda err: print(err))

#usr = zm.User(0, 'alm', '123')
#zo.addUser(usr)

ppl = zm.Pipeline(uId=1, name="ppl1")
zo.addPipeline(ppl)
ppl = zm.Pipeline(uId=1, name="ppl2")
zo.addPipeline(ppl)
ppl = zm.Pipeline(uId=1, name="ppl3")
zo.addPipeline(ppl)

plls = zo.getAllPipelines(1)

ttl = zm.TaskTemplate(uId=1, name="ttl1", script="script")
zo.addTaskTemplate(ttl)
ttl = zm.TaskTemplate(uId=1, name="ttl2", script="script")
zo.addTaskTemplate(ttl)
ttl = zm.TaskTemplate(uId=1, name="ttl3", script="script")
zo.addTaskTemplate(ttl)
# d = ttl.__dict__



# 

for x in range(1000000000):
  ttls = zo.getAllTaskTemplates(3)

