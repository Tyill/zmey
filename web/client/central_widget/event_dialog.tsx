import React from "react";
import { Col, Button, Modal, Form} from "react-bootstrap";
 
import { IEvent } from "../types";
import { Events} from "../store/store";
import * as ServerAPI from "../server_api/server_api"

import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  show : boolean;
  onHide : () => any;
  selEvent : IEvent;
};

interface IState {
  statusMess : string; 
}; 

export default
class EventDialogModal extends React.Component<IProps, IState>{
   
  private m_refObj : object;
  private m_toutMess : number = 0;
  private m_isNewEvent : boolean;
  private m_hasAdded : boolean;

  constructor(props : IProps){
    super(props);
    
    this.state = { 
      statusMess : "" 
    };    
    this.hSubmit = this.hSubmit.bind(this); 
    this.m_refObj = {};
    this.m_toutMess = 0;
    this.m_isNewEvent = this.props.selEvent.id == 0;
    this.m_hasAdded = false;
  }

  shouldComponentUpdate(nextProps, nextState) {
    this.m_isNewEvent = nextProps.selEvent.id == 0;
    return true;
  }

  hSubmit(event) {
    
    if (this.m_hasAdded) return;
        
    let error = "",
        name = this.m_refObj["name"].value;
    if (!name)
      error = "Name is empty"; 
    else if (this.m_isNewEvent && Events.getByName(name))
      error = `This name '${name}' already exists`;
    else if (!this.m_isNewEvent && (this.props.selEvent.name != name) && Events.getByName(name))
      error = `This name '${name}' already exists`;
           
    if (error){
      this.setStatusMess(error);
      return;
    }
   
    let newEvent = {
      id : this.props.selEvent.id || 0,
      isEnabled : this.props.selEvent.isEnabled || true,     
      nextTasksId: !this.m_isNewEvent ? this.props.selEvent.nextTasksId : [],
      params : this.m_refObj["params"].value,
      name : this.m_refObj["name"].value,           
      description : this.m_refObj["description"].value,
    } as IEvent;
    
    if (this.m_isNewEvent){
      this.m_hasAdded = true;
      ServerAPI.addEvent(newEvent, 
        (resp)=>{
          Events.add(resp);      
          this.setStatusMess("Success create of Event", 1,
           ()=>{this.props.onHide(); this.m_hasAdded = false;});
        },
        ()=>{this.setStatusMess("Server error create of Event"); this.m_hasAdded = false;}
      )         
    }
    else{
      ServerAPI.changeEvent(newEvent, 
        (resp)=>{
          Events.upd(resp); 
          this.setStatusMess("Success change of Event");
        },
        () =>this.setStatusMess("Server error change of Event")
      )
    }      
  }

  setStatusMess(mess : string, delaySec : number = 3, cback : ()=>void = null){
    this.setState({statusMess : mess});    
    if (this.m_toutMess) clearTimeout(this.m_toutMess);
    this.m_toutMess = setTimeout(() => { 
      this.setState({statusMess : ""});
      if (cback) cback();
    }, delaySec * 1000)
  }

  render(){  

    const evt = this.props.selEvent;
           
    return (
      <Modal show={this.props.show} onHide={()=>this.props.onHide()} >
        <Modal.Header closeButton>
          <Modal.Title> {this.m_isNewEvent ? "Create of Event" : `${evt.id}# Edit of Event`}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Row>
              <Form.Group as={Col} style={{maxWidth:"200px"}} controlId="name">
                <Form.Label>Name</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["name"] = input }} placeholder="any" defaultValue={evt.name}/>
              </Form.Group>
              <Form.Group as={Col} controlId="description">
                <Form.Label>Description</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["description"] = input }} placeholder="optional" defaultValue={evt.description}/>
              </Form.Group>
            </Form.Row>
            <Form.Row>
              <Form.Group as={Col} controlId="params">
                <Form.Label>Parameters</Form.Label>
                <Form.Control as="textarea" ref={(input) => {this.m_refObj["params"] = input }} placeholder="" defaultValue={evt.params} rows={5} />
              </Form.Group>
            </Form.Row>
            <Form.Row style={{height:"20px"}}> 
              <Form.Label style={{marginLeft:"5px"}}>{this.state.statusMess}</Form.Label>   
            </Form.Row>  
          </Form>          
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" type="close" onClick={()=> this.props.onHide()}>Close</Button>
          <Button variant="primary" type="submit" onClick={this.hSubmit}> {this.m_isNewEvent ? "Create" : "Save changes"}</Button>
        </Modal.Footer>        
      </Modal>
    )
  } 
}
