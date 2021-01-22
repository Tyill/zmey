/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Tabs, Tab, Image, Card, ListGroup } from "react-bootstrap";
import DialogTaskTemplateRedux from "./taskTemplateDialog";

import * as Action from "./redux/actions";
import Store from "./redux/store"; 
import { IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "./types";

import "../css/app.css";
import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  user : IUser;                                // | Store
  pipelines : Map<number, IPipeline>;          // | 
  taskGroups : Map<number, ITaskGroup>;        // |
  taskTemplates : Map<number, ITaskTemplate>;  // |
  tasks : Map<number, ITask>;                  // |  

  onFillTaskTemplates : (tasktemplates : Array<ITaskTemplate>) => any; // | Actions
  onFillPipelines : (pipelines : Array<IPipeline>) => any;             // |
  onFillTaskGroups : (taskGroups : Array<ITaskGroup>) => any;          // |
  onFillTasks : (tasks : Array<ITask>) => any;                         // |
};

interface IState {
  isShowTaskTemplateConfig : boolean;
};


class App extends React.Component<IProps, IState>{
   
  constructor(props : IProps){
    super(props);
    
    this.state  = { isShowTaskTemplateConfig : false };
   
    this.hShowTaskTemplateConfig = this.hShowTaskTemplateConfig.bind(this); 
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

  hShowTaskTemplateConfig(){

    this.setState((oldState, props)=>{

      let isShowTaskTemplateConfig = !oldState.isShowTaskTemplateConfig;

      return {isShowTaskTemplateConfig};
    });
  }

  render(){
    
    let pipelines = []
    for (let v of this.props.pipelines.values()){
      pipelines.push(<ListGroup.Item key={v.id} action >{v.name}</ListGroup.Item>);
    }
    let taskTemlates = []
    for (let v of this.props.taskTemplates.values()){
      taskTemlates.push(<ListGroup.Item key={v.id} action >{v.name}</ListGroup.Item>); 
    }
       
    return (
      <div>
        <Container fluid style={{ margin: 0, padding: 0}}>
          <Row noGutters={true} style={{borderBottom: "1px solid #dbdbdb", boxShadow: "0px 1px 5px #dbdbdb"}}>
            <Col className="col">               
              <Image src="/images/label.png" style={{ margin: 5}}></Image>
            </Col>
          </Row>
          <Row noGutters={true} >
            <Col className="col-2" >   
              <Card style={{minHeight: "70vh"}}>  
                <Card.Header as="h6">
                  Task templates 
                  <Image className="btnNew" title="New task" style={{marginLeft: "15px"}} onClick={(e)=>{ console.log("dfdfdf");}}></Image>
                  <Image className="btnEdit" title="Edit task" style={{marginLeft: "15px"}} onClick={(e)=>{ console.log("dfdfdf");}}></Image>
                  <Image className="btnDelete" title="Delete task" style={{marginLeft: "15px"}} onClick={(e)=>{ console.log("dfdfdf");}}></Image>
                </Card.Header>
                <ListGroup className="list-group-flush" style={{minHeight: "20vh"}} >
                  {taskTemlates}
                </ListGroup>               
                <Card.Header as="h6">
                  Pipelines
                  <Image className="btnNew" title="New pipeline" style={{marginLeft: "57px"}} onClick={(e)=>{ console.log("dfdfdf");}}></Image>
                  <Image className="btnEdit" title="Edit pipeline" style={{marginLeft: "15px"}} onClick={(e)=>{ console.log("dfdfdf");}}></Image>
                  <Image className="btnDelete" title="Delete pipeline" style={{marginLeft: "15px"}} onClick={(e)=>{ console.log("dfdfdf");}}></Image>
                </Card.Header>    
                <ListGroup className="list-group-flush" style={{minHeight: "20vh", maxHeight: "40vh", overflowY:"auto"}}>
                  {pipelines}
                </ListGroup>
              </Card>
            </Col>
            <Col className="col-8" >   
            <Tabs defaultActiveKey="profile"  id="uncontrolled-tab-example" style={{height: "47px"}}>
              <Tab eventKey="home" title="Home">
               
              </Tab>
              <Tab eventKey="profile" title="Profile">
                
              </Tab>
              <Tab eventKey="contact" title="Contact">
                
              </Tab>
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
              <Card> 
                <Card.Header style={{ padding: "0px 0px 0px 10px"}}>Message list</Card.Header>
              </Card>
            </Col>
          </Row>
        </Container> 

        <DialogTaskTemplateRedux show={this.state.isShowTaskTemplateConfig} onHide={this.hShowTaskTemplateConfig}/>
      </div>
    )
  } 
}


// //////////////////////////////////////////////////

const mapStoreToProps = (store) => {
  return store;
}

const mapDispatchToProps = (dispatch) => {
  return { 
    onFillTaskTemplates : Action.fillTaskTemplates(dispatch), 
    onFillPipelines : Action.fillPipelines(dispatch),   
    onFillTasks : Action.fillTasks(dispatch), 
    onFillTaskGroups : Action.fillTaskGroups(dispatch), 
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