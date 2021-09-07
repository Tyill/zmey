import React from "react";
import { Container, Row, Col, ListGroup } from "react-bootstrap";
import { observer } from "mobx-react-lite"

import TaskTemplateDialogModal from "./task_template_dialog";
import PipelineDialogModal from "./pipeline_dialog";
import PipelineTaskDialogModal from "./pipeline_task_dialog";
import EventDialogModal from "./event_dialog";
import AckDeleteModal, {IAckDeleteDialog} from "../common/ack_delete_modal";
import ListItem from "../common/list_item";
import ListHeader from "../common/list_header";
import TabItem from "../common/tab_item";
import GraphPanel from "../graph/graph_panel";
import TaskStatusWidget from "../task_status_widget/task_status_widget";

import { IPipeline, IPipelineTask, ITaskTemplate, IEvent, MessType } from "../types";
import { Pipelines, TaskTemplates, PipelineTasks, Events} from "../store/store";
import * as ServerAPI from "../server_api/server_api";

import "../css/style.less";
import "../css/fontello.css";
import "bootstrap/dist/css/bootstrap.min.css";


interface IProps {
  setStatusMess : (mess : string, State)=>void;
};

interface IState {
  isShowTaskTemplateConfig : boolean;
  isShowPipelineConfig : boolean;
  isShowPipelineTaskConfig : boolean;
  isShowEventConfig : boolean;
  isShowAckDeleteDialog : boolean;
};

export default
class CentralWidget extends React.Component<IProps, IState>{
   
  private m_selTaskTemplate : ITaskTemplate = {} as ITaskTemplate;
  private m_selPipeline : IPipeline = {} as IPipeline;
  private m_selPipelineTask : IPipelineTask = {} as IPipelineTask;
  private m_selEvent : IEvent = {} as IEvent;
  private m_ackDeleteDialog : IAckDeleteDialog = {} as IAckDeleteDialog;
  
  constructor(props : IProps){
    super(props);

    this.delTaskTemplate = this.delTaskTemplate.bind(this);
    this.delPipeline = this.delPipeline.bind(this);
    this.delPipelineTask = this.delPipelineTask.bind(this);
    this.delEvent = this.delEvent.bind(this);
    this.selectPipeline = this.selectPipeline.bind(this);
    this.hidePipeline = this.hidePipeline.bind(this);
    this.getSelectedPipelineId = this.getSelectedPipelineId.bind(this);
   
    this.m_selTaskTemplate.id = 0;
    this.m_selPipeline.id = 0;
    this.m_selPipelineTask.id = 0;
    this.m_selEvent.id = 0;

    this.state  = { isShowTaskTemplateConfig : false,
                    isShowPipelineConfig : false,
                    isShowPipelineTaskConfig : false,
                    isShowEventConfig : false,
                    isShowAckDeleteDialog : false,    
                  };   
  }    

  delTaskTemplate(){
    ServerAPI.delTaskTemplate(this.m_selTaskTemplate,
      ()=>{
        TaskTemplates.del(this.m_selTaskTemplate.id); 
        this.props.setStatusMess(`Task Template '${this.m_selTaskTemplate.name}' is delete`, MessType.Ok);
        this.setState({isShowAckDeleteDialog : false});
      },
      ()=>this.props.setStatusMess("Server error delete of Task Template", MessType.Error))
  }

  delPipeline(){    
    this.hidePipeline(this.m_selPipeline.id);

    ServerAPI.delPipeline(this.m_selPipeline,
      ()=>{  
        this.setState({isShowAckDeleteDialog : false});
        Pipelines.del(this.m_selPipeline.id);                        
        this.props.setStatusMess(`Pipeline '${this.m_selPipeline.name}' is delete`, MessType.Ok);
      },
      ()=>this.props.setStatusMess("Server error delete of Pipeline", MessType.Error))
  }

