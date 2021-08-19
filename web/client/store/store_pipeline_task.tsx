import { IPipelineTask } from "../types"
import { makeObservable, observable, action } from "mobx"

///////////////////////////////////////////////////////////////
/// PipelineTasks

class PipelineTasksStoreClass {
  m_pipelineTasks : Map<number, IPipelineTask>;  
  constructor() {
      makeObservable(this, {
        m_pipelineTasks: observable,
        setAll: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_pipelineTasks = new Map<number, IPipelineTask>();
  }
  copy(pt : IPipelineTask) : IPipelineTask {
    let ret = Object.assign({}, pt);
    ret.nextTasksId = [...pt.nextTasksId];
    ret.nextEventsId = [...pt.nextEventsId]; 
    return ret;   
  }
  getAll() : Map<number, IPipelineTask>{
    return this.m_pipelineTasks;
  }
  get(id : number) : IPipelineTask {
    return this.m_pipelineTasks.has(id) ? this.copy(this.m_pipelineTasks.get(id)) : null;
  }
  getByPPlId(pplId : number) : Array<IPipelineTask> {
    let ret = [] as Array<IPipelineTask>;
    this.m_pipelineTasks.forEach(tt => {
      if (tt.pplId == pplId){        
        ret.push(this.copy(tt));
      }
    })
    return ret;
  }
  getByName(name : string) : IPipelineTask {
    this.m_pipelineTasks.forEach(pt => {
      if (pt.name == name){        
        return this.copy(pt);
      }
    })
    return null;
  }
  setAll(ppt : Map<number, IPipelineTask>){
    this.m_pipelineTasks = ppt;
  }
  add(ppt : IPipelineTask){
    this.m_pipelineTasks.set(ppt.id, ppt);
  }
  del(id : number){
    this.m_pipelineTasks.delete(id);
  }
  upd(ppt : IPipelineTask){
    if (this.m_pipelineTasks.has(ppt.id))
      this.m_pipelineTasks.set(ppt.id, ppt);
  }
}
export
let PipelineTasks = new PipelineTasksStoreClass();
