import ComboReducer from "./reducers";
import { createStore } from 'redux'
import {IStoreType, IPipeline, ITaskTemplate, ITaskPipeline } from "../types"

const initialState : IStoreType = {
  user : { id : 0, name : '', description : ''},
  pipelines : new Map<number, IPipeline>(),
  taskTemplates : new Map<number, ITaskTemplate>(),
  taskPipeline : new Map<number, ITaskPipeline>(),
};

const Store = createStore(ComboReducer, initialState);

export default Store; 