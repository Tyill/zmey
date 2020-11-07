/* eslint-disable no-unused-vars */

import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import {Container, Row, Col, Button, Modal, Form, ListGroup} from "react-bootstrap";
 
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
  accountNumber : string;
};

class DialogTaskTemplate extends React.Component<IProps, IState>{
   
  constructor(props : IProps){
    super(props);
   
    this.hSubmit = this.hSubmit.bind(this); 
    this.handleChange = this.handleChange.bind(this); 

    this.state = {accountNumber: ''}
  }

  hSubmit(event) {
    console.log("dfdsfdsf")
    this.setState({accountNumber: '12345'});
  }

  handleChange(event){
    console.log("dsfffffffffff")
    this.setState({accountNumber: event.target.value.replace(/[^0-9]/ig,'')});
  }

  render(){  
    return (
      <Modal  show={this.props.show} onHide={this.props.onHide} >
        <Modal.Header closeButton>
          <Modal.Title>Task template</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form onSubmit={this.hSubmit}>
            <Form.Row>
              <Form.Group as={Col} controlId="taskTemplateForm.Name">
                <Form.Label>Name</Form.Label>
                <Form.Control type="text" placeholder="any name" value={this.state.accountNumber} onChange={this.handleChange}/>
              </Form.Group>
              <Form.Group as={Col} controlId="taskTemplateForm.Description">
                <Form.Label>Description</Form.Label>
                <Form.Control type="text" placeholder="optional description" />
              </Form.Group>
            </Form.Row>
            <Form.Group controlId="taskTemplateForm.Script">
              <Form.Label>Script</Form.Label>
              <Form.Control as="textarea" placeholder="required script" rows={6} />
            </Form.Group>
            <Form.Row>
              <Form.Group as={Col} controlId="taskTemplateForm.averDurationSec">
                <Form.Label>Average duration, sec</Form.Label>
                <Form.Control type="number" placeholder="from 1 sec" />
              </Form.Group>
              <Form.Group as={Col} controlId="taskTemplateForm.maxDurationSec">
                <Form.Label>Maximum duration, sec</Form.Label>
                <Form.Control type="number" placeholder="from 1 sec, -1 - not limited" />
              </Form.Group>
          </Form.Row>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" type="close" onClick={this.props.onHide} >Close</Button>
          <Button variant="primary" type="submit" onClick={this.hSubmit} >Save changes</Button>
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