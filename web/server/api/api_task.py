

import json
from flask import request

from .. import auth 
from .api import bp 

###############################################################################
### Task object

@bp.route('/tasks', methods=(['POST']))
@auth.loginRequired
def startTask():
  # if _zmCommon.startTask(tId):
  #   _zmTaskWatch.setChangeTaskStateCBack(tId, hChangeTask)
  #   return True
  # return False
  return None