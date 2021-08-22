import { IEvent } from "../types"
import { makeObservable, observable, action } from "mobx"

///////////////////////////////////////////////////////////////
/// Events

class EventsStoreClass {
  m_events : Map<number, IEvent>;  
  constructor() {
      makeObservable(this, {
        m_events: observable,
        setAll: action,
        add: action,
        del: action,
        upd: action,
      });
      this.m_events = new Map<number, IEvent>();
  }
  copy(evt : IEvent) : IEvent {
    let ret = Object.assign({}, evt);
    ret.setts = {...ret.setts};
    ret.nextTasksId = [...evt.nextTasksId];
    ret.prevTasksId = [...evt.prevTasksId];
    ret.nextEventsId = [...evt.nextEventsId]; 
    ret.prevEventsId = [...evt.prevEventsId]; 
    return ret;   
  }
  getAll() : Map<number, IEvent>{
    return this.m_events;
  }
  get(id : number) : IEvent {
    return this.m_events.has(id) ? this.copy(this.m_events.get(id)) : null;
  }
  getByName(name : string) : IEvent {
    for (let v of this.m_events.values()){
      if (v.name == name) return this.copy(v);
    }
    return null;
  }
  setAll(ev : Map<number, IEvent>){
    this.m_events = ev;
  }
  add(ev : IEvent){
    this.m_events.set(ev.id, ev);
  }
  del(id : number){
    this.m_events.delete(id);
  }
  upd(ev : IEvent){
    if (this.m_events.has(ev.id))
      this.m_events.set(ev.id, ev);
  }
}
export
let Events = new EventsStoreClass();