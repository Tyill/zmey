
import os
import sys
import time
import subprocess
sys.path.append('C:/cpp/other/zmey/python/')
import zmClient as zm

os.add_dll_directory('c:/Program Files/PostgreSQL/10/bin/')
os.add_dll_directory('c:/cpp/other/zmey/build/Release/')

zm.loadLib('c:/cpp/other/zmey/build/Release/zmClient.dll')
zo = zm.Connection("host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10")

zo.setErrorCBack(lambda err: print(err))

ttls = zo.getAllTaskTemplates(3)
