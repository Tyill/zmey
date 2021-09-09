"use strict";

let reqSchers = function(){
  fetch('api/v1/schedulers')
    .then(response => response.json())    
    .then(fillScheds)
    .catch(e=>console.log(e)); 
}
reqSchers();
setInterval(reqSchers, 5000)

function fillScheds(scheds){ 
  let table  = document.getElementById("tblSchedulers")
  table.innerHTML="";
  for (let s of scheds){
    let offsTime = new Date().getTimezoneOffset(),
        startTime = new Date(s.startTime),
        stopTime = new Date(s.stopTime),
        pingTime = new Date(s.pingTime);
    startTime.setMinutes(startTime.getMinutes() - offsTime);
    stopTime.setMinutes(stopTime.getMinutes() - offsTime);
    pingTime.setMinutes(pingTime.getMinutes() - offsTime);
    table.insertAdjacentHTML("afterbegin", `<tr>
                        <th>${s.id}</th>
                        <td>${s.connectPnt}</td>
                        <td>${s.state}</td>
                        <td>${s.capacityTask}</td>
                        <td>${s.activeTask}</td>
                        <td>${dateFormat(startTime, "yyyy-mm-dd hh:ii:ss")}</td>
                        <td>${dateFormat(stopTime, "yyyy-mm-dd hh:ii:ss")}</td>
                        <td>${dateFormat(pingTime, "yyyy-mm-dd hh:ii:ss")}</td>
                        <td>${s.name}</td>
                        <td>${s.description}</td>
                      </tr>`)
  }
}

function appendSchedr(){
  
  let newSchedr={
    connectPnt : document.getElementById("schedrIpAddr").value,
    capacityTask : document.getElementById("schedrCapacityTask").value,
    name : document.getElementById("schedrName").value,
    description : document.getElementById("schedrDescr").value
  }

  fetch('api/v1/schedulers', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newSchedr)})
  .then(response => response.json())    
  .then(reqSchers)
  .catch(e=>console.log("api/v1/schedulers error", e));  
}

function dateFormat(date, format) {
  let yyyy = date.getFullYear().toString();
  format = format.replace(/yyyy/g, yyyy)
  let mm = (date.getMonth()+1).toString(); 
  format = format.replace(/mm/g, (mm[1]?mm:"0"+mm[0]));
  let dd  = date.getDate().toString();
  format = format.replace(/dd/g, (dd[1]?dd:"0"+dd[0]));
  let hh = date.getHours().toString();
  format = format.replace(/hh/g, (hh[1]?hh:"0"+hh[0]));
  let ii = date.getMinutes().toString();
  format = format.replace(/ii/g, (ii[1]?ii:"0"+ii[0]));
  let ss  = date.getSeconds().toString();
  format = format.replace(/ss/g, (ss[1]?ss:"0"+ss[0]));
  let ms  = date.getMilliseconds().toString();
  if (!ms[2]) ms ="0" + ms;
  if (!ms[1]) ms ="00" + ms;
  format = format.replace(/ms/g, ms);
  return format;
};