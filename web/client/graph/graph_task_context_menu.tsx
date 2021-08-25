import React from "react";
import { Button, ButtonGroup, DropdownButton, Dropdown} from "react-bootstrap";

import { ITask, MessType} from "../types";
import { PipelineTasks } from "../store/store_pipeline_task";

import * as ServerAPI from "../server_api/server_api";

interface IProps { 
  id : number;
  hStatusMess : (mess : string, mtype : MessType)=>void;
};
interface IState { 
};

export default
class TaskContextMenu extends React.Component<IProps, IState>{  
    
  constructor(props : IProps){
    super(props);    
   
    this.startTask = this.startTask.bind(this);

    this.state  = { };   
  } 
  
  startTask(){

    let task = {
      pplTaskId : this.props.id,
    } as ITask;

    let pplTask = PipelineTasks.get(this.props.id);

    ServerAPI.startTask(task, 
      (resp)=>this.props.hStatusMess(`Success start of Task ${pplTask.name}`, MessType.Ok),
      ()=>{ this.props.hStatusMess(`Server error start of Task ${pplTask.name}`, MessType.Error); }
    );
  }
  
  render(){  
  
    let task = PipelineTasks.get(this.props.id);
    
    const left = task ? task.setts.positionX + task.setts.width : 0;
    const top = task ? task.setts.positionY : 0;
    
    return (
      task ? 
      <ButtonGroup vertical style={{display: this.props.id != 0 ? "inline": "none", width: "min-content", position:"absolute", 
                                    left: left.toString() + "px", top: top + "px"}}>
        <Button variant="light" style={{textAlign: "left"}}>Start task</Button>
                         
        <DropdownButton variant="light" as={ButtonGroup} 
                        drop="right"
                        title="Delete next connection &ensp;&ensp;&ensp;"
                        onMouseDown={(e) => e.stopPropagation()}>
          {task.nextTasksId.map((v, i)=>{
            return <Dropdown.Item eventKey={v.toString()} key={v}>{PipelineTasks.get(v).name}</Dropdown.Item>
          })}
        </DropdownButton>
        <DropdownButton variant="light" as={ButtonGroup} 
                        drop="right"
                        title="Delete previous connection"
                        onMouseDown={(e) => e.stopPropagation()}>
          {task.prevTasksId.map((v, i)=>{
            return <Dropdown.Item eventKey={v.toString()} key={v}>{PipelineTasks.get(v).name}</Dropdown.Item>
          })}
        </DropdownButton>
        <Button variant="light" style={{textAlign: "left"}}>Copy task to clipboard</Button>
        <Button variant="light" style={{textAlign: "left"}}>Delete task from map</Button>
      </ButtonGroup>  
      : ""
    )
  }
}