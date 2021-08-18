import React from "react";
import {Card} from "react-bootstrap";

interface IProps {
  hNew : () => any;
  title : string;
  labelNew : string;
};
interface IState { 
  isShowBtn : boolean; 
};

export default
class ListHeader extends React.Component<IProps, IState>{  
  constructor(props : IProps){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){           
    return (
      <Card.Header className="unselectable" as="h6" style={{ borderRadius:0, height: "46px"}}
                   onMouseEnter={(e)=>this.setState({isShowBtn: true})}
                   onMouseLeave={(e)=>this.setState({isShowBtn: false})}>
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