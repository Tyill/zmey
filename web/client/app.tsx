import React from "react";
import ReactDOM from "react-dom";
import CentralWidget from "./central_widget/central_widget";
import * as ServerAPI from "./server_api/server_api"
import { Container, Row, Col, Image } from "react-bootstrap";

import "./css/style.less";
import { IUser, IPipeline, IPipelineTask, ITaskTemplate, IEvent, ITask, MessType } from "./types";
import { Pipelines, TaskTemplates, PipelineTasks, Events, Tasks} from "./store/store";

interface IProps {
};

interface IState {
  userName : string;
  statusMessType : MessType;
  statusMess : string; 
};

class App extends React.Component<IProps, IState>{
  
  private m_toutStatusMess : number = 0;
  private m_toutTaskUpdate : number = 0;
  
  constructor(props : IProps){
    super(props);

    this.objFromJS = this.objFromJS.bind(this);

    this.state = { userName : "",
                   statusMess: "",
                   statusMessType: MessType.Ok}

    this.updateTaskState = this.updateTaskState.bind(this);
  }
  
  objFromJS(s : string){
    s = s.replace(/'/g, '"');
    s = s.replace(/True/g, 'true');
    s = s.replace(/False/g, 'false');
    return JSON.parse(s);
  }

  componentDidMount() {    
    ServerAPI.getUser((usr : IUser)=>{  
      this.setState({userName : usr.name});
    },
    ()=>this.setStatusMess("Server error get user", MessType.Error));

    ServerAPI.getAllPipelines((pipelines : Array<IPipeline>)=>{  
      let ppl = new Map<number, IPipeline>();
      for (let p of pipelines){
        p.setts = this.objFromJS(p.setts as unknown as string);
        ppl.set(p.id, p);
      }      
      Pipelines.setAll(ppl);   
    },
    ()=>this.setStatusMess("Server error fill Pipelines", MessType.Error));

    ServerAPI.getAllTaskTemplates((taskTemplates : Array<ITaskTemplate>)=>{
      let ttl = new Map<number, ITaskTemplate>();
      for (let t of taskTemplates){
        ttl.set(t.id, t);
      }
      TaskTemplates.setAll(ttl);
    },
    ()=>this.setStatusMess("Server error fill TaskTeplates", MessType.Error));

    ServerAPI.getAllPipelineTasks((pipelineTasks : Array<IPipelineTask>)=>{
      let ppt = new Map<number, IPipelineTask>();
      for (let pt of pipelineTasks){
        pt.setts = this.objFromJS(pt.setts as unknown as string);
        ppt.set(pt.id, pt);
      }
      PipelineTasks.setAll(ppt);
    },
    ()=>this.setStatusMess("Server error fill PipelineTasks", MessType.Error));

    ServerAPI.getAllEvents((events : Array<IEvent>)=>{
      let evs = new Map<number, IEvent>();
      for (let ev of events){
        evs.set(ev.id, ev);
      }
      Events.setAll(evs);
    },
    ()=>this.setStatusMess("Server error fill Events", MessType.Error));

    this.updateTaskState();
  }

  updateTaskState(){
    if (this.m_toutTaskUpdate) 
      clearTimeout(this.m_toutTaskUpdate);

    let pplTaskId = 0;
    for (let t of PipelineTasks.getAll().values()){
      if (t.setts.isSelected && !t.setts.isMoved){
        pplTaskId = t.id;
        break;
      }
    }   
    if (pplTaskId){
      ServerAPI.getTaskState(pplTaskId, (states : Array<ITask>)=>{
        Tasks.setAll(states); 
        this.m_toutTaskUpdate = setTimeout(this.updateTaskState, 1000);
      },      
      ()=>{
        this.setStatusMess("Server error of task state", MessType.Error);
        this.m_toutTaskUpdate = setTimeout(this.updateTaskState, 1000);
      })
    }else{
      this.m_toutTaskUpdate = setTimeout(this.updateTaskState, 1000);
    }    
  }
  
  setStatusMess(mess : string, statusMessType : MessType = MessType.Ok){
    this.setState({statusMess : mess, statusMessType});    
    if (this.m_toutStatusMess) clearTimeout(this.m_toutStatusMess);
    this.m_toutStatusMess = setTimeout(() => { 
      this.setState({statusMess : "", statusMessType : MessType.Ok});
    }, 3000)
  }

  render(){         
    return(
      <Container className="d-flex flex-column h-100 m-0 p-0" fluid> 
        <Row noGutters={true} className="borderBottom  menuHeader">
          <Col className="d-flex flex-row">               
            <Image src="images/label.svg" style={{ margin: 5}} title="Application for schedule and monitor workflows"></Image>
            <p style={{ marginLeft: "auto", alignSelf: "center", marginTop:"15px", marginRight: "10px"}}>{this.state.userName}</p>
            <a className = "icon-logout"
               style={{ alignSelf: "center", marginRight: "20px"}}
               title="Logout"
               href="auth/login"></a>
          </Col>         
        </Row>
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