import { IUser } from "../types";

/////////////////////////////////////////////////////////////////////////////////////
/// User 

export
function getUser(onSucces : (resp : IUser) => any, onError : () => any ){
  fetch('api/v1/users')
  .then(response =>response.json())
  .then(onSucces)
  .catch(onError);    
} 