import React from "react";
import { Col, Button, Modal, Form, Table} from "react-bootstrap";
 
import { IPipelineTask } from "../types";
import { PipelineTasks, TaskTemplates, Pipelines} from "../store/store";
import * as ServerAPI from "../server_api/server_api"

import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  show : boolean;
  onHide : (selPipelineTask : IPipelineTask) => any;
  selPipelineTask : IPipelineTask;
};

interface IState {
  statusMess : string; 
}; 

export default
class PipelineTaskDialogModal extends React.Component<IProps, IState>{
   
  private m_refObj : object;
  private m_tout : number = 0;
  private m_isNewPipelineTask : boolean;
  private m_hasAdded : boolean;

  constructor(props : IProps){
    super(props);
    
    this.state = { 
      statusMess : "" 
    };    
    this.hSubmit = this.hSubmit.bind(this); 
    this.m_refObj = {};
    this.m_tout = 0;
    this.m_isNewPipelineTask = this.props.selPipelineTask.id == 0;
    this.m_hasAdded = false;
  }

  shouldComponentUpdate(nextProps, nextState) {
    this.m_isNewPipelineTask = nextProps.selPipelineTask.id == 0;
    return true;
  }

  hSubmit(event) {
    
    if (this.m_hasAdded) return;

    let error = "",
        name = this.m_refObj["name"].value;
    if (!name)
      error = "Name is empty"; 
    else if (this.m_isNewPipelineTask && PipelineTasks.getByName(name))
      error = `This name '${name}' already exists`;
    else if (!this.m_isNewPipelineTask && (this.props.selPipelineTask.name != name) && PipelineTasks.getByName(name))
      error = `This name '${name}' already exists`;
       
    if (error){
      this.setStatusMess(error);
      return;
    }
   
    let newPipelineTask = {
      id : this.props.selPipelineTask.id,
      pplId : this.m_refObj["pipeline"].value,
      ttId: this.m_refObj["taskTemplate"].value,
      isEnabled : 1,
      isVisible : 1,
      positionX : 0,
      positionY : 0,
      nextTasksId: this.props.selPipelineTask.nextTasksId || [],
      nextEventsId: this.props.selPipelineTask.nextEventsId || [],
      params : this.m_refObj["params"].value,
      name : this.m_refObj["name"].value,           
      description : this.m_refObj["description"].value,
    } as IPipelineTask;
     
    if (this.m_isNewPipelineTask){
      this.m_hasAdded = true;
      ServerAPI.addPipelineTask(newPipelineTask, 
        (respPipeline)=>{
          PipelineTasks.add(respPipeline);      
          this.setStatusMess("Success create of Pipeline", 1,
           ()=>{this.props.onHide(respPipeline); this.m_hasAdded = false;});
        },
        ()=>{this.setStatusMess("Server error create of Pipeline"); this.m_hasAdded = false;}
      )         
    }
    else{
      ServerAPI.changePipelineTask(newPipelineTask, 
        (respPipelineTask)=>{
          PipelineTasks.upd(respPipelineTask); 
          this.setStatusMess("Success change of Pipeline");
        },
        () =>this.setStatusMess("Server error change of Pipeline")
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

    const task = this.props.selPipelineTask;
    
    let pipelines = [];
    Pipelines.getAll().forEach(p=>{
      pipelines.push(<option id={p.id.toString()} key={p.id.toString()} value={p.id}>
                        {p.name}
                     </option>);
    });

    let taskTemplates = [];
    TaskTemplates.getAll().forEach(t=>{
      taskTemplates.push(<option id={t.id.toString()} key={t.id.toString()} value={t.id}>
                          {t.name}
                        </option>);
    });
   
    return (
      <Modal show={this.props.show} onHide={()=>this.props.onHide(task)} >
        <Modal.Header closeButton>
          <Modal.Title> {this.m_isNewPipelineTask ? "Create of Pipeline Task" : "Edit of Pipeline Task"}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Row>
              <Form.Group as={Col} style={{maxWidth:"200px"}} controlId="name">
                <Form.Label>Name</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["name"] = input }} placeholder="any" defaultValue={task.name}/>
              </Form.Group>
              <Form.Group as={Col} controlId="description">
                <Form.Label>Description</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["description"] = input }} placeholder="optional" defaultValue={task.description}/>
              </Form.Group>
            </Form.Row>
            <Form.Row>
              <Form.Group as={Col} style={{maxWidth:"200px"}}>
                <Form.Label>Pipeline</Form.Label>
                <Form.Control as="select" custom defaultValue={task.pplId} ref={(input) => {this.m_refObj["pipeline"] = input }}>
                  {pipelines}
                </Form.Control>
                <p/>
                <Form.Label>Task Template</Form.Label>
                <Form.Control as="select" custom defaultValue={task.ttId} ref={(input) => {this.m_refObj["taskTemplate"] = input }}>
                  {taskTemplates}
                </Form.Control>
              </Form.Group>
              <Form.Group as={Col} controlId="params">
                <Form.Label>Parameters</Form.Label>
                <Form.Control as="textarea" ref={(input) => {this.m_refObj["params"] = input }} placeholder="" defaultValue={task.params} rows={5} />
              </Form.Group>
            </Form.Row>
            <Form.Row style={{height:"20px"}}> 
              <Form.Label style={{marginLeft:"5px"}}>{this.state.statusMess}</Form.Label>   
            </Form.Row>  
          </Form>          
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" type="close" onClick={()=> this.props.onHide(task)}>Close</Button>
          <Button variant="primary" type="submit" onClick={this.hSubmit}> {this.m_isNewPipelineTask ? "Create" : "Save changes"}</Button>
        </Modal.Footer>        
      </Modal>
    )
  } 
}
