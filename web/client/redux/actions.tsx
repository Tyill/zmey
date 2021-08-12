import { IPipeline, ITaskTemplate, ITask, ITaskPipeline } from "../types"

export
enum ActionType{
  ADD_PIPELINE,
  CHANGE_PIPELINE,
  DEL_PIPELINE,
  FILL_PIPELINES,
  
  ADD_TASKTEMPLATE,
  CHANGE_TASKTEMPLATE,
  DEL_TASKTEMPLATE,
  FILL_TASKTEMPLATES,

  ADD_TASKPIPELINE,
  CHANGE_TASKPIPELINE,
  DEL_TASKPIPELINE,
  FILL_TASKPIPELINE,

  START_TASK,
  STOP_TASK,
}

export
function addPipeline(dispatch){
  return function(pipeline : IPipeline){
    dispatch({ type : ActionType.ADD_PIPELINE, pipeline });
  }
}
export
function changePipeline(dispatch){
  return function(pipeline : IPipeline){
    dispatch({ type : ActionType.CHANGE_PIPELINE, pipeline });
  }
}
export
function delPipeline(dispatch){
  return function(pipeline : IPipeline){
    dispatch({ type : ActionType.DEL_PIPELINE, pipeline });
  }
}
export
function fillPipelines(dispatch){
  return function(pipelines : Array<IPipeline>){
    dispatch({ type : ActionType.FILL_PIPELINES, allPipelines : pipelines });
  }
}

export
function addTaskTemplate(dispatch){
  return function(taskTemplate : ITaskTemplate){
    dispatch({ type : ActionType.ADD_TASKTEMPLATE, taskTemplate });
  }
}
export
function changeTaskTemplate(dispatch){
  return function(taskTemplate : ITaskTemplate){
    dispatch({ type : ActionType.CHANGE_TASKTEMPLATE, taskTemplate });
  }
}
export
function delTaskTemplate(dispatch){
  return function(taskTemplate : ITaskTemplate){
    dispatch({ type : ActionType.DEL_TASKTEMPLATE, taskTemplate });
  }
}
export
function fillTaskTemplates(dispatch){
  return function(taskTemplates : Array<ITaskTemplate>){
    dispatch({ type : ActionType.FILL_TASKTEMPLATES, allTaskTemplates : taskTemplates });
  }
}

export
function addTaskPipeline(dispatch){
  return function(task : ITaskPipeline){
    dispatch({ type : ActionType.ADD_TASKPIPELINE, task });
  }
}
export
function changeTaskPipeline(dispatch){
  return function(task : ITaskPipeline){
    dispatch({ type : ActionType.CHANGE_TASKPIPELINE, task });
  }
}
export
function delTaskPipeline(dispatch){
  return function(task : ITaskPipeline){
    dispatch({ type : ActionType.DEL_TASKPIPELINE, task });
  }
}
export
function fillTaskPipeline(dispatch){
  return function(tasks : Array<ITaskPipeline>){
    dispatch({ type : ActionType.FILL_TASKPIPELINE, allTasks : tasks });
  }
}

export
function startTask(dispatch){
  return function(task : ITask){
    dispatch({ type : ActionType.START_TASK, task });
  }
}
export
function stopTask(dispatch){
  return function(task : ITask){
    dispatch({ type : ActionType.STOP_TASK, task });
  }
}
