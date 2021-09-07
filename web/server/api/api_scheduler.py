import json
from flask import request

from .. import auth 
from .. import scheduler as sh
from .api import bp 

###############################################################################
### Scheduler

@bp.route('/schedulers', methods=(['GET']))
@auth.adminRequired
def allSchedulers():
  ret = []
  for p in sh.all():
    ret.append(p.__dict__)
  return json.dumps(ret) 