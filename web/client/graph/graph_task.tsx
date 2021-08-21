import React from "react";
import Draggable, {ControlPosition} from 'react-draggable';
import { PipelineTasks } from "../store/store_pipeline_task";
import * as ServerAPI from "../server_api/server_api";
import {SocketType} from "./graph_panel";

interface IProps { 
  title : string;
  id : number;
  moveEnabled: boolean;
  hHide : (id : number) => any;
  //hChangeSocketPos : (SocketType) => {x : number, y : number};
  hSocketInputСaptured : (id : number, mpos : {x:number, y:number}) => any;
  hSocketOutputСaptured : (id : number, mpos : {x:number, y:number}) => any;
};
interface IState { 
};

export default
class GraphItem extends React.Component<IProps, IState>{  
  constructor(props : IProps){
    super(props);    
    this.state  = {  };   
  }   
  render(){  
        
    let task = PipelineTasks.get(this.props.id);

    return (

      <Draggable disabled={!this.props.moveEnabled} bounds="parent" 
                 position={{x:task.setts.positionX,y:task.setts.positionY}}
                 onStop={(e, data)=>{
                   PipelineTasks.setPosition(this.props.id, data.x, data.y);
                   ServerAPI.changePipelineTask(PipelineTasks.get(this.props.id), ()=>0, ()=>0);
                 }}>
        <div className="graphPplTaskContainer">           
          <div className="graphPplTaskSocketInput unselectable" 
               onMouseDown={(e)=>this.props.hSocketInputСaptured(this.props.id, {x : e.clientX, y : e.clientY})}/>
          <div className="graphPplTask unselectable">
            {this.props.title}
          </div>
          <div className="graphPplTaskSocketOutput unselectable"
               onMouseDown={(e)=>{
                 const x = task.setts.positionX + e.currentTarget.offsetLeft;
                 const y = task.setts.positionY + e.currentTarget.offsetTop + e.currentTarget.offsetHeight / 2;
                 this.props.hSocketOutputСaptured(this.props.id, {x, y});
               }}/>
        </div>
      </Draggable>
    )
  }
}
