import React from "react";
import GraphTask from "./graph_task"
import { observer} from "mobx-react-lite"

import { IPoint, IRect } from "../types";
import { PipelineTasks, Pipelines} from "../store/store";
import * as ServerAPI from "../server_api/server_api";

export
enum SocketType{
  Input,
  Output,
}

interface IProps { 
};
interface IState { 
  socketCaptured : {id : number, type : SocketType};  
};

export default
class GraphPanel extends React.Component<IProps, IState>{  
  
  private m_canvasRef : HTMLCanvasElement;
  private m_mouseStartMem : {x : number, y : number};
  private m_firstLoad : boolean;
  private m_selectedPplId : number;

  constructor(props : IProps){
    super(props);  
    this.m_canvasRef = null;  
    this.m_mouseStartMem = {x : 0, y : 0};
    this.m_firstLoad = false;
    this.m_selectedPplId = 0;

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
    
    if (this.state.socketCaptured.id){

      let brect = this.m_canvasRef.getBoundingClientRect();  
      const mpos = { x : e.clientX - brect.left,
                     y : e.clientY - brect.top};    
     
      let isPntIntoRect = function (pnt : IPoint, rect : IRect) {
        return rect.x <= pnt.x && pnt.x <= rect.x + rect.w &&
              rect.y <= pnt.y && pnt.y <= rect.y + rect.h;
      }
     
      let task = PipelineTasks.get(this.state.socketCaptured.id);
      for (const pt of PipelineTasks.getByPPlId(this.m_selectedPplId).values()){
        if ((this.state.socketCaptured.type == SocketType.Output) && 
             isPntIntoRect(mpos, pt.setts.socketInRect) && !task.nextTasksId.find(v=>v==pt.id)){
          task.nextTasksId.push(pt.id);
          PipelineTasks.upd(task);
          ServerAPI.changePipelineTask(task);
         
          let nextTask = PipelineTasks.get(pt.id);
          nextTask.prevTasksId.push(task.id);
          PipelineTasks.upd(nextTask);
          ServerAPI.changePipelineTask(nextTask);          
        }
        else 
        if ((this.state.socketCaptured.type == SocketType.Input) &&
             isPntIntoRect(mpos, pt.setts.socketOutRect) && !task.prevTasksId.find(v=>v==pt.id)){
          task.prevTasksId.push(pt.id);
          PipelineTasks.upd(task);
          ServerAPI.changePipelineTask(task);

          let prevTask = PipelineTasks.get(pt.id);
          prevTask.nextTasksId.push(task.id);
          PipelineTasks.upd(prevTask);
          ServerAPI.changePipelineTask(prevTask);
        }
      }

      this.drawAll(this.getCanvasContext());

      this.setState({socketCaptured : {id : 0, type : SocketType.Input}});
    }
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
    }
  }

  hTaskMove(id : number){
       
    this.drawAll(this.getCanvasContext());
  }

  drawAll(ctx : CanvasRenderingContext2D){
    for (const t of PipelineTasks.getByPPlId(this.m_selectedPplId)){  
      for (const nt of t.nextTasksId){        
        this.drawLine(ctx, t.setts.socketOutPoint, PipelineTasks.get(nt).setts.socketInPoint);
      }
    }
  } 

  getCanvasContext(){
    
    var ctx = this.m_canvasRef.getContext("2d");

    ctx.clearRect(0, 0, this.m_canvasRef.clientWidth, this.m_canvasRef.clientHeight);

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
      let selPipelines = Pipelines.getSelected();
      if (selPipelines.length && (selPipelines[0].id != this.m_selectedPplId)){
        this.m_selectedPplId = selPipelines[0].id;
        this.drawAll(this.getCanvasContext());
      } 
      else if (!selPipelines.length && (this.m_selectedPplId != 0)){
        this.m_selectedPplId = 0;
        this.drawAll(this.getCanvasContext());
      } 
      
      if (!this.m_firstLoad && this.m_canvasRef && PipelineTasks.getByPPlId(this.m_selectedPplId).length){
        this.m_firstLoad = true;
        this.drawAll(this.getCanvasContext());
      }

      let tasks = [];
      for (let t of PipelineTasks.getByPPlId(this.m_selectedPplId)){ 
        if (t.setts.isVisible){    
          tasks.push(<GraphTask title={t.name} id={t.id} key={t.id}
                                moveEnabled={this.state.socketCaptured.id != t.id} 
                                hMove={this.hTaskMove}
                                hSocketInputСaptured={(id, mpos)=>{
                                  this.m_mouseStartMem = mpos;
                                  this.setState({socketCaptured : {id, type : SocketType.Input}});
                                }} 
                                hSocketOutputСaptured={(id, mpos)=>{
                                  this.m_mouseStartMem = mpos;
                                  this.setState({socketCaptured : {id, type : SocketType.Output}});
                                }} 
                                hHide={(id)=>{
                                  let task = PipelineTasks.get(t.id);
                                  task.setts.isVisible = false;
                                  PipelineTasks.upd(task);
                                  ServerAPI.changePipelineTask(task);
                                }}/>);
        }
      }

      return <div className="graphPanel" 
                  onMouseMove = { this.hMouseMove }
                  onMouseUp = { this.hMouseUp }
                  onMouseDown = { this.hMouseDown }>
               {tasks}
             </div>           
    });
    return <div>   
             <canvas className="graphPanel" height="2048px" width="4096px" 
                     ref={ el => this.m_canvasRef = el }/>
             <Tasks/>
           </div>
  }
}
