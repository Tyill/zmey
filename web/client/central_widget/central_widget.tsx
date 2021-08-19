import React from "react";
import { Container, Row, Col, Image, ListGroup } from "react-bootstrap";
import { observer } from "mobx-react-lite"

import TaskTemplateDialogModal from "./task_template_dialog";
import PipelineDialogModal from "./pipeline_dialog";
import PipelineTaskDialogModal from "./pipeline_task_dialog";
import AckDeleteModal, {IAckDeleteDialog} from "../common/ack_delete_modal";
import ListItem from "../common/list_item";
import ListHeader from "../common/list_header";
import TabItem from "../common/tab_item";
import GraphPanel from "../graph_panel/graph_panel";

import { IPipeline, IPipelineTask, ITaskTemplate } from "../types";
import { Pipelines, TaskTemplates, PipelineTasks} from "../store/store";
import * as ServerAPI from "../server_api/server_api";

import "../css/style.less";
import "../css/fontello.css";
import "bootstrap/dist/css/bootstrap.min.css";

enum StateEnum{
  Error,
  Ok,
}

interface IProps {
  setStatusMess : (mess : string, State)=>void;
};

interface IState {
  isShowTaskTemplateConfig : boolean;
  isShowPipelineConfig : boolean;
  isShowPipelineTaskConfig : boolean;
  isShowAckDeleteDialog : boolean;
  selTabPipelineId : number;
};

export default
class CentralWidget extends React.Component<IProps, IState>{
   
  private m_selTaskTemplate : ITaskTemplate = {} as ITaskTemplate;
  private m_selPipeline : IPipeline = {} as IPipeline;
  private m_selPipelineTask : IPipelineTask = {} as IPipelineTask;
  private m_ackDeleteDialog : IAckDeleteDialog = {} as IAckDeleteDialog;
  
  constructor(props : IProps){
    super(props);

    this.delTaskTemplate = this.delTaskTemplate.bind(this);
    this.delPipeline = this.delPipeline.bind(this);
    this.delPipelineTask = this.delPipelineTask.bind(this);
    
    this.state  = { isShowTaskTemplateConfig : false,
                    isShowPipelineConfig : false,
                    isShowPipelineTaskConfig : false,
                    isShowAckDeleteDialog : false,
                    selTabPipelineId : 0,                
                  };   
  }    

  delTaskTemplate(){
    ServerAPI.delTaskTemplate(this.m_selTaskTemplate,
      ()=>{
        TaskTemplates.del(this.m_selTaskTemplate.id); 
        this.props.setStatusMess(`Task Template '${this.m_selTaskTemplate.name}' is delete`, StateEnum.Ok);
        this.setState({isShowAckDeleteDialog : false});
      },
      ()=>this.props.setStatusMess("Server error delete of Task Template", StateEnum.Error))
  }

  delPipeline(){
    ServerAPI.delPipeline(this.m_selPipeline,
      ()=>{  
        this.setState({isShowAckDeleteDialog : false});
        Pipelines.del(this.m_selPipeline.id);                        
        this.props.setStatusMess(`Pipeline '${this.m_selPipeline.name}' is delete`, StateEnum.Ok);
      },
      ()=>this.props.setStatusMess("Server error delete of Pipeline", StateEnum.Error))
  }

  delPipelineTask(){
    ServerAPI.delPipelineTask(this.m_selPipelineTask,
      ()=>{  
        PipelineTasks.del(this.m_selPipelineTask.id);
        this.props.setStatusMess(`Pipeline Task '${this.m_selPipelineTask.name}' is delete`, StateEnum.Ok);
        this.setState({isShowAckDeleteDialog : false});  
      },
      ()=>this.props.setStatusMess("Server error delete of Pipeline Task", StateEnum.Error))
  }
 
