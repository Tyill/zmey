/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Tabs, Tab, Image, Card, Modal, Button, OverlayTrigger, ListGroup } from "react-bootstrap";
import TaskTemplateDialogModal from "./taskTemplateDialog";
import PipelineDialogModal from "./pipelineDialog";

import * as Action from "./redux/actions";
import Store from "./redux/store"; 
import { IUser, IPipeline, IGroup, ITaskTemplate, ITask } from "./types";

import "../css/app.css";
import "../css/fontello.css";
import "bootstrap/dist/css/bootstrap.min.css";

interface IPropsApp {
  user : IUser;                                // | Store
  pipelines : Map<number, IPipeline>;          // | 
  groups : Map<number, IGroup>;                // |
  taskTemplates : Map<number, ITaskTemplate>;  // |
  tasks : Map<number, ITask>;                  // |  

  onFillTaskTemplates : (taskTemplates : Array<ITaskTemplate>) => any; // | Actions
  onFillPipelines : (pipelines : Array<IPipeline>) => any;             // |
  onFillTaskGroups : (groups : Array<IGroup>) => any;                  // |
  onFillTasks : (tasks : Array<ITask>) => any;                         // |
  onDelTaskTemplate : (taskTemplate : ITaskTemplate) => any;           // |
  onDelPipeline : (pipeline : IPipeline) => any;                       // |
};

interface IStateApp {
  isShowTaskTemplateConfig : boolean;
  isShowPipelineConfig : boolean;
  isShowAckTaskTemplateDelete : boolean;
  isShowAckPipelineDelete : boolean;
};

class App extends React.Component<IPropsApp, IStateApp>{
   
  private m_selTaskTemplate : ITaskTemplate = {} as ITaskTemplate;
  private m_selPipeline : IPipeline = {} as IPipeline;

  constructor(props : IPropsApp){
    super(props);
    
    this.state  = { isShowTaskTemplateConfig : false,
                    isShowPipelineConfig : false,
                    isShowAckTaskTemplateDelete : false,
                    isShowAckPipelineDelete : false };   
  }
    
  componentDidMount() {    
    fetch('api/allTaskTemplates')
    .then(response => response.json())    
    .then(taskTemplates =>{   
      this.props.onFillTaskTemplates(taskTemplates);           
    })
    .catch(() => console.log('api/allTaskTemplates error')); 
    
    fetch('api/allPipelines')
    .then(response => response.json())    
    .then(pipelines =>{   
      this.props.onFillPipelines(pipelines);           
    })
    .catch(() => console.log('api/allPipelines error'));   
    
    fetch('api/allTaskGroups')
    .then(response => response.json())    
    .then(groups =>{   
      this.props.onFillTaskGroups(groups);           
    })
    .catch(() => console.log('api/allTaskGroups error'));  
    
    fetch('api/allTasks')
    .then(response => response.json())    
    .then(tasks =>{   
      this.props.onFillTasks(tasks);           
    })
    .catch(() => console.log('api/allTasks error'));  
  }
 
