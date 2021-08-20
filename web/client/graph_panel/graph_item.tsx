import React from "react";
import Draggable, {ControlPosition} from 'react-draggable';
import {CloseButton} from "react-bootstrap";
import { Pipelines } from "../store/store_pipeline";
import { PipelineTasks } from "../store/store_pipeline_task";
import * as ServerAPI from "../server_api/server_api";

interface IProps { 
  title : string;
  id : number;
  hHide : (id : number) => any;
};
interface IState { 
  isShowBtn : boolean; 
};

export default
class GraphItem extends React.Component<IProps, IState>{  
  constructor(props : IProps){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    
    let closeBtnStyle = { 
      paddingLeft: "5px",
      border : "none",
    } as React.CSSProperties;    
    if (!this.state.isShowBtn){
      closeBtnStyle.visibility = "hidden"; 
    }

    let task = PipelineTasks.get(this.props.id);

    return (

      <Draggable bounds="parent" position={{x:task.positionX,y:task.positionY}}
                 onStop={(e, data)=>{
                   PipelineTasks.setPosition(this.props.id, data.x, data.y);
                   ServerAPI.changePipelineTask(PipelineTasks.get(this.props.id), ()=>0, ()=>0);
                 }}>
      <div className="graphPipelineTask unselectable" 
            onMouseEnter={(e)=>this.setState({isShowBtn : true})}
            onMouseLeave={(e)=>this.setState({isShowBtn : false})}>
        {this.props.title}
        <CloseButton style={closeBtnStyle} 
                     title="Hide"
                     onClick={(e)=>{this.props.hHide(this.props.id);
                                    e.stopPropagation();}}/>        
      </div>
      </Draggable>
    )
  }
}
