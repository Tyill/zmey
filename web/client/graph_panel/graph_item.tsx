import React from "react";
import {Card} from "react-bootstrap";
import Draggable from 'react-draggable';

interface IProps { 
  title : string;
  labelDelete : string;
  labelEdit : string;
  hEdit : () => any;
  hDelete : () => any;
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
    return (
      <Draggable bounds="parent">
      <div style={{ maxWidth:"150px", border: "1px solid #dbdbdb", padding: 10, borderRadius: "5px"}}
            onMouseEnter={(e)=>this.setState({isShowBtn : true})}
            onMouseLeave={(e)=>this.setState({isShowBtn : false})}>
        {this.props.title}
        {this.state.isShowBtn ?
          <span>
            <a className = "icon-delete"
               title={this.props.labelDelete}
               style={{ float:"right", marginLeft: "10px" }} 
               onClick={this.props.hDelete}>
            </a>
            <a className="icon-edit" 
               title= {this.props.labelEdit}
               style={{ float:"right", marginLeft: "10px" }} 
               onClick={this.props.hEdit}>
            </a>            
          </span>  
        : ""}      
      </div>
      </Draggable>
    )
  }
}
