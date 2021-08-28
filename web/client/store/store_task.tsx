import { ITask } from "../types"
import { makeObservable, observable, action } from "mobx"

///////////////////////////////////////////////////////////////
/// Task

class TaskStoreClass {
  m_tasks : Array<ITask>;  
  constructor() {
      makeObservable(this, {
        m_tasks: observable,
        setAll: action,
      });
      this.m_tasks = new Array<ITask>();
  }
  getAll() : Array<ITask>{
    return this.m_tasks;
  }  
  setAll(tsk : Array<ITask>){
    this.m_tasks = tsk;
  }  
}
export
let Tasks = new TaskStoreClass();