
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

# usr = zm.User(0, 'alm', '123')
# ttls = zo.addUser(usr)

# ttl = zm.TaskTemplate(uId=3, name="ttl1", script="script")

# d = ttl.__dict__



# zo.addTaskTemplate(ttl)

for x in range(1000000000):
  ttls = zo.getAllTaskTemplates(3)

