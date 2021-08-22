import { IPipelineTask } from "../types";

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
function changePipelineTask(newPipelineTask : IPipelineTask, onSucces : (respPipeline : IPipelineTask) => any = ()=>0, onError : () => any = ()=>0 ){
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
  fetch('api/v1/pipelineTasks/' + pipelineTask.id, { method: 'DELETE'})
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