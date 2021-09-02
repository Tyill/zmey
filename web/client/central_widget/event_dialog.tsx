import React from "react";
import { Col, Button, Modal, Form, ListGroup} from "react-bootstrap";
 
import { IEvent, IPplTaskId } from "../types";
import { Events, Pipelines, PipelineTasks } from "../store/store";
import * as ServerAPI from "../server_api/server_api"

import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  show : boolean;
  onHide : () => any;
  selEvent : IEvent;
};

interface IState {
  tasksForStart : Array<IPplTaskId>,
  selPplId : number,
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
      tasksForStart : props.selEvent.tasksForStart || [],
      selPplId : 0,
      statusMess : "" 
    };    
    this.hSubmit = this.hSubmit.bind(this); 
    this.appendTaskToList = this.appendTaskToList.bind(this);
    this.setStatusMess = this.setStatusMess.bind(this);
    this.hShow = this.hShow.bind(this); 

    this.m_refObj = {};
    this.m_toutMess = 0;
    this.m_isNewEvent = this.props.selEvent.id == 0;
    this.m_hasAdded = false;
  }

  shouldComponentUpdate(nextProps, nextState) {
    this.m_isNewEvent = nextProps.selEvent.id == 0;
    return true;
  }

  hShow(){
    if (this.props.selEvent.tasksForStart)
      this.setState({tasksForStart : this.props.selEvent.tasksForStart});
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
      id : !this.m_isNewEvent ? this.props.selEvent.id : 0,
      isEnabled : !this.m_isNewEvent ? this.props.selEvent.isEnabled : true,     
      tasksForStart: this.state.tasksForStart,
      timeStartEverySec: this.m_refObj["startTimeEvery"].value ? parseInt(this.m_refObj["startTimeEvery"].value, 10) : 0,
      timeStartOnceOfDay: this.m_refObj["startTimeOnce"].value ? this.m_refObj["startTimeOnce"].value : "",
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

  appendTaskToList(){
    
    const pplId = parseInt(this.m_refObj["pipeline"].value, 10);
    const taskId = parseInt(this.m_refObj["pipelineTask"].value, 10);

    if (pplId && taskId){
      this.setState((prev, props)=>{
        let tasksForStart = [...prev.tasksForStart];
        tasksForStart.push({pplId, taskId});
        return {tasksForStart};
      })
    }
  }

  render(){  

    const evt = this.props.selEvent;
    
    let pipelines = [];
    Pipelines.getAll().forEach(p=>{
      pipelines.push(<option id={p.id.toString()} key={p.id.toString()} value={p.id}>
                      {p.name}
                     </option>);
    });

    let selPplId = this.state.selPplId   
    if ((selPplId == 0) && pipelines.length){
      selPplId = Pipelines.getAll().keys().next().value;
    }    
      
    let pplTasks = [];
    PipelineTasks.getByPPlId(selPplId).forEach(t=>{
      pplTasks.push(<option id={t.id.toString()} key={t.id.toString()} value={t.id}>
                      {t.name}
                    </option>);
    });

    let timeStartOnceOfDay = "";
    if (evt.timeStartOnceOfDay){
      evt.timeStartOnceOfDay.forEach(v=>{
        timeStartOnceOfDay += v + ";";
      });
    }

    let tasksForStart = [];
    if (this.state.tasksForStart){
      this.state.tasksForStart.forEach(v=>{
        if (Pipelines.get(v.pplId)){
          tasksForStart.push(<div className="border borderRadius" id={ v.pplId.toString() + v.taskId.toString()} 
                                            key={v.pplId.toString() + v.taskId.toString()}
                                            style={{ margin :"0px", padding:"5px"}}>
                              {Pipelines.get(v.pplId).name + " : " + PipelineTasks.get(v.taskId).name }
                            </div>
        )}
      });
    }
           
    return (
      <Modal show={this.props.show} onShow={this.hShow} onHide={()=>this.props.onHide()} >
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
              <Form.Group as={Col} style={{maxWidth:"200px"}} controlId="startTimeEvery"> 
                <Form.Label>Start time every seconds</Form.Label>
                <Form.Control type="number" min="0" ref={(input) => {this.m_refObj["startTimeEvery"] = input }} placeholder="0" defaultValue={evt.timeStartEverySec} />
              </Form.Group>
              <Form.Group as={Col} controlId="startTimeOnce">
                <Form.Label>Start time once a day</Form.Label>
                <Form.Control type="text" ref={(input) => {this.m_refObj["startTimeOnce"] = input }} placeholder="12:15:00; 14:25:35;..." defaultValue={timeStartOnceOfDay}/>
              </Form.Group>                
            </Form.Row>
            <Form.Row>
              <Form.Group as={Col} style={{maxWidth:"200px"}}>
                <Form.Label>Pipeline</Form.Label>
                <Form.Control as="select" custom  ref={(input) => {this.m_refObj["pipeline"] = input }}
                              onChange={(e)=>this.setState({selPplId : parseInt(e.currentTarget.value, 10)})}>
                  {pipelines}
                </Form.Control>
                <p/>
                <Form.Label>Pipeline Task</Form.Label>
                <Form.Control as="select" custom  ref={(input) => {this.m_refObj["pipelineTask"] = input }}>
                  {pplTasks}
                </Form.Control>
                <p/>
                <Button variant="success" onClick={this.appendTaskToList}> {"Append Task"}</Button>
              </Form.Group>
              <Form.Group as={Col} controlId="params">                
                <Form.Label>List of Tasks for start</Form.Label>
                <ListGroup>
                  {tasksForStart}
                </ListGroup>
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
