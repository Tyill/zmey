import json
from flask import request

from .. import auth 
from .. import pipeline as pp
from .api import bp 


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
    ppl.setts = jnReq['setts']
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
    ppl.setts = jnReq['setts']
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
  return json.dumps(ret).replace("'", '"') 