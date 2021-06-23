import { IPipeline, ITaskTemplate, ITaskPipeline } from "./types";

export namespace ServerAPI {

export
function getAllPipelines(onFillPipelines : (pipelines : Array<IPipeline>) => any){
  fetch('api/v1/pipelines')
  .then(response => response.json())    
  .then(onFillPipelines)
  .catch(() => console.log('api/v1/pipelines fill error'));
}

export
function getAllTaskTemplates(onFillTaskTemplates : (taskTemplates : Array<ITaskTemplate>) => any){
  fetch('api/v1/taskTemplates')
  .then(response => response.json())    
  .then(onFillTaskTemplates)
  .catch(() => console.log('api/v1/taskTemplates fill error')); 
}

export
function getAllTaskPipeline(onFillTasks : (tasks : Array<ITaskPipeline>) => any){
  fetch('api/v1/taskPipeline')
  .then(response => response.json())    
  .then(onFillTasks)
  .catch(() => console.log('api/v1/taskPipeline fill error'));  
}

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
function changeTaskTemplate(taskTemplate : ITaskTemplate, onSucces : (respTaskTemplate : ITaskTemplate) => any, onError : () => any ){
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
function delTaskTemplate(taskTemlate : ITaskTemplate, onSucces : () => any){
  fetch('api/taskTemplates/' + taskTemlate.id, { method: 'DELETE'})
  .then(onSucces)
  .catch(() => console.log('api/v1/taskTemplates/' + taskTemlate.id + ' delete error'));
}

export
function addPipeline(newPipeline : IPipeline, onSucces : (respPipeline : IPipeline) => any, onError : () => any ){
  fetch('api/v1/pipelines', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newPipeline)})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError);    
} 

export
function changePipeline(newPipeline : IPipeline, onSucces : (respPipeline : IPipeline) => any, onError : () => any ){
  fetch('api/v1/pipelines/' + newPipeline.id, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newPipeline)})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError);    
} 

export
function deletePipeline(pipeline : IPipeline, onSucces : () => any){
  fetch('api/v1/pipelines/' + pipeline.id, { method: 'DELETE'})
  .then(onSucces) 
  .catch(() => console.log('api/v1/pipelines/' + pipeline.id + ' delete error'));
}
}