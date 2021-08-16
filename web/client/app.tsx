import React from "react";
import ReactDOM from "react-dom";
import CentralWidget from "./central_widget/central_widget";
import { ServerAPI } from "./server_api/server_api"
import { Container, Row } from "react-bootstrap";

import "./css/style.less";
import { IPipeline, IPipelineTask, ITaskTemplate } from "./types";
import { Pipelines, TaskTemplates, PipelineTasks} from "./store/store";

interface IProps {
};

interface IState {
  isStatusOk : boolean;
  statusMess : string; 
};

class App extends React.Component<IProps, IState>{
  
  private m_tout : number = 0;
  
  constructor(props : IProps){
    super(props);

    this.state = { isStatusOk : true, statusMess:""}
  }
  
  componentDidMount() {    
    ServerAPI.getAllPipelines((pipelines : Array<IPipeline>)=>{      
      let ppl = new Map<Number, IPipeline>();
      for (let p of pipelines){
        ppl.set(p.id, p);
      }
      Pipelines.setAll(ppl);
    },
    ()=>this.setStatusMess("Server error fill Pipelines"));

    ServerAPI.getAllTaskTemplates((taskTemplates : Array<ITaskTemplate>)=>{
      let ttl = new Map<Number, ITaskTemplate>();
      for (let t of taskTemplates){
        ttl.set(t.id, t);
      }
      TaskTemplates.setAll(ttl);
    },
    ()=>this.setStatusMess("Server error fill TaskTeplates"));

    ServerAPI.getAllPipelineTasks((pipelineTasks : Array<IPipelineTask>)=>{
      let ppt = new Map<Number, IPipelineTask>();
      for (let pt of pipelineTasks){
        ppt.set(pt.id, pt);
      }
      PipelineTasks.setAll(ppt);
    },
    ()=>this.setStatusMess("Server error fill PipelineTasks"));
  }
  
  setStatusMess(mess : string, ok : boolean = true){
    this.setState({statusMess : mess, isStatusOk : ok});    
    if (this.m_tout) clearTimeout(this.m_tout);
    this.m_tout = setTimeout(() => { 
      this.setState({statusMess : "", isStatusOk : true});
    }, 3000)
  }

  render(){         
    return(
      <Container className="d-flex flex-column h-100 m-0 p-0" fluid> 
        <Row noGutters={true} className="h-100">      
          <CentralWidget setStatusMess={(mess:string)=>this.setStatusMess(mess)}/>
        </Row>
        <Row noGutters={true} className="borderTop"
             style={{ color: this.state.isStatusOk ? "black" : "red",
                      width:"100vw", height: 30, paddingLeft:5}} >        
            {this.state.statusMess}         
        </Row>
      </Container>
  )} 
}

////////////////////////////////////////////////////

const root = document.getElementById('root')
if (root){
  ReactDOM.render(
    <App/>,
    root
  );
}