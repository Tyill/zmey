import { IEvent } from "../types";

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
function changeEvent(newEvent : IEvent, onSucces : (resp : IEvent) => any = ()=>0, onError : () => any = ()=>0 ){
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