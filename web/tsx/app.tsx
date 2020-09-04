/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Button, Modal} from "react-bootstrap";
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
      <div>
      <Container className="col app-container"
                 style={{overflow: "auto", height: 500}}>
        <Row noGutters={true} className="m-1 p-2"
             style = {{  border: "1px solid #dbdbdb", borderRadius: "5px"}}>
            <Col className="col-auto"> 
              <TreeNav dirs={dirs} />            
            </Col>            
           <Col className="col"> 
           </Col>
        </Row>
      </Container>  
      </div>
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