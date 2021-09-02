import React from "react";
import { Table } from "react-bootstrap";
import { observer} from "mobx-react-lite"

import { PipelineTasks, Tasks } from "../store/store";
import { dateFormat } from "../common/common";
import { stateToString } from "../types"

interface IProps {   
};
interface IState { 
};

export default
class TaskStatusWidget extends React.Component<IProps, IState>{  
    
  constructor(props : IProps){
    super(props);  
  
    this.state = {};   
  }

  render(){  

    const headStyle = {fontWeight: 500, padding: "1px", paddingLeft: "5px", paddingRight: "5px",
                       borderWidth: "1px", borderTop: "none"};
    const bodyStyle = {padding: "1px", paddingLeft: "5px", paddingRight: "5px"};

    const TasksRet = observer(() => {
           
      let tasks = [];
      for (let t of Tasks.getAll()){  
        
        let prevPplTask = PipelineTasks.get(t.prevPplTaskId); 
     
        let offsTime = new Date().getTimezoneOffset(),
            startTime = new Date(t.startTime),
            stopTime = new Date(t.stopTime);
        startTime.setMinutes(startTime.getMinutes() - offsTime);
        stopTime.setMinutes(stopTime.getMinutes() - offsTime);
        
        tasks.push(<tr key={t.id}>
          <td style={bodyStyle}>{t.id}</td>
          <td style={bodyStyle}>{prevPplTask.name}</td>
          <td style={bodyStyle}>{stateToString(t.state)}</td>
          <td style={bodyStyle}>{stateToString(t.state)}</td>
          <td style={bodyStyle}>{t.startTime ? dateFormat(startTime, "yyyy-mm-dd hh:ii:ss.ms") : ""}</td>
          <td style={bodyStyle}>{t.stopTime ? dateFormat(stopTime, "yyyy-mm-dd hh:ii:ss.ms") : ""}</td>
          <td style={bodyStyle}>{t.result}</td>
        </tr>)
      }
      return <tbody>
              {tasks}
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
