import json
from flask import request

from .. import auth 
from .. import event as ev
from .api import bp 

###############################################################################
### Scheduler

@bp.route('/schedulers')
@auth.adminRequired
def allSchedrs():
  return None