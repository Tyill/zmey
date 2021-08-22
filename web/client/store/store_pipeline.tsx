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
        setSelected: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_pipelines = new Map<number, IPipeline>();
  }
  copy(p : IPipeline) : IPipeline {
    let ret = Object.assign({}, p);
    ret.setts = {...p.setts};
    return ret;   
  }
  getAll() : Map<number, IPipeline>{
    return this.m_pipelines;
  }
  get(id : number) : IPipeline {
    return this.m_pipelines.has(id) ? this.copy(this.m_pipelines.get(id)) : null;
  }
  getByName(name : string) : IPipeline {
    for (let v of this.m_pipelines.values()){
      if (v.name == name) return this.copy(v);
    }
    return null;
  }
  getSelected() : Array<IPipeline>{
    let ret = [] as Array<IPipeline>;
    this.m_pipelines.forEach(p => {
      if (p.setts.isSelected){        
        ret.push(this.copy(p));
      }
    })
    return ret;
  }  
  setSelected(id : number, on : boolean){
    if (this.m_pipelines.has(id)){
      let ppl = this.m_pipelines.get(id);
      ppl.setts.isSelected = on;
      this.m_pipelines.set(ppl.id, ppl);    
    }
  }
  getVisible() : Array<IPipeline>{
    let ret = [] as Array<IPipeline>;
    this.m_pipelines.forEach(p => {
      if (p.setts.isVisible){        
        ret.push(this.copy(p));
      }
    })
    return ret;
  }
  setVisible(id : number, on : boolean){
    if (this.m_pipelines.has(id)){
      let ppl = this.m_pipelines.get(id);
      ppl.setts.isVisible = on;
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