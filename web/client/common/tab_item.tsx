import React from "react";
import {Tab, CloseButton} from "react-bootstrap";

interface IProps { 
  title : string;
  key : number; 
  id : number;
  hDelete : (id : number) => any;
};
interface IState { 
  isShowBtn : boolean; 
};

export default
class TabItem extends React.Component<IProps, IState>{  
  constructor(props : IProps){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    return (
      <Tab id={this.props.id.toString()} 
          eventKey={this.props.id.toString()}
          onMouseEnter={(e)=>this.setState({isShowBtn : true})}
          onMouseLeave={(e)=>this.setState({isShowBtn : false})}
          title={
          <span>
            {this.props.title} 
            {this.state.isShowBtn ?
            <CloseButton style={{paddingLeft:5}} onClick={(e)=>this.props.hDelete(this.props.id)}/> 
            :""}
          </span>}>
      </Tab>
    )
  }
}
