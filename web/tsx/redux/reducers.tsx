/* eslint-disable no-unused-vars */

import { EnumActions } from "./actions";
import { combineReducers } from 'redux'
import {IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "../types"


function user(curUser : IUser, action : {type : EnumActions, user : IUser}) :
  IUser{

  return {name : 'alm'};
}

function pipelines(curPipelines : Map<number, IPipeline>, action : {type : EnumActions, pipeline : IPipeline, allPipelines : Array<IPipeline>}) : 
  Map<number, IPipeline>{

    if (!curPipelines) return new Map<number, IPipeline>();

    let curPipelinesCpy = Object.assign({}, curPipelines);  
    switch (action.type) {
      case EnumActions.ADD_PIPELINE:
      case EnumActions.CHANGE_PIPELINE:
        curPipelinesCpy.set(action.pipeline.id, action.pipeline);
        break;
      case EnumActions.DEL_PIPELINE:
        break;  
      case EnumActions.FILL_PIPELINES:
        for (let p of action.allPipelines)
          curPipelinesCpy.set(p.id, p);
        break;  
      default:
        curPipelinesCpy = new Map<number, IPipeline>();
        break;
    }
    return curPipelinesCpy; 
}

function taskGroups(curTaskGroups : Map<number, ITaskGroup>, action : {type : EnumActions, taskGroup : ITaskGroup, allTaskGroups : Array<ITaskGroup>}) : 
  Map<number, ITaskGroup>{

    if (!curTaskGroups) return new Map<number, ITaskGroup>();

    let taskGroupsCpy = Object.assign({}, curTaskGroups);  
    switch (action.type) {
      case EnumActions.ADD_TASKGROUP:
      case EnumActions.CHANGE_TASKGROUP:
        taskGroupsCpy.set(action.taskGroup.id, action.taskGroup);
        break;
      case EnumActions.DEL_TASKGROUP:
        break;  
      case EnumActions.FILL_TASKGROUPS:
        for (let t of action.allTaskGroups)
          taskGroupsCpy.set(t.id, t);
        break;  
      default:
        taskGroupsCpy = new Map<number, ITaskGroup>();
        break;
    }
    return taskGroupsCpy; 
}

function taskTemplates(curTaskTemplates : Map<number, ITaskTemplate>, action : {type : EnumActions, taskTemplate : ITaskTemplate, allTaskTemplates : Array<ITaskTemplate>}) :
  Map<number, ITaskTemplate>{
  
  if (!curTaskTemplates) return new Map<number, ITaskTemplate>();

  let taskTemplatesCpy = Object.assign({}, curTaskTemplates);  
  switch (action.type) {
    case EnumActions.ADD_TASKTEMPLATE:
    case EnumActions.CHANGE_TASKTEMPLATE:
      taskTemplatesCpy.set(action.taskTemplate.id, action.taskTemplate);
      break;
    case EnumActions.DEL_TASKTEMPLATE:
      break;  
    case EnumActions.FILL_TASKTEMPLATES:
      for (let t of action.allTaskTemplates)
        taskTemplatesCpy.set(t.id, t);
      break;  
    default:
      taskTemplatesCpy = new Map<number, ITaskTemplate>();
      break;
  }
  return taskTemplatesCpy; 
}

function tasks(curTasks : Map<number, ITask>, action : {type : EnumActions, task : ITask, allTasks : Array<ITask>}) : 
  Map<number, ITask>{

    if (!curTasks) return new Map<number, ITask>();

    let tasksCpy = Object.assign({}, curTasks);  
    switch (action.type) {
      case EnumActions.ADD_TASK:
      case EnumActions.CHANGE_TASK:
        tasksCpy.set(action.task.id, action.task);
        break;
      case EnumActions.DEL_TASK:
        break;  
      case EnumActions.FILL_TASKS:
        for (let t of action.allTasks)
          tasksCpy.set(t.id, t);
        break;  
      default:
        tasksCpy = new Map<number, ITask>();
        break;
    }
    return tasksCpy; 
}

const ComboReducer = combineReducers({user, pipelines, taskGroups, taskTemplates, tasks });

export default ComboReducer;