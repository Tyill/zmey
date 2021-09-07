"use strict";

let reqSchers = function(){
  fetch('api/v1/schedulers')
    .then(response => response.json())    
    .then(fillScheds)
    .catch(e=>console.log(e)); 
}
reqSchers();
setInterval(reqSchers, 10000)

function fillScheds(scheds){ 
  let table  = document.getElementById("tblSchedulers")
  for (let ch of table.children)
    table.removeChild(ch)
  for (let s of scheds){
    table.insertAdjacentHTML("afterbegin", `<tr>
                        <th>${s.id}</th>
                        <td>${s.connectPnt}</td>
                        <td>${s.state}</td>
                        <td>${s.capacityTask}</td>
                        <td>${s.activeTask}</td>
                        <td>${s.pingTime}</td>
                        <td>${s.startTime}</td>
                        <td>${s.stopTime}</td>
                        <td>${s.name}</td>
                        <td>${s.description}</td>
                      </tr>`)
  }
}