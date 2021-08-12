import {IStoreType, IPipeline, ITaskTemplate, IPipelineTask } from "../types"

const Store : IStoreType = {
  user : { id : 0, name : '', description : ''},
  pipelines : new Map<number, IPipeline>(),
  taskTemplates : new Map<number, ITaskTemplate>(),
  pipelineTasks : new Map<number, IPipelineTask>(),
};

export default Store;