  render(){
   
    let PipelineTabs = observer(() => {
      let pipelines = [];
      for (let id of Pipelines.getVisibleId()){     
        pipelines.push(<TabItem key={id} id={id}
                                isSelect={this.state.selTabPipelineId == id}
                                title={Pipelines.get(id).name}
                                hSelect={(id:number) => this.setState({selTabPipelineId : id})}
                                hHide={(id:number)=>{
                                  Pipelines.setVisible(id, false);
                                  ServerAPI.changePipeline(Pipelines.get(id),()=>0,()=>0);
                                  if (this.state.selTabPipelineId == id){ 
                                    const ppls = Pipelines.getVisibleId();
                                    this.setState({selTabPipelineId : (ppls.length ? ppls[0] : 0)});
                                  }
                                }}
                                >
                        </TabItem>);
      }
      return <div className="tabPanelPipeline">
              {pipelines}
            </div>   
    });
        
    let TaskTemplateList = observer(() => {
      let taskTemlates = [];
      for (let v of TaskTemplates.getAll().values()){
        taskTemlates.push(<ListItem key={v.id} id={v.id} title={v.name} tooltip={v.description}
                                    labelEdit={"Edit Task Template"} labelDelete={"Delete Task Template"}                                                   
                                    hEdit={()=>{
                                      this.m_selTaskTemplate = TaskTemplates.get(v.id);
                                      this.setState({isShowTaskTemplateConfig : true});
                                    }}
                                    hDelete={()=>{
                                      this.m_selTaskTemplate = TaskTemplates.get(v.id);
                                      this.m_ackDeleteDialog.description= this.m_selTaskTemplate.description;
                                      this.m_ackDeleteDialog.title = `Delete '${this.m_selTaskTemplate.name}' Task Template?`;
                                      this.m_ackDeleteDialog.onYes = this.delTaskTemplate; 

                                      this.setState({isShowAckDeleteDialog : true});
                                    }}
                                    hDClickItem={()=>0}>
                          </ListItem>);
      }
      return <ListGroup className="list-group-flush borderBottom" style={{ maxHeight: "35vh", overflowY:"auto"}} >
               {taskTemlates}
             </ListGroup>
    });

    let PipilineList = observer(() => {
      let pipelines = [];
      for (let v of Pipelines.getAll().values()){
        pipelines.push(<ListItem key={v.id} id={v.id} title={v.name} tooltip={v.description}
                                labelEdit={"Edit Pipeline"} labelDelete={"Delete Pipeline"}                                                   
                                hEdit={()=>{
                                  this.m_selPipeline = Pipelines.get(v.id);
                                  this.setState({isShowPipelineConfig : true});
                                }}
                                hDelete={()=>{
                                  this.m_selPipeline = Pipelines.get(v.id);
                                  this.m_ackDeleteDialog.description= this.m_selPipeline.description;
                                  this.m_ackDeleteDialog.title = `Delete '${this.m_selPipeline.name}' Pipeline?`;
                                  this.m_ackDeleteDialog.onYes = this.delPipeline; 

                                  this.setState({isShowAckDeleteDialog : true});
                                }}
                                hDClickItem={(id : number)=>{ 
                                  if (!Pipelines.get(id).isVisible){
                                    Pipelines.setVisible(id, true);
                                    ServerAPI.changePipeline(Pipelines.get(id),()=>0,()=>0);
                                    this.setState((prev, props)=>{
                                      return {selTabPipelineId : id};
                                    });
                                  }
                                }}>
                          </ListItem>);
      }
      return <ListGroup className="list-group-flush borderBottom" style={{ maxHeight: "35vh", overflowY:"auto"}} >
               {pipelines}
             </ListGroup>
    });

    let PipelineTaskList = observer(() => {
      let tasks = [];
      for (let v of PipelineTasks.getByPPlId(this.state.selTabPipelineId).values()){
        tasks.push(<ListItem key={v.id} id={v.id} title={v.name} tooltip={v.description}
                              labelEdit={"Edit Pipeline Task"} labelDelete={"Delete Pipeline Task"}                                                   
                              hEdit={()=>{
                                this.m_selPipelineTask = PipelineTasks.get(v.id);
                                this.setState({isShowPipelineTaskConfig : true});
                              }}
                              hDelete={()=>{
                                this.m_selPipelineTask = PipelineTasks.get(v.id);
                                this.m_ackDeleteDialog.description = this.m_selPipelineTask.description;
                                this.m_ackDeleteDialog.title = `Delete '${this.m_selPipelineTask.name}' Pipeline Task?`;
                                this.m_ackDeleteDialog.onYes = this.delPipelineTask; 

                                this.setState({isShowAckDeleteDialog : true});
                              }}
                              hDClickItem={(id : number)=>{     
                                this.m_selPipelineTask = PipelineTasks.get(v.id);
                                this.m_selPipelineTask.isVisible = 1;
                                PipelineTasks.upd(this.m_selPipelineTask);
                              }}>
                          </ListItem>);
      }
      return <ListGroup className="list-group-flush borderBottom" style={{ maxHeight: "35vh", overflowY:"auto"}} >
               {tasks}
             </ListGroup>
    });
   
    return (
      <>
        <Container className="d-flex flex-column h-100 m-0 p-0" fluid >
          <Row noGutters={true} className="borderBottom">
            <Col className="col menuHeader">               
              <Image src="../images/label.svg" style={{ margin: 5}} title="Application for schedule and monitor workflows"></Image>
            </Col>
          </Row>
          <Row noGutters={true} className="h-100" >
            <Col className="col-2 m-0 p-0 borderRight">   
              <ListHeader title={"Pipeline Tasks"}
                          labelNew={"New Pipeline Task"}
                          hNew={()=>{ this.m_selPipelineTask.id = 0;
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
                <GraphPanel pplId={this.state.selTabPipelineId}/>
              </Row>
            </Col>
            <Col className="col-2 m-0 p-0 borderRight">   
              <ListHeader title={"Task Templates"}
                          labelNew={"New Task Template"}
                          hNew={()=>{ this.m_selTaskTemplate.id = 0;
                                      this.setState({isShowTaskTemplateConfig : true}); }}/>               
              <TaskTemplateList/>
            </Col>            
          </Row>
        </Container> 

        <TaskTemplateDialogModal selTaskTemplate={this.m_selTaskTemplate} 
                                 show={this.state.isShowTaskTemplateConfig} 
                                 onHide={(selTaskTemplate : ITaskTemplate)=>{
                                   this.m_selTaskTemplate = TaskTemplates.copy(selTaskTemplate);
                                   this.setState({isShowTaskTemplateConfig : false});
                                 }}/>
        
        <PipelineDialogModal selPipeline={this.m_selPipeline} 
                             show={this.state.isShowPipelineConfig} 
                             onHide={(selPipeline : IPipeline)=>{
                               this.m_selPipeline = Pipelines.copy(selPipeline);
                               this.setState({isShowPipelineConfig : false});
                             }}/>

        <PipelineTaskDialogModal selPipelineTask={this.m_selPipelineTask} 
                                 show={this.state.isShowPipelineTaskConfig} 
                                 onHide={(selPipelineTask : IPipelineTask)=>{
                                   this.m_selPipelineTask = PipelineTasks.copy(selPipelineTask);
                                   this.setState({isShowPipelineTaskConfig : false});
                                 }}/>
       
        <AckDeleteModal description={this.m_ackDeleteDialog.description}
                        title={this.m_ackDeleteDialog.title} 
                        show={this.state.isShowAckDeleteDialog}
                        onYes={this.m_ackDeleteDialog.onYes}  
                        onHide={()=>this.setState({isShowAckDeleteDialog : false})}/>
      </>
    )
  } 
}
