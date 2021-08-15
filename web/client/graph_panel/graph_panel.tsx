import React from "react";
import GraphItem from "./graph_item"

interface IProps { 
  pplId : Number;
};
interface IState { 
};

export default
class GraphPanel extends React.Component<IProps, IState>{  
  constructor(props : IProps){
    super(props);    
    this.state  = { };   
  }

  render(){  

    //let tasks = PipelineTasks.getByPPlId(this.props.pplId);

    return (
      <div style={{ width:"10000px", height:"10000px", position:"absolute"}}>
        <GraphItem title="task" labelDelete="delete" labelEdit="edit" hEdit={()=>0} hDelete={()=>0}/>
      </div>
    )
  }
}
