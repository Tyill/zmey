import React from "react";
import GraphItem from "./graph_task"
import { observer } from "mobx-react-lite"

import { IPipelineTask } from "../types";
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

  constructor(props : IProps){
    super(props);  
    this.m_canvasRef = null;  
    this.m_mouseStartMem = {x : 0, y : 0};

    this.state = {socketCaptured : {id : 0, type : SocketType.Input}};

    this.hMouseUp = this.hMouseUp.bind(this);
    this.hMouseDown = this.hMouseDown.bind(this);
    this.hMouseMove = this.hMouseMove.bind(this);
  }

  hMouseUp(e){
    this.setState({socketCaptured : {id : 0, type : SocketType.Input}});
    this.m_canvasRef.style.zIndex = "0";
  }

  hMouseDown(e){
    
  }

  hMouseMove(e : React.MouseEvent<HTMLElement>){

    if (this.state.socketCaptured.id){

      let brect = this.m_canvasRef.getBoundingClientRect();     
      const mpos = { x : e.clientX - brect.left,
                    y : e.clientY - brect.top};    
     
      const view = {width : this.m_canvasRef.parentElement.offsetWidth,
                    height: this.m_canvasRef.parentElement.offsetHeight}                   
      
      var ctx = this.m_canvasRef.getContext("2d");
        
      ctx.clearRect(this.m_canvasRef.scrollLeft, this.m_canvasRef.scrollTop,
                    view.width, view.height);

      ctx.strokeStyle = "green";
      ctx.lineWidth = 1;
      ctx.globalAlpha = 1;
    
      ctx.beginPath();

      let bPnt = this.m_mouseStartMem;

      ctx.moveTo(bPnt.x, bPnt.y);
      ctx.bezierCurveTo(bPnt.x + (mpos.x - bPnt.x)/2, bPnt.y, 
                        bPnt.x + (mpos.x - bPnt.x)/2, mpos.y, 
                        mpos.x, mpos.y);   
      ctx.stroke();
    }
  }

  render(){  

    const Tasks = observer(() => {
      let tasks = [];
      let selPipelines = Pipelines.getSelected();
      let selPplId = selPipelines.length ? selPipelines[0].id : 0;
      for (let t of PipelineTasks.getByPPlId(selPplId)){ 
        if (t.setts.isVisible){    
          tasks.push(<GraphItem title={t.name} id={t.id} key={t.id}
                                moveEnabled={this.state.socketCaptured.id != t.id} 
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
    return <div style={{width:"100vw"}}>
             <canvas className="graphPanel" height="10000px" width="10000px" ref={ el => this.m_canvasRef = el }
                     onMouseMove = { this.hMouseMove }
                     onMouseUp = { this.hMouseUp }
                     onMouseDown = { this.hMouseDown }></canvas>
             <Tasks/>             
           </div>
  }
}
