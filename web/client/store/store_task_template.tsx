import { ITaskTemplate } from "../types"
import { makeObservable, observable, action } from "mobx"

///////////////////////////////////////////////////////////////
/// TaskTemplates

class TaskTemplatesStoreClass {
  m_taskTemplates : Map<number, ITaskTemplate>;  
  constructor() {
      makeObservable(this, {
        m_taskTemplates: observable,
        setAll: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_taskTemplates = new Map<number, ITaskTemplate>();
  }
  copy(evt : ITaskTemplate) : ITaskTemplate {
    return Object.assign({}, evt);   
  }
  getAll() : Map<number, ITaskTemplate>{
    return this.m_taskTemplates;
  }
  get(id : number) : ITaskTemplate {
    return this.m_taskTemplates.has(id) ? this.copy(this.m_taskTemplates.get(id)) : null;
  }
  getByName(name : string) : ITaskTemplate {
    for (let v of this.m_taskTemplates.values()){
      if (v.name == name) return this.copy(v);
    }
    return null;
  }
  setAll(ttl : Map<number, ITaskTemplate>){
    this.m_taskTemplates = ttl;
  }
  add(ttl : ITaskTemplate){
    this.m_taskTemplates.set(ttl.id, ttl);
  }
  del(id : number){
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