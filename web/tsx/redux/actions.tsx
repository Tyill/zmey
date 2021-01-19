/* eslint-disable no-unused-vars */
import { IPipeline, ITaskGroup, ITaskTemplate, ITask } from "../types"

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
    dispatch({ type : EnumActions.FILL_PIPELINES, pipelines });
  }
}

export
function addTaskGroup(dispatch){
  return function(taskgroup : ITaskGroup){
    dispatch({ type : EnumActions.ADD_TASKGROUP, taskgroup });
  }
}
export
function changeTaskGroup(dispatch){
  return function(taskgroup : ITaskGroup){
    dispatch({ type : EnumActions.CHANGE_TASKGROUP, taskgroup });
  }
}
export
function delTaskGroup(dispatch){
  return function(taskgroup : ITaskGroup){
    dispatch({ type : EnumActions.DEL_TASKGROUP, taskgroup });
  }
}
export
function fillTaskGroups(dispatch){
  return function(taskgroups : Array<ITaskGroup>){
    dispatch({ type : EnumActions.FILL_TASKGROUPS, taskgroups });
  }
}

export
function addTaskTemplate(dispatch){
  return function(tasktemplate : ITaskTemplate){
    dispatch({ type : EnumActions.ADD_TASKTEMPLATE, tasktemplate });
  }
}
export
function changeTaskTemplate(dispatch){
  return function(tasktemplate : ITaskTemplate){
    dispatch({ type : EnumActions.CHANGE_TASKTEMPLATE, tasktemplate });
  }
}
export
function delTaskTemplate(dispatch){
  return function(tasktemplate : ITaskTemplate){
    dispatch({ type : EnumActions.DEL_TASKTEMPLATE, tasktemplate });
  }
}
export
function fillTaskTemplates(dispatch){
  return function(tasktemplates : Array<ITaskTemplate>){
    dispatch({ type : EnumActions.FILL_TASKTEMPLATES, tasktemplates });
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
    dispatch({ type : EnumActions.FILL_TASKS, tasks });
  }
}