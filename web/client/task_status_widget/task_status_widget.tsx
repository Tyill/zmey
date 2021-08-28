import React from "react";
import { Table } from "react-bootstrap";
import { observer} from "mobx-react-lite"

import { Tasks } from "../store/store";

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
    const bodyStyle = {padding: "1px"};

    const TasksRet = observer(() => {
           
      let tasks = [];
      for (let t of Tasks.getAll()){ 
                       
        return <tr>
          <td style={bodyStyle}>{t.id}</td>
          <td style={bodyStyle}> </td>
          <td style={bodyStyle}>{t.startTime}</td>
          <td style={bodyStyle}>{t.stopTime}</td>
          <td style={bodyStyle}>{t.result}</td>
        </tr>
      }
      return <tbody>
              {tasks}
             </tbody> 
    });

    return  <div className="d-flex flex-row p-0 m-0" >            
              <div className="p-0 mr-auto p-0 m-0">

                
              </div>
              <div className="p-0 m-0" style={{ overflow: "auto", maxHeight: "20vh" }} >
                <Table striped bordered hover style={{border: "none"}} >
                  <thead >
                    <tr >
                      <th style={headStyle}>Id</th>
                      <th style={headStyle}>Start from</th>
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
