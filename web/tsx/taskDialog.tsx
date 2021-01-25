/* eslint-disable no-unused-vars */

import React from "react";
import { connect } from "react-redux";
import { Col, Button, Modal, Form} from "react-bootstrap";
 
import * as Action from "./redux/actions"; 
import { IUser, IPipeline, IGroup, ITaskTemplate, ITask } from "./types";

import "../css/app.css";
import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  show : boolean;
  onHide : (selPipeline : IPipeline) => any;
  selPipeline : IPipeline;
  
  user : IUser;                                     // | Store
  pipelines : Map<number, IPipeline>;               // | 
  groups : Map<number, IGroup>;                     // |
  taskTemplates : Map<number, ITaskTemplate>;       // |
  tasks : Map<number, ITask>;                       // |
  
  onAddPipeline : (pipeline : IPipeline) => any;    // | Actions 
  onChangePipeline : (pipeline : IPipeline) => any; // |
};

interface IState {
  statusMess : string; 
}; 

class PipelineDialog extends React.Component<IProps, IState>{
   
  private m_refObj : object;
  private m_tout : number;
  private m_isNewPipeline : boolean;
  private m_nameMem : string;

  constructor(props : IProps){
    super(props);
    
    this.state = { 
      statusMess : "" 
    };    
    this.hSubmit = this.hSubmit.bind(this); 
    this.m_refObj = {};
    this.m_tout = 0;
    this.m_isNewPipeline = this.props.selPipeline.id == 0;
    this.m_nameMem = this.props.selPipeline.name;
  }

  shouldComponentUpdate(nextProps, nextState) {
    this.m_isNewPipeline = nextProps.selPipeline.id == 0;
    return true;
  }

  hSubmit(event) {
    
    let error = "",
        name = this.m_refObj["name"].value,
        description = this.m_refObj["description"].value;
    if (!name)
      error = "Name is empty"; 
    else if (this.m_isNewPipeline && [...this.props.pipelines].find(item => item[1].name == name))
      error = `This name '${name}' already exists`;
    else if (!this.m_isNewPipeline && this.m_nameMem && (this.m_nameMem != name) && [...this.props.pipelines].find(item => item[1].name == name))
      error = `This name '${name}' already exists`;
       
    if (error){
      this.setState({statusMess : error});
      clearTimeout(this.m_tout);
      this.m_tout = setTimeout(() => this.setState({statusMess : ""}), 3000);
      return;
    }
    this.m_nameMem = name;
    
    let newPipeline : IPipeline = {
      id : this.props.selPipeline.id,
      name,           
      description
    }
    if (this.m_isNewPipeline){
      fetch('api/addPipeline', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json;charset=utf-8'
        },
        body: JSON.stringify(newPipeline)})
      .then(response => response.json())    
      .then(pipeline =>{
        this.props.onAddPipeline(pipeline);      
        this.setState({statusMess : "Success create of Task Pipeline"});    
        clearTimeout(this.m_tout);
        this.m_tout = setTimeout(() => { 
          this.setState({statusMess : ""});
          this.props.onHide(newPipeline);
        }, 1000)})
      .catch(() => {
        this.setState({statusMess : "api/addPipeline error"});
        console.log("api/addPipeline error");
        clearTimeout(this.m_tout);
        this.m_tout = setTimeout(() => this.setState({statusMess : ""}), 3000);  
      }); 
    }
    else{
      fetch('api/changePipeline', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json;charset=utf-8'
        },
        body: JSON.stringify(newPipeline)})
      .then(response => response.json())    
      .then(respPipeline =>{ 
        this.props.onChangePipeline(respPipeline); 
        this.setState({statusMess : "Success change of Task Pipeline"}); 
        clearTimeout(this.m_tout);
        this.m_tout = setTimeout(() => this.setState({statusMess : ""}), 3000);})
      .catch(() => {
        this.setState({statusMess : "api/changePipeline error"});
        console.log("api/changePipeline error");
        clearTimeout(this.m_tout);
        this.m_tout = setTimeout(() => this.setState({statusMess : ""}), 3000);  
      }); 
    }      
  }

  render(){  

    let ppl = this.props.selPipeline;
    
    return (
      <Modal show={this.props.show} onHide={()=>this.props.onHide(ppl)} >
        <Modal.Header closeButton>
          <Modal.Title> {this.m_isNewPipeline ? "Create of Task Pipeline" : "Edit of Task Pipeline"}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Row>
              <Form.Group as={Col} controlId="name">
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

////////////////////////////////////////////////////

const mapStoreToProps = (store) => {
  return store;
};

const mapDispatchToProps = (dispatch) => {
  return {
    onAddPipeline : Action.addPipeline(dispatch),
    onChangePipeline : Action.changePipeline(dispatch),
  }
};

let PipelineDialogModal = connect(mapStoreToProps, mapDispatchToProps)(PipelineDialog);

export default PipelineDialogModal;