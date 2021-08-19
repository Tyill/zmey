import React from "react";
import GraphItem from "./graph_item"
import { IPipelineTask } from "../types";
import { PipelineTasks} from "../store/store";
import { observer } from "mobx-react-lite"

interface IProps { 
  pplId : number;
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

    const Tasks = observer(() => {
      let tasks = [];
      for (let t of PipelineTasks.getByPPlId(this.props.pplId).values()){ 
        if (t.isVisible){    
          tasks.push(<GraphItem title={t.name} id={t.id} key={t.id}
                                hHide={(id)=>{
                                  t.isVisible = 0;
                                  PipelineTasks.upd(t)}}/>);
        }
      }

      return <div style={{ width:"10000px", height:"10000px", position:"absolute"}}>
               {tasks}
             </div>           
    });
    return <div>
             <Tasks/>
           </div>
  }
}
