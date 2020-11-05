/* eslint-disable no-unused-vars */

import { EnumActions } from "./actions.jsx";
import { combineReducers } from 'redux'
import {IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "../types"


function user(curState : IUser, action : EnumActions) : IUser{
  return {name : ''};
}

function pipelines(curState : Map<number, IPipeline>, action : EnumActions) : Map<number, IPipeline>{
  return new Map<number, IPipeline>();
}

function taskGroups(curState : Map<number, ITaskGroup>, action : EnumActions) : Map<number, ITaskGroup>{
  return new Map<number, ITaskGroup>();
}

function taskTemplates(curState : Map<number, ITaskTemplate>, action : EnumActions) : Map<number, ITaskTemplate>{
  return new Map<number, ITaskTemplate>();
}

function tasks(curState : Map<number, ITask>, action : EnumActions) : Map<number, ITask>{
  return new Map<number, ITask>();
}

const ComboReducer = combineReducers({user, pipelines, taskGroups, taskTemplates, tasks });

export default ComboReducer;