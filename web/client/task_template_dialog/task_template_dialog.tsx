import React from "react";
import { connect } from "react-redux";
import { Col, Button, Modal, Form} from "react-bootstrap";
 
import * as Action from "../redux/actions"; 
import { ITaskTemplate } from "../types";
import {ServerAPI} from "../server_api"

import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  show : boolean;
  onHide : (selTaskTemplate : ITaskTemplate) => any;
  selTaskTemplate : ITaskTemplate;
  
  taskTemplates : Map<number, ITaskTemplate>;                   // | Store
  
  onAddTaskTemplate : (taskTemplate : ITaskTemplate) => any;    // | Actions 
  onChangeTaskTemplate : (taskTemplate : ITaskTemplate) => any; // |
};

interface IState {
  statusMess : string; 
}; 

class TaskTemplateDialog extends React.Component<IProps, IState>{
   
  private m_refObj : object;
  private m_tout : number;
  private m_isNewTask : boolean;
  private m_nameMem : string;

  constructor(props : IProps){
    super(props);
    
    this.state = { 
      statusMess : "" 
    };    
    this.hSubmit = this.hSubmit.bind(this); 
    this.m_refObj = {};
    this.m_tout = 0;
    this.m_isNewTask = this.props.selTaskTemplate.id == 0;
    this.m_nameMem = this.props.selTaskTemplate.name;
  }

  shouldComponentUpdate(nextProps, nextState) {
    this.m_isNewTask = nextProps.selTaskTemplate.id == 0;
    return true;
  }

  hSubmit(event) {
    
    let error = "",
        name = this.m_refObj["name"].value,
        script = this.m_refObj["script"].value,
        description = this.m_refObj["description"].value,
        averDurationSec = parseInt(this.m_refObj["averDurationSec"].value ? this.m_refObj["averDurationSec"].value : 1),
        maxDurationSec = parseInt(this.m_refObj["maxDurationSec"].value ? this.m_refObj["maxDurationSec"].value : 1);
    if (!name)
      error = "Name is empty"; 
    else if (!script)
      error = "Script is empty";
    else if (this.m_isNewTask && [...this.props.taskTemplates].find(item => item[1].name == name))
      error = `This name '${name}' already exists`;
    else if (!this.m_isNewTask && this.m_nameMem && (this.m_nameMem != name) && [...this.props.taskTemplates].find(item => item[1].name == name))
      error = `This name '${name}' already exists`;
       
    if (error){
      this.setState({statusMess : error});
      clearTimeout(this.m_tout);
      this.m_tout = setTimeout(() => this.setState({statusMess : ""}), 3000);
      return;
    }
    this.m_nameMem = name;
    
    let newTaskTemplate : ITaskTemplate = {
      id : this.props.selTaskTemplate.id,
      name,           
      script,
      averDurationSec, 
      maxDurationSec,
      description
    }
    if (this.m_isNewTask){
      ServerAPI.addTaskTemplate(newTaskTemplate, 
        (respTaskTemplate)=>{
          this.props.onAddTaskTemplate(respTaskTemplate);      
          this.setState({statusMess : "Success create of Task Template"});    
          clearTimeout(this.m_tout);
          this.m_tout = setTimeout(() => { 
            this.setState({statusMess : ""});
            this.props.onHide(respTaskTemplate);
          }, 1000)
        },
        ()=>{
          this.setState({statusMess : "ServerAPI.addTaskTemplate error"});
          clearTimeout(this.m_tout);
          this.m_tout = setTimeout(() => this.setState({statusMess : ""}), 3000);
        });       
    }
    else{
      ServerAPI.changeTaskTemplate(newTaskTemplate, 
        (respTaskTemplate)=>{
          this.props.onChangeTaskTemplate(respTaskTemplate); 
          this.setState({statusMess : "Success change of Task Template"}); 
          clearTimeout(this.m_tout);
          this.m_tout = setTimeout(() => this.setState({statusMess : ""}), 3000);
        },
        ()=>{
          this.setState({statusMess : "ServerAPI.changeTaskTemplate error"});
          clearTimeout(this.m_tout);
          this.m_tout = setTimeout(() => this.setState({statusMess : ""}), 3000);  
        });
    }      
  }

  render(){  

    let ttask = this.props.selTaskTemplate;
    
    return (
      <Modal show={this.props.show} onHide={()=>this.props.onHide(ttask)} >
        <Modal.Header closeButton>
          <Modal.Title> {this.m_isNewTask ? "Create of Task Template" : "Edit of Task Template"}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Row>
              <Form.Group as={Col} controlId="name">
                <Form.Label>Name</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["name"] = input }} placeholder="any" defaultValue={ttask.name}/>
              </Form.Group>
              <Form.Group as={Col} controlId="description">
                <Form.Label>Description</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["description"] = input }} placeholder="optional" defaultValue={ttask.description}/>
              </Form.Group>
            </Form.Row>
            <Form.Group controlId="script">
              <Form.Label>Script</Form.Label>
              <Form.Control as="textarea" ref={(input) => {this.m_refObj["script"] = input }} placeholder="" defaultValue={ttask.script} rows={6} />
            </Form.Group>
            <Form.Row>
              <Form.Group as={Col} controlId="averDurationSec">
                <Form.Label>Average duration, sec</Form.Label>
                <Form.Control type="number" min="1" ref={(input) => {this.m_refObj["averDurationSec"] = input }} defaultValue={ttask.averDurationSec} />
              </Form.Group>
              <Form.Group as={Col} controlId="maxDurationSec">
                <Form.Label>Maximum duration, sec</Form.Label>
                <Form.Control type="number" min="1" ref={(input) => {this.m_refObj["maxDurationSec"] = input }}  defaultValue={ttask.maxDurationSec} />
              </Form.Group>
          </Form.Row>  
          <Form.Row style={{height:"20px"}}> 
            <Form.Label style={{marginLeft:"5px"}}>{this.state.statusMess}</Form.Label>   
          </Form.Row>  
          </Form>          
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" type="close" onClick={()=> this.props.onHide(this.props.selTaskTemplate)}>Close</Button>
          <Button variant="primary" type="submit" onClick={this.hSubmit}> {this.m_isNewTask ? "Create" : "Save changes"}</Button>
        </Modal.Footer>        
      </Modal>
    )
  } 
  }

////////////////////////////////////////////////////

const mapStoreToProps = (store) => {
  return store;
};

const mapDispatchToProps = (dispatch) => {
  return {
    onAddTaskTemplate : Action.addTaskTemplate(dispatch),
    onChangeTaskTemplate : Action.changeTaskTemplate(dispatch),
  }
};

let TaskTemplateDialogModal = connect(mapStoreToProps, mapDispatchToProps)(TaskTemplateDialog);

export default TaskTemplateDialogModal;