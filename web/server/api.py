import os
import sys
sys.path.append("../python")
import python.zmClient as zm
from python.zmClient import(
  User, TaskTemplate, Pipeline
)
import json
import functools
from flask import(
  g, Blueprint, redirect, url_for, request, render_template
)

_zmCommon = None
_zmTaskWatch = None

def initApp(zmeyConnStr : str):

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

bp = Blueprint('zmey', __name__, url_prefix='/api')

def loginRequired(view):
  @functools.wraps(view)
  def wrapped_view(**kwargs):
    if (g.userId is None):
      return redirect(url_for('auth.login'))
    return view(**kwargs)
  return wrapped_view

def adminRequired(view):
  @functools.wraps(view)
  def wrapped_view(**kwargs):
    if (g.userId is None) or (g.userName != "admin"):
      return redirect(url_for('auth.login'))
    return view(**kwargs)
  return wrapped_view

###############################################################################
### User

def addUser(usr : User) -> bool:
  return _zmCommon.addUser(usr)

def getUser(uname : str, passw : str) -> User:  
  usr = User(0, uname, passw)
  return usr if _zmCommon.getUserId(usr) else None

def changeUser(usr : User) -> bool:
  return _zmCommon.changeUser(usr)

def allUsers() -> [User]:
  return _zmCommon.getAllUsers()

###############################################################################
### Scheduler

@bp.route('/addSchedr')
@adminRequired
def addSchedr():
  jn = request.get_json(silent=True)
  return None#_zmCommon.addScheduler(schr)

@bp.route('/delSchedr')
@adminRequired
def delSchedr():
  return None#_zmCommon.delScheduler(schr.id)

@bp.route('/allSchedrs')
@adminRequired
def allSchedrs():
  return None#_zmCommon.getAllSchedulers()

###############################################################################
### Worker

@bp.route('/addWorker')
@adminRequired
def addWorker():
  return None#_zmCommon.addWorker(wkr)

@bp.route('/delWorker')
@adminRequired
def delWorker():
  return None#_zmCommon.delWorker(wkr.id)

@bp.route('/allWorkers')
@adminRequired
def allWorkers():
  return None#_zmCommon.getAllWorkers(schId)

###############################################################################
### Pipeline

@bp.route('/addPipeline', methods=(['POST']))
@loginRequired
def addPipeline():
  try: 
    jnReq = request.get_json(silent=True)  

    ppl = Pipeline()
    ppl.name = jnReq['name']
    ppl.uId = g.userId
    ppl.description = jnReq['description']  
    return json.dumps(ppl.__dict__) if _zmCommon.addPipeline(ppl) else ('internal error', 500)
  except Exception:
    return ('bad request', 400)

@bp.route('/changePipeline', methods=(['POST']))
@loginRequired 
def changePipeline():
  try:
    jnReq = request.get_json(silent=True)
  
    ppl = Pipeline()
    ppl.id = jnReq['id']
    ppl.uId = g.userId
    ppl.name = jnReq['name']
    ppl.description = jnReq['description']    
    return json.dumps(ppl.__dict__) if _zmCommon.changePipeline(ppl) else ('internal error', 500)
  except Exception:
    return ('bad request', 400)

@bp.route('/delPipeline')
@loginRequired
def delPipeline():
  try:
    id = int(request.args.get('id', '0'))
    return ('ok', 200) if _zmCommon.delPipeline(id) else ('bad request', 400)  
  except Exception:
    return ('bad request', 400)

@bp.route('/allPipelines')
@loginRequired
def allPipelines():
  ret = []
  for p in _zmCommon.getAllPipelines(g.userId):
    ret.append(p.__dict__)
  return json.dumps(ret)

###############################################################################
### TaskTemplate

@bp.route('/addTaskTemplate', methods=(['POST']))
@loginRequired
def addTaskTemplate():
  try: 
    jnReq = request.get_json(silent=True)  

    ttl = TaskTemplate()
    ttl.name = jnReq['name']
    ttl.uId = g.userId
    ttl.script = jnReq['script']
    ttl.averDurationSec = int(jnReq['averDurationSec'])
    ttl.maxDurationSec = int(jnReq['maxDurationSec'])
    ttl.description = jnReq['description']  
    return json.dumps(ttl.__dict__) if _zmCommon.addTaskTemplate(ttl) else ('internal error', 500)
  except Exception:
    return ('bad request', 400)

@bp.route('/changeTaskTemplate', methods=(['POST']))
@loginRequired 
def changeTaskTemplate():
  try:
    jnReq = request.get_json(silent=True)
  
    ttl = TaskTemplate()
    ttl.id = jnReq['id']
    ttl.uId = g.userId
    ttl.name = jnReq['name']
    ttl.script = jnReq['script']
    ttl.averDurationSec = jnReq['averDurationSec']
    ttl.maxDurationSec = jnReq['maxDurationSec']
    ttl.description = jnReq['description']    
    return json.dumps(ttl.__dict__) if _zmCommon.changeTaskTemplate(ttl) else ('internal error', 500)
  except Exception:
    return ('bad request', 400)

@bp.route('/delTaskTemplate')
@loginRequired
def delTaskTemplate():
  try:
    id = int(request.args.get('id', '0'))
    return ('ok', 200) if _zmCommon.delTaskTemplate(id) else ('bad request', 400)  
  except Exception:
    return ('bad request', 400)

@bp.route('/allTaskTemplates')
@loginRequired
def allTaskTemplates():  
  ret = []
  for t in _zmCommon.getAllTaskTemplates(g.userId):
    ret.append(t.__dict__)
  return json.dumps(ret)

###############################################################################
### Group of tasks

@bp.route('/addTaskGroup')
@loginRequired
def addTaskGroup():
  return None#_zmCommon.addTask(tsk)

@bp.route('/changeTaskGroup')
@loginRequired
def changeTaskGroup():
  return None#_zmCommon.changeTask(tsk)

@bp.route('/delTaskGroup')
@loginRequired
def delTaskGroup():
  return None#_zmCommon.delTask(tsk.id)

@bp.route('/allTaskGroups')
@loginRequired
def allTaskGroups():
  ret = []
  for p in _zmCommon.getAllPipelines(g.userId):
    for gr in _zmCommon.getAllGroups(p.id):
      ret.append(gr.__dict__)
  return json.dumps(ret)

###############################################################################
### Task

@bp.route('/addTask')
@loginRequired
def addTask():
  return None#_zmCommon.addTask(tsk)

@bp.route('/changeTask')
@loginRequired
def changeTask():
  return None#_zmCommon.changeTask(tsk)

@bp.route('/delTask')
@loginRequired
def delTask():
  return None#_zmCommon.delTask(tsk.id)

@bp.route('/allTasks')
@loginRequired
def allTasks():
  ret = []
  for p in _zmCommon.getAllPipelines(g.userId):
    for t in _zmCommon.getAllTasks(p.id):
      t.state = t.state.value
      ret.append(t.__dict__) 
  return json.dumps(ret)

@bp.route('/startTask')
@loginRequired
def startTask():
  # if _zmCommon.startTask(tId):
  #   _zmTaskWatch.setChangeTaskStateCBack(tId, hChangeTask)
  #   return True
  # return False
  return None

@bp.route('/stopTask')
@loginRequired
def stopTask():
  return None#_zmCommon.stopTask(tId)