import json
from flask import request

from .. import auth 
from .. import event as ev
from .api import bp 

###############################################################################
### Events

@bp.route('/events', methods=(['POST']))
@auth.loginRequired
def addEvent():
  try: 
    jnReq = request.get_json(silent=True)  

    evt = ev.Event()
    evt.name = jnReq['name']
    evt.isEnabled = int(jnReq['isEnabled'])
    evt.timeStartEverySec = int(jnReq['timeStartEverySec'])
    evt.timeStartOnceOfDay = jnReq['timeStartOnceOfDay']
    evt.tasksForStart = jnReq['tasksForStart']
    
    evt.description = jnReq['description']
    return json.dumps(evt.__dict__) if ev.add(evt) else ('internal error', 500)
  except Exception as err:
    print(f'/events POST {request.get_json(silent=True)} failed: %s' % str(err))
    return ('bad request', 400)

@bp.route('/events/<int:id>', methods=(['PUT']))
@auth.loginRequired
def changeEvent(id : int):
  try:
    jnReq = request.get_json(silent=True)
    
    evt = ev.Event()
    evt.id = id
    evt.name = jnReq['name']
    evt.isEnabled = int(jnReq['isEnabled'])
    evt.timeStartEverySec = int(jnReq['timeStartEverySec'])
    evt.timeStartOnceOfDay = jnReq['timeStartOnceOfDay']
    evt.tasksForStart = jnReq['tasksForStart']
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