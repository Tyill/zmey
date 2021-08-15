import React from "react";
import { Container, Row, Col, Tabs, Image, ListGroup } from "react-bootstrap";
import { observer } from "mobx-react-lite"

import TaskTemplateDialogModal from "./task_template_dialog";
import PipelineDialogModal from "./pipeline_dialog";
import AckDeleteModal from "../common/ack_delete_modal";
import ListItem from "../common/list_item";
import ListHeader from "../common/list_header";
import TabItem from "../common/tab_item";
import GraphPanel from "../graph_panel/graph_panel";

import { IPipeline, IPipelineTask, ITaskTemplate } from "../types";
import { Pipelines, TaskTemplates, PipelineTasks} from "../store/store";
import { ServerAPI } from "../server_api/server_api";

import "../css/style.css";
import "../css/fontello.css";
import "bootstrap/dist/css/bootstrap.min.css";

enum State{
  Error,
  Ok,
}

interface IProps {
  setStatusMess : (mess : string, State)=>void;
};

interface IState {
  isShowTaskTemplateConfig : boolean;
  isShowPipelineConfig : boolean;
  isShowAckTaskTemplateDelete : boolean;
  isShowAckPipelineDelete : boolean;
  opensPipelines : Array<IPipeline>;
};

export default
class CentralWidget extends React.Component<IProps, IState>{
   
  private m_selTaskTemplate : ITaskTemplate = {} as ITaskTemplate;
  private m_selPipeline : IPipeline = {} as IPipeline;
  
  constructor(props : IProps){
    super(props);
    
    this.state  = { isShowTaskTemplateConfig : false,
                    isShowPipelineConfig : false,
                    isShowAckTaskTemplateDelete : false,
                    isShowAckPipelineDelete : false,
                    opensPipelines : []};   
  }    
 
