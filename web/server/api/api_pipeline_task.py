import json
from flask import request

from .. import auth 
from .. import pipeline_task as pt
from .api import bp 

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
    plt.isEnabled = int(jnReq['isEnabled'])
    plt.setts = jnReq['setts']
    plt.nextTasksId = jnReq['nextTasksId']
    plt.nextEventsId = jnReq['nextEventsId']  
    plt.prevTasksId = jnReq['prevTasksId']
    plt.prevEventsId = jnReq['prevEventsId']  
    plt.params = jnReq['params'] 
    plt.description = jnReq['description']  
    return json.dumps(plt.__dict__) if pt.add(plt) else ('internal error', 500)
  except Exception as err:
    print(f'/pipelineTasks POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/pipelineTasks/<int:id>', methods=(['PUT']))
@auth.loginRequired
def changePipelineTask(id : int):
  try:
    jnReq = request.get_json(silent=True)
  
    plt = pt.PipelineTask()
    plt.id = id
    plt.name = jnReq['name']
    plt.pplId = int(jnReq['pplId'])
    plt.ttId = int(jnReq['ttId'])
    plt.isEnabled = int(jnReq['isEnabled'])
    plt.setts = jnReq['setts']
    plt.nextTasksId = jnReq['nextTasksId']
    plt.nextEventsId = jnReq['nextEventsId']  
    plt.prevTasksId = jnReq['prevTasksId']
    plt.prevEventsId = jnReq['prevEventsId']  
    plt.params = jnReq['params']  
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