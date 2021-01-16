/* eslint-disable no-unused-vars */

import { EnumActions } from "./actions.jsx";
import { combineReducers } from 'redux'
import {IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "../types"


function user(curUser : IUser, action : {type : EnumActions, user : IUser}) :
  IUser{

  return {name : ''};
}

function pipelines(curPipelines : Map<number, IPipeline>, action : {type : EnumActions, pipeline : IPipeline}) : 
  Map<number, IPipeline>{

  return new Map<number, IPipeline>();
}

function taskGroups(curTaskGroups : Map<number, ITaskGroup>, action : {type : EnumActions, taskGroup : ITaskGroup}) : 
  Map<number, ITaskGroup>{

  return new Map<number, ITaskGroup>();
}

function taskTemplates(curTaskTemplates : Map<number, ITaskTemplate>, action : {type : EnumActions, taskTemplate : ITaskTemplate}) :
  Map<number, ITaskTemplate>{

  let taskTemplatesCpy = Object.assign({}, curTaskTemplates);  
  switch (action.type) {
    case EnumActions.ADD_TASKTEMPLATE:
    case EnumActions.CHANGE_TASKTEMPLATE:
      taskTemplatesCpy.set(action.taskTemplate.id, action.taskTemplate);
      break;
    case EnumActions.DEL_TASKTEMPLATE:
      break;    
  }
  return taskTemplatesCpy; 
}

function tasks(curState : Map<number, ITask>, action : EnumActions) : 
  Map<number, ITask>{

  return new Map<number, ITask>();
}

const ComboReducer = combineReducers({user, pipelines, taskGroups, taskTemplates, tasks });

export default ComboReducer;