  render(){
   
    let PipelineTabs = observer(() => {
      let pipelines = [];
      for (let v of this.state.opensPipelines){     
        pipelines.push(<TabItem key={v.id} id={v.id}
                                title={v.name}
                                hDelete={(id:number)=>{
                                  this.setState((prev, props)=>{
                                    let ppls = prev.opensPipelines.filter(v=>{
                                      return v.id != id;
                                    });
                                    return {opensPipelines : ppls};
                                  });
                                }}>
                        </TabItem>);
      }
      return <Tabs defaultActiveKey="profile" style={{ width: "100vw", height: "48px"}}
        onSelect={(key) => this.m_selPipeline = Pipelines.get(parseInt(key))}>
        {pipelines}
      </Tabs>   
    });
        
    let TaskTemplateList = observer(() => {
      let taskTemlates = [];
      for (let v of TaskTemplates.getAll().values()){
        taskTemlates.push(<ListItem key={v.id} id={v.id} title={v.name} tooltip={v.description}
                                    labelEdit={"Edit TaskTemplate"} labelDelete={"Delete TaskTemplate"}                                                   
                                    hEdit={()=>{
                                      this.m_selTaskTemplate = TaskTemplates.get(v.id);
                                      this.setState({isShowTaskTemplateConfig : true});
                                    }}
                                    hDelete={()=>{
                                      this.m_selTaskTemplate = TaskTemplates.get(v.id);
                                      this.setState({isShowAckTaskTemplateDelete : true});
                                    }}
                                    hDClickItem={(id : number)=>{
                                      
                                      
                                    }}>
                          </ListItem>);
      }
      return <ListGroup className="list-group-flush" style={{ borderBottom: "1px solid #dbdbdb", maxHeight: "35vh", overflowY:"auto"}} >
               {taskTemlates}
             </ListGroup>
    });

    let PipilineList = observer(() => {
      let pipelines = [];
      for (let v of Pipelines.getAll().values()){
        pipelines.push(<ListItem key={v.id} id={v.id} title={v.name} tooltip={v.description}
                                labelEdit={"Edit Pipeline"} labelDelete={"Delete Pipeline"}                                                   
                                hEdit={()=>{
                                  this.m_selPipeline = Pipelines.get(v.id);
                                  this.setState({isShowPipelineConfig : true});
                                }}
                                hDelete={()=>{
                                  this.m_selPipeline = Pipelines.get(v.id);
                                  this.setState({isShowAckPipelineDelete : true});
                                }}
                                hDClickItem={(id : number)=>{                                  
                                  this.setState((prev, props)=>{
                                    const ppls = [...prev.opensPipelines, Pipelines.get(id)];
                                    return {opensPipelines : ppls};
                                  });
                                }}>
                          </ListItem>);
      }
      return <ListGroup className="list-group-flush" style={{ borderBottom: "1px solid #dbdbdb", maxHeight: "35vh", overflowY:"auto"}} >
               {pipelines}
             </ListGroup>
    });
   
    return (
      <>
        <Container className="d-flex flex-column h-100 m-0 p-0" fluid >
          <Row noGutters={true} style={{borderBottom: "1px solid #dbdbdb"}}>
            <Col className="col menuHeader">               
              <Image src="../images/label.svg" style={{ margin: 5}} title="Application for schedule and monitor workflows"></Image>
            </Col>
          </Row>
          <Row noGutters={true} className="h-100" >
            <Col className="col-2 m-0 p-0" style={{ borderRight: "1px solid #dbdbdb"}}>   
              <ListHeader title={"TaskTemplates"}
                          labelNew={"New TaskTemplate"}
                          hNew={()=>{ this.m_selTaskTemplate.id = 0;
                                      this.setState({isShowTaskTemplateConfig : true}); }}/>               
              <TaskTemplateList/>

              <ListHeader title={"Pipelines"}
                          labelNew={"New Pipeline"}
                          hNew={()=>{ this.m_selPipeline.id = 0;
                                      this.setState({isShowPipelineConfig : true}); }}/>               
              <PipilineList/> 
            </Col>            
            <Col className="col-8 d-flex flex-column" style={{ borderRight: "1px solid #dbdbdb"}}>                            
              <Row noGutters={true} >
                <PipelineTabs />
              </Row>
              <Row noGutters={true} className="h-100" style={{ position:"relative", overflow:"auto"}}>
                <GraphPanel pplId={0}/>
              </Row>
            </Col>
          </Row>
        </Container> 

        <TaskTemplateDialogModal selTaskTemplate={this.m_selTaskTemplate} 
                                 show={this.state.isShowTaskTemplateConfig} 
                                 onHide={(selTaskTemplate : ITaskTemplate)=>{
                                   this.m_selTaskTemplate = selTaskTemplate;
                                   this.setState({isShowTaskTemplateConfig : false});
                                 }}/>
        
        <PipelineDialogModal selPipeline={this.m_selPipeline} 
                             show={this.state.isShowPipelineConfig} 
                             onHide={(selPipeline : IPipeline)=>{
                               this.m_selPipeline = selPipeline;
                               this.setState({isShowPipelineConfig : false});
                             }}/>

        <AckDeleteModal description={this.m_selTaskTemplate.description}
                        title={`Delete '${this.m_selTaskTemplate.name}' TaskTemplate?`}
                        show={this.state.isShowAckTaskTemplateDelete}
                        onYes={() =>
                          ServerAPI.delTaskTemplate(this.m_selTaskTemplate,
                            ()=>{
                              TaskTemplates.del(this.m_selTaskTemplate.id); 
                              this.props.setStatusMess(`TaskTemplate '${this.m_selTaskTemplate.name}' is delete`, State.Ok);
                              this.setState({isShowAckTaskTemplateDelete : false});
                            },
                            ()=>this.props.setStatusMess("Server error delete of TaskTemplate", State.Error))
                        } 
                        onHide={()=>this.setState({isShowAckTaskTemplateDelete : false})}/>

        <AckDeleteModal description={this.m_selPipeline.description}
                        title={`Delete '${this.m_selPipeline.name}' Pipeline?`}
                        show={this.state.isShowAckPipelineDelete}
                        onYes={() =>
                          ServerAPI.delPipeline(this.m_selPipeline,
                            ()=>{  
                              Pipelines.del(this.m_selPipeline.id); 
                              this.props.setStatusMess(`Pipeline '${this.m_selPipeline.name}' is delete`, State.Ok);
                              this.setState({isShowAckPipelineDelete : false});
                            },
                            ()=>this.props.setStatusMess("Server error delete of Pipeline", State.Error))
                        } 
                        onHide={()=>this.setState({isShowAckPipelineDelete : false})}/>
      </>
    )
  } 
}
