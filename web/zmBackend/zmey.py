import zmBackend.zmClient as zm
from zmBackend.zmClient import(
  User
)
import functools
from flask import(
  g, Blueprint, redirect, url_for, request, render_template
)

_zmCommon = None
_zmTaskWatch = None

def initApp(connStr : str):
  global _zmCommon
  _zmCommon = zm.Connection(connStr)
  global _zmTaskWatch 
  _zmTaskWatch = zm.Connection(connStr)  
  _zmCommon.createTables()

###############################################################################
### Common

def lastError() -> str:
  _zmCommon.getLastError()
def isConnection() -> bool:
  return _zmCommon.isOK()

bp = Blueprint('zmey', __name__)

def loginRequired(view):
  @functools.wraps(view)
  def wrapped_view(**kwargs):
    if g.userId is None:
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

def getUser(uname : str, passw : str) -> User:  
  usr = User(0, uname, passw)
  return usr if _zmCommon.getUserId(usr) else None

def addUser(usr : User) -> bool:
  return _zmCommon.addUser(usr)
   
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
  return None#_zmCommon.getAllPipelines(userId)

###############################################################################
### TaskTemplate

@bp.route('/addTaskTemplate')
@loginRequired
def addTaskTemplate():
  return None#_zmCommon.addTaskTemplate(ttl)

@bp.route('/changeTaskTemplate')
@loginRequired
def changeTaskTemplate():
  return None#_zmCommon.changeTaskTemplate(ttl)

@bp.route('/delTaskTemplate')
@loginRequired
def delTaskTemplate():
  return None#_zmCommon.delTaskTemplate(ttl.id)

@bp.route('/allTaskTemplates')
@loginRequired
def allTaskTemplates():
  return None#_zmCommon.getAllTaskTemplates(userId)

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
  return None#_zmCommon.getAllTasks(pplId)

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

def hChangeTask(tId : int, prevState : zm.StateType, newState : zm.StateType):
  pass