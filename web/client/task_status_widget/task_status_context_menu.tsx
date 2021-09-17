import React from "react";
import { Button, ButtonGroup} from "react-bootstrap";

import { Tasks } from "../store/store_task";
import { IPoint, StateType} from "../types"

import * as ServerAPI from "../server_api/server_api";

interface IProps { 
  id : number;
  mpos : IPoint;
  hHide : ()=>void;
};
interface IState { 
};

export default
class TaskStatusContextMenu extends React.Component<IProps, IState>{  
    
  constructor(props : IProps){
    super(props);    
   
    this.continueTask = this.continueTask.bind(this);
    this.pauseTask = this.pauseTask.bind(this);
    this.stopTask = this.stopTask.bind(this);
    
    this.state  = { };   
  } 
  
  continueTask(){
    let task = Tasks.get(this.props.id);

    ServerAPI.continueTask(task);
    this.props.hHide();
  }

  pauseTask(){
    let task = Tasks.get(this.props.id);

    ServerAPI.pauseTask(task);
    this.props.hHide();
  }
  
  stopTask(){
    let task = Tasks.get(this.props.id);

    ServerAPI.stopTask(task);
    this.props.hHide();
  }
    
  render(){  
  
    let task = Tasks.get(this.props.id);
    
    const left = task ? this.props.mpos.x : 0;
    const top = task ? this.props.mpos.y : 0;
   
    return (
      task && ((task.state == StateType.RUNNING) || (task.state == StateType.PAUSE)) ? 
      <ButtonGroup className="m-0 p-0" vertical 
                   style={{ position:"absolute",
                           left: left  + "px", top: top + "px"}}>
        <Button variant="light" style={{textAlign: "left"}}
                onClick={ task.state != StateType.PAUSE ? this.pauseTask : this.continueTask}>
                  {task.state != StateType.PAUSE ? "Pause task " + this.props.id : "Continue task " + this.props.id}</Button>
        <Button variant="light" style={{textAlign: "left"}}
                onClick={this.stopTask}>{"Stop task " + this.props.id}</Button>
      </ButtonGroup>
      : ""
    )
  }
}