import { IPipeline, ITaskTemplate, IPipelineTask, IEvent } from "../types"
import { makeObservable, observable, action } from "mobx"

///////////////////////////////////////////////////////////////
/// Pipelines

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
  getByName(name : string) : IPipeline {
    for (const pp of this.m_pipelines){
      if (pp[1].name == name) return pp[1];
    }
    return null;
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
      this.m_pipelines.set(ppl.id, ppl);
  }
}
export
let Pipelines = new PipelinesStoreClass();

///////////////////////////////////////////////////////////////
/// TaskTemplates

class TaskTemplatesStoreClass {
  m_taskTemplates : Map<Number, ITaskTemplate>;  
  constructor() {
      makeObservable(this, {
        m_taskTemplates: observable,
        setAll: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_taskTemplates = new Map<Number, ITaskTemplate>();
  }
  getAll() : Map<Number, ITaskTemplate>{
    return this.m_taskTemplates;
  }
  get(id : Number) : ITaskTemplate {
    return this.m_taskTemplates.has(id) ? this.m_taskTemplates.get(id) : null;
  }
  getByName(name : string) : ITaskTemplate {
    for (const tt of this.m_taskTemplates){
      if (tt[1].name == name) return tt[1];
    }
    return null;
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
    if (this.m_taskTemplates.has(ttl.id)){
      this.m_taskTemplates.set(ttl.id, ttl);
    }
  }
}
export
let TaskTemplates = new TaskTemplatesStoreClass();

///////////////////////////////////////////////////////////////
/// PipelineTasks

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
  getByPPlId(pplId : Number) : Array<IPipelineTask> {
    let ret = [] as Array<IPipelineTask>;
    this.m_pipelineTasks.forEach(tt => {
      if (tt.pplId == pplId){        
        ret.push(tt);
      }
    })
    return ret;
  }
  getByName(name : string) : IPipelineTask {
    for (const pt of this.m_pipelineTasks){
      if (pt[1].name == name) return pt[1];
    }
    return null;
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
      this.m_pipelineTasks.set(ppt.id, ppt);
  }
}
export
let PipelineTasks = new PipelineTasksStoreClass();

///////////////////////////////////////////////////////////////
/// Events

class EventsStoreClass {
  m_events : Map<Number, IEvent>;  
  constructor() {
      makeObservable(this, {
        m_events: observable,
        setAll: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_events = new Map<Number, IEvent>();
  }
  getAll() : Map<Number, IEvent>{
    return this.m_events;
  }
  get(id : Number) : IEvent {
    return this.m_events.has(id) ? this.m_events.get(id) : null;
  }
  getByName(name : string) : IEvent {
    for (const ev of this.m_events){
      if (ev[1].name == name) return ev[1];
    }
    return null;
  }
  setAll(ev : Map<Number, IEvent>){
    this.m_events = ev;
  }
  add(ev : IEvent){
    this.m_events.set(ev.id, ev);
  }
  del(id : Number){
    this.m_events.delete(id);
  }
  upd(ev : IEvent){
    if (this.m_events.has(ev.id))
      this.m_events.set(ev.id, ev);
  }
}
export
let Events = new EventsStoreClass();