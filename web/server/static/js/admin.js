"use strict";

console.log('hello');


// Schedr{       
//   id : number;
//   isEnabled : boolean;
//   timeStartOnceOfDay : Array<string>;
//   timeStartEverySec : number;
//   tasksForStart : Array<number>;
//   name : string;
//   description : string;  
// };

fetch('api/v1/schedulers')
  .then(response => response.json())    
  .then(())
  .catch(onError); 