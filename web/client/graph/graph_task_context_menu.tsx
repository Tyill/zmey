import React from "react";
import ReactDOM from "react-dom";
import { Button, ButtonGroup, DropdownButton, Dropdown} from "react-bootstrap";
import { observer } from "mobx-react-lite"

import { PipelineTasks } from "../store/store_pipeline_task";
import * as ServerAPI from "../server_api/server_api";

interface IProps { 
  id : number;
};
interface IState { 
};

export default
class TaskContextMenu extends React.Component<IProps, IState>{  
  
  private m_taskWidth : number = 0;
  
  constructor(props : IProps){
    super(props);    
   
    this.state  = { };   
  }   

  componentDidMount(){

    this.m_taskWidth = ReactDOM.findDOMNode(this).parentNode.offsetWidth;
  }

  render(){  
        
    let Menu = observer(() => {      

      const task =  PipelineTasks.get(this.props.id);
      const isVisible = task.setts.isContextMenuVisible;
      
      return (
        <ButtonGroup vertical style={{display: isVisible ? "inline": "none", position:"absolute", left:this.m_taskWidth.toString() + "px", top:"0px"}}>
          <Button variant="light" style={{textAlign: "left"}}>Start task</Button>
          <Button variant="light" style={{textAlign: "left"}}>Start task sequence</Button>
                            
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
    )});

    return <Menu/>;
  }
}