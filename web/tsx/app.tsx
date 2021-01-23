/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Tabs, Tab, Image, Card, Modal, Button, ListGroup } from "react-bootstrap";
import DialogTaskTemplateModal from "./taskTemplateDialog";

import * as Action from "./redux/actions";
import Store from "./redux/store"; 
import { IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "./types";

import "../css/app.css";
import "bootstrap/dist/css/bootstrap.min.css";

interface IPropsApp {
  user : IUser;                                // | Store
  pipelines : Map<number, IPipeline>;          // | 
  taskGroups : Map<number, ITaskGroup>;        // |
  taskTemplates : Map<number, ITaskTemplate>;  // |
  tasks : Map<number, ITask>;                  // |  

  onFillTaskTemplates : (taskTemplates : Array<ITaskTemplate>) => any; // | Actions
  onFillPipelines : (pipelines : Array<IPipeline>) => any;             // |
  onFillTaskGroups : (taskGroups : Array<ITaskGroup>) => any;          // |
  onFillTasks : (tasks : Array<ITask>) => any;                         // |
  onDelTaskTemplate : (taskTemplate : ITaskTemplate) => any;           // |
};

interface IStateApp {
  isShowTaskTemplateConfig : boolean;
  isShowTaskPipelineConfig : boolean;
  isShowAckTaskDelete : boolean;
};

class App extends React.Component<IPropsApp, IStateApp>{
   
  private m_selTaskTemplate : ITaskTemplate = {} as ITaskTemplate;

  constructor(props : IPropsApp){
    super(props);
    
    this.state  = { isShowTaskTemplateConfig : false,
                    isShowTaskPipelineConfig : false,
                    isShowAckTaskDelete : false };   
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
      pipelines.push(<Tab key={v.id} eventKey={v.id.toString()} title={v.name} tooltip="TaskPipeline"></Tab>);
    }
    let taskTemlates = []
    for (let v of this.props.taskTemplates.values()){
      taskTemlates.push(<TaskItem key={v.id} title={v.name} tooltip={v.description} tooltipEdit={"Edit Task Template"} tooltipDelete={"Delete Task Template"}
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
                                      let isShowAckTaskDelete = true;
                                      return {isShowAckTaskDelete};
                                    });
                                  }}>
                        </TaskItem>);
    }
       
    return (
      <div>
        <Container fluid style={{ margin: 0, padding: 0}}>
          <Row noGutters={true} style={{borderBottom: "1px solid #dbdbdb", boxShadow: "0px 1px 5px #dbdbdb"}}>
            <Col className="col menuHeader">               
              <Image src="/images/label.png" style={{ margin: 5}} title="Application for schedule and monitor workflows"></Image>
            </Col>
          </Row>
          <Row noGutters={true} >
            <Col className="col-2">   
              <Card style={{minHeight: "70vh"}}>  
                <TaskHeader title="Task Templates" tooltip="New Task Template" 
                            hNew={()=>{
                              this.m_selTaskTemplate.id = 0;
                              this.setState((oldState, props)=>{
                                let isShowTaskTemplateConfig = true;
                                return {isShowTaskTemplateConfig};
                              });
                }}/>                
                <ListGroup className="list-group-flush" style={{minHeight: "20vh", maxHeight: "70vh", overflowY:"auto"}} >
                  {taskTemlates}
                </ListGroup>
              </Card>
            </Col>
            <Col className="col" >   
            <Tabs defaultActiveKey="profile" id="uncontrolled-tab-example" style={{height: "47px"}}>
              {pipelines}
            </Tabs>
            </Col>
            <Col className="col-2" >   
              <Card style={{minHeight: "40vh"}}>  
                <Card.Header as="h6">Active tasks</Card.Header>    
                <ListGroup className="list-group-flush">               
                </ListGroup>            
              </Card>
            </Col>
          </Row>
          <Row noGutters={true} >
            <Col className="col" >   
              <Card style={{height: "50px"}}> 
              </Card>
            </Col>
          </Row>
        </Container> 

        <DialogTaskTemplateModal selTaskTemplate={this.m_selTaskTemplate} 
                                 show={this.state.isShowTaskTemplateConfig} 
                                 onHide={()=>{
                                   this.setState((oldState, props)=>{
                                     let isShowTaskTemplateConfig = false;
                                     return {isShowTaskTemplateConfig};
                                   });
                                 }}/>

        <AckTaskDeleteModal selTaskTemplate={this.m_selTaskTemplate}
                            show={this.state.isShowAckTaskDelete}
                            onYes={()=>{
                                fetch('api/delTaskTemplate?id=' + this.m_selTaskTemplate.id)
                                .then(() => {
                                  let taskTemplate = this.m_selTaskTemplate;
                                  this.props.onDelTaskTemplate(taskTemplate); 
                                  this.setState((oldState, props)=>{
                                    let isShowAckTaskDelete = false;
                                    return {isShowAckTaskDelete};
                                  }); 
                                }) 
                                .catch(() => console.log('api/delTaskTemplate error')); }}
                            onHide={()=>{
                              this.setState((oldState, props)=>{
                                let isShowAckTaskDelete = false;
                                return {isShowAckTaskDelete};
                              });
                            }}/>
      </div>
    )
  } 
}


////////////////////////////////////////////////////

interface IPropsTaskHeader {
  title : string;
  tooltip : string;
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
        {this.props.title}
        {this.state.isShowBtn ? 
          <Image className="btnNew" 
                title={this.props.tooltip}
                style={{float: "right"}} 
                onClick={this.props.hNew}>
          </Image>
        : ""}
      </Card.Header>
    )
  }
}

////////////////////////////////////////////////////

interface IPropsTaskItem { 
  title : string;
  tooltip : string;
  tooltipEdit : string;
  tooltipDelete : string;
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
            <Image className="btnDelete" 
                  title={this.props.tooltipDelete}
                  style={{float: "right", marginLeft: "20px", marginTop: "5px" }} 
                  onClick={this.props.hDelete}>
            </Image>
            <Image className="btnEdit" 
                  title={this.props.tooltipEdit}
                  style={{float: "right", marginTop: "5px"}} 
                  onClick={this.props.hEdit}>
            </Image>            
          </span>
        : ""}
      </ListGroup.Item>
    )
  }
}

////////////////////////////////////////////////////

interface IPropsAckTaskDeleteModal { 
  show : boolean;
  selTaskTemplate : ITaskTemplate;
  onYes : () => any;  
  onHide : () => any;  
};
interface IStateAckTaskDeleteModal { 
};
class AckTaskDeleteModal extends React.Component<IPropsAckTaskDeleteModal, IStateAckTaskDeleteModal>{  
  constructor(props : IPropsAckTaskDeleteModal){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    return (
      <Modal show={this.props.show} onHide={this.props.onHide}>
        <Modal.Header closeButton>
          <Modal.Title>Delete the Task Template?</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>{this.props.selTaskTemplate.name}</p>
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