  delPipelineTask(){    
    ServerAPI.delPipelineTask(this.m_selPipelineTask,
      ()=>{          
        PipelineTasks.delAllConnections(this.m_selPipelineTask.id);

        PipelineTasks.del(this.m_selPipelineTask.id);
        
        this.props.setStatusMess(`Pipeline Task '${this.m_selPipelineTask.name}' is delete`, MessType.Ok);
        this.setState({isShowAckDeleteDialog : false});  
      },
      ()=>this.props.setStatusMess("Server error delete of Pipeline Task", MessType.Error))
  }

  delEvent(){    
    ServerAPI.delEvent(this.m_selEvent,
      ()=>{     
        Events.del(this.m_selEvent.id);
        
        this.props.setStatusMess(`Event '${this.m_selEvent.name}' is delete`, MessType.Ok);
        this.setState({isShowAckDeleteDialog : false});  
      },
      ()=>this.props.setStatusMess("Server error delete of Event", MessType.Error))
  }
 
  selectPipeline(id : number){
    if (!Pipelines.get(id).setts.isVisible){
      Pipelines.setVisible(id, true);                                    
    }
    for (let p of Pipelines.getSelected()){
      if (p.id != id){
        let ppl = Pipelines.get(p.id);
        ppl.setts.isSelected = false;
        Pipelines.upd(ppl);
        ServerAPI.changePipeline(ppl);
      }
    }
    Pipelines.setSelected(id, true);
    ServerAPI.changePipeline(Pipelines.get(id));
  }

  hidePipeline(id : number){
    Pipelines.setVisible(id, false); 
    if (Pipelines.getSelected().find(p=>p.id == id)){
      const vppls = Pipelines.getVisible();
      if (vppls.length){
        Pipelines.setSelected(vppls[0].id, true)
        ServerAPI.changePipeline(vppls[0]);
      }
    }
    Pipelines.setSelected(id, false);
    ServerAPI.changePipeline(Pipelines.get(id));
  }

  getSelectedPipelineId(){
    let selPipelines = Pipelines.getSelected();
    return selPipelines.length ? selPipelines[0].id : 0;    
  }

