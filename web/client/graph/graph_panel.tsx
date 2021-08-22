import React from "react";
import GraphTask from "./graph_task"
import { observer } from "mobx-react-lite"

import { IPoint, IRect } from "../types";
import { PipelineTasks, Pipelines} from "../store/store";
import * as ServerAPI from "../server_api/server_api";

export
enum SocketType{
  Input,
  Output,
}

interface IProps { 
  pplId : number;
};
interface IState { 
  socketCaptured : {id : number, type : SocketType};
};

export default
class GraphPanel extends React.Component<IProps, IState>{  
  
  private m_canvasRef : HTMLCanvasElement;
  private m_viewRef : HTMLDivElement;
  private m_mouseStartMem : {x : number, y : number};
  private m_firstLoad : boolean;

  constructor(props : IProps){
    super(props);  
    this.m_canvasRef = null;  
    this.m_viewRef = null;
    this.m_mouseStartMem = {x : 0, y : 0};
    this.m_firstLoad = false;

    this.state = {socketCaptured : {id : 0, type : SocketType.Input}};

    this.getCanvasContext = this.getCanvasContext.bind(this);
    this.drawLine = this.drawLine.bind(this);
    this.drawAll = this.drawAll.bind(this);
    this.hTaskMove = this.hTaskMove.bind(this);
    this.hMouseUp = this.hMouseUp.bind(this);
    this.hMouseDown = this.hMouseDown.bind(this);
    this.hMouseMove = this.hMouseMove.bind(this);
  }

  hMouseUp(e){
    this.setState({socketCaptured : {id : 0, type : SocketType.Input}});
    this.m_canvasRef.style.zIndex = "0";
  }

  hMouseDown(e: React.MouseEvent<HTMLElement>){
            
  }

  hMouseMove(e : React.MouseEvent<HTMLElement>){

    if (this.state.socketCaptured.id){

      let brect = this.m_canvasRef.getBoundingClientRect();  
      const mpos = { x : e.clientX - brect.left,
                     y : e.clientY - brect.top};    
     
      let ctx = this.getCanvasContext();
            
      this.drawLine(ctx, this.m_mouseStartMem, mpos);

      this.drawAll(ctx);
          
      let isPntIntoRect = function (pnt : IPoint, rect : IRect) {
          return rect.x <= pnt.x && pnt.x <= rect.x + rect.w &&
                 rect.y <= pnt.y && pnt.y <= rect.y + rect.h;
      }

      let task = PipelineTasks.get(this.state.socketCaptured.id);
      for (const pt of PipelineTasks.getAll().values()){
        if (isPntIntoRect(mpos, pt.setts.socketInRect) && !task.nextTasksId.find(v=>v==pt.id)){
          task.nextTasksId.push(pt.id);
          PipelineTasks.upd(task);
          ServerAPI.changePipelineTask(task, ()=>0, ()=>0);

          let nextTask = PipelineTasks.get(pt.id);
          nextTask.prevTasksId.push(task.id);
          PipelineTasks.upd(nextTask);
          ServerAPI.changePipelineTask(nextTask, ()=>0, ()=>0);          
        }
      }
    }
  }

  hTaskMove(id : number){
   
    let ctx = this.getCanvasContext();

    this.drawAll(ctx);
  }

  drawAll(ctx : CanvasRenderingContext2D){
    for (const t of PipelineTasks.getAll().values()){
      for (const nt of t.nextTasksId){        
        this.drawLine(ctx, t.setts.socketOutPoint, PipelineTasks.get(nt).setts.socketInPoint);
      }
    }
  } 

  getCanvasContext(){
    const view = {left : this.m_viewRef.scrollLeft,
                  top : this.m_viewRef.scrollTop,
                  width : this.m_viewRef.offsetWidth,
                  height: this.m_viewRef.offsetHeight}

    var ctx = this.m_canvasRef.getContext("2d");

    ctx.clearRect(view.left, view.top, view.width, view.height);

    ctx.strokeStyle = "green";
    ctx.lineWidth = 1;
    ctx.globalAlpha = 1;   

    return ctx;
  }

  drawLine(ctx : CanvasRenderingContext2D, bPt : IPoint, ePt : IPoint){
    ctx.beginPath();
    
    ctx.moveTo(bPt.x, bPt.y);
    ctx.bezierCurveTo(bPt.x + (ePt.x - bPt.x)/2, bPt.y, 
                      bPt.x + (ePt.x - bPt.x)/2, ePt.y, 
                      ePt.x, ePt.y);   
    ctx.stroke();
  }

  render(){  

    const Tasks = observer(() => {
      if (!this.m_firstLoad && this.m_viewRef && PipelineTasks.getAll().size){
        this.m_firstLoad = true;
        let ctx = this.getCanvasContext();
        this.drawAll(ctx);
      }

      let tasks = [];
      let selPipelines = Pipelines.getSelected();
      let selPplId = selPipelines.length ? selPipelines[0].id : 0;
      for (let t of PipelineTasks.getByPPlId(selPplId)){ 
        if (t.setts.isVisible){    
          tasks.push(<GraphTask title={t.name} id={t.id} key={t.id}
                                moveEnabled={this.state.socketCaptured.id != t.id} 
                                hMove={this.hTaskMove}
                                hSocketInputСaptured={(id, mpos)=>{
                                  this.m_mouseStartMem = mpos;
                                  this.setState({socketCaptured : {id, type : SocketType.Input}});
                                  this.m_canvasRef.style.zIndex = "1000";
                                }} 
                                hSocketOutputСaptured={(id, mpos)=>{
                                  this.m_mouseStartMem = mpos;
                                  this.setState({socketCaptured : {id, type : SocketType.Output}});
                                  this.m_canvasRef.style.zIndex = "1000";
                                }} 
                                hHide={(id)=>{
                                  t.setts.isVisible = false;
                                  PipelineTasks.upd(t);
                                  ServerAPI.changePipelineTask(t,()=>0,()=>0);
                                }}/>);
        }
      }

      return <div className="graphPanel">
               {tasks}
             </div>           
    });
    return <div style={{ width:"100vw", position:"relative", overflow:"auto"}} ref={ el => this.m_viewRef = el} >   
             <canvas className="graphPanel" height="2048px" width="4096px" ref={ el => this.m_canvasRef = el }
                     onMouseMove = { this.hMouseMove }
                     onMouseUp = { this.hMouseUp }
                     onMouseDown = { this.hMouseDown }></canvas>
             <Tasks/>
           </div>
  }
}
