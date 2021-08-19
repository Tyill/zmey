import { IPipeline } from "../types"
import { makeObservable, observable, action } from "mobx"

///////////////////////////////////////////////////////////////
/// Pipelines

class PipelinesStoreClass {
  m_pipelines : Map<number, IPipeline>;  
  constructor() {
      makeObservable(this, {
        m_pipelines: observable,
        setAll: action,
        setVisible: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_pipelines = new Map<number, IPipeline>();
  }
  copy(evt : IPipeline) : IPipeline {
    return Object.assign({}, evt);   
  }
  getAll() : Map<number, IPipeline>{
    return this.m_pipelines;
  }
  getVisibleId() : Array<number>{
    let ppls = [];
    this.m_pipelines.forEach((p)=>{
      if (p.isVisible) ppls.push(p.id);
    });
    return ppls;
  }
  get(id : number) : IPipeline {
    return this.m_pipelines.has(id) ? this.copy(this.m_pipelines.get(id)) : null;
  }
  getByName(name : string) : IPipeline {
    this.m_pipelines.forEach((v)=>{
      if (v.name == name) return this.copy(v);
    })
    return null;
  }
  setVisible(id : number, visible : boolean){
    if (this.m_pipelines.has(id)){
      let ppl = this.m_pipelines.get(id);
      ppl.isVisible = visible ? 1 : 0;
      this.m_pipelines.set(ppl.id, ppl);
    }
  }
  setAll(ppl : Map<number, IPipeline>){
    this.m_pipelines = ppl;
  }
  add(ppl : IPipeline){
    this.m_pipelines.set(ppl.id, ppl);
  }
  del(id : number){
    this.m_pipelines.delete(id);
  }
  upd(ppl : IPipeline){
    if (this.m_pipelines.has(ppl.id))
      this.m_pipelines.set(ppl.id, ppl);
  }
}
export
let Pipelines = new PipelinesStoreClass();