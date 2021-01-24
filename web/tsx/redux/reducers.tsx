/* eslint-disable no-unused-vars */

import { EnumActions } from "./actions";
import { combineReducers } from 'redux'
import {IUser, IPipeline, IGroup, ITaskTemplate, ITask } from "../types"


function user(curUser : IUser, action : {type : EnumActions, user : IUser}) :
  IUser{

  return {name : 'alm', description : ''};
}

function pipelines(curPipelines : Map<number, IPipeline>, action : {type : EnumActions, pipeline : IPipeline, allPipelines : Array<IPipeline>}) : 
  Map<number, IPipeline>{

    if (!curPipelines) return new Map<number, IPipeline>();
        
    let pipelinesCpy = new Map<number, IPipeline>();   
    if (curPipelines instanceof Map){ 
      for (let t of curPipelines)
        pipelinesCpy.set(t[0], t[1]);  
    }

    switch (action.type) {
      case EnumActions.ADD_PIPELINE:
      case EnumActions.CHANGE_PIPELINE:
        pipelinesCpy.set(action.pipeline.id, action.pipeline);
        break;
      case EnumActions.DEL_PIPELINE:
        pipelinesCpy.delete(action.pipeline.id);
        break;  
      case EnumActions.FILL_PIPELINES:
        for (let p of action.allPipelines)
          pipelinesCpy.set(p.id, p);
        break;  
    }
    return pipelinesCpy; 
}

function groups(curTaskGroups : Map<number, IGroup>, action : {type : EnumActions, taskGroup : IGroup, allTaskGroups : Array<IGroup>}) : 
  Map<number, IGroup>{

    if (!curTaskGroups) return new Map<number, IGroup>();

    let taskGroupsCpy = new Map<number, IGroup>();   
    if (curTaskGroups instanceof Map){ 
      for (let t of curTaskGroups)
        taskGroupsCpy.set(t[0], t[1]);  
    }   
    
    switch (action.type) {
      case EnumActions.ADD_TASKGROUP:
      case EnumActions.CHANGE_TASKGROUP:
        taskGroupsCpy.set(action.taskGroup.id, action.taskGroup);
        break;
      case EnumActions.DEL_TASKGROUP:
        taskGroupsCpy.delete(action.taskGroup.id);
        break;  
      case EnumActions.FILL_TASKGROUPS:
        for (let t of action.allTaskGroups)
          taskGroupsCpy.set(t.id, t);
        break;
    }
    return taskGroupsCpy; 
}

function taskTemplates(curTaskTemplates : Map<number, ITaskTemplate>, action : {type : EnumActions, taskTemplate : ITaskTemplate, allTaskTemplates : Array<ITaskTemplate>}) :
  Map<number, ITaskTemplate>{
  
  if (!curTaskTemplates) return new Map<number, ITaskTemplate>();
  
  let taskTemplatesCpy = new Map<number, ITaskTemplate>();   
  if (curTaskTemplates instanceof Map){ 
    for (let t of curTaskTemplates)
      taskTemplatesCpy.set(t[0], t[1]);  
  }
  
  switch (action.type) {
    case EnumActions.ADD_TASKTEMPLATE:
    case EnumActions.CHANGE_TASKTEMPLATE:
      taskTemplatesCpy.set(action.taskTemplate.id, action.taskTemplate);
      break;
    case EnumActions.DEL_TASKTEMPLATE:
      taskTemplatesCpy.delete(action.taskTemplate.id);
      break;  
    case EnumActions.FILL_TASKTEMPLATES:      
      for (let t of action.allTaskTemplates)
        taskTemplatesCpy.set(t.id, t); 
      break;
  }  
  return taskTemplatesCpy; 
}

function tasks(curTasks : Map<number, ITask>, action : {type : EnumActions, task : ITask, allTasks : Array<ITask>}) : 
  Map<number, ITask>{

    if (!curTasks) return new Map<number, ITask>();

    let tasksCpy = new Map<number, ITask>();   
    if (curTasks instanceof Map){ 
      for (let t of curTasks)
        tasksCpy.set(t[0], t[1]);  
    }

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
    }
    return tasksCpy; 
}

const ComboReducer = combineReducers({user, pipelines, groups, taskTemplates, tasks });

export default ComboReducer;