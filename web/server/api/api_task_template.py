import json
from flask import request

from .. import auth 
from .. import task_template as tt
from .api import bp 

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
