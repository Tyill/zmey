import React from "react";
import {Card} from "react-bootstrap";

interface IPropsTaskHeader {
  hNew : () => any;
  title : string;
  labelNew : string;
};
interface IStateTaskHeader { 
  isShowBtn : boolean; 
};

export default
class ListHeader extends React.Component<IPropsTaskHeader, IStateTaskHeader>{  
  constructor(props : IPropsTaskHeader){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){           
    return (
      <Card.Header as="h6" style={{height: "46px"}}
                           onMouseEnter={(e)=>this.setState((oldState, props)=>{ let isShowBtn = true; return {isShowBtn}})}
                           onMouseLeave={(e)=>this.setState((oldState, props)=>{ let isShowBtn = false; return {isShowBtn}})}>
        {this.props.title}
        {this.state.isShowBtn ? 
          <a className="icon-new"      
             title={this.props.labelNew}     
             style={{float: "right"}} 
             onClick={this.props.hNew}>
          </a>
        : ""}
      </Card.Header>
    )
  }
}