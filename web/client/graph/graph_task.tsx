import React from "react";
import Draggable from 'react-draggable';
import { PipelineTasks } from "../store/store_pipeline_task";
import * as ServerAPI from "../server_api/server_api";
import {SocketType} from "./graph_panel";
import TaskContextMenu from "./graph_task_context_menu";
import {IPoint, IRect} from "../types"

interface IProps { 
  title : string;
  id : number;
  moveEnabled: boolean;
  hHide : (id : number) => any;
  hMove : (id : number) => any;
  hSocketInputСaptured : (id : number, mpos : IPoint) => any;
  hSocketOutputСaptured : (id : number, mpos : IPoint) => any;
};
interface IState {
};

export default
class GraphTask extends React.Component<IProps, IState>{  

  private m_socketInput : HTMLDivElement;
  private m_socketOutput : HTMLDivElement;
  
  constructor(props : IProps){
    super(props);    

    this.getSocketPoint = this.getSocketPoint.bind(this);
    this.getSocketRect = this.getSocketRect.bind(this);
    this.showContextMenu = this.showContextMenu.bind(this);

    this.state  = { };   
  }   

  getSocketPoint(type : SocketType, cpX : number, cpY : number) : IPoint{
    let point = {x : 0, y : 0};
    if (type == SocketType.Input){
      const view = { left : this.m_socketInput.parentElement.offsetLeft,
            top : this.m_socketInput.parentElement.offsetTop};
      point.x = cpX + view.left + this.m_socketInput.offsetWidth;
      point.y = cpY + view.top +  this.m_socketInput.offsetTop + this.m_socketInput.offsetHeight/2;
    }else{
      const view = { left : this.m_socketOutput.parentElement.offsetLeft,
            top : this.m_socketOutput.parentElement.offsetTop,
            width : this.m_socketOutput.parentElement.offsetWidth};
      point.x = cpX + view.left + view.width - this.m_socketOutput.offsetWidth;
      point.y = cpY + view.top + this.m_socketOutput.offsetTop + this.m_socketOutput.offsetHeight / 2;
    }
    return point;
  }

  getSocketRect(type : SocketType, cpX : number, cpY : number) : IRect{
    let rect = {x : 0, y : 0, w : 0, h: 0};
    if (type == SocketType.Input){
      const view = { left : this.m_socketInput.parentElement.offsetLeft,
            top : this.m_socketInput.parentElement.offsetTop};
      rect.x = cpX + view.left;
      rect.y = cpY + view.top + this.m_socketInput.offsetTop;
    }else{
      const view = { left : this.m_socketOutput.parentElement.offsetLeft,
            top : this.m_socketOutput.parentElement.offsetTop,
            width : this.m_socketOutput.parentElement.offsetWidth};
      rect.x = cpX + view.left + view.width - this.m_socketOutput.offsetWidth;
      rect.y = cpY + view.top + this.m_socketOutput.offsetTop;
    }
    rect.w = this.m_socketInput.offsetWidth;
    rect.h = this.m_socketInput.offsetHeight;
    return rect;
  }

  showContextMenu(){
    
    let task = PipelineTasks.get(this.props.id);
   
    PipelineTasks.getByPPlId(task.pplId).forEach(v=>{
      if (v.setts.isContextMenuVisible){
        let t = PipelineTasks.get(v.id);
        t.setts.isContextMenuVisible = false;
        PipelineTasks.upd(t);
      }
    });

    task.setts.isContextMenuVisible = true;
    PipelineTasks.upd(task);
  }

  render(){  
        
    let task = PipelineTasks.get(this.props.id);
    
    return (
      <Draggable disabled={!this.props.moveEnabled} bounds="parent"
                 position={{x:task.setts.positionX,y:task.setts.positionY}}
                 onDrag={(e, data)=>{
                  let point = this.getSocketPoint(SocketType.Input, data.x, data.y);
                  let rect = this.getSocketRect(SocketType.Input, data.x, data.y);
                  let inputSocket = {point, rect};

                  point = this.getSocketPoint(SocketType.Output, data.x, data.y);
                  rect = this.getSocketRect(SocketType.Output, data.x, data.y);
                  let outputSocket = {point, rect};
                 
                  PipelineTasks.setSockets(this.props.id, inputSocket, outputSocket);
                  
                  this.props.hMove(this.props.id);
                 }}
                 onStop={(e, data)=>{
                   PipelineTasks.setPosition(this.props.id, data.x, data.y);                   
                   ServerAPI.changePipelineTask(PipelineTasks.get(this.props.id));
                 }}>
          <div className="graphPplTaskContainer">           
            <div className="graphPplTaskSocketInput unselectable" ref={el => this.m_socketInput = el}
                onMouseDown={(e)=>{
                  const point = this.getSocketPoint(SocketType.Input, task.setts.positionX, task.setts.positionY);
                  this.props.hSocketInputСaptured(this.props.id, {...point});
                }}/>
            <div className="graphPplTask unselectable"
                 onContextMenu = { (e) => e.preventDefault() }
                 onMouseDown={(e : React.MouseEvent)=>{
                   if (e.button == 2){
                     this.showContextMenu();
                     e.stopPropagation();
                   }
                 }}>
              {this.props.title}
            </div>
            <TaskContextMenu id = {this.props.id} />
            <div className="graphPplTaskSocketOutput unselectable" ref={el => this.m_socketOutput = el}
                 onMouseDown={(e)=>{
                  const point = this.getSocketPoint(SocketType.Output, task.setts.positionX, task.setts.positionY);
                  this.props.hSocketOutputСaptured(this.props.id, {...point});
                }}/>
          </div>       
      </Draggable>

      
    )
  }
}
