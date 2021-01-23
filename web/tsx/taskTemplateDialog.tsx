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
  selTaskTemplate : ITaskTemplate;
  
  user : IUser;                                                 // | Store
  pipelines : Map<number, IPipeline>;                           // | 
  taskGroups : Map<number, ITaskGroup>;                         // |
  taskTemplates : Map<number, ITaskTemplate>;                   // |
  tasks : Map<number, ITask>;                                   // |
  
  onAddTaskTemplate : (taskTemplate : ITaskTemplate) => any;    // | Actions 
  onChangeTaskTemplate : (taskTemplate : ITaskTemplate) => any; // |
};

interface IState {
  statusMess : string; 
}; 

class DialogTaskTemplate extends React.Component<IProps, IState>{
   
  private _refObj : object;

  constructor(props : IProps){
    super(props);
    
    this.state = { 
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
        averDurationSec = parseInt(this._refObj["averDurationSec"].value ? this._refObj["averDurationSec"].value : 1),
        maxDurationSec = parseInt(this._refObj["maxDurationSec"].value ? this._refObj["maxDurationSec"].value : 1);
    if (!name)
      error = "Name is empty"; 
    else if (!script)
      error = "Script is empty";
        
    if (error){
      this.setState({statusMess : error});
      setTimeout(() => this.setState({statusMess : ""}), 3000);
      return;
    }
  
    let existTask = [...this.props.taskTemplates].find(item => item[1].name == name);
    if (existTask)
      this.props.selTaskTemplate.id = existTask[1].id;

    let newTaskTemplate : ITaskTemplate = {
      id : this.props.selTaskTemplate.id,
      name,           
      script,
      averDurationSec, 
      maxDurationSec,
      description
    }
    if (this.props.selTaskTemplate.id == 0){
      fetch('api/addTaskTemplate', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json;charset=utf-8'
        },
        body: JSON.stringify(newTaskTemplate)})
      .then(response => response.json())    
      .then(taskTemplate =>{
        this.props.onAddTaskTemplate(taskTemplate);      
        this.setState({statusMess : "Success addition of Task Template"});    
        setTimeout(() => this.setState({statusMess : ""}), 3000);  
      })
      .catch(() => {
        this.setState({statusMess : "api/addTaskTemplate error"});
        console.log("api/addTaskTemplate error");
        setTimeout(() => this.setState({statusMess : ""}), 3000);  
      }); 
    }
    else{
      fetch('api/changeTaskTemplate', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json;charset=utf-8'
        },
        body: JSON.stringify(newTaskTemplate)})
      .then(response => response.json())    
      .then(respTaskTemplate =>{ 
        this.props.onChangeTaskTemplate(respTaskTemplate); 
        this.setState({statusMess : "Success change of Task Template"}); 
        setTimeout(() => this.setState({statusMess : ""}), 3000);         
      })
      .catch(() => {
        this.setState({statusMess : "api/changeTaskTemplate error"});
        console.log("api/changeTaskTemplate error");
        setTimeout(() => this.setState({statusMess : ""}), 3000);  
      }); 
    }      
  }

  render(){  

    let ttask = this.props.selTaskTemplate;
    
    return (
      <Modal show={this.props.show} onHide={this.props.onHide} >
        <Modal.Header closeButton>
          <Modal.Title> {ttask.id == 0 ? "Addition of Task Template" : "Edit of Task Template"}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Row>
              <Form.Group as={Col} controlId="name">
                <Form.Label>Name</Form.Label>
                <Form.Control type="text" ref={(input) => {this._refObj["name"] = input }} placeholder="any" defaultValue={ttask.name}/>
              </Form.Group>
              <Form.Group as={Col} controlId="description">
                <Form.Label>Description</Form.Label>
                <Form.Control type="text" ref={(input) => {this._refObj["description"] = input }} placeholder="optional" defaultValue={ttask.description}/>
              </Form.Group>
            </Form.Row>
            <Form.Group controlId="script">
              <Form.Label>Script</Form.Label>
              <Form.Control as="textarea" ref={(input) => {this._refObj["script"] = input }} placeholder="" defaultValue={ttask.script} rows={6} />
            </Form.Group>
            <Form.Row>
              <Form.Group as={Col} controlId="averDurationSec">
                <Form.Label>Average duration, sec</Form.Label>
                <Form.Control type="number" min="1" ref={(input) => {this._refObj["averDurationSec"] = input }} defaultValue={ttask.averDurationSec} />
              </Form.Group>
              <Form.Group as={Col} controlId="maxDurationSec">
                <Form.Label>Maximum duration, sec</Form.Label>
                <Form.Control type="number" min="1" ref={(input) => {this._refObj["maxDurationSec"] = input }}  defaultValue={ttask.maxDurationSec} />
              </Form.Group>
          </Form.Row>  
          <Form.Row style={{height:"20px"}}> 
            <Form.Label style={{marginLeft:"5px"}}>{this.state.statusMess}</Form.Label>   
          </Form.Row>  
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

const mapStoreToProps = (store) => {
  return store;
};

const mapDispatchToProps = (dispatch) => {
  return {
    onAddTaskTemplate : Action.addTaskTemplate(dispatch),
    onChangeTaskTemplate : Action.changeTaskTemplate(dispatch),
  }
};

let DialogTaskTemplateModal = connect(mapStoreToProps, mapDispatchToProps)(DialogTaskTemplate);

export default DialogTaskTemplateModal;