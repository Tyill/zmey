/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Button, Modal, ListGroup} from "react-bootstrap";
 
import * as Action from "./redux/actions"; 
import Store from "./redux/store"; 
import { IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "./types";

import "../css/app.css";
import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  show : boolean;
  onHide : () => any;
  user : IUser;                                                 // | Store
  pipelines : Map<number, IPipeline>;                           // | 
  taskGroups : Map<number, ITaskGroup>;                         // |
  taskTemplates : Map<number, ITaskTemplate>;                   // |
  tasks : Map<number, ITask>;                                   // |
  
  onAddTaskTemplate : (tasktemplate : ITaskTemplate) => any;    // | Actions
  onChangeTaskTemplate : (tasktemplate : ITaskTemplate) => any; // |
  onDelTaskTemplate : (tasktemplate : ITaskTemplate) => any;    // |
};

interface IState {
};

class DialogTaskTemplate extends React.Component<IProps, IState>{
   
  constructor(props : IProps){
    super(props);
   
   
  }

  render(){  
    return (
      <Modal show={this.props.show} onHide={this.props.onHide} >
        <Modal.Header closeButton>
          <Modal.Title>Modal title</Modal.Title>
        </Modal.Header>

        <Modal.Body>
          <p>Modal body text goes here.</p>
        </Modal.Body>

        <Modal.Footer>
          <Button variant="secondary">Close</Button>
          <Button variant="primary">Save changes</Button>
        </Modal.Footer>
      </Modal>
    )
  } 
}


// //////////////////////////////////////////////////

const mapStateToProps = (state) => {
  return state;
};

const mapDispatchToProps = (dispatch) => {
  return {
    onAddTaskTemplate : Action.addTaskTemplate(dispatch),
    onChangeTaskTemplate : Action.changeTaskTemplate(dispatch),
    onDelTaskTemplate : Action.delTaskTemplate(dispatch),
  }
};

let DialogTaskTemplateRedux = connect(mapStateToProps, mapDispatchToProps)(DialogTaskTemplate);

export default DialogTaskTemplateRedux;