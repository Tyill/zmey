import { ITaskTemplate } from "../types";


/////////////////////////////////////////////////////////////////////////////////////
/// TaskTemplate

export
function addTaskTemplate(newTaskTemplate : ITaskTemplate, onSucces : (respTaskTemplate : ITaskTemplate) => any, onError : () => any ){
  fetch('api/v1/taskTemplates', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newTaskTemplate)})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError); 
}

export
function changeTaskTemplate(taskTemplate : ITaskTemplate, onSucces : (respTaskTemplate : ITaskTemplate) => any = ()=>0, onError : () => any = ()=>0){
  fetch('api/v1/taskTemplates/' + taskTemplate.id, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(taskTemplate)})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError); 
}

export
function delTaskTemplate(taskTemlate : ITaskTemplate, onSucces : () => any, onError : () => any){
  fetch('api/v1/taskTemplates/' + taskTemlate.id, { method: 'DELETE'})
  .then((resp : Response)=> resp.ok ? onSucces() : onError())
  .catch(onError); 
}

export
function getAllTaskTemplates(onFillTaskTemplates : (taskTemplates : Array<ITaskTemplate>) => any, onError : () => any){
  fetch('api/v1/taskTemplates')
  .then(response => response.json())    
  .then(onFillTaskTemplates)
  .catch(onError); 
}
