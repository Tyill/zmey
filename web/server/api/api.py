from flask import(
  Blueprint
)
 
bp = Blueprint('api', __name__, url_prefix='/api/v1')

from . import api_event
from . import api_pipeline
from . import api_pipeline_task
from . import api_schedr
from . import api_worker
from . import api_task
from . import api_task_template