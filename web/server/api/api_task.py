

import json
from flask import request

from .. import auth 
from .api import bp 

###############################################################################
### Task object

@bp.route('/tasks', methods=(['GET']))
@auth.loginRequired
def startTask():
  # if _zmCommon.startTask(tId):
  #   _zmTaskWatch.setChangeTaskStateCBack(tId, hChangeTask)
  #   return True
  # return False
  return None

@bp.route('/stopTask')
@auth.loginRequired
def stopTask():
  return None#_zmCommon.stopTask(tId)