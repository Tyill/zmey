import React from "react";
import {Card} from "react-bootstrap";

interface IPropsPipelineHeader {
  hNew : () => any;
  hEdit : () => any;
  hDelete : () => any;
};
interface IStatePipelineHeader { 
  isShowBtn : boolean; 
};

export default
class PipelineHeader extends React.Component<IPropsPipelineHeader, IStatePipelineHeader>{  
  constructor(props : IPropsPipelineHeader){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){           
    return (
      <Card.Header as="h6" style={{height: "46px"}}
                           onMouseEnter={(e)=>this.setState((oldState, props)=>{ let isShowBtn = true; return {isShowBtn}})}
                           onMouseLeave={(e)=>this.setState((oldState, props)=>{ let isShowBtn = false; return {isShowBtn}})}>
        Task Pipelines
        {this.state.isShowBtn ?
         <span>
          <a className="icon-new" 
            title="New Task Pipeline"
            style={{marginLeft: "20px"}} 
            onClick={this.props.hNew}>
          </a> 
          <a className="icon-edit" 
            title= "Edit Task Pipeline"
            style={{marginLeft: "20px"}} 
            onClick={this.props.hEdit}>
          </a>
          <a className="icon-delete" 
            title="Delete Task Pipeline"
            style={{marginLeft: "20px" }} 
            onClick={this.props.hDelete}>
          </a>
          </span>
        : ""}
      </Card.Header>
    )
  }
}