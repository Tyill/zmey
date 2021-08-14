import React from "react";
import {ListGroup} from "react-bootstrap";

interface IPropsTaskItem { 
  title : string;
  labelDelete : string;
  labelEdit : string;
  tooltip : string;
  key : number;   
  hEdit : () => any;
  hDelete : () => any;
};
interface IStateTaskItem { 
  isShowBtn : boolean; 
};

export default
class ListItem extends React.Component<IPropsTaskItem, IStateTaskItem>{  
  constructor(props : IPropsTaskItem){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    return (
      <ListGroup.Item action title={this.props.tooltip}
                      onMouseEnter={(e)=>this.setState({isShowBtn : true})}
                      onMouseLeave={(e)=>this.setState({isShowBtn : false})}>
        {this.props.title}
        {this.state.isShowBtn ?
          <span>
            <a className = "icon-delete"
                  title={this.props.labelDelete}
                  style={{float: "right", marginLeft: "20px" }} 
                  onClick={this.props.hDelete}>
            </a>
            <a className="icon-edit" 
                  title= {this.props.labelEdit}
                  style={{float: "right"}} 
                  onClick={this.props.hEdit}>
            </a>            
          </span>
        : ""}
      </ListGroup.Item>
    )
  }
}
