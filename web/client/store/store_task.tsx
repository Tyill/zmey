import { ITask } from "../types"
import { makeObservable, observable, action } from "mobx"

///////////////////////////////////////////////////////////////
/// Task

class TaskStoreClass {
  m_pplTaskId : number;
  m_tasks : Array<ITask>;
  constructor() {
      makeObservable(this, {
        m_tasks: observable,
        m_pplTaskId: observable, 
        setAll: action,
        setPplTaskId: action,
      });
      this.m_tasks = new Array<ITask>();
      this.m_pplTaskId = 0;
  }
  getAll() : Array<ITask>{
    return this.m_tasks;
  }  
  setAll(tsk : Array<ITask>){
    this.m_tasks = tsk;
  } 
  get(id : number) : ITask {
    return this.m_tasks.find((t)=>t.id == id);
  }

  setPplTaskId(id : number){
    this.m_pplTaskId = id;
  } 
  getPplTaskId(){
    return this.m_pplTaskId;
  } 
  
  isEqual(one : ITask, two : ITask) : boolean{
    return (one.state == two.state) &&
           (one.startTime == two.startTime) &&
           (one.stopTime == two.stopTime) && 
           (one.progress == two.progress);            
  }  
}
export
let Tasks = new TaskStoreClass();