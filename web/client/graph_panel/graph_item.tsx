import React from "react";
import Draggable from 'react-draggable';
import {CloseButton} from "react-bootstrap";

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

    return (
      <Draggable bounds="parent" onStop={(e)=>console.log(e)}>
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
