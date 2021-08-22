import { IPipeline } from "../types";

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
function changePipeline(newPipeline : IPipeline, onSucces : (respPipeline : IPipeline) => any = ()=>0, onError : () => any = ()=>0){
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