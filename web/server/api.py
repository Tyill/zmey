import os
from typing import List
import json

import python.zmClient as zm 
from python.zmClient import(
  User, TaskTemplate, Pipeline
)

from . import auth
from flask import(
  g, Blueprint, request
)

_zmCommon = None
_zmTaskWatch = None

def init(zmeyConnStr : str):

  libname = 'libzmClient.so'
  if os.name == 'nt':
    libname = 'zmClient.dll'
      
  zm.loadLib(libname)
  
  global _zmCommon
  _zmCommon = zm.Connection(zmeyConnStr)

  if not _zmCommon.isOK():
    raise RuntimeError('Error connection with PostgreSQL: ' + _zmCommon.getLastError())
  
  _zmCommon.setErrorCBack(lambda err: print(err))  
  
  _zmCommon.createTables()
   
  global _zmTaskWatch 
  _zmTaskWatch = zm.Connection(zmeyConnStr) 
  
###############################################################################
### Common

def lastError() -> str:
  _zmCommon.getLastError()

bp = Blueprint('api', __name__, url_prefix='/api/v1')

###############################################################################
### User

def addUser(usr : User) -> bool:
  return _zmCommon.addUser(usr)

def getUser(uname : str, passw : str) -> User:  
  usr = User(0, uname, passw)
  return usr if _zmCommon.getUserId(usr) else None

def changeUser(usr : User) -> bool:
  return _zmCommon.changeUser(usr)

def allUsers() -> List[User]:
  return _zmCommon.getAllUsers()

###############################################################################
### Scheduler

@bp.route('/schedulers')
@auth.adminRequired
def allSchedrs():
  return None#_zmCommon.getAllSchedulers()

###############################################################################
### Worker

@bp.route('/workers')
@auth.adminRequired
def allWorkers():
  return None#_zmCommon.getAllWorkers(schId)

###############################################################################
### Pipeline

@bp.route('/pipelines', methods=(['POST']))
@auth.loginRequired
def addPipeline():
  try: 
    jnReq = request.get_json(silent=True)  

    ppl = Pipeline()
    ppl.name = jnReq['name']
    ppl.userId = g.userId
    ppl.description = jnReq['description']  
    return json.dumps(ppl.__dict__) if _zmCommon.addPipeline(ppl) else ('internal error', 500)
  except Exception:
    return ('bad request', 400)

@bp.route('/pipelines/<int:id>', methods=(['PUT']))
@auth.loginRequired 
def changePipeline(id : int):
  try:
    jnReq = request.get_json(silent=True)
  
    ppl = Pipeline()
    ppl.id = id
    ppl.userId = g.userId
    ppl.name = jnReq['name']
    ppl.description = jnReq['description']    
    return json.dumps(ppl.__dict__) if _zmCommon.changePipeline(ppl) else ('internal error', 500)
  except Exception:
    return ('bad request', 400)

@bp.route('/pipelines/<int:id>', methods=(['DELETE']))
@auth.loginRequired
def delPipeline(id : int):
  try:
    return ('ok', 200) if _zmCommon.delPipeline(id) else ('bad request', 400)  
  except Exception:
    return ('bad request', 400)

@bp.route('/pipelines', methods=(['GET']))
@auth.loginRequired
def allPipelines():
  ret = []
  for p in _zmCommon.getAllPipelines(g.userId):
    ret.append(p.__dict__)
  return json.dumps(ret)

###############################################################################
### TaskTemplate

@bp.route('/taskTemplates', methods=(['POST']))
@auth.loginRequired
def addTaskTemplate():
  try: 
    jnReq = request.get_json(silent=True)  

    ttl = TaskTemplate()
    ttl.name = jnReq['name']
    ttl.userId = g.userId
    ttl.script = jnReq['script']
    ttl.averDurationSec = int(jnReq['averDurationSec'])
    ttl.maxDurationSec = int(jnReq['maxDurationSec'])
    ttl.description = jnReq['description']  
    return json.dumps(ttl.__dict__) if _zmCommon.addTaskTemplate(ttl) else ('internal error', 500)
  except Exception:
    return ('bad request', 400)

@bp.route('/taskTemplates/<int:id>', methods=(['PUT']))
@auth.loginRequired 
def changeTaskTemplate(id : int):
  try:
    jnReq = request.get_json(silent=True)
  
    ttl = TaskTemplate()
    ttl.id = id
    ttl.userId = g.userId
    ttl.name = jnReq['name']
    ttl.script = jnReq['script']
    ttl.averDurationSec = jnReq['averDurationSec']
    ttl.maxDurationSec = jnReq['maxDurationSec']
    ttl.description = jnReq['description']    
    return json.dumps(ttl.__dict__) if _zmCommon.changeTaskTemplate(ttl) else ('internal error', 500)
  except Exception:
    return ('bad request', 400)

@bp.route('/taskTemplates/<int:id>', methods=(['DELETE']))
@auth.loginRequired
def delTaskTemplate(id : int):  
  try:
    return ('ok', 200) if _zmCommon.delTaskTemplate(id) else ('bad request', 400)  
  except Exception:
    return ('bad request', 400)

@bp.route('/taskTemplates', methods=(['GET']))
@auth.loginRequired
def allTaskTemplates():  
  ret = []
  for t in _zmCommon.getAllTaskTemplates(g.userId):
    ret.append(t.__dict__)
  return json.dumps(ret)

###############################################################################
### Task of pipeline

@bp.route('/pipelineTasks', methods=(['POST']))
@auth.loginRequired
def addTask():
  return None#_zmCommon.addTask(tsk)

@bp.route('/pipelineTasks', methods=(['UPDATE']))
@auth.loginRequired
def changeTask():
  return None

@bp.route('/pipelineTasks', methods=(['DELETE']))
@auth.loginRequired
def delTask():
  return None

@bp.route('/pipelineTasks', methods=(['GET']))
@auth.loginRequired
def allTasks():
  ret = []
  for p in _zmCommon.getAllPipelines(g.userId):
    for t in _zmCommon.getAllPipelineTasks(p.id):
      t.state = t.state.value
      ret.append(t.__dict__)
  return json.dumps(ret)

###############################################################################
### Task object

@bp.route('/tasks', methods=(['GET']))
@auth.loginRequired
def startTask():
  # if _zmCommon.startTask(tId):
  #   _zmTaskWatch.setChangeTaskStateCBack(tId, hChangeTask)
  #   return True
  # return False
  return None

@bp.route('/stopTask')
@auth.loginRequired
def stopTask():
  return None#_zmCommon.stopTask(tId)