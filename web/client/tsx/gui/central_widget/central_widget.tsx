import React from "react";
import { connect } from "react-redux";
import {Container, Row, Col, Tabs, Tab, Image, Card, Modal, Button, OverlayTrigger, ListGroup } from "react-bootstrap";
import TaskTemplateDialogModal from "../task_template_dialog/task_template_dialog";
import PipelineDialogModal from "../pipeline_dialog/pipeline_dialog";
import TaskItem from "./task_item";
import TaskHeader from "./task_header";
import PipelineHeader from "./pipeline_header";
import AckDeleteModal from "../common/ack_delete_modal";

import * as Action from "../../redux/actions";
import { IUser, IPipeline, IGroup, ITaskTemplate, ITask } from "../../types";

import "../../../css/app.css";
import "../../../css/fontello.css";
import "bootstrap/dist/css/bootstrap.min.css";

interface IPropsApp {
  user : IUser;                                               // | Store
  pipelines : Map<number, IPipeline>;                         // | 
  groups : Map<number, IGroup>;                               // |
  taskTemplates : Map<number, ITaskTemplate>;                 // |
  tasks : Map<number, ITask>;                                 // |  

  onDelTaskTemplate : (taskTemplate : ITaskTemplate) => any;  // | Actions
  onDelPipeline : (pipeline : IPipeline) => any;              // |
};

interface IStateApp {
  isShowTaskTemplateConfig : boolean;
  isShowPipelineConfig : boolean;
  isShowAckTaskTemplateDelete : boolean;
  isShowAckPipelineDelete : boolean;
};

class CentralWidgetClass extends React.Component<IPropsApp, IStateApp>{
   
  private m_selTaskTemplate : ITaskTemplate = {} as ITaskTemplate;
  private m_selPipeline : IPipeline = {} as IPipeline;

  constructor(props : IPropsApp){
    super(props);
    
    this.state  = { isShowTaskTemplateConfig : false,
                    isShowPipelineConfig : false,
                    isShowAckTaskTemplateDelete : false,
                    isShowAckPipelineDelete : false };   
  }    
 
