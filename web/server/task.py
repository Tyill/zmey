from typing import List
from contextlib import closing
from flask import g

from .core import zmConn
from . import zm_client as zm 

class Task(zm.Task):
  None

def start(t : Task) -> bool:
  #if zmConn and g.userId and ('db' in g):
    
    # if zmConn.startTask(t):
    #   try:
    #     with closing(g.db.cursor()) as cr:
    #       cr.execute(
    #         "INSERT INTO tblTaskTemplate (id, name, description, script, averDurationSec, maxDurationSec) VALUES("
    #         f'"{iott.id}",'
    #         f'"{iott.name}",'
    #         f'"{iott.description}",'
    #         f'"{iott.script}",'
    #         f'"{iott.averDurationSec}",'
    #         f'"{iott.maxDurationSec}");'
    #       )
    #       g.db.commit()
    #     return True
    #   except Exception as err:
    #     print('Local db query failed: %s' % str(err))
  return False
