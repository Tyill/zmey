import React from "react";
import { Button, ButtonGroup, DropdownButton, Dropdown} from "react-bootstrap";

import { ITask, MessType} from "../types";
import { PipelineTasks } from "../store/store_pipeline_task";

import * as ServerAPI from "../server_api/server_api";

interface IProps { 
  id : number;
  hHide : ()=>void;
  hStatusMess : (mess : string, mtype : MessType)=>void;
};
interface IState { 
};

export default
class TaskContextMenu extends React.Component<IProps, IState>{  
    
  constructor(props : IProps){
    super(props);    
   
    this.startTask = this.startTask.bind(this);
    this.deleteNextConnect = this.deleteNextConnect.bind(this);
    this.deletePrevConnect = this.deletePrevConnect.bind(this);
    this.deleteTaskFromMap = this.deleteTaskFromMap.bind(this);

    this.state  = { };   
  } 
  
  startTask(){

    let task = {
      pplTaskId : this.props.id,
      starterPplTaskId : this.props.id,
      starterEventId : 0,
      ttlId : PipelineTasks.get(this.props.id).ttId,
    } as ITask;

    let pplTask = PipelineTasks.get(this.props.id);    

    ServerAPI.startTask(task, 
      (resp)=>this.props.hStatusMess(`Success start of Task ${pplTask.name}`, MessType.Ok),
      ()=>this.props.hStatusMess(`Server error start of Task ${pplTask.name}`, MessType.Error)
    );
    this.props.hHide();
  }

  deleteNextConnect(id: number){

    let ppt = PipelineTasks.get(this.props.id);
    let inxNext = ppt.nextTasksId.findIndex(nid=>{
      return nid == id;
    })
    let nextTask = PipelineTasks.get(ppt.nextTasksId[inxNext]);
    
    ppt.nextTasksId.splice(inxNext, 1);
    ppt.isStartNext.splice(inxNext, 1);
    ppt.isSendResultToNext.splice(inxNext, 1);
    PipelineTasks.upd(ppt);
    ServerAPI.changePipelineTask(ppt);
    
    if (nextTask.id != this.props.id){
      let inxPrev = nextTask.prevTasksId.findIndex(pid=>{
        return pid == this.props.id;
      })
      nextTask.prevTasksId.splice(inxPrev, 1);
      PipelineTasks.upd(nextTask);
      ServerAPI.changePipelineTask(nextTask);
    }
    else{
      let inxPrev = ppt.prevTasksId.findIndex(pid=>{
        return pid == this.props.id;
      })
      ppt.prevTasksId.splice(inxPrev, 1);
      PipelineTasks.upd(ppt);
      ServerAPI.changePipelineTask(ppt);
    }
    this.props.hHide();
  }

  deletePrevConnect(id : number){

    let ppt = PipelineTasks.get(this.props.id);
    let inxPrev = ppt.prevTasksId.findIndex(nid=>{
      return nid == id;
    })
    let prevTask = PipelineTasks.get(ppt.prevTasksId[inxPrev]);
    
    ppt.prevTasksId.splice(inxPrev, 1);
    PipelineTasks.upd(ppt);
    ServerAPI.changePipelineTask(ppt);
    
    if (prevTask.id != this.props.id){
      let inxNext = prevTask.nextTasksId.findIndex(pid=>{
        return pid == this.props.id;
      })
      prevTask.nextTasksId.splice(inxNext, 1);
      prevTask.isStartNext.splice(inxNext, 1);
      prevTask.isSendResultToNext.splice(inxNext, 1);
      PipelineTasks.upd(prevTask);
      ServerAPI.changePipelineTask(prevTask);
    }
    else{
      let inxNext = ppt.nextTasksId.findIndex(pid=>{
        return pid == this.props.id;
      })
      ppt.nextTasksId.splice(inxNext, 1);
      ppt.isStartNext.splice(inxNext, 1);
      ppt.isSendResultToNext.splice(inxNext, 1);
      PipelineTasks.upd(ppt);
      ServerAPI.changePipelineTask(ppt);
    }
    this.props.hHide();
  }

  deleteTaskFromMap(){

    PipelineTasks.delAllConnections(this.props.id);
    PipelineTasks.setVisible(this.props.id, false);
    ServerAPI.changePipelineTask(PipelineTasks.get(this.props.id));
   
    this.props.hHide();
  }
  
  render(){  
  
    let task = PipelineTasks.get(this.props.id);
    
    const left = task ? task.setts.positionX + task.setts.width : 0;
    const top = task ? task.setts.positionY : 0;
    
    return (
      task ? 
      <ButtonGroup vertical style={{display: this.props.id != 0 ? "inline": "none", width: "min-content", position:"absolute", 
                                    left: left.toString() + "px", top: top + "px"}}>
        <Button variant="light" style={{textAlign: "left"}} onClick={this.startTask}>Start task</Button>
                         
        <DropdownButton variant="light" as={ButtonGroup} 
                        drop="right"
                        title="Delete next connection &ensp;&ensp;&ensp;"
                        onMouseDown={(e) => e.stopPropagation()}>
          {task.nextTasksId.map((v, i)=>{
            return <Dropdown.Item eventKey={v.toString()} 
                                  key={v}
                                  onClick={()=>this.deleteNextConnect(v)}>
                      {PipelineTasks.get(v).name}
                    </Dropdown.Item>
          })}
        </DropdownButton>
        <DropdownButton variant="light" as={ButtonGroup} 
                        drop="right"
                        title="Delete previous connection"
                        onMouseDown={(e) => e.stopPropagation()}>
          {task.prevTasksId.map((v, i)=>{
            return <Dropdown.Item eventKey={v.toString()} 
                                  key={v}
                                  onClick={()=>this.deletePrevConnect(v)}>
                      {PipelineTasks.get(v).name}
                    </Dropdown.Item>
          })}
        </DropdownButton>
        {/* <Button variant="light" style={{textAlign: "left"}}>Copy task to clipboard</Button> */}
        <Button variant="light" style={{textAlign: "left"}}
                onClick={this.deleteTaskFromMap}>Delete task from map</Button>
      </ButtonGroup>  
      : ""
    )
  }
}