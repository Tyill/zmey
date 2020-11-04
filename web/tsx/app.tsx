/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Button, Modal, ListGroup} from "react-bootstrap";
import TreeNav, {ITreeNavDir} from "./treeNav";
 
// import { updateFromServer, 
//          setDataParams, 
//          setSignalsFromServer,
//          signalBufferEnable,
//          changeConfig } from "./redux/actions.jsx"; 
// import Store from "./redux/store.jsx"; 

import "../css/app.css";
import "bootstrap/dist/css/bootstrap.min.css";

class App extends React.Component{
   
  constructor(props){
    super(props);
    
  }
    
  render(){

    let obj : ITreeNavDir = { name : "dir2222222222222222222222221",
    files : [],
    subdirs : [],
    isShow : false };
    for(let i = 0; i < 5; ++i){
      obj.files.push("f" + i);
    }   
    let obj1 : ITreeNavDir = { name : "dir2",
                               files : ["f3", "f4"],
                               subdirs : [],
                               isShow : false };
    obj.subdirs.push(obj1);

    let dirs: Array<ITreeNavDir> = [];
    dirs.push(obj);

    return (
        <Container className="col app-container"
                  style={{overflow: "auto", height: 500}}>
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
              <Button variant="primary">Primary</Button>
              <Button variant="secondary">Secondary</Button>
            </Col>
          </Row>
        </Container> 
    )
  } 
}


// //////////////////////////////////////////////////

// const mapStateToProps = (state) => {
//   return state;
// }

// const mapDispatchToProps = (dispatch) => {
//   return {
//       onSetSignalsFromServer: setSignalsFromServer(dispatch),
//       onUpdateFromServer: updateFromServer(dispatch),
//       onSetDataParams: setDataParams(dispatch),
//       onSignalBufferEnable: signalBufferEnable(dispatch),
//       onChangeConfig: changeConfig(dispatch),  
//   }
// }

// let AppRedux = connect(mapStateToProps, mapDispatchToProps)(App);

const root = document.getElementById('root')

if (root){
  ReactDOM.render(
    React.createElement(App, null),
    root
  );
}