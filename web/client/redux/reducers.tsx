import { ActionType } from "./actions";
import { combineReducers } from 'redux'
import {IUser, IPipeline, ITaskTemplate, ITaskPipeline } from "../types"


function user(curUser : IUser, action : {type : ActionType, user : IUser}) :
  IUser{

  return {id : 0, name : 'alm', description : ''};
}

function pipelines(curPipelines : Map<number, IPipeline>, action : {type : ActionType, pipeline : IPipeline, allPipelines : Array<IPipeline>}) : 
  Map<number, IPipeline>{

    if (!curPipelines) return new Map<number, IPipeline>();
        
    let pipelinesCpy = new Map<number, IPipeline>();   
    if (curPipelines instanceof Map){ 
      for (let t of curPipelines)
        pipelinesCpy.set(t[0], t[1]);  
    }

    switch (action.type) {
      case ActionType.ADD_PIPELINE:
      case ActionType.CHANGE_PIPELINE:
        pipelinesCpy.set(action.pipeline.id, action.pipeline);
        break;
      case ActionType.DEL_PIPELINE:
        pipelinesCpy.delete(action.pipeline.id);
        break;  
      case ActionType.FILL_PIPELINES:
        for (let p of action.allPipelines)
          pipelinesCpy.set(p.id, p);
        break;  
    }
    return pipelinesCpy; 
}

function taskTemplates(curTaskTemplates : Map<number, ITaskTemplate>, action : {type : ActionType, taskTemplate : ITaskTemplate, allTaskTemplates : Array<ITaskTemplate>}) :
  Map<number, ITaskTemplate>{
  
  if (!curTaskTemplates) return new Map<number, ITaskTemplate>();
  
  let taskTemplatesCpy = new Map<number, ITaskTemplate>();   
  if (curTaskTemplates instanceof Map){ 
    for (let t of curTaskTemplates)
      taskTemplatesCpy.set(t[0], t[1]);  
  }
  
  switch (action.type) {
    case ActionType.ADD_TASKTEMPLATE:
    case ActionType.CHANGE_TASKTEMPLATE:
      taskTemplatesCpy.set(action.taskTemplate.id, action.taskTemplate);
      break;
    case ActionType.DEL_TASKTEMPLATE:
      taskTemplatesCpy.delete(action.taskTemplate.id);
      break;  
    case ActionType.FILL_TASKTEMPLATES:      
      for (let t of action.allTaskTemplates)
        taskTemplatesCpy.set(t.id, t); 
      break;
  }  
  return taskTemplatesCpy; 
}

function taskPipeline(curTasks : Map<number, ITaskPipeline>, action : {type : ActionType, task : ITaskPipeline, allTasks : Array<ITaskPipeline>}) : 
  Map<number, ITaskPipeline>{

    if (!curTasks) return new Map<number, ITaskPipeline>();

    let tasksCpy = new Map<number, ITaskPipeline>();   
    if (curTasks instanceof Map){ 
      for (let t of curTasks)
        tasksCpy.set(t[0], t[1]);  
    }

    switch (action.type) {
      case ActionType.ADD_TASKPIPELINE:
      case ActionType.CHANGE_TASKPIPELINE:
        tasksCpy.set(action.task.id, action.task);
        break;
      case ActionType.DEL_TASKPIPELINE:
        tasksCpy.delete(action.task.id);
        break;  
      case ActionType.FILL_TASKPIPELINE:
        for (let t of action.allTasks)
          tasksCpy.set(t.id, t);
        break;  
    }
    return tasksCpy; 
}

const ComboReducer = combineReducers({user, pipelines, taskTemplates, taskPipeline });

export default ComboReducer;