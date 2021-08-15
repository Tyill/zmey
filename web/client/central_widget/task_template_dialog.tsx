import React from "react";
import { Col, Button, Modal, Form} from "react-bootstrap";
 
import { ITaskTemplate } from "../types";
import {ServerAPI} from "../server_api/server_api"
import { TaskTemplates} from "../store/store";

import "bootstrap/dist/css/bootstrap.min.css";
import "../css/style.css";

interface IProps {
  show : boolean;
  onHide : (selTaskTemplate : ITaskTemplate) => any;
  selTaskTemplate : ITaskTemplate;
};

interface IState {
  statusMess : string; 
}; 

export default
class TaskTemplateDialogModal extends React.Component<IProps, IState>{
   
  private m_refObj : object;
  private m_tout : number = 0;
  private m_isNewTask : boolean;
  private m_nameMem : string;
  private m_hasAdded : boolean;

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
    this.m_hasAdded = false;
  }

  shouldComponentUpdate(nextProps, nextState) {
    this.m_isNewTask = nextProps.selTaskTemplate.id == 0;
    return true;
  }

  hSubmit(event) {

    if (this.m_hasAdded) return;
    
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
    else if (this.m_isNewTask && TaskTemplates.getByName(name))
      error = `This name '${name}' already exists`;
    else if (!this.m_isNewTask && this.m_nameMem && (this.m_nameMem != name) && TaskTemplates.getByName(name))
      error = `This name '${name}' already exists`;
       
    if (error){
      this.setStatusMess(error);
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
      this.m_hasAdded = true;
      ServerAPI.addTaskTemplate(newTaskTemplate, 
        (respTaskTemplate)=>{
          TaskTemplates.add(respTaskTemplate);      
          this.setStatusMess("Success create of TaskTemplate", 1, ()=>{this.props.onHide(respTaskTemplate); this.m_hasAdded = false;});          
        },
        ()=>{this.setStatusMess("Server error create of TaskTemplate"); this.m_hasAdded = false}
      );       
    }
    else{
      ServerAPI.changeTaskTemplate(newTaskTemplate, 
        (respTaskTemplate)=>{
          TaskTemplates.upd(respTaskTemplate); 
          this.setStatusMess("Success change of TaskTemplate");
        },
        ()=>this.setStatusMess("Server error change of TaskTemplate")
      );
    }      
  }

  setStatusMess(mess : string, delaySec : number = 3, cback : ()=>void = null){
    this.setState({statusMess : mess});    
    if (this.m_tout) clearTimeout(this.m_tout);
    this.m_tout = setTimeout(() => { 
      this.setState({statusMess : ""});
      if (cback) cback();
    }, delaySec * 1000)
  }

  render(){  

    let ttask = this.props.selTaskTemplate;

    return (
      <Modal dialogClassName="taskTemplateDialogModal" show={this.props.show} onHide={()=>this.props.onHide(ttask)}  >
        <Modal.Header closeButton>
          <Modal.Title> {this.m_isNewTask ? "Create of TaskTemplate" : "Edit of TaskTemplate"}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Row >
              <Form.Group as={Col} style={{maxWidth:"200px"}}  controlId="name">
                <Form.Label>Name</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["name"] = input }} placeholder="any" defaultValue={ttask.name}/>
              </Form.Group>
              <Form.Group as={Col} controlId="description">
                <Form.Label>Description</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["description"] = input }} placeholder="optional" defaultValue={ttask.description}/>
              </Form.Group>
              <Form.Group as={Col} style={{maxWidth:"200px"}} controlId="averDurationSec">
                <Form.Label>Average duration, sec</Form.Label>
                <Form.Control type="number" min="1" ref={(input) => {this.m_refObj["averDurationSec"] = input }} defaultValue={ttask.averDurationSec} />
              </Form.Group>
              <Form.Group as={Col} style={{maxWidth:"200px"}} controlId="maxDurationSec">
                <Form.Label>Maximum duration, sec</Form.Label>
                <Form.Control type="number" min="1" ref={(input) => {this.m_refObj["maxDurationSec"] = input }}  defaultValue={ttask.maxDurationSec} />
              </Form.Group>
            </Form.Row>
            <Form.Group  controlId="script">
              <Form.Label>Script</Form.Label>
              <Form.Control  style={{height:"60vh"}} as="textarea" ref={(input) => {this.m_refObj["script"] = input }} placeholder="" defaultValue={ttask.script} rows={6} />
            </Form.Group>         
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