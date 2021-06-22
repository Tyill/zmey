import React from "react";
import {ListGroup} from "react-bootstrap";

interface IPropsTaskItem { 
  title : string;
  tooltip : string;
  key : number;   
  hEdit : () => any;
  hDelete : () => any;
};
interface IStateTaskItem { 
  isShowBtn : boolean; 
};

export default
class TaskItem extends React.Component<IPropsTaskItem, IStateTaskItem>{  
  constructor(props : IPropsTaskItem){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    return (
      <ListGroup.Item action title={this.props.tooltip}
                      onMouseEnter={(e)=>this.setState((oldState, props)=>{ let isShowBtn = true; return {isShowBtn}})}
                      onMouseLeave={(e)=>this.setState((oldState, props)=>{ let isShowBtn = false; return {isShowBtn}})}>
        {this.props.title}
        {this.state.isShowBtn ?
          <span>
            <a className = "icon-delete"
                  title="Delete Task Template"
                  style={{float: "right", marginLeft: "20px" }} 
                  onClick={this.props.hDelete}>
            </a>
            <a className="icon-edit" 
                  title= "Edit Task Template"
                  style={{float: "right"}} 
                  onClick={this.props.hEdit}>
            </a>            
          </span>
        : ""}
      </ListGroup.Item>
    )
  }
}
