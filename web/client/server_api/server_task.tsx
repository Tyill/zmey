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
function getTaskState(pplTaskId : number, onSucces : (resp : Array<ITask>) => any, onError : () => any ){
  fetch('api/v1/tasks/' + pplTaskId, {
    method: 'GET'})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError);    
} 