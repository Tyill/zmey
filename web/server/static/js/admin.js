"use strict";

let m_schedrSelected ={
  id : 0,
};

let m_workerSelected ={
  id : 0,
};

let reqWorkers = function(){
  if (m_schedrSelected.id != 0){
    fetch('api/v1/workers?schedId=' + m_schedrSelected.id)
      .then(response => response.json())    
      .then(fillWorkers)
      .catch(e=>console.log(e)); 
  } 
}

let reqSchers = function(){
  fetch('api/v1/schedulers')
    .then(response => response.json())    
    .then(fillScheds)
    .then(reqWorkers)
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
    
    if (m_schedrSelected.id == 0)
      m_schedrSelected.id = s.id;

    const trStyle = s.id == m_schedrSelected.id ? "style=\"background-color:gray\"" : "";
    table.insertAdjacentHTML("afterbegin", `<tr onclick="hSelectSchedr(event)" ${trStyle}>
                        <td>${s.id}</td>
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
  .catch(e=>console.log("api/v1/schedulers POST error", e));  
}

function changeSchedr(){
  
  let newSchedr={
    connectPnt : document.getElementById("schedrIpAddr").value,
    capacityTask : document.getElementById("schedrCapacityTask").value,
    name : document.getElementById("schedrName").value,
    description : document.getElementById("schedrDescr").value
  }

  fetch('api/v1/schedulers/' + m_schedrSelected.id, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newSchedr)})
  .then(response => response.json())    
  .then(reqSchers)
  .catch(e=>console.log("api/v1/schedulers PUT error", e));  
}

function deleteSchedr(){
 
  fetch('api/v1/schedulers/' + m_schedrSelected.id, {
    method: 'DELETE'}) 
  .then(reqSchers)
  .catch(e=>console.log("api/v1/schedulers DELETE error", e));  
}

function hSelectSchedr(e){
  m_schedrSelected.id = parseInt(e.srcElement.parentElement.cells[0].textContent, 10);
  document.getElementById("schedrIpAddr").value = e.srcElement.parentElement.cells[1].textContent;
  document.getElementById("schedrCapacityTask").value = e.srcElement.parentElement.cells[3].textContent;
  document.getElementById("schedrName").value = e.srcElement.parentElement.cells[8].textContent;
  document.getElementById("schedrDescr").value = e.srcElement.parentElement.cells[9].textContent;
  reqSchers();
}

///////////////////////////////////////////////////////////////////////////////////

function fillWorkers(workers){ 
  let table  = document.getElementById("tblWorkers")
  table.innerHTML="";
  for (let w of workers){
    let offsTime = new Date().getTimezoneOffset(),
        startTime = new Date(w.startTime),
        stopTime = new Date(w.stopTime),
        pingTime = new Date(w.pingTime);
    startTime.setMinutes(startTime.getMinutes() - offsTime);
    stopTime.setMinutes(stopTime.getMinutes() - offsTime);
    pingTime.setMinutes(pingTime.getMinutes() - offsTime);

    const trStyle = w.id == m_workerSelected.id ? "style=\"background-color:gray\"" : "";
    table.insertAdjacentHTML("afterbegin", `<tr onclick="hSelectWorker(event)" ${trStyle}>
                        <td>${w.id}</td>
                        <td>${w.connectPnt}</td>
                        <td>${w.state}</td>
                        <td>${w.load}</td>
                        <td>${w.capacityTask}</td>
                        <td>${w.activeTask}</td>
                        <td>${dateFormat(startTime, "yyyy-mm-dd hh:ii:ss")}</td>
                        <td>${dateFormat(stopTime, "yyyy-mm-dd hh:ii:ss")}</td>
                        <td>${dateFormat(pingTime, "yyyy-mm-dd hh:ii:ss")}</td>
                        <td>${w.name}</td>
                        <td>${w.description}</td>
                      </tr>`)
  }
}

function appendWorker(){
  
  let newWorker={
    sId : m_schedrSelected.id,
    connectPnt : document.getElementById("workerIpAddr").value,
    capacityTask : document.getElementById("workerCapacityTask").value,
    name : document.getElementById("workerName").value,
    description : document.getElementById("workerDescr").value
  }

  fetch('api/v1/workers', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newWorker)})
  .then(response => response.json())    
  .then(reqWorkers)
  .catch(e=>console.log("api/v1/workers POST error", e));  
}

function changeWorker(){
  
  let newWorker={
    sId : m_schedrSelected.id,
    connectPnt : document.getElementById("workerIpAddr").value,
    capacityTask : document.getElementById("workerCapacityTask").value,
    name : document.getElementById("workerName").value,
    description : document.getElementById("workerDescr").value
  }

  fetch('api/v1/workers/' + m_workerSelected.id, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json;charset=utf-8'
    },
    body: JSON.stringify(newWorker)})
  .then(response => response.json())    
  .then(reqWorkers)
  .catch(e=>console.log("api/v1/workers PUT error", e));  
}

function deleteWorker(){
 
  fetch('api/v1/workers/' + m_workerSelected.id, {
    method: 'DELETE'}) 
  .then(reqWorkers)
  .catch(e=>console.log("api/v1/workers DELETE error", e));  
}

function hSelectWorker(e){
  m_workerSelected.id = parseInt(e.srcElement.parentElement.cells[0].textContent, 10);
  document.getElementById("workerIpAddr").value = e.srcElement.parentElement.cells[1].textContent;
  document.getElementById("workerCapacityTask").value = e.srcElement.parentElement.cells[4].textContent;
  document.getElementById("workerName").value = e.srcElement.parentElement.cells[9].textContent;
  document.getElementById("workerDescr").value = e.srcElement.parentElement.cells[10].textContent;
  reqSchers();
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