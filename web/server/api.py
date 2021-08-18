import json
from flask import(
  Blueprint, request
)

from . import auth
from . import (
  task_template as tt,
  pipeline as pp,
)
 
bp = Blueprint('api', __name__, url_prefix='/api/v1')

###############################################################################
### Scheduler

@bp.route('/schedulers')
@auth.adminRequired
def allSchedrs():
  return None

###############################################################################
### Worker

@bp.route('/workers')
@auth.adminRequired
def allWorkers():
  return None

###############################################################################
### Pipeline

@bp.route('/pipelines', methods=(['POST']))
@auth.loginRequired
def addPipeline():
  try: 
    jnReq = request.get_json(silent=True)  

    ppl = pp.Pipeline()
    ppl.name = jnReq['name']
    ppl.description = jnReq['description']  
    return json.dumps(ppl.__dict__) if pp.add(ppl) else ('internal error', 500)
  except Exception as err:
    print(f'/pipelines POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/pipelines/<int:id>', methods=(['PUT']))
@auth.loginRequired 
def changePipeline(id : int):
  try:
    jnReq = request.get_json(silent=True)
  
    ppl = pp.Pipeline()
    ppl.id = id
    ppl.name = jnReq['name']
    ppl.description = jnReq['description']    
    return json.dumps(ppl.__dict__) if pp.change(ppl) else ('internal error', 500)
  except Exception as err:
    print(f'/pipelines/{id} PUT {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/pipelines/<int:id>', methods=(['DELETE']))
@auth.loginRequired
def delPipeline(id : int):
  try:
    return ('ok', 200) if pp.delete(id) else ('bad request', 400)  
  except Exception as err:
    print(f'/pipelines/{id} DELETE failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/pipelines', methods=(['GET']))
@auth.loginRequired
def allPipelines():
  ret = []
  for p in pp.all():
    ret.append(p.__dict__)
  return json.dumps(ret)

###############################################################################
### TaskTemplate

@bp.route('/taskTemplates', methods=(['POST']))
@auth.loginRequired
def addTaskTemplate():
  try: 
    jnReq = request.get_json(silent=True)  

    ttl = tt.TaskTemplate()
    ttl.name = jnReq['name']
    ttl.script = jnReq['script']
    ttl.averDurationSec = int(jnReq['averDurationSec'])
    ttl.maxDurationSec = int(jnReq['maxDurationSec'])
    ttl.description = jnReq['description']  
    return json.dumps(ttl.__dict__) if tt.add(ttl) else ('internal error', 500)
  except Exception as err:
    print(f'/taskTemplates POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/taskTemplates/<int:id>', methods=(['PUT']))
@auth.loginRequired 
def changeTaskTemplate(id : int):
  try:
    jnReq = request.get_json(silent=True)
  
    ttl = tt.TaskTemplate()
    ttl.id = id
    ttl.name = jnReq['name']
    ttl.script = jnReq['script']
    ttl.averDurationSec = jnReq['averDurationSec']
    ttl.maxDurationSec = jnReq['maxDurationSec']
    ttl.description = jnReq['description']    
    return json.dumps(ttl.__dict__) if tt.change(ttl) else ('internal error', 500)
  except Exception as err:
    print(f'/taskTemplates/{id} PUT {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/taskTemplates/<int:id>', methods=(['DELETE']))
@auth.loginRequired
def delTaskTemplate(id : int):  
  try:
    return ('ok', 200) if tt.delete(id) else ('bad request', 400)  
  except Exception as err:
    print(f'/taskTemplates/{id} DELETE failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/taskTemplates', methods=(['GET']))
@auth.loginRequired
def allTaskTemplates():  
  ret = []
  for t in tt.all():
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
  # for p in _zmCommon.getAllPipelines(g.userId):
  #   for t in _zmCommon.getAllPipelineTasks(p.id):
  #     t.state = t.state.value
  #     ret.append(t.__dict__)
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