  render(){
    
    let pipelines = []
    for (let v of this.props.pipelines.values()){     
      pipelines.push(<Tab key={v.id} eventKey={v.id.toString()} title={v.name}></Tab>);
    }
    let taskTemlates = []
    for (let v of this.props.taskTemplates.values()){
      taskTemlates.push(<TaskItem key={v.id} title={v.name} tooltip={v.description}
                                  hEdit={()=>{
                                    this.m_selTaskTemplate = this.props.taskTemplates.get(v.id);
                                    this.setState((oldState, props)=>{
                                      let isShowTaskTemplateConfig = true;
                                      return {isShowTaskTemplateConfig};
                                    });
                                  }}
                                  hDelete={()=>{
                                    this.m_selTaskTemplate = this.props.taskTemplates.get(v.id);
                                    this.setState((oldState, props)=>{
                                      let isShowAckTaskTemplateDelete = true;
                                      return {isShowAckTaskTemplateDelete};
                                    });
                                  }}>
                        </TaskItem>);
    }
       
    return (
      <div>
        <Container fluid style={{ margin: 0, padding: 0}}>
          <Row noGutters={true} style={{borderBottom: "1px solid #dbdbdb"}}>
            <Col className="col menuHeader">               
              <Image src="/images/label.png" style={{ margin: 5}} title="Application for schedule and monitor workflows"></Image>
            </Col>
          </Row>
          <Row noGutters={true} >
            <Col className="col-2" style={{borderRight: "1px solid #dbdbdb"}}>   
              <TaskHeader hNew={()=>{ 
                            this.m_selTaskTemplate.id = 0;
                            this.setState((oldState, props)=>{
                              let isShowTaskTemplateConfig = true;
                              return {isShowTaskTemplateConfig};
                            });
              }}/>                
              <ListGroup className="list-group-flush" style={{minHeight: "70vh", maxHeight: "70vh", overflowY:"auto"}} >
                {taskTemlates}
              </ListGroup>
            </Col>
            <Col className="col" style={{borderRight: "1px solid #dbdbdb"}}> 
              <PipelineHeader hNew={()=>{ 
                                this.m_selPipeline.id = 0;
                                this.setState((oldState, props)=>{
                                  let isShowPipelineConfig = true;
                                  return {isShowPipelineConfig};
                                });}}
                              hEdit={()=>{
                                // this.m_selPipeline = this.props.pipelines.get(v.id);
                                this.setState((oldState, props)=>{
                                  let isShowPipelineConfig = true;
                                  return {isShowPipelineConfig};
                                }); }}
                              hDelete={()=>{
                                // this.m_selTaskTemplate = this.props.taskTemplates.get(v.id);
                                this.setState((oldState, props)=>{
                                  let isShowAckPipelineDelete = true;
                                  return {isShowAckPipelineDelete};
                                }); }} />
              <Tabs defaultActiveKey="profile" id="uncontrolled-tab-example" style={{height: "48px"}}
                    onSelect={(key) => this.m_selPipeline = this.props.pipelines.get(parseInt(key))}>
                {pipelines}
              </Tabs>                                       
            </Col>
            <Col className="col-2" > 
            </Col>
          </Row>
          <Row noGutters={true} >
            <Col className="col" >   
              <Card style={{height: "50px"}}> 
              </Card>
            </Col>
          </Row>
        </Container> 

        <TaskTemplateDialogModal selTaskTemplate={this.m_selTaskTemplate} 
                                 show={this.state.isShowTaskTemplateConfig} 
                                 onHide={(selTaskTemplate : ITaskTemplate)=>{
                                   this.m_selTaskTemplate = selTaskTemplate;
                                   this.setState((oldState, props)=>{
                                     let isShowTaskTemplateConfig = false;
                                     return {isShowTaskTemplateConfig};
                                   });
                                 }}/>
        
        <PipelineDialogModal selPipeline={this.m_selPipeline} 
                             show={this.state.isShowPipelineConfig} 
                             onHide={(selPipeline : IPipeline)=>{
                               this.m_selPipeline = selPipeline;
                               this.setState((oldState, props)=>{
                                 let isShowPipelineConfig = false;
                                 return {isShowPipelineConfig};
                               });
                             }}/>

        <AckDeleteModal name={this.m_selTaskTemplate.name}
                        title="Delete the Task Template?"
                        show={this.state.isShowAckTaskTemplateDelete}
                        onYes={()=>{
                                fetch('api/delTaskTemplate?id=' + this.m_selTaskTemplate.id)
                                .then(() => {
                                  let taskTemplate = this.m_selTaskTemplate;
                                  this.props.onDelTaskTemplate(taskTemplate); 
                                  this.setState((oldState, props)=>{
                                    let isShowAckTaskTemplateDelete = false;
                                    return {isShowAckTaskTemplateDelete};
                                  }); 
                                }) 
                                .catch(() => console.log('api/delTaskTemplate error')); }}
                        onHide={()=>{
                          this.setState((oldState, props)=>{
                            let isShowAckTaskTemplateDelete = false;
                            return {isShowAckTaskTemplateDelete};
                          });
                        }}/>

        <AckDeleteModal name={this.m_selPipeline.name}
                        title="Delete the Task Pipeline?"
                        show={this.state.isShowAckPipelineDelete}
                        onYes={()=>{
                                fetch('api/delPipeline?id=' + this.m_selPipeline.id)
                                .then(() => {
                                  let pipeline = this.m_selPipeline;
                                  this.props.onDelPipeline(pipeline); 
                                  this.setState((oldState, props)=>{
                                    let isShowAckPipelineDelete = false;
                                    return {isShowAckPipelineDelete};
                                  }); 
                                }) 
                                .catch(() => console.log('api/delPipeline error')); }}
                        onHide={()=>{
                          this.setState((oldState, props)=>{
                            let isShowAckPipelineDelete = false;
                            return {isShowAckPipelineDelete};
                          });
                        }}/>
      </div>
    )
  } 
}


////////////////////////////////////////////////////

