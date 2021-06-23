import { IPipeline, IGroup, ITaskTemplate, ITask } from "./types";

export
namespace ServerAPI {

export
function getAllTaskTemplates(onFillTaskTemplates : (taskTemplates : Array<ITaskTemplate>) => any){
  fetch('api/v1/taskTemplates')
  .then(response => response.json())    
  .then(onFillTaskTemplates)
  .catch(() => console.log('api/v1/taskTemplates error')); 
}

export
function getAllPipelines(onFillPipelines : (pipelines : Array<IPipeline>) => any){
  fetch('api/v1/pipelines')
  .then(response => response.json())    
  .then(onFillPipelines)
  .catch(() => console.log('api/v1/pipelines error'));   
}

export
function getAllTaskGroups(onFillTaskGroups : (groups : Array<IGroup>) => any){
  fetch('api/v1/taskGroups')
  .then(response => response.json())    
  .then(onFillTaskGroups)
  .catch(() => console.log('api/v1/taskGroups error'));  
}
 
export
function getAllTasks(onFillTasks : (tasks : Array<ITask>) => any){
  fetch('api/v1/tasks')
  .then(response => response.json())    
  .then(onFillTasks)
  .catch(() => console.log('api/v1/tasks error'));  
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


}