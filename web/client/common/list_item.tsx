import React from "react";
import {ListGroup} from "react-bootstrap";

interface IProps { 
  title : string;
  labelDelete : string;
  labelEdit : string;
  tooltip : string;
  key : number; 
  id : number;   
  hEdit : () => any;
  hDelete : () => any;
  hDClickItem : (id : number) => any;
};
interface IState { 
  isShowBtn : boolean; 
};

export default
class ListItem extends React.Component<IProps, IState>{  
  constructor(props : IProps){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    return (
      <ListGroup.Item action id={this.props.id.toString()}
                      title={this.props.tooltip}
                      onMouseEnter={(e)=>this.setState({isShowBtn : true})}
                      onMouseLeave={(e)=>this.setState({isShowBtn : false})}
                      onDoubleClick={(e)=>this.props.hDClickItem(parseInt(e.target.id, 10))}
                      >
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
