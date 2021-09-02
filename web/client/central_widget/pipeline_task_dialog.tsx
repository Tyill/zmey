import React from "react";
import { Col, Button, Modal, Form} from "react-bootstrap";
 
import { IPipelineTask } from "../types";
import { PipelineTasks, TaskTemplates, Pipelines} from "../store/store";
import * as ServerAPI from "../server_api/server_api"

import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  show : boolean;
  onHide : () => any;
  selPipelineTask : IPipelineTask;
};

interface IState {
  statusMess : string; 
}; 

export default
class PipelineTaskDialogModal extends React.Component<IProps, IState>{
   
  private m_refObj : object;
  private m_toutMess : number = 0;
  private m_isNewPipelineTask : boolean;
  private m_hasAdded : boolean;

  constructor(props : IProps){
    super(props);
    
    this.state = { 
      statusMess : "" 
    };    
    this.hSubmit = this.hSubmit.bind(this); 
    this.m_refObj = {};
    this.m_toutMess = 0;
    this.m_isNewPipelineTask = this.props.selPipelineTask.id == 0;
    this.m_hasAdded = false;
  }

  shouldComponentUpdate(nextProps, nextState) {
    this.m_isNewPipelineTask = nextProps.selPipelineTask.id == 0;
    return true;
  }

  hSubmit(event) {
    
    if (this.m_hasAdded) return;
    
    const pplId = this.m_refObj["pipeline"].value;
    const ttId = this.m_refObj["taskTemplate"].value;
    let selTask = this.props.selPipelineTask;

    let error = "",
        name = this.m_refObj["name"].value;
    if (!name)
      error = "Name is empty"; 
    else if (this.m_isNewPipelineTask && PipelineTasks.getByName(pplId, name))
      error = `This name '${name}' already exists`;
    else if (!this.m_isNewPipelineTask && (selTask.name != name) && PipelineTasks.getByName(pplId, name))
      error = `This name '${name}' already exists`;
    else if (!pplId)
      error = `Not select Pipeline`;
    else if (!ttId)
      error = `Not select TaskTemplate`;
       
    if (error){
      this.setStatusMess(error);
      return;
    }
       
    if (!this.m_isNewPipelineTask && selTask.nextTasksId && selTask.nextTasksId.length){
      selTask.nextTasksId.forEach((id, ix)=>{
        selTask.isStartNext[ix] = this.m_refObj["isStartNext" + id].checked ? 1 : 0;
        selTask.isSendResultToNext[ix] = this.m_refObj["isSendResultToNext" + id].checked ? 1 : 0;
      });
    }

    let newPipelineTask = {
      id : !this.m_isNewPipelineTask ? selTask.id : 0,
      pplId,
      ttId,
      isEnabled : !this.m_isNewPipelineTask ? selTask.isEnabled : true,
      setts : !this.m_isNewPipelineTask ? selTask.setts : {
        isVisible : true,
        isSelected : false,
        positionX : 0,
        positionY : 0,
        width : 0,
        height : 0,
      },
      nextTasksId: !this.m_isNewPipelineTask ? selTask.nextTasksId : [],
      prevTasksId: !this.m_isNewPipelineTask ? selTask.prevTasksId : [],
      isStartNext: !this.m_isNewPipelineTask ? selTask.isStartNext : [],
      isSendResultToNext: !this.m_isNewPipelineTask ? selTask.isSendResultToNext : [],
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
           ()=>{this.props.onHide(); this.m_hasAdded = false;});
        },
        ()=>{this.setStatusMess("Server error create of Pipeline Task"); this.m_hasAdded = false;}
      )         
    }
    else{
      if (pplId != selTask.pplId){
        PipelineTasks.delAllConnections(selTask.id);
        newPipelineTask.nextTasksId = [];
        newPipelineTask.prevTasksId = [];
      }
      ServerAPI.changePipelineTask(newPipelineTask, 
        (respPipelineTask)=>{
          PipelineTasks.upd(respPipelineTask);           
          this.setStatusMess("Success change of Pipeline Task");

          let selPipelines = Pipelines.getSelected();
          if (selPipelines.length){
            Pipelines.setChangeTask(selPipelines[0].id, true);
          }           
        },
        () =>this.setStatusMess("Server error change of Pipeline Task")
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

    let nextTask = [];
    if (task.nextTasksId && task.nextTasksId.length){
      task.nextTasksId.forEach((id, ix)=>{
        const nt = PipelineTasks.get(id);
        nextTask.push(   
          <div className="d-flex flex-row mb-2 p-0 border borderRadius" key={id} style={{ maxWidth:"200px"}} >
            <p className="m-2 mr-auto p-0 ">{nt.name}</p>
            <input className="m-2 p-0 align-self-center" 
                    ref={(input) => {this.m_refObj["isStartNext" + id] = input }}
                    type="checkbox"
                    title="Start of next task"
                    defaultChecked={task.isStartNext[ix] == 1} />
            <input className="m-2 p-0 align-self-center" 
                    ref={(input) => {this.m_refObj["isSendResultToNext" + id] = input }} 
                    type="checkbox"                     
                    title="Send result to next task"
                    defaultChecked={task.isSendResultToNext[ix] == 1} />
          </div>
        )
      })
    }
   
   
    return (
      <Modal show={this.props.show} onHide={()=>this.props.onHide()} >
        <Modal.Header closeButton>
          <Modal.Title> {this.m_isNewPipelineTask ? "Create of Pipeline Task" : `${task.id}# Edit of Pipeline Task`}</Modal.Title>
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
                <Form.Control as="textarea" ref={(input) => {this.m_refObj["params"] = input }} placeholder="optional" defaultValue={task.params} rows={5} />
              </Form.Group>
            </Form.Row>
            {task.nextTasksId && task.nextTasksId.length ?
            <Form.Row >
              <Form.Group as={Col}>
                <Form.Label>Next tasks</Form.Label>   
                <div style={{ maxHeight:"150px", overflow:"auto"}}>
                {nextTask}
                </div>            
              </Form.Group>              
            </Form.Row>
            : ""}
            <Form.Row style={{height:"20px"}}> 
              <Form.Label style={{marginLeft:"5px"}}>{this.state.statusMess}</Form.Label>   
            </Form.Row>  
          </Form>          
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" type="close" onClick={()=> this.props.onHide()}>Close</Button>
          <Button variant="primary" type="submit" onClick={this.hSubmit}> {this.m_isNewPipelineTask ? "Create" : "Save changes"}</Button>
        </Modal.Footer>        
      </Modal>
    )
  } 
}
