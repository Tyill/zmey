import React from "react";

import "../css/style.less";

interface IProps { 
  tooltip : string;
  isChecked : boolean;
  onChange : (on : boolean) => any;
};
interface IState { 
  
};

export default
class Switch extends React.Component<IProps, IState>{  
  constructor(props : IProps){
    super(props);    
    this.state  = {  };   
  }   
  render(){  
    return (
      <label className="switch" title={this.props.tooltip}>
        <input type="checkbox" defaultChecked={this.props.isChecked} onChange={e=>this.props.onChange(e.currentTarget.checked)}/>
        <span className="slider round"></span>
      </label>
    )
  }
}
