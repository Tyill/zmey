/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Button, Modal, ListGroup} from "react-bootstrap";
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
    .then(jsTaskTemplates =>{   
      this.props.onFillTaskTemplates(jsTaskTemplates);           
    })
    .catch(() => console.log('api/allTaskTemplates error')); 
    
    fetch('api/allPipelines')
    .then(response => response.json())    
    .then(jsPipelines =>{   
      this.props.onFillPipelines(jsPipelines);           
    })
    .catch(() => console.log('api/allPipelines error'));   
    
    fetch('api/allGroups')
    .then(response => response.json())    
    .then(jsGroups =>{   
      this.props.onFillTaskGroups(jsGroups);           
    })
    .catch(() => console.log('api/allGroups error'));  
    
    fetch('api/allTasks')
    .then(response => response.json())    
    .then(jsTasks =>{   
      this.props.onFillTasks(jsTasks);           
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
        
    let clientHeight = document.documentElement ? document.documentElement.clientHeight : 300;

    let pipelines = []
    for (let v of this.props.pipelines.values()){
      pipelines.push(<ListGroup.Item key={v.id}>{v.name}</ListGroup.Item>);
    }
    let taskTemlates = []
    for (let v of this.props.taskTemplates.values()){
      taskTemlates.push(<ListGroup.Item key={v.id}>{v.name}</ListGroup.Item>); 
    }
    
    return (
      <div>
        <Container className="col app-container"
                   style={{overflow: "auto", height: clientHeight}}>
          <Row noGutters={true} className="m-1 p-2"
              style = {{  border: "1px solid #dbdbdb", borderRadius: "5px"}}>
            <Col className="col-auto">               
              <ListGroup>
                {taskTemlates}
              </ListGroup>
            </Col> 
            <Col className="col-auto">               
              <ListGroup>
                {pipelines}
              </ListGroup>
            </Col> 
            <Col className="col"> 
              <Button variant="primary" 
                      onClick= {this.hShowTaskTemplateConfig}>Primary</Button>
              <Button variant="secondary">Secondary</Button>
            </Col>
          </Row>
        </Container> 

        <DialogTaskTemplateRedux show = {this.state.isShowTaskTemplateConfig} onHide = {this.hShowTaskTemplateConfig}/>
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
       <AppRedux/>,
    </Provider>,
    root
  );
}