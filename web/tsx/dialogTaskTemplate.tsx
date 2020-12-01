/* eslint-disable no-unused-vars */

import React from "react";
import { connect } from "react-redux";
import { Col, Button, Modal, Form} from "react-bootstrap";
 
import * as Action from "./redux/actions"; 
import { IUser, IPipeline, ITaskGroup, ITaskTemplate, ITask } from "./types";

import "../css/app.css";
import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  show : boolean;
  onHide : () => any;
  setTaskTempl : ITaskTemplate;
  user : IUser;                                                 // | Store
  pipelines : Map<number, IPipeline>;                           // | 
  taskGroups : Map<number, ITaskGroup>;                         // |
  taskTemplates : Map<number, ITaskTemplate>;                   // |
  tasks : Map<number, ITask>;                                   // |
  
  onAddTaskTemplate : (tasktemplate : ITaskTemplate) => any;    // | Actions
  onChangeTaskTemplate : (tasktemplate : ITaskTemplate) => any; // |
};

interface IState {
  statusMess : string;
};

class DialogTaskTemplate extends React.Component<IProps, IState>{
   
  private _refObj : object;

  constructor(props : IProps){
    super(props);
    
    this.state = { statusMess : ""};    
    this.hSubmit = this.hSubmit.bind(this); 
    this._refObj = {};
  }

  hSubmit(event) {
    
    let error = "";
    if (!this._refObj["Form.name"].value){
      error = "Name is empty"; 
    }
    else if (!this._refObj["Form.script"].value){
      error = "Script is empty"; 
    }
    
    if (error != this.state.statusMess){
      this.setState({statusMess : error});
    }
  }

  render(){  

    let ttask = this.props.setTaskTempl;

    return (
      <Modal show={this.props.show} onHide={this.props.onHide} >
        <Modal.Header closeButton>
          <Modal.Title>Task template</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Row>
              <Form.Group as={Col} controlId="Form.name">
                <Form.Label>Name</Form.Label>
                <Form.Control type="text" ref={(input) => {this._refObj["Form.name"] = input }} placeholder="any name" defaultValue={ttask.name}/>
              </Form.Group>
              <Form.Group as={Col} controlId="Form.Description">
                <Form.Label>Description</Form.Label>
                <Form.Control type="text" ref={(input) => {this._refObj["Form.description"] = input }} placeholder="optional description" defaultValue={ttask.description}/>
              </Form.Group>
            </Form.Row>
            <Form.Group controlId="Form.Script">
              <Form.Label>Script</Form.Label>
              <Form.Control as="textarea" ref={(input) => {this._refObj["Form.script"] = input }} placeholder="" defaultValue={ttask.script} rows={6} />
            </Form.Group>
            <Form.Row>
              <Form.Group as={Col} controlId="Form.averDurationSec">
                <Form.Label>Average duration, sec</Form.Label>
                <Form.Control type="number" ref={(input) => {this._refObj["Form.averDurationSec"] = input }} defaultValue={ttask.averDurationSec} />
              </Form.Group>
              <Form.Group as={Col} controlId="Form.maxDurationSec">
                <Form.Label>Maximum duration, sec</Form.Label>
                <Form.Control type="number" ref={(input) => {this._refObj["Form.maxDurationSec"] = input }}  defaultValue={ttask.maxDurationSec} />
              </Form.Group>
          </Form.Row>          
          <Form.Label>{this.state.statusMess}</Form.Label>      
          </Form>          
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" type="close" onClick={this.props.onHide}>Close</Button>
          <Button variant="primary" type="submit" onClick={this.hSubmit}>Save changes</Button>
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
  }
};

let DialogTaskTemplateRedux = connect(mapStateToProps, mapDispatchToProps)(DialogTaskTemplate);

export default DialogTaskTemplateRedux;