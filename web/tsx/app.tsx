/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Button, Modal, ListGroup} from "react-bootstrap";
import TreeNav, {ITreeNavDir} from "./treeNav";
 
import * as Action from "./redux/actions"; 
import Store from "./redux/store"; 
import { IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "./types";

import "../css/app.css";
import "bootstrap/dist/css/bootstrap.min.css";


interface IProps {
  user : IUser;
  pipelines : Map<number, IPipeline>;
  taskGroups : Map<number, ITaskGroup>;
  taskTemplates : Map<number, ITaskTemplate>;
  tasks : Map<number, ITask>;
  onAddPipeline : (pipeline : IPipeline) => any;
  onChangePipeline : (pipeline : IPipeline) => any;
  onDelPipeline : (pipeline : IPipeline) => any;
  onAddTaskGroup : (taskgroup : ITaskGroup) => any;
  onChangeTaskGroup : (taskgroup : ITaskGroup) => any;
  onDelTaskGroup : (taskgroup : ITaskGroup) => any;
  onAddTaskTemplate : (tasktemplate : ITaskTemplate) => any;
  onChangeTaskTemplate : (tasktemplate : ITaskTemplate) => any;
  onDelTaskTemplate : (tasktemplate : ITaskTemplate) => any;
  onAddTask : (task : ITask) => any;
  onChangeTask : (task : ITask) => any;
  onDelTask : (task : ITask) => any;
  onStartTask : (task : ITask) => any;
  onStopTask : (task : ITask) => any;
};

interface IState {
  name : string;
};


class App extends React.Component<IProps, IState>{
   
  constructor(props : IProps){
    super(props);
    
    this.state  = { name : "" };

    this.hAddTaskTemplate = this.hAddTaskTemplate.bind(this); 
    this.hDelTaskTemplate = this.hDelTaskTemplate.bind(this); 
  }
    
  hAddTaskTemplate(){

    // fetch('api/addTaskTemplate')
    // .then(response => response.json())    
    // .then(signs =>{ 
        
    //   for (let k in signs){
    //     signs[k].isBuffEna = false,
    //     signs[k].buffVals = []    
    //   }
      
    //   this.props.onSetSignalsFromServer(signs);
    
    //   setNavScheme(signs);
    // })
    // .catch(() => console.log('api/allSignals error'))  


    // this.setState((oldState, props) => (
    //    { listGraph : [...oldState.listGraph, []] } 
    //    ));
  }
  hDelTaskTemplate(){

    // this.setState((oldState, props) => (
    //    { listGraph : [...oldState.listGraph, []] } 
    //    ));
  }

  componentDidMount() {
   
    let nd = ReactDOM.findDOMNode(this);
    if (nd){
      nd.addEventListener('wheel', (event /*:: : any*/) => {
        event.preventDefault();
      }, false);
    }
      
    // fetch('app/allSignals')
    // .then(response => response.json())    
    // .then(signs =>{ 
        
    //   for (let k in signs){
    //     signs[k].isBuffEna = false,
    //     signs[k].buffVals = []    
    //   }
      
    //   this.props.onSetSignalsFromServer(signs);
    
    //   setNavScheme(signs);
    // })
    // .catch(() => console.log('api/allSignals error'))  


    // fetch('api/dataParams')
    // .then(response => response.json())    
    // .then(dataParams => {
     
    //   this.props.onSetDataParams(dataParams);
      
    //   this.updateSignalData(dataParams);
    // })    
    // .catch(() => console.log('api/dataParams error'));    


    // let setNavScheme = (signs /*:: : {obj : signalType} */) => {
      
    //   let navScheme /*:: : Array<navSchemeType | string> */ = [];
    //   for (let k in signs){
    
    //     let s = signs[k];

    //     let it /*:: : any */ = navScheme.find((it) => {

    //       return (typeof it === 'object') && (typeof it.submenu === 'string') ?
    //                 s.module == it.submenu : false;
    //     });

    //     if (!it){ 

    //       it = { submenu : s.module,
    //                         isShow : true,
    //                         isActive : true,
    //                         items : []};

    //       navScheme.push(it);
    //     }
        
    //     it.items.push(s.name);
    //   }
          
    //   this.setState({navScheme});
    // }
  }

  render(){

    // let obj : ITreeNavDir = { name : "dir2222222222222222222222221",
    // files : [],
    // subdirs : [],
    // isShow : false };
    // for(let i = 0; i < 5; ++i){
    //   obj.files.push("f" + i);
    // }   
    // let obj1 : ITreeNavDir = { name : "dir2",
    //                            files : ["f3", "f4"],
    //                            subdirs : [],
    //                            isShow : false };
    // obj.subdirs.push(obj1);

    // let dirs: Array<ITreeNavDir> = [];
    // dirs.push(obj);

    let clientHeight = document.documentElement ? document.documentElement.clientHeight : 300;

    return (
        <Container className="col app-container"
                   style={{overflow: "auto", height: clientHeight}}>
          <Row noGutters={true} className="m-1 p-2"
              style = {{  border: "1px solid #dbdbdb", borderRadius: "5px"}}>
            <Col className="col-auto"> 
              {/* <TreeNav dirs={dirs} />             */}
              <ListGroup>
                <ListGroup.Item>Cras justo odio</ListGroup.Item>
                <ListGroup.Item>Dapibus ac facilisis in</ListGroup.Item>
                <ListGroup.Item>Morbi leo risus</ListGroup.Item>
                <ListGroup.Item>Porta ac consectetur ac</ListGroup.Item>
                <ListGroup.Item>Vestibulum at eros</ListGroup.Item>
            </ListGroup>
            </Col> 
            <Col className="col"> 
              <Button variant="primary"
                      onClick={this.hAddTaskTemplate}>Primary</Button>
              <Button variant="secondary"
                      onClick={this.hDelTaskTemplate}>Secondary</Button>
            </Col>
          </Row>
        </Container> 
    )
  } 
}


// //////////////////////////////////////////////////

const mapStateToProps = (state) => {
  return state;
}

const mapDispatchToProps = (dispatch) => {
  return {
    onAddPipeline : Action.addPipeline(dispatch),
    onChangePipeline : Action.changePipeline(dispatch),
    onDelPipeline : Action.delPipeline(dispatch),
    onAddTaskGroup : Action.addTaskGroup(dispatch),
    onChangeTaskGroup : Action.changeTaskGroup(dispatch), 
    onDelTaskGroup : Action.delTaskGroup(dispatch),
    onAddTaskTemplate : Action.addTaskTemplate(dispatch),
    onChangeTaskTemplate : Action.changeTaskTemplate(dispatch),
    onDelTaskTemplate : Action.delTaskTemplate(dispatch),
    onAddTask : Action.addTask(dispatch),
    onChangeTask : Action.changeTask(dispatch),
    onDelTask : Action.delTask(dispatch),
    onStartTask : Action.startTask(dispatch),
    onStopTask : Action.stopTask(dispatch),
  }
}

let AppRedux = connect(mapStateToProps, mapDispatchToProps)(App);

const root = document.getElementById('root')

if (root){
  ReactDOM.render(
    <Provider store={Store}>
       <AppRedux/>,
    </Provider>,
    root
  );
}