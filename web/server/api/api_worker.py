import json
from flask import request

from .. import auth 
from .. import event as ev
from .api import bp 

###############################################################################
### Worker

@bp.route('/workers')
@auth.adminRequired
def allWorkers():
  return None