import json
from flask import request

from .. import auth 
from .. import user
from .api import bp 
from flask import g

###############################################################################
### User

@bp.route('/users', methods=(['GET']))
@auth.loginRequired
def getUser():
  try: 
   
    usr = user.User()
    usr.name = g.userName

    return json.dumps(usr.__dict__)
  except Exception as err:
    print(f'/users GET failed: %s' % str(err))
    return ('bad request', 400)
