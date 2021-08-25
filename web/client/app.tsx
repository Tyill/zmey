import React from "react";
import ReactDOM from "react-dom";
import CentralWidget from "./central_widget/central_widget";
import * as ServerAPI from "./server_api/server_api"
import { Container, Row } from "react-bootstrap";

import "./css/style.less";
import { IPipeline, IPipelineTask, ITaskTemplate, IEvent, MessType } from "./types";
import { Pipelines, TaskTemplates, PipelineTasks, Events} from "./store/store";

interface IProps {
};

interface IState {
  statusMessType : MessType;
  statusMess : string; 
};

class App extends React.Component<IProps, IState>{
  
  private m_tout : number = 0;
  
  constructor(props : IProps){
    super(props);

    this.objFromJS = this.objFromJS.bind(this);

    this.state = { statusMess: "",
                   statusMessType: MessType.Ok}
  }
  
  objFromJS(s : string){
    s = s.replace(/'/g, '"');
    s = s.replace(/True/g, 'true');
    s = s.replace(/False/g, 'false');
    return JSON.parse(s);
  }

  componentDidMount() {    
    ServerAPI.getAllPipelines((pipelines : Array<IPipeline>)=>{  
      let ppl = new Map<number, IPipeline>();
      for (let p of pipelines){
        p.setts = this.objFromJS(p.setts as unknown as string);
        ppl.set(p.id, p);
      }      
      Pipelines.setAll(ppl);   
    },
    ()=>this.setStatusMess("Server error fill Pipelines"));

    ServerAPI.getAllTaskTemplates((taskTemplates : Array<ITaskTemplate>)=>{
      let ttl = new Map<number, ITaskTemplate>();
      for (let t of taskTemplates){
        ttl.set(t.id, t);
      }
      TaskTemplates.setAll(ttl);
    },
    ()=>this.setStatusMess("Server error fill TaskTeplates"));

    ServerAPI.getAllPipelineTasks((pipelineTasks : Array<IPipelineTask>)=>{
      let ppt = new Map<number, IPipelineTask>();
      for (let pt of pipelineTasks){
        pt.setts = this.objFromJS(pt.setts as unknown as string);
        ppt.set(pt.id, pt);
      }
      PipelineTasks.setAll(ppt);
    },
    ()=>this.setStatusMess("Server error fill PipelineTasks"));

    ServerAPI.getAllEvents((events : Array<IEvent>)=>{
      let evs = new Map<number, IEvent>();
      for (let ev of events){
        ev.setts = this.objFromJS(ev.setts as unknown as string);
        evs.set(ev.id, ev);
      }
      Events.setAll(evs);
    },
    ()=>this.setStatusMess("Server error fill Events"));
  }
  
  setStatusMess(mess : string, statusMessType : MessType = MessType.Ok){
    this.setState({statusMess : mess, statusMessType});    
    if (this.m_tout) clearTimeout(this.m_tout);
    this.m_tout = setTimeout(() => { 
      this.setState({statusMess : "", statusMessType : MessType.Ok});
    }, 3000)
  }

  render(){         
    return(
      <Container className="d-flex flex-column h-100 m-0 p-0" fluid> 
        <Row noGutters={true} className="h-100">      
          <CentralWidget setStatusMess={(mess:string)=>this.setStatusMess(mess)}/>
        </Row>
        <Row noGutters={true} className="borderTop"
             style={{ color: this.state.statusMessType == MessType.Ok ? "black" : "red",
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