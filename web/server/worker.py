from typing import List
from flask import g

from .core import zmConn
from . import zm_client as zm 

class Worker(zm.Worker):
  None

def add(worker : Worker) -> bool:
  if zmConn and ('userId' in g):
    ok = zmConn.addWorker(worker)
    return ok
  return False

def change(worker : Worker) -> bool:
  if zmConn and ('userId' in g):
    return zmConn.changeWorker(worker)
  return False

def delete(wId : int) -> bool:
  if zmConn and ('userId' in g):
    return zmConn.delWorker(wId)
  return False

def all(sId) -> List[Worker]:
  if zmConn and ('userId' in g):
    workers = zmConn.getAllWorkers(sId)
    zmConn.workerState(workers)
    for w in workers:
      w.state = zm.stateStr(w.state)    
    return workers
  return []