  render(){
    
    let pipelines = []
    for (let v of this.props.pipelines.values()){     
      pipelines.push(<Tab key={v.id} eventKey={v.id.toString()} title={v.name}></Tab>);
    }
    let taskTemlates = []
    for (let v of this.props.taskTemplates.values()){
      taskTemlates.push(<TaskItem key={v.id} title={v.name} tooltip={v.description}
                                  hEdit={()=>{
                                    this.m_selTaskTemplate = this.props.taskTemplates.get(v.id);
                                    this.setState((oldState, props)=>{
                                      let isShowTaskTemplateConfig = true;
                                      return {isShowTaskTemplateConfig};
                                    });
                                  }}
                                  hDelete={()=>{
                                    this.m_selTaskTemplate = this.props.taskTemplates.get(v.id);
                                    this.setState((oldState, props)=>{
                                      let isShowAckTaskTemplateDelete = true;
                                      return {isShowAckTaskTemplateDelete};
                                    });
                                  }}>
                        </TaskItem>);
    }
       
    return (
      <div>
        <Container fluid style={{ margin: 0, padding: 0}}>
          <Row noGutters={true} style={{borderBottom: "1px solid #dbdbdb"}}>
            <Col className="col menuHeader">               
              <Image src="client/images/label.png" style={{ margin: 5}} title="Application for schedule and monitor workflows"></Image>
            </Col>
          </Row>
          <Row noGutters={true} >
            <Col className="col-2" style={{borderRight: "1px solid #dbdbdb"}}>   
              <TaskHeader hNew={()=>{ 
                            this.m_selTaskTemplate.id = 0;
                            this.setState((oldState, props)=>{
                              let isShowTaskTemplateConfig = true;
                              return {isShowTaskTemplateConfig};
                            });
              }}/>                
              <ListGroup className="list-group-flush" style={{minHeight: "70vh", maxHeight: "70vh", overflowY:"auto"}} >
                {taskTemlates}
              </ListGroup>
            </Col>
            <Col className="col" style={{borderRight: "1px solid #dbdbdb"}}> 
              <PipelineHeader hNew={()=>{ 
                                this.m_selPipeline.id = 0;
                                this.setState((oldState, props)=>{
                                  let isShowPipelineConfig = true;
                                  return {isShowPipelineConfig};
                                });}}
                              hEdit={()=>{
                                // this.m_selPipeline = this.props.pipelines.get(v.id);
                                this.setState((oldState, props)=>{
                                  let isShowPipelineConfig = true;
                                  return {isShowPipelineConfig};
                                }); }}
                              hDelete={()=>{
                                // this.m_selTaskTemplate = this.props.taskTemplates.get(v.id);
                                this.setState((oldState, props)=>{
                                  let isShowAckPipelineDelete = true;
                                  return {isShowAckPipelineDelete};
                                }); }} />
              <Tabs defaultActiveKey="profile" id="uncontrolled-tab-example" style={{height: "48px"}}
                    onSelect={(key) => this.m_selPipeline = this.props.pipelines.get(parseInt(key))}>
                {pipelines}
              </Tabs>                                       
            </Col>
            <Col className="col-2" > 
            </Col>
          </Row>
          <Row noGutters={true} >
            <Col className="col" >   
              <Card style={{height: "50px"}}> 
              </Card>
            </Col>
          </Row>
        </Container> 

        <TaskTemplateDialogModal selTaskTemplate={this.m_selTaskTemplate} 
                                 show={this.state.isShowTaskTemplateConfig} 
                                 onHide={(selTaskTemplate : ITaskTemplate)=>{
                                   this.m_selTaskTemplate = selTaskTemplate;
                                   this.setState((oldState, props)=>{
                                     let isShowTaskTemplateConfig = false;
                                     return {isShowTaskTemplateConfig};
                                   });
                                 }}/>
        
        <PipelineDialogModal selPipeline={this.m_selPipeline} 
                             show={this.state.isShowPipelineConfig} 
                             onHide={(selPipeline : IPipeline)=>{
                               this.m_selPipeline = selPipeline;
                               this.setState((oldState, props)=>{
                                 let isShowPipelineConfig = false;
                                 return {isShowPipelineConfig};
                               });
                             }}/>

        <AckDeleteModal name={this.m_selTaskTemplate.name}
                        title="Delete the Task Template?"
                        show={this.state.isShowAckTaskTemplateDelete}
                        onYes={()=>{
                                fetch('api/delTaskTemplate?id=' + this.m_selTaskTemplate.id)
                                .then(() => {
                                  let taskTemplate = this.m_selTaskTemplate;
                                  this.props.onDelTaskTemplate(taskTemplate); 
                                  this.setState((oldState, props)=>{
                                    let isShowAckTaskTemplateDelete = false;
                                    return {isShowAckTaskTemplateDelete};
                                  }); 
                                }) 
                                .catch(() => console.log('api/delTaskTemplate error')); }}
                        onHide={()=>{
                          this.setState((oldState, props)=>{
                            let isShowAckTaskTemplateDelete = false;
                            return {isShowAckTaskTemplateDelete};
                          });
                        }}/>

        <AckDeleteModal name={this.m_selPipeline.name}
                        title="Delete the Task Pipeline?"
                        show={this.state.isShowAckPipelineDelete}
                        onYes={()=>{
                                fetch('api/delPipeline?id=' + this.m_selPipeline.id)
                                .then(() => {
                                  let pipeline = this.m_selPipeline;
                                  this.props.onDelPipeline(pipeline); 
                                  this.setState((oldState, props)=>{
                                    let isShowAckPipelineDelete = false;
                                    return {isShowAckPipelineDelete};
                                  }); 
                                }) 
                                .catch(() => console.log('api/delPipeline error')); }}
                        onHide={()=>{
                          this.setState((oldState, props)=>{
                            let isShowAckPipelineDelete = false;
                            return {isShowAckPipelineDelete};
                          });
                        }}/>
      </div>
    )
  } 
}

const mapStoreToProps = (store) => {
  return store;
}

const mapDispatchToProps = (dispatch) => {
  return {
    onDelTaskTemplate : Action.delTaskTemplate(dispatch),
    onDelPipeline : Action.delPipeline(dispatch), 
  }
}

let CentralWidget = connect(mapStoreToProps, mapDispatchToProps)(CentralWidgetClass);

export default CentralWidget;