import React from "react";
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

    let iconStyle={ } as React.CSSProperties;
    if (!this.state.isShowBtn){
      iconStyle.visibility = "hidden"; 
    }

    return (
      <Draggable bounds="parent">
      <div className="graphPipelineTask unselectable" 
            onMouseEnter={(e)=>this.setState({isShowBtn : true})}
            onMouseLeave={(e)=>this.setState({isShowBtn : false})}>
        {this.props.title}
        &nbsp;&nbsp;
        <a className="icon-edit" 
            title= {this.props.labelEdit}
            style={iconStyle}
            onClick={this.props.hEdit}>
        </a> 
        &nbsp;   
        <a className = "icon-delete"
            title={this.props.labelDelete}
            style={iconStyle}
            onClick={this.props.hDelete}>
        </a>  
      </div>
      </Draggable>
    )
  }
}
