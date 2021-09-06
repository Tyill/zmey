import React from "react";
import { Table } from "react-bootstrap";
import { observer} from "mobx-react-lite"

import { PipelineTasks, Tasks, Events} from "../store/store";
import { dateFormat } from "../common/common";
import { ITask, stateToString } from "../types"

interface IProps {  
};
interface IState { 
};

export default
class TaskStatusWidget extends React.Component<IProps, IState>{  
  
  private m_tasks : Array<ITask> = [];
  private m_tasksWidget  = [];
  private m_hasNew = false;

  constructor(props : IProps){
    super(props);  
  
    this.state = {};   
  }

  shouldComponentUpdate(){
    return this.m_hasNew;
  }

  render(){  

    const headStyle = {fontWeight: 500, padding: "1px", paddingLeft: "5px", paddingRight: "5px",
                       borderWidth: "1px", borderTop: "none"};
    const bodyStyle = {padding: "1px", paddingLeft: "5px", paddingRight: "5px"};

    const TasksRet = observer(() => {
      
      const newTasks = Tasks.getAll();
      this.m_hasNew = newTasks.length != this.m_tasks.length;
      if (!this.m_hasNew){
        for (let i = 0; i < newTasks.length; ++i){
          if (!Tasks.isEqual(newTasks[i], this.m_tasks[i])){
            this.m_hasNew = true; 
            break;         
          }
        }
      }

      if (this.m_hasNew){
        this.m_tasks = newTasks;
        
        this.m_tasksWidget = [];
        for (let t of Tasks.getAll()){  
                    
          let starterPplTask = PipelineTasks.get(t.starterPplTaskId);
          let starterName = starterPplTask ? starterPplTask.name : Events.get(t.starterEventId).name;
        
          let offsTime = new Date().getTimezoneOffset(),
              startTime = new Date(t.startTime),
              stopTime = new Date(t.stopTime);
          startTime.setMinutes(startTime.getMinutes() - offsTime);
          stopTime.setMinutes(stopTime.getMinutes() - offsTime);
          
          this.m_tasksWidget.push(<tr key={t.id}>
            <td style={bodyStyle}>{t.id}</td>
            <td style={bodyStyle}>{starterName}</td>
            <td style={bodyStyle}>{stateToString(t.state)}</td>
            <td style={bodyStyle}>{t.progress}</td>
            <td style={bodyStyle}>{t.startTime ? dateFormat(startTime, "yyyy-mm-dd hh:ii:ss.ms") : ""}</td>
            <td style={bodyStyle}>{t.stopTime ? dateFormat(stopTime, "yyyy-mm-dd hh:ii:ss.ms") : ""}</td>
            <td style={bodyStyle}>{t.result}</td>
          </tr>)
        }      
      }        
      return <tbody>
              {this.m_tasksWidget}
             </tbody> 
    });

    return  <div className="d-flex flex-row p-0 m-0" >            
              <div className="p-0 mr-auto p-0 m-0">
                
                                
              </div>
              <div className="p-0 m-0" style={{ overflow: "auto", whiteSpace: "nowrap", maxHeight: "20vh", maxWidth: "40vw" }} >
                <Table striped bordered hover style={{border: "none"}} >
                  <thead >
                    <tr >
                      <th style={headStyle}>Id</th>
                      <th style={headStyle}>Start from</th>
                      <th style={headStyle}>State</th>
                      <th style={headStyle}>Progress</th>
                      <th style={headStyle}>Begin time</th>
                      <th style={headStyle}>End time</th>
                      <th style={headStyle}>Result</th>                 
                    </tr>
                  </thead>
                  <TasksRet/>
                </Table>
              </div>
            </div>
  }
}
