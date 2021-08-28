import React from "react";
import {CloseButton} from "react-bootstrap";

interface IProps { 
  title : string;
  key : number; 
  id : number;
  isSelected : boolean;
  hSelect : (id : number) => any;
  hHide : (id : number) => any;
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

    let titleStyle={
      color : "gray"
    } as React.CSSProperties;
    if (this.props.isSelected){
      titleStyle.color = "black";
      titleStyle.borderBottom = "1px solid white"; 
    }

    let closeBtnStyle = { 
      paddingLeft: "5px",
      border : "none",
    } as React.CSSProperties;    
    if (!this.state.isShowBtn){
      closeBtnStyle.visibility = "hidden"; 
    }

    return (
      <div id={this.props.id.toString()} 
          className="tabPipeline unselectable" style={titleStyle}
          onMouseEnter={(e)=>this.setState({isShowBtn : true})}
          onMouseLeave={(e)=>this.setState({isShowBtn : false})}
          onClick={(e)=>this.props.hSelect(this.props.id)}
          >
        {this.props.title}
        <CloseButton style={closeBtnStyle} 
                     title="Close"
                     onClick={(e)=>{this.props.hHide(this.props.id);
                                    e.stopPropagation();}}/>
      </div>
    )
  }
}
