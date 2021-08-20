import React from "react";
import GraphItem from "./graph_item"
import { observer } from "mobx-react-lite"

import { IPipelineTask } from "../types";
import { PipelineTasks, Pipelines} from "../store/store";
import * as ServerAPI from "../server_api/server_api";


interface IProps { 
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
      let selPipelines = Pipelines.getSelected();
      let selPplId = selPipelines.length ? selPipelines[0].id : 0;
      for (let t of PipelineTasks.getByPPlId(selPplId)){ 
        if (t.isVisible){    
          tasks.push(<GraphItem title={t.name} id={t.id} key={t.id}
                                hHide={(id)=>{
                                  t.isVisible = false;
                                  PipelineTasks.upd(t);
                                  ServerAPI.changePipelineTask(t,()=>0,()=>0);
                                }}/>);
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