  render(){
      
    let PipelineTabs = observer(() => {
      let pipelines = [];
      for (let p of Pipelines.getVisible()){     
        pipelines.push(<TabItem key={p.id} id={p.id}
                                isSelected={ p.setts.isSelected }
                                title={p.name}
                                tooltip={p.description}
                                hSelect={this.selectPipeline} 
                                hHide={this.hidePipeline}
                                >
                        </TabItem>);
      }
      return <div className="tabPanelPipeline">
              {pipelines}
            </div>   
    });
        
    let TaskTemplateList = observer(() => {
      let taskTemlates = [];
      for (let t of TaskTemplates.getAll().values()){
        taskTemlates.push(<ListItem key={t.id} id={t.id} title={t.name} tooltip={t.description}
                                    labelEdit={"Edit Task Template"} labelDelete={"Delete Task Template"}                                                   
                                    hEdit={()=>{
                                      this.m_selTaskTemplate = TaskTemplates.get(t.id);
                                      this.setState({isShowTaskTemplateConfig : true});
                                    }}
                                    hDelete={()=>{
                                      this.m_selTaskTemplate = TaskTemplates.get(t.id);
                                      this.m_ackDeleteDialog.description= this.m_selTaskTemplate.description;
                                      this.m_ackDeleteDialog.title = `Delete '${this.m_selTaskTemplate.name}' Task Template?`;
                                      this.m_ackDeleteDialog.onYes = this.delTaskTemplate; 

                                      this.setState({isShowAckDeleteDialog : true});
                                    }}
                                    hDClickItem={()=>{
                                      this.m_selTaskTemplate = TaskTemplates.get(t.id);
                                      this.setState({isShowTaskTemplateConfig : true});
                                    }}>
                          </ListItem>);
      }
      return <ListGroup className="list-group-flush borderBottom" style={{ maxHeight: "35vh", overflowY:"auto"}} >
               {taskTemlates}
             </ListGroup>
    });

    let PipilineList = observer(() => {
      let pipelines = [];
      for (let p of Pipelines.getAll().values()){
        pipelines.push(<ListItem key={p.id} id={p.id} title={p.name} tooltip={p.description}
                                labelEdit={"Edit Pipeline"} labelDelete={"Delete Pipeline"}                                                   
                                hEdit={()=>{
                                  this.m_selPipeline = Pipelines.get(p.id);
                                  this.setState({isShowPipelineConfig : true});
                                }}
                                hDelete={()=>{
                                  this.m_selPipeline = Pipelines.get(p.id);
                                  this.m_ackDeleteDialog.description= this.m_selPipeline.description;
                                  this.m_ackDeleteDialog.title = `Delete '${this.m_selPipeline.name}' Pipeline and all his tasks?`;
                                  this.m_ackDeleteDialog.onYes = this.delPipeline; 

                                  this.setState({isShowAckDeleteDialog : true});
                                }}
                                hDClickItem={this.selectPipeline}>
                          </ListItem>);
      }
      return <ListGroup className="list-group-flush borderBottom" style={{ maxHeight: "35vh", overflowY:"auto"}} >
               {pipelines}
             </ListGroup>
    });
    
    let PipelineTaskList = observer(() => {
      let tasks = [];      
      for (let t of PipelineTasks.getByPPlId(this.getSelectedPipelineId())){
        tasks.push(<ListItem  key={t.id} id={t.id} title={t.name} tooltip={t.description}
                              labelEdit={"Edit Pipeline Task"}
                              labelDelete={"Delete Pipeline Task"}    
                              hEdit={()=>{
                                this.m_selPipelineTask = PipelineTasks.get(t.id);
                                this.setState({isShowPipelineTaskConfig : true});
                              }}
                              hDelete={()=>{
                                this.m_selPipelineTask = PipelineTasks.get(t.id);
                                this.m_ackDeleteDialog.description = this.m_selPipelineTask.description;
                                this.m_ackDeleteDialog.title = `Delete '${this.m_selPipelineTask.name}' Pipeline Task?`;
                                this.m_ackDeleteDialog.onYes = this.delPipelineTask; 

                                this.setState({isShowAckDeleteDialog : true});
                              }}
                              hDClickItem={()=>{     
                                this.m_selPipelineTask = PipelineTasks.get(t.id);
                                PipelineTasks.setSelected(t.id, true);
                                PipelineTasks.setVisible(t.id, true);
                                ServerAPI.changePipelineTask(this.m_selPipelineTask);

                                for (let st of PipelineTasks.getAll().values()){
                                  if (st.setts.isSelected && (st.id != t.id)){
                                    PipelineTasks.setSelected(st.id, false);
                                    ServerAPI.changePipelineTask(st);
                                  }
                                }                                
                              }}>
                          </ListItem>);
      }
      return <ListGroup className="list-group-flush borderBottom" style={{ maxHeight: "35vh", overflowY:"auto"}} >
               {tasks}
             </ListGroup>
    });

    let EventList = observer(() => {
      let events = [];  
      for (let ev of Events.getAll().values()){
        events.push(<ListItem key={ev.id} id={ev.id} title={ev.name} tooltip={ev.description}
                              labelEdit={"Edit Event"}
                              labelDelete={"Delete Event"}    
                              hEdit={()=>{
                                this.m_selEvent = Events.get(ev.id);
                                this.setState({isShowEventConfig : true});
                              }}
                              hDelete={()=>{
                                this.m_selEvent = Events.get(ev.id);
                                this.m_ackDeleteDialog.description = this.m_selEvent.description;
                                this.m_ackDeleteDialog.title = `Delete '${this.m_selEvent.name}' Event?`;
                                this.m_ackDeleteDialog.onYes = this.delEvent; 

                                this.setState({isShowAckDeleteDialog : true});
                              }}
                              hDClickItem={()=>{
                                this.m_selEvent = Events.get(ev.id);
                                this.setState({isShowEventConfig : true});
                              }}>
                          </ListItem>);
      }
      return <ListGroup className="list-group-flush borderBottom" style={{ maxHeight: "35vh", overflowY:"auto"}} >
               {events}
             </ListGroup>
    });
   
    
    return (
      <>
        <Container className="d-flex flex-column h-100 m-0 p-0" fluid >
          <Row noGutters={true} className="h-100" >
            <Col className="col-2 m-0 p-0 borderRight">   
              <ListHeader title={"Pipeline Tasks"}
                          labelNew={"New Pipeline Task"}
                          hNew={()=>{ this.m_selPipelineTask.id = 0;
                                      this.m_selPipelineTask.pplId = this.getSelectedPipelineId()                            
                                      this.setState({isShowPipelineTaskConfig : true}); }}/>               
              <PipelineTaskList/>

              <ListHeader title={"Pipelines"}
                          labelNew={"New Pipeline"}
                          hNew={()=>{ this.m_selPipeline.id = 0;
                                      this.setState({isShowPipelineConfig : true}); }}/>               
              <PipilineList/> 
            </Col>            
            <Col className="col-8 d-flex flex-column borderRight">                            
              <Row noGutters={true} >
                <PipelineTabs />
              </Row>
              <Row noGutters={true} className="h-100" style={{ position:"relative", overflow:"auto"}}>
                <GraphPanel hStatusMess={this.props.setStatusMess}
                            hShowTaskDialog={(id)=>{
                              this.m_selPipelineTask = PipelineTasks.get(id);
                              this.setState({isShowPipelineTaskConfig : true});
                            }}/>
              </Row>
              <Row noGutters={true} className="borderTop" style={{ minHeight: "20vh"}}>
                <Col className="col">
                  <TaskStatusWidget/>
                </Col>
              </Row>
            </Col>
            <Col className="col-2 m-0 p-0 borderRight">   
              <ListHeader title={"Task Templates"}
                          labelNew={"New Task Template"}
                          hNew={()=>{ this.m_selTaskTemplate.id = 0;
                                      this.setState({isShowTaskTemplateConfig : true}); }}/>               
              <TaskTemplateList/>

              <ListHeader title={"Events"}
                          labelNew={"New Event"}
                          hNew={()=>{ this.m_selEvent.id = 0;
                                      this.setState({isShowEventConfig : true}); }}/>               
              <EventList/> 
            </Col>            
          </Row>
        </Container> 

        <TaskTemplateDialogModal selTaskTemplate={this.m_selTaskTemplate} 
                                 show={this.state.isShowTaskTemplateConfig} 
                                 onHide={()=>this.setState({isShowTaskTemplateConfig : false})}/>
        
        <PipelineDialogModal selPipeline={this.m_selPipeline} 
                             show={this.state.isShowPipelineConfig} 
                             onHide={()=>this.setState({isShowPipelineConfig : false})}
                             onNew={(selPipeline : IPipeline)=>{
                               this.selectPipeline(selPipeline.id);
                             }}/>

        <PipelineTaskDialogModal selPipelineTask={this.m_selPipelineTask} 
                                 show={this.state.isShowPipelineTaskConfig} 
                                 onHide={()=>this.setState({isShowPipelineTaskConfig : false})}/>
       
        <EventDialogModal selEvent={this.m_selEvent} 
                          show={this.state.isShowEventConfig} 
                          onHide={()=>this.setState({isShowEventConfig : false})}/>
       
        <AckDeleteModal description={this.m_ackDeleteDialog.description}
                        title={this.m_ackDeleteDialog.title} 
                        show={this.state.isShowAckDeleteDialog}
                        onYes={this.m_ackDeleteDialog.onYes}  
                        onHide={()=>this.setState({isShowAckDeleteDialog : false})}/>
      </>
    )
  } 
}
