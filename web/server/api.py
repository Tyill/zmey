import json
from flask import(
  Blueprint, request
)

from . import (
  auth,
  task_template as tt,
  pipeline as pp,
  pipeline_task as pt,
  event as ev,
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
def addPipelineTask():
  try: 
    jnReq = request.get_json(silent=True)  

    plt = pt.PipelineTask()
    plt.name = jnReq['name']
    plt.pplId = int(jnReq['pplId'])
    plt.ttId = int(jnReq['ttId'])
    plt.nextTasksId = jnReq['nextTasksId']
    plt.nextEventsId = jnReq['nextEventsId']  
    plt.description = jnReq['description']  
    return json.dumps(plt.__dict__) if pt.add(plt) else ('internal error', 500)
  except Exception as err:
    print(f'/pipelineTasks POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/pipelineTasks/<int:id>', methods=(['UPDATE']))
@auth.loginRequired
def changePipelineTask(id : int):
  try:
    jnReq = request.get_json(silent=True)
  
    plt = pt.PipelineTask()
    plt.id = id
    plt.name = jnReq['name']
    plt.pplId = int(jnReq['pplId'])
    plt.ttId = int(jnReq['ttId'])
    plt.nextTasksId = jnReq['nextTasksId']
    plt.nextEventsId = jnReq['nextEventsId']  
    plt.description = jnReq['description']  
    return json.dumps(plt.__dict__) if pt.change(plt) else ('internal error', 500)
  except Exception as err:
    print(f'/pipelineTasks/{id} PUT {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/pipelineTasks/<int:id>', methods=(['DELETE']))
@auth.loginRequired
def delPipelineTask(id : int):
  try:
    return ('ok', 200) if pt.delete(id) else ('bad request', 400)  
  except Exception as err:
    print(f'/pipelineTasks/{id} DELETE failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/pipelineTasks', methods=(['GET']))
@auth.loginRequired
def allPipelineTasks():
  ret = []
  for t in pt.all():
    ret.append(t.__dict__)
  return json.dumps(ret)

###############################################################################
### Events

@bp.route('/events', methods=(['POST']))
@auth.loginRequired
def addEvent():
  try: 
    jnReq = request.get_json(silent=True)  

    evt = ev.Event()
    evt.name = jnReq['name']
    evt.nextTasksId = jnReq['nextTasksId']
    evt.nextEventsId = jnReq['nextEventsId']  
    evt.description = jnReq['description']  
    return json.dumps(evt.__dict__) if ev.add(evt) else ('internal error', 500)
  except Exception as err:
    print(f'/events POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/events/<int:id>', methods=(['UPDATE']))
@auth.loginRequired
def changeEvent(id : int):
  try:
    jnReq = request.get_json(silent=True)
  
    evt = ev.Event()
    evt.id = id
    evt.name = jnReq['name']
    evt.nextTasksId = jnReq['nextTasksId']
    evt.nextEventsId = jnReq['nextEventsId']  
    evt.description = jnReq['description']  
    return json.dumps(evt.__dict__) if ev.change(evt) else ('internal error', 500)
  except Exception as err:
    print(f'/changeEvent/{id} PUT {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/events/<int:id>', methods=(['DELETE']))
@auth.loginRequired
def delEvent(id : int):
  try:
    return ('ok', 200) if ev.delete(id) else ('bad request', 400)  
  except Exception as err:
    print(f'/events/{id} DELETE failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/events', methods=(['GET']))
@auth.loginRequired
def allEvents():
  ret = []
  for e in ev.all():
    ret.append(e.__dict__)
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