import json
from flask import request

from .. import auth 
from .. import scheduler as sh
from .api import bp 

###############################################################################
### Scheduler

@bp.route('/schedulers', methods=(['POST']))
@auth.adminRequired
def addScheduler():
  try: 
    jnReq = request.get_json(silent=True) 

    sched = sh.Scheduler()
    sched.name = jnReq['name']
    sched.description = jnReq['description']
    sched.capacityTask = int(jnReq['capacityTask'])
    sched.connectPnt = jnReq['connectPnt']
    
    return json.dumps(sched.__dict__) if sh.add(sched) else ('internal error', 500)
  except Exception as err:
    print(f'/schedulers POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/schedulers/<int:id>', methods=(['PUT']))
@auth.adminRequired
def changeScheduler(id : int):
  try:
    jnReq = request.get_json(silent=True)
    
    sched = sh.Scheduler()
    sched.id = id
    sched.name = jnReq['name']
    sched.description = jnReq['description']
    sched.capacityTask = int(jnReq['capacityTask'])
    sched.connectPnt = jnReq['connectPnt']

    return json.dumps(sched.__dict__) if sh.change(sched) else ('internal error', 500)
  except Exception as err:
    print(f'/schedulers/{id} PUT {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/schedulers/<int:id>', methods=(['DELETE']))
@auth.adminRequired
def delScheduler(id : int):
  try:
    return ('ok', 200) if sh.delete(id) else ('bad request', 400)  
  except Exception as err:
    print(f'/schedulers/{id} DELETE failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/schedulers', methods=(['GET']))
@auth.adminRequired
def allSchedulers():
  try:
    ret = []
    for p in sh.all():
      ret.append(p.__dict__)
    return json.dumps(ret) 
  except Exception as err:
    print(f'/schedulers GET failed: %s' % str(err))
    return ('bad request', 400)

