import { IPipeline, ITaskTemplate, IPipelineTask } from "../types";

export namespace ServerAPI {

export
function getAllPipelines(onFillPipelines : (pipelines : Array<IPipeline>) => any, onError : () => any){
  fetch('api/v1/pipelines')
  .then(response => response.json())    
  .then(onFillPipelines)
  .catch(onError); 
}

export
function getAllTaskTemplates(onFillTaskTemplates : (taskTemplates : Array<ITaskTemplate>) => any, onError : () => any){
  fetch('api/v1/taskTemplates')
  .then(response => response.json())    
  .then(onFillTaskTemplates)
  .catch(onError); 
}

export
function getAllPipelineTasks(onFillTasks : (tasks : Array<IPipelineTask>) => any, onError : () => any){
  fetch('api/v1/pipelineTasks')
  .then(response => response.json())    
  .then(onFillTasks)
  .catch(onError); 
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
function delTaskTemplate(taskTemlate : ITaskTemplate, onSucces : () => any, onError : () => any){
  fetch('api/v1/taskTemplates/' + taskTemlate.id, { method: 'DELETE'})
  .then((resp : Response)=> resp.ok ? onSucces() : onError())
  .catch(onError); 
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
function delPipeline(pipeline : IPipeline, onSucces : () => any, onError : () => any ){
  fetch('api/v1/pipelines/' + pipeline.id, { method: 'DELETE'})
  .then((resp : Response)=> resp.ok ? onSucces() : onError())
  .catch(onError); 
}
}