interface IPropsTaskHeader {
  hNew : () => any;
};
interface IStateTaskHeader { 
  isShowBtn : boolean; 
};
class TaskHeader extends React.Component<IPropsTaskHeader, IStateTaskHeader>{  
  constructor(props : IPropsTaskHeader){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){           
    return (
      <Card.Header as="h6" style={{height: "46px"}}
                           onMouseEnter={(e)=>this.setState((oldState, props)=>{ let isShowBtn = true; return {isShowBtn}})}
                           onMouseLeave={(e)=>this.setState((oldState, props)=>{ let isShowBtn = false; return {isShowBtn}})}>
        Task Templates
        {this.state.isShowBtn ? 
          <a className="icon-new"           
             title="New Task Template"
             style={{float: "right"}} 
             onClick={this.props.hNew}>
          </a>
        : ""}
      </Card.Header>
    )
  }
}

////////////////////////////////////////////////////

interface IPropsTaskItem { 
  title : string;
  tooltip : string;
  key : number;   
  hEdit : () => any;
  hDelete : () => any;
};
interface IStateTaskItem { 
  isShowBtn : boolean; 
};
class TaskItem extends React.Component<IPropsTaskItem, IStateTaskItem>{  
  constructor(props : IPropsTaskItem){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    return (
      <ListGroup.Item action title={this.props.tooltip}
                      onMouseEnter={(e)=>this.setState((oldState, props)=>{ let isShowBtn = true; return {isShowBtn}})}
                      onMouseLeave={(e)=>this.setState((oldState, props)=>{ let isShowBtn = false; return {isShowBtn}})}>
        {this.props.title}
        {this.state.isShowBtn ?
          <span>
            <a className = "icon-delete"
                  title="Delete Task Template"
                  style={{float: "right", marginLeft: "20px" }} 
                  onClick={this.props.hDelete}>
            </a>
            <a className="icon-edit" 
                  title= "Edit Task Template"
                  style={{float: "right"}} 
                  onClick={this.props.hEdit}>
            </a>            
          </span>
        : ""}
      </ListGroup.Item>
    )
  }
}

////////////////////////////////////////////////////

interface IPropsPipelineHeader {
  hNew : () => any;
  hEdit : () => any;
  hDelete : () => any;
};
interface IStatePipelineHeader { 
  isShowBtn : boolean; 
};
class PipelineHeader extends React.Component<IPropsPipelineHeader, IStatePipelineHeader>{  
  constructor(props : IPropsPipelineHeader){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){           
    return (
      <Card.Header as="h6" style={{height: "46px"}}
                           onMouseEnter={(e)=>this.setState((oldState, props)=>{ let isShowBtn = true; return {isShowBtn}})}
                           onMouseLeave={(e)=>this.setState((oldState, props)=>{ let isShowBtn = false; return {isShowBtn}})}>
        Task Pipelines
        {this.state.isShowBtn ?
         <span>
          <a className="icon-new" 
            title="New Task Pipeline"
            style={{marginLeft: "20px"}} 
            onClick={this.props.hNew}>
          </a> 
          <a className="icon-edit" 
            title= "Edit Task Pipeline"
            style={{marginLeft: "20px"}} 
            onClick={this.props.hEdit}>
          </a>
          <a className="icon-delete" 
            title="Delete Task Pipeline"
            style={{marginLeft: "20px" }} 
            onClick={this.props.hDelete}>
          </a>
          </span>
        : ""}
      </Card.Header>
    )
  }
}

////////////////////////////////////////////////////

interface IPropsAckDeleteModal { 
  show : boolean;
  name : string;
  title : string;
  onYes : () => any;  
  onHide : () => any;  
};
interface IStateAckDeleteModal { 
};
class AckDeleteModal extends React.Component<IPropsAckDeleteModal, IStateAckDeleteModal>{  
  constructor(props : IPropsAckDeleteModal){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    return (
      <Modal show={this.props.show} onHide={this.props.onHide}>
        <Modal.Header closeButton>
          <Modal.Title>{this.props.title}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>{this.props.name}</p>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" style={{minWidth:"100px"}} onClick={this.props.onYes}>Yes</Button>
          <Button variant="primary" style={{minWidth:"100px"}} onClick={this.props.onHide} >No</Button>
        </Modal.Footer>
      </Modal>
    )
  }
}

////////////////////////////////////////////////////

const mapStoreToProps = (store) => {
  return store;
}

const mapDispatchToProps = (dispatch) => {
  return { 
    onFillTaskTemplates : Action.fillTaskTemplates(dispatch), 
    onFillPipelines : Action.fillPipelines(dispatch),   
    onFillTasks : Action.fillTasks(dispatch), 
    onFillTaskGroups : Action.fillTaskGroups(dispatch),
    onDelTaskTemplate : Action.delTaskTemplate(dispatch),
    onDelPipeline : Action.delPipeline(dispatch), 
  }
}

let AppRedux = connect(mapStoreToProps, mapDispatchToProps)(App);

const root = document.getElementById('root')

if (root){
  ReactDOM.render(
    <Provider store={Store}>
       <AppRedux/>
    </Provider>,
    root
  );
}