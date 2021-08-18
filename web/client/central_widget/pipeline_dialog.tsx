import React from "react";
import { Col, Button, Modal, Form} from "react-bootstrap";
 
import { IPipeline } from "../types";
import { Pipelines} from "../store/store";
import {ServerAPI} from "../server_api/server_api"

import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  show : boolean;
  onHide : (selPipeline : IPipeline) => any;
  selPipeline : IPipeline;
};

interface IState {
  statusMess : string; 
}; 

export default
class PipelineDialogModal extends React.Component<IProps, IState>{
   
  private m_refObj : object;
  private m_tout : number = 0;
  private m_isNewPipeline : boolean;
  private m_hasAdded : boolean;

  constructor(props : IProps){
    super(props);
    
    this.state = { 
      statusMess : "" 
    };    
    this.hSubmit = this.hSubmit.bind(this); 
    this.m_refObj = {};
    this.m_tout = 0;
    this.m_isNewPipeline = this.props.selPipeline.id == 0;
    this.m_hasAdded = false;
  }

  shouldComponentUpdate(nextProps, nextState) {
    this.m_isNewPipeline = nextProps.selPipeline.id == 0;
    return true;
  }

  hSubmit(event) {
    
    if (this.m_hasAdded) return;

    let error = "",
        name = this.m_refObj["name"].value,
        description = this.m_refObj["description"].value;
    if (!name)
      error = "Name is empty"; 
    else if (this.m_isNewPipeline && Pipelines.getByName(name))
      error = `This name '${name}' already exists`;
    else if (!this.m_isNewPipeline && (this.props.selPipeline.name != name) && Pipelines.getByName(name))
      error = `This name '${name}' already exists`;
       
    if (error){
      this.setStatusMess(error);
      return;
    }
    
    let newPipeline : IPipeline = {
      id : this.props.selPipeline.id,
      name,           
      description
    }
    
    if (this.m_isNewPipeline){
      this.m_hasAdded = true;
      ServerAPI.addPipeline(newPipeline, 
        (respPipeline)=>{
          Pipelines.add(respPipeline);      
          this.setStatusMess("Success create of Pipeline", 1,
           ()=>{this.props.onHide(respPipeline); this.m_hasAdded = false;});
        },
        ()=>{this.setStatusMess("Server error create of Pipeline"); this.m_hasAdded = false;}
      )         
    }
    else{
      ServerAPI.changePipeline(newPipeline, 
        (respPipeline)=>{
          Pipelines.upd(respPipeline); 
          this.setStatusMess("Success change of Pipeline");
        },
        () =>this.setStatusMess("Server error change of pipeline")
      )
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

    let ppl = this.props.selPipeline;
    
    return (
      <Modal show={this.props.show} onHide={()=>this.props.onHide(ppl)} >
        <Modal.Header closeButton>
          <Modal.Title> {this.m_isNewPipeline ? "Create of Pipeline" : "Edit of Task Pipeline"}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Row>
              <Form.Group as={Col} style={{maxWidth:"200px"}} controlId="name">
                <Form.Label>Name</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["name"] = input }} placeholder="any" defaultValue={ppl.name}/>
              </Form.Group>
              <Form.Group as={Col} controlId="description">
                <Form.Label>Description</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["description"] = input }} placeholder="optional" defaultValue={ppl.description}/>
              </Form.Group>
            </Form.Row>
          <Form.Row style={{height:"20px"}}> 
            <Form.Label style={{marginLeft:"5px"}}>{this.state.statusMess}</Form.Label>   
          </Form.Row>  
          </Form>          
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" type="close" onClick={()=> this.props.onHide(this.props.selPipeline)}>Close</Button>
          <Button variant="primary" type="submit" onClick={this.hSubmit}> {this.m_isNewPipeline ? "Create" : "Save changes"}</Button>
        </Modal.Footer>        
      </Modal>
    )
  } 
}
