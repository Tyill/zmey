import json
from flask import request

from .. import auth 
from .. import worker as wr
from .api import bp 

###############################################################################
### Worker

@bp.route('/workers', methods=(['POST']))
@auth.adminRequired
def addWorker():
  try: 
    jnReq = request.get_json(silent=True) 

    worker = wr.Worker()
    worker.sId = int(jnReq['sId'])
    worker.name = jnReq['name']
    worker.description = jnReq['description']
    worker.capacityTask = int(jnReq['capacityTask'])
    worker.connectPnt = jnReq['connectPnt']
    
    return json.dumps(worker.__dict__) if wr.add(worker) else ('internal error', 500)
  except Exception as err:
    print(f'/workers POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/workers/<int:id>', methods=(['PUT']))
@auth.adminRequired
def changeWorker(id : int):
  try:
    jnReq = request.get_json(silent=True)
    
    worker = wr.Worker()
    worker.id = id
    worker.sId = int(jnReq['sId'])
    worker.name = jnReq['name']
    worker.description = jnReq['description']
    worker.capacityTask = int(jnReq['capacityTask'])
    worker.connectPnt = jnReq['connectPnt']

    return json.dumps(worker.__dict__) if wr.change(worker) else ('internal error', 500)
  except Exception as err:
    print(f'/workers/{id} PUT {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/workers/<int:id>', methods=(['DELETE']))
@auth.adminRequired
def delWorker(id : int):
  try:
    return ('ok', 200) if wr.delete(id) else ('bad request', 400)  
  except Exception as err:
    print(f'/workers/{id} DELETE failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/workers', methods=(['GET']))
@auth.adminRequired
def allWorkers():
  try:
    ret = []
    for p in wr.all(int(request.args.get('schedId'))):
      ret.append(p.__dict__)
    return json.dumps(ret) 
  except Exception as err:
    print(f'/workers GET failed: %s' % str(err))
    return ('bad request', 400)
  