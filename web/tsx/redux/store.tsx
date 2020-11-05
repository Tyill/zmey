/* eslint-disable no-unused-vars */

import ComboReducer from "./reducers";
import { createStore } from 'redux'
import {IStoreType, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "../types"

const initialState : IStoreType = {
  user : { name : ''},
  pipelines : new Map<number, IPipeline>(),
  taskGroups : new Map<number, ITaskGroup>(),
  taskTemplates : new Map<number, ITaskTemplate>(),
  tasks : new Map<number, ITask>(),
};

const Store = createStore(ComboReducer, initialState);

export default Store; 