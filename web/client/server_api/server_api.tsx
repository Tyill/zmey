import { IPipeline, ITaskTemplate, IPipelineTask, IEvent } from "../types";

export namespace ServerAPI {

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
function getAllTaskTemplates(onFillTaskTemplates : (taskTemplates : Array<ITaskTemplate>) => any, onError : () => any){
  fetch('api/v1/taskTemplates')
  .then(response => response.json())    
  .then(onFillTaskTemplates)
  .catch(onError); 
}

/////////////////////////////////////////////////////////////////////////////////////
/// Pipeline

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

export
function getAllPipelines(onFillPipelines : (pipelines : Array<IPipeline>) => any, onError : () => any){
  fetch('api/v1/pipelines')
  .then(response => response.json())    
  .then(onFillPipelines)
  .catch(onError); 
}

/////////////////////////////////////////////////////////////////////////////////////
/// PipelineTask 

export
function addPipelineTask(newPipelineTask : IPipelineTask, onSucces : (respPipeline : IPipelineTask) => any, onError : () => any ){
  fetch('api/v1/pipelineTasks', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newPipelineTask)})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError);    
} 

export
function changePipelineTask(newPipelineTask : IPipelineTask, onSucces : (respPipeline : IPipelineTask) => any, onError : () => any ){
  fetch('api/v1/pipelineTasks/' + newPipelineTask.id, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newPipelineTask)})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError);    
} 

export
function delPipelineTask(pipelineTask : IPipelineTask, onSucces : () => any, onError : () => any ){
  fetch('api/v1/pipelineTask/' + pipelineTask.id, { method: 'DELETE'})
  .then((resp : Response)=> resp.ok ? onSucces() : onError())
  .catch(onError); 
}

export
function getAllPipelineTasks(onFillTasks : (tasks : Array<IPipelineTask>) => any, onError : () => any){
  fetch('api/v1/pipelineTasks')
  .then(response => response.json())    
  .then(onFillTasks)
  .catch(onError); 
}

/////////////////////////////////////////////////////////////////////////////////////
/// Event 

export
function addEvent(newEvent : IEvent, onSucces : (resp : IEvent) => any, onError : () => any ){
  fetch('api/v1/events', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newEvent)})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError);    
} 

export
function changeEvent(newEvent : IEvent, onSucces : (resp : IPipelineTask) => any, onError : () => any ){
  fetch('api/v1/events/' + newEvent.id, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newEvent)})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError);    
} 

export
function delEvent(event : IEvent, onSucces : () => any, onError : () => any ){
  fetch('api/v1/events/' + event.id, { method: 'DELETE'})
  .then((resp : Response)=> resp.ok ? onSucces() : onError())
  .catch(onError); 
}

export
function getAllEvents(onFillEvents : (events : Array<IEvent>) => any, onError : () => any){
  fetch('api/v1/events')
  .then(response => response.json())    
  .then(onFillEvents)
  .catch(onError); 
}

}