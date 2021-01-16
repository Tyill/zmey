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
  
  user : IUser;                                                 // | Store
  pipelines : Map<number, IPipeline>;                           // | 
  taskGroups : Map<number, ITaskGroup>;                         // |
  taskTemplates : Map<number, ITaskTemplate>;                   // |
  tasks : Map<number, ITask>;                                   // |
  
  onAddTaskTemplate : (tasktemplate : ITaskTemplate) => any;    // | Actions
  onChangeTaskTemplate : (tasktemplate : ITaskTemplate) => any; // |
};

interface IState {
  curTaskTempl : ITaskTemplate;
  statusMess : string;
};

class DialogTaskTemplate extends React.Component<IProps, IState>{
   
  private _refObj : object;

  constructor(props : IProps){
    super(props);
    
    this.state = { 
      curTaskTempl : {
        id : 0,
        name : "",           
        script : "",
        taskIdList : new Set<number>(),
        averDurationSec : 0, 
        maxDurationSec : 0,
        description : ""
      }, 
      statusMess : "" 
    };    
    this.hSubmit = this.hSubmit.bind(this); 
    this._refObj = {};
  }

  hSubmit(event) {
    
    let error = "",
        name = this._refObj["name"].value,
        script = this._refObj["script"].value,
        description = this._refObj["description"].value,
        averDurationSec = this._refObj["averDurationSec"].value,
        maxDurationSec = this._refObj["maxDurationSec"].value;
    if (!name){
      error = "Name is empty"; 
    }
    else if (!script){
      error = "Script is empty"; 
    }
    
    if (!error){
      let newTaskTemplate : ITaskTemplate = {
        id : 0,
        name,           
        script,
        taskIdList : new Set<number>(),
        averDurationSec, 
        maxDurationSec,
        description
      }
      if (this.props.taskTemplates.has(name))
        fetch('api/addTaskTemplate', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json;charset=utf-8'
          },
          body: JSON.stringify(newTaskTemplate)})
        .then(response => response.json())    
        .then(respTaskTemplate =>{           
          this.props.onAddTaskTemplate(respTaskTemplate);           
        })
        .catch(() => console.log('api/addTaskTemplate error')); 
      else
        fetch('api/changeTaskTemplate', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json;charset=utf-8'
          },
          body: JSON.stringify(newTaskTemplate)})
        .then(response => response.json())    
        .then(respTaskTemplate =>{           
          this.props.onChangeTaskTemplate(respTaskTemplate);           
        })
        .catch(() => console.log('api/addTaskTemplate error')); 

      this.setState({curTaskTempl : newTaskTemplate});
    } 
    
    if (error != this.state.statusMess){
      this.setState({statusMess : error});
    }    
  }

  render(){  

    let ttask = this.state.curTaskTempl;

    return (
      <Modal show={this.props.show} onHide={this.props.onHide} >
        <Modal.Header closeButton>
          <Modal.Title>Task template</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Row>
              <Form.Group as={Col} controlId="name">
                <Form.Label>Name</Form.Label>
                <Form.Control type="text" ref={(input) => {this._refObj["name"] = input }} placeholder="any name" defaultValue={ttask.name}/>
              </Form.Group>
              <Form.Group as={Col} controlId="description">
                <Form.Label>Description</Form.Label>
                <Form.Control type="text" ref={(input) => {this._refObj["description"] = input }} placeholder="optional description" defaultValue={ttask.description}/>
              </Form.Group>
            </Form.Row>
            <Form.Group controlId="script">
              <Form.Label>Script</Form.Label>
              <Form.Control as="textarea" ref={(input) => {this._refObj["script"] = input }} placeholder="" defaultValue={ttask.script} rows={6} />
            </Form.Group>
            <Form.Row>
              <Form.Group as={Col} controlId="averDurationSec">
                <Form.Label>Average duration, sec</Form.Label>
                <Form.Control type="number" min="0" ref={(input) => {this._refObj["averDurationSec"] = input }} defaultValue={ttask.averDurationSec} />
              </Form.Group>
              <Form.Group as={Col} controlId="maxDurationSec">
                <Form.Label>Maximum duration, sec</Form.Label>
                <Form.Control type="number" min="0" ref={(input) => {this._refObj["maxDurationSec"] = input }}  defaultValue={ttask.maxDurationSec} />
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