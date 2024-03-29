import { ITask } from "../types";

/////////////////////////////////////////////////////////////////////////////////////
/// Task 

export
function startTask(newTask : ITask, onSucces : (resp : ITask) => any, onError : () => any ){
  fetch('api/v1/tasks', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newTask)})
  .then(response =>response.json())
  .then(onSucces)
  .catch(onError);    
}

export
function continueTask(task : ITask, onSucces : () => any = ()=>0, onError : () => any = ()=>0 ){
  fetch('api/v1/tasks/' + task.id + '?continue', {
    method: 'PUT'})
    .then((resp : Response)=> resp.ok ? onSucces() : onError())
    .catch(onError);     
}

export
function pauseTask(task : ITask, onSucces : () => any = ()=>0, onError : () => any = ()=>0 ){
  fetch('api/v1/tasks/' + task.id + '?pause', {
    method: 'PUT'})
    .then((resp : Response)=> resp.ok ? onSucces() : onError())
    .catch(onError);  
}

export
function stopTask(task : ITask, onSucces : () => any = ()=>0, onError : () => any = ()=>0){
  fetch('api/v1/tasks/' + task.id + '?stop', {
    method: 'PUT'})
    .then((resp : Response)=> resp.ok ? onSucces() : onError())
    .catch(onError);  
}

export
function getTaskState(pplTaskId : number, ifChange : boolean, onSucces : (resp : Array<ITask>) => any, onError : () => any ){
  const param = ifChange ? "?ifChange" : ""
  fetch('api/v1/tasks/' + pplTaskId + param, {
    method: 'GET'})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError);    
} 