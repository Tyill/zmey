/* eslint-disable no-unused-vars */
import {IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "../types"

export
enum EnumActions{
  ADD_PIPELINE,
  CHANGE_PIPELINE,
  DEL_PIPELINE,
  ADD_TASKGROUP,
  CHANGE_TASKGROUP,
  DEL_TASKGROUP,
  ADD_TASKTEMPLATE,
  CHANGE_TASKTEMPLATE,
  DEL_TASKTEMPLATE,
  ADD_TASK,
  CHANGE_TASK,
  START_TASK,
  STOP_TASK,
  DEL_TASK,
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