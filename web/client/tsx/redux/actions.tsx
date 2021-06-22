/* eslint-disable no-unused-vars */
import { IPipeline, IGroup, ITaskTemplate, ITask } from "../types"

export
enum EnumActions{
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
    dispatch({ type : EnumActions.ADD_PIPELINE, pipeline });
  }
}
export
function changePipeline(dispatch){
  return function(pipeline : IPipeline){
    dispatch({ type : EnumActions.CHANGE_PIPELINE, pipeline });
  }
}
export
function delPipeline(dispatch){
  return function(pipeline : IPipeline){
    dispatch({ type : EnumActions.DEL_PIPELINE, pipeline });
  }
}
export
function fillPipelines(dispatch){
  return function(pipelines : Array<IPipeline>){
    dispatch({ type : EnumActions.FILL_PIPELINES, allPipelines : pipelines });
  }
}

export
function addTaskGroup(dispatch){
  return function(taskGroup : IGroup){
    dispatch({ type : EnumActions.ADD_TASKGROUP, taskGroup });
  }
}
export
function changeTaskGroup(dispatch){
  return function(taskGroup : IGroup){
    dispatch({ type : EnumActions.CHANGE_TASKGROUP, taskGroup });
  }
}
export
function delTaskGroup(dispatch){
  return function(taskGroup : IGroup){
    dispatch({ type : EnumActions.DEL_TASKGROUP, taskGroup });
  }
}
export
function fillTaskGroups(dispatch){
  return function(groups : Array<IGroup>){
    dispatch({ type : EnumActions.FILL_TASKGROUPS, allTaskGroups : groups });
  }
}

export
function addTaskTemplate(dispatch){
  return function(taskTemplate : ITaskTemplate){
    dispatch({ type : EnumActions.ADD_TASKTEMPLATE, taskTemplate });
  }
}
export
function changeTaskTemplate(dispatch){
  return function(taskTemplate : ITaskTemplate){
    dispatch({ type : EnumActions.CHANGE_TASKTEMPLATE, taskTemplate });
  }
}
export
function delTaskTemplate(dispatch){
  return function(taskTemplate : ITaskTemplate){
    dispatch({ type : EnumActions.DEL_TASKTEMPLATE, taskTemplate });
  }
}
export
function fillTaskTemplates(dispatch){
  return function(taskTemplates : Array<ITaskTemplate>){
    dispatch({ type : EnumActions.FILL_TASKTEMPLATES, allTaskTemplates : taskTemplates });
  }
}

export
function addTask(dispatch){
  return function(task : ITask){
    dispatch({ type : EnumActions.ADD_TASK, task });
  }
}
export
function changeTask(dispatch){
  return function(task : ITask){
    dispatch({ type : EnumActions.CHANGE_TASK, task });
  }
}
export
function delTask(dispatch){
  return function(task : ITask){
    dispatch({ type : EnumActions.DEL_TASK, task });
  }
}
export
function startTask(dispatch){
  return function(task : ITask){
    dispatch({ type : EnumActions.START_TASK, task });
  }
}
export
function stopTask(dispatch){
  return function(task : ITask){
    dispatch({ type : EnumActions.STOP_TASK, task });
  }
}
export
function fillTasks(dispatch){
  return function(tasks : Array<ITask>){
    dispatch({ type : EnumActions.FILL_TASKS, allTasks : tasks });
  }
}