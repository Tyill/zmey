import json
from flask import(
  g, request
)
from .. import auth 
from .. import task as t
from .api import bp 

###############################################################################
### Task object

@bp.route('/tasks', methods=(['POST']))
@auth.loginRequired
def startTask():
  try: 
    jnReq = request.get_json(silent=True)  

    task = t.Task()
    task.pplTaskId = int(jnReq['pplTaskId'])
    task.starterPplTaskId = int(jnReq['starterPplTaskId'])
    task.starterEventId = int(jnReq['starterEventId'])
    task.ttlId = int(jnReq['ttlId'])
    task.params = jnReq['params']

    return json.dumps(task.__dict__) if t.start(g.db, g.userId, task) else ('internal error', 500)
  except Exception as err:
    print(f'/tasks POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/tasks/<int:pplTaskid>', methods=(['GET']))
@auth.loginRequired
def getTaskState(pplTaskid : int):
  try:
    ret = []
    for st in t.getState(pplTaskid):
      ret.append(st.__dict__)
    return json.dumps(ret)
  except Exception as err:
    print(f'/tasks/{pplTaskid} GET {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400) 