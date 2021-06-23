/* eslint-disable no-unused-vars */
import { IPipeline, IGroup, ITaskTemplate, ITask } from "../types"

export
enum ActionType{
  ADD_PIPELINE,
  CHANGE_PIPELINE,
  DEL_PIPELINE,
  FILL_PIPELINES,

  ADD_TASKGROUP,
  CHANGE_TASKGROUP,
  DEL_TASKGROUP,
  FILL_TASKGROUPS,

  ADD_TASKTEMPLATE,
  CHANGE_TASKTEMPLATE,
  DEL_TASKTEMPLATE,
  FILL_TASKTEMPLATES,

  ADD_TASK,
  CHANGE_TASK,
  START_TASK,
  STOP_TASK,
  DEL_TASK,
  FILL_TASKS,
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
function addTaskGroup(dispatch){
  return function(taskGroup : IGroup){
    dispatch({ type : ActionType.ADD_TASKGROUP, taskGroup });
  }
}
export
function changeTaskGroup(dispatch){
  return function(taskGroup : IGroup){
    dispatch({ type : ActionType.CHANGE_TASKGROUP, taskGroup });
  }
}
export
function delTaskGroup(dispatch){
  return function(taskGroup : IGroup){
    dispatch({ type : ActionType.DEL_TASKGROUP, taskGroup });
  }
}
export
function fillTaskGroups(dispatch){
  return function(groups : Array<IGroup>){
    dispatch({ type : ActionType.FILL_TASKGROUPS, allTaskGroups : groups });
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
function addTask(dispatch){
  return function(task : ITask){
    dispatch({ type : ActionType.ADD_TASK, task });
  }
}
export
function changeTask(dispatch){
  return function(task : ITask){
    dispatch({ type : ActionType.CHANGE_TASK, task });
  }
}
export
function delTask(dispatch){
  return function(task : ITask){
    dispatch({ type : ActionType.DEL_TASK, task });
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
export
function fillTasks(dispatch){
  return function(tasks : Array<ITask>){
    dispatch({ type : ActionType.FILL_TASKS, allTasks : tasks });
  }
}