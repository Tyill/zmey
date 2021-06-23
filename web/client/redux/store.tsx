/* eslint-disable no-unused-vars */

import ComboReducer from "./reducers";
import { createStore } from 'redux'
import {IStoreType, IPipeline, IGroup, ITaskTemplate, ITask } from "../types"

const initialState : IStoreType = {
  user : { id : 0, name : '', description : ''},
  pipelines : new Map<number, IPipeline>(),
  groups : new Map<number, IGroup>(),
  taskTemplates : new Map<number, ITaskTemplate>(),
  tasks : new Map<number, ITask>(),
};

const Store = createStore(ComboReducer, initialState);

export default Store; 