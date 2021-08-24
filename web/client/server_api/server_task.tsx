import { IPipelineTask } from "../types";

/////////////////////////////////////////////////////////////////////////////////////
/// Task 

export
function startTask(newPipelineTask : IPipelineTask, onSucces : (respPipeline : IPipelineTask) => any, onError : () => any ){
  fetch('api/v1/tasks', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newPipelineTask)})
  .then(response => response.json())    
  .then(onSucces)
  .catch(onError);    
} 
