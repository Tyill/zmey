import json
from flask import request

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
    task.ttlId = int(jnReq['ttlId'])

    return json.dumps(task.__dict__) if t.start(task) else ('internal error', 500)
  except Exception as err:
    print(f'/tasks POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)