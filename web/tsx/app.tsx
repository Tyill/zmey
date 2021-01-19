/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Button, Modal, ListGroup} from "react-bootstrap";
import DialogTaskTemplateRedux from "./taskTemplateDialog";

import Store from "./redux/store"; 
import { IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "./types";

import "../css/app.css";
import "bootstrap/dist/css/bootstrap.min.css";

let DEBUG = 1;

interface IProps {
  user : IUser;                                // | Store
  pipelines : Map<number, IPipeline>;          // | 
  taskGroups : Map<number, ITaskGroup>;        // |
  taskTemplates : Map<number, ITaskTemplate>;  // |
  tasks : Map<number, ITask>;                  // |  
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
    
    if (DEBUG){
      fetch('auth/login')
      .then(response => response.json())    
      .then(jsTaskTemplates =>{   
        console.log(jsTaskTemplates);        
        //this.props.onChangeTaskTemplate(respTaskTemplate);           
      })
      .catch(() => console.log('api/allTaskTemplates error')); 
    }

    fetch('api/allTaskTemplates?userId=1')
    .then(response => response.json())    
    .then(jsTaskTemplates =>{   
      console.log(jsTaskTemplates);        
      //this.props.onChangeTaskTemplate(respTaskTemplate);           
    })
    .catch(() => console.log('api/allTaskTemplates error')); 
    
  }

  hShowTaskTemplateConfig(){

    this.setState((oldState, props)=>{

      let isShowTaskTemplateConfig = !oldState.isShowTaskTemplateConfig;

      return {isShowTaskTemplateConfig};
    });
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
      <div>
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

const mapStateToProps = (state) => {
  return state;
}

const mapDispatchToProps = (dispatch) => {
  return {    
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