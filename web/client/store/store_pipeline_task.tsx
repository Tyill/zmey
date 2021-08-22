import { IPipelineTask, IRect, IPoint } from "../types"
import { makeObservable, observable, action } from "mobx"

///////////////////////////////////////////////////////////////
/// PipelineTasks

class PipelineTasksStoreClass {
  m_pipelineTasks : Map<number, IPipelineTask>;  
  constructor() {
      makeObservable(this, {
        m_pipelineTasks: observable,
        setAll: action,
        setPosition: action,
        setSockets: action,
        setVisible: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_pipelineTasks = new Map<number, IPipelineTask>();
  }
  copy(pt : IPipelineTask) : IPipelineTask {
    let ret = Object.assign({}, pt);
    ret.setts = {...pt.setts};
    ret.setts.socketInPoint = {...pt.setts.socketInPoint};
    ret.setts.socketInRect = {...pt.setts.socketInRect};
    ret.setts.socketOutPoint = {...pt.setts.socketOutPoint};
    ret.setts.socketOutRect = {...pt.setts.socketOutRect};
    ret.nextTasksId = [...pt.nextTasksId];
    ret.prevTasksId = [...pt.prevTasksId];
    ret.nextEventsId = [...pt.nextEventsId]; 
    ret.prevEventsId = [...pt.prevEventsId]; 
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
    for (let v of this.m_pipelineTasks.values()){
      if (v.name == name) return this.copy(v);
    }
    return null;
  }
  getVisible(pplId) : Array<IPipelineTask>{
    let ret = [] as Array<IPipelineTask>;
    this.m_pipelineTasks.forEach(tt => {
      if ((tt.pplId == pplId) && tt.setts.isVisible){        
        ret.push(this.copy(tt));
      }
    })
    return ret;
  }
  setVisible(id : number, visible : boolean){
    if (this.m_pipelineTasks.has(id)){
      let ppt = this.m_pipelineTasks.get(id);
      ppt.setts.isVisible = visible;
      this.m_pipelineTasks.set(ppt.id, ppt);
    }
  }
  setPosition(ptId: number, posX : number, posY : number){
    if (this.m_pipelineTasks.has(ptId)){
      let ppt = this.m_pipelineTasks.get(ptId);
      ppt.setts.positionX = posX;
      ppt.setts.positionY = posY;
      this.m_pipelineTasks.set(ppt.id, ppt);
    }
  }
  setSockets(ptId: number, inputSocket : {point:IPoint, rect:IRect}, outputSocket : {point:IPoint, rect:IRect}){
    if (this.m_pipelineTasks.has(ptId)){
      let ppt = this.m_pipelineTasks.get(ptId);
      ppt.setts.socketInPoint = inputSocket.point;
      ppt.setts.socketInRect = inputSocket.rect;
      ppt.setts.socketOutPoint = outputSocket.point;
      ppt.setts.socketOutRect = outputSocket.rect;
      this.m_pipelineTasks.set(ppt.id, ppt);
    }
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
