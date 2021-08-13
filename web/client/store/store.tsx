import { IUser, IPipeline, ITaskTemplate, IPipelineTask } from "../types"
import { makeObservable, observable, action,computed } from "mobx"

class UserStoreClass {
  m_user : IUser;  
  constructor() {
      makeObservable(this, {
        m_user: observable,
        set: action,
      });
      this.m_user = { id : 0, name : '', description : ''};
  }
  get() {
    return this.m_user;
  }
  set(usr : IUser){
    this.m_user = usr;
  }
}
export
let User = new UserStoreClass();

///////////////////////////////////////////////////////////////

class PipelinesStoreClass {
  m_pipelines : Map<Number, IPipeline>;  
  constructor() {
      makeObservable(this, {
        m_pipelines: observable,
        setAll: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_pipelines = new Map<Number, IPipeline>();
  }
  getAll() : Map<Number, IPipeline>{
    return this.m_pipelines;
  }
  get(id : Number) : IPipeline {
    return this.m_pipelines.has(id) ? this.m_pipelines.get(id) : null;
  }
  setAll(ppl : Map<Number, IPipeline>){
    this.m_pipelines = ppl;
  }
  add(ppl : IPipeline){
    this.m_pipelines.set(ppl.id, ppl);
  }
  del(id : Number){
    this.m_pipelines.delete(id);
  }
  upd(ppl : IPipeline){
    if (this.m_pipelines.has(ppl.id))
      this.m_pipelines[ppl.id] = ppl;
  }
}
export
let Pipelines = new PipelinesStoreClass();

///////////////////////////////////////////////////////////////

class TaskTemplatesStoreClass {
  m_taskTemplates : Map<Number, ITaskTemplate>;  
  constructor() {
      makeObservable(this, {
        m_taskTemplates: observable,
        setAll: action,
        add: action,
        del: action,
        upd: action,
        count : computed,
      });
      this.m_taskTemplates = new Map<Number, ITaskTemplate>();
  }
  get count(){
    return this.m_taskTemplates.size;
  }
  getAll() : Map<Number, ITaskTemplate>{
    return this.m_taskTemplates;
  }
  get(id : Number) : ITaskTemplate {
    return this.m_taskTemplates.has(id) ? this.m_taskTemplates.get(id) : null;
  }
  setAll(ttl : Map<Number, ITaskTemplate>){
    this.m_taskTemplates = ttl;
  }
  add(ttl : ITaskTemplate){
    this.m_taskTemplates.set(ttl.id, ttl);
  }
  del(id : Number){
    this.m_taskTemplates.delete(id);
  }
  upd(ttl : ITaskTemplate){
    if (this.m_taskTemplates.has(ttl.id))
      this.m_taskTemplates[ttl.id] = ttl;
  }
}
export
let TaskTemplates = new TaskTemplatesStoreClass();

///////////////////////////////////////////////////////////////

class PipelineTasksStoreClass {
  m_pipelineTasks : Map<Number, IPipelineTask>;  
  constructor() {
      makeObservable(this, {
        m_pipelineTasks: observable,
        setAll: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_pipelineTasks = new Map<Number, IPipelineTask>();
  }
  getAll() : Map<Number, IPipelineTask>{
    return this.m_pipelineTasks;
  }
  get(id : Number) : IPipelineTask {
    return this.m_pipelineTasks.has(id) ? this.m_pipelineTasks.get(id) : null;
  }
  setAll(ppt : Map<Number, IPipelineTask>){
    this.m_pipelineTasks = ppt;
  }
  add(ppt : IPipelineTask){
    this.m_pipelineTasks.set(ppt.id, ppt);
  }
  del(id : Number){
    this.m_pipelineTasks.delete(id);
  }
  upd(ppt : IPipelineTask){
    if (this.m_pipelineTasks.has(ppt.id))
      this.m_pipelineTasks[ppt.id] = ppt;
  }
}
export
let PipelineTasks = new PipelineTasksStoreClass();