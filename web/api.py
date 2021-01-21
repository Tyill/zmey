import os
import sys
sys.path.append("../python")
import python.zmClient as zm
from python.zmClient import(
  User, TaskTemplate
)
import json
import functools
from flask import(
  g, Blueprint, redirect, url_for, request, render_template
)

DEBUG = 1

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
    if (g.userId is None) and (DEBUG == 0):
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

@bp.route('/addPipeline')
@loginRequired
def addPipeline():
  return None#_zmCommon.addPipeline(ppl)

@bp.route('/delPipeline')
@loginRequired
def delPipeline():
  return None#_zmCommon.delPipeline(ppl.id)

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

  jnReq = request.get_json(silent=True)
  
  ttl = TaskTemplate()
  ttl.name = jnReq['name']
  ttl.uId = g.userId
  ttl.script = jnReq['script']
  ttl.averDurationSec = jnReq['averDurationSec']
  ttl.maxDurationSec = jnReq['maxDurationSec']
  ttl.description = jnReq['description']
  
  return json.dumps(ttl) if _zmCommon.addTaskTemplate(ttl) else "{}"

@bp.route('/changeTaskTemplate', methods=(['POST']))
@loginRequired 
def changeTaskTemplate():
  jnReq = request.get_json(silent=True)
  
  ttl = TaskTemplate()
  ttl.id = jnReq['id']
  ttl.uId = g.userId
  ttl.name = jnReq['name']
  ttl.script = jnReq['script']
  ttl.averDurationSec = jnReq['averDurationSec']
  ttl.maxDurationSec = jnReq['maxDurationSec']
  ttl.description = jnReq['description']
  
  return json.dumps(ttl) if _zmCommon.changeTaskTemplate(ttl) else "{}"

@bp.route('/delTaskTemplate')
@loginRequired
def delTaskTemplate():
  return None#_zmCommon.delTaskTemplate(ttl.id)

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
  jn = request.get_json(silent=True)
  grps = _zmCommon.getAllGroups(jn["pplId"])
  return json.dumps(grps)

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
  jn = request.get_json(silent=True)
  tsks = _zmCommon.getAllTasks(jn["pplId"])
  return json.dumps(tsks)

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