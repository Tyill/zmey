import React from "react";
import { Table } from "react-bootstrap";
import { observer} from "mobx-react-lite"

import { PipelineTasks, Tasks, Events} from "../store/store";
import { dateFormat } from "../common/common";
import { ITask, IPoint, stateToString, StateType } from "../types"
import TaskStatusContextMenu from "./task_status_context_menu"

interface IProps { 
};
interface IState { 
  selTaskId : number;
  mpos : IPoint;
};

export default
class TaskStatusWidget extends React.Component<IProps, IState>{  
  
  private m_refObj : object;
  private m_tasks : Array<ITask> = [];
  private m_tasksWidget  = [];
  private m_hasNew = false;
  private m_hasHideContextMenu = false;
  private m_hasShowContextMenu = false;
  private m_hasFilterCheckbox : boolean;
  private m_table : HTMLTableElement;

  constructor(props : IProps){
    super(props);  
  
    this.hShowContextMenu = this.hShowContextMenu.bind(this); 
    this.hHideContextMenu = this.hHideContextMenu.bind(this);
    this.m_refObj = {};

    this.state = {selTaskId : 0, mpos : {x:0,y:0}};   
  }

  shouldComponentUpdate(){
    return this.m_hasNew || this.m_hasHideContextMenu || this.m_hasShowContextMenu || this.m_hasFilterCheckbox;
  }

  hShowContextMenu(e){  
    if (e.button == 2){  
      let brect = this.m_table.getBoundingClientRect();  
     
      const mpos = { x : (e.clientX  - brect.left),
                    y : (e.clientY  - brect.top)}; 
      
      let selTaskId = parseInt(e.currentTarget.cells[0].textContent, 10);
      this.m_hasShowContextMenu = true;          
      this.setState({mpos, selTaskId});      
      e.stopPropagation();
    }
  }

  hHideContextMenu(){
    if (this.state.selTaskId != 0){
      this.m_hasHideContextMenu = true;
      this.setState({selTaskId : 0});
    }
  }

  render(){  

    const headStyle = {fontWeight: 500, padding: "1px", paddingLeft: "5px", paddingRight: "5px",
                       borderWidth: "1px", borderTop: "none"};
    const bodyStyle = {padding: "1px", paddingLeft: "5px", paddingRight: "5px"};

    const TasksRet = observer(() => {
      
      const newTasks = Tasks.getAll();
      this.m_hasNew = newTasks.length != this.m_tasks.length;
      if (!this.m_hasNew){
        for (let i = 0; i < newTasks.length; ++i){
          if (!Tasks.isEqual(newTasks[i], this.m_tasks[i])){
            this.m_hasNew = true; 
            break;         
          }
        }
      }

      if (this.m_hasNew || this.m_hasFilterCheckbox){
        this.m_tasks = newTasks;
        
        this.m_tasksWidget = [];
        const onlyRunning = this.m_refObj["onlyRunningTasks"].checked;
        const onlyCompleted = this.m_refObj["onlyCompletedTasks"].checked;
        const onlyError = this.m_refObj["onlyErrorTasks"].checked;
        for (let t of Tasks.getAll()){  

          if (onlyRunning && (t.state != StateType.RUNNING))
            continue;
          else if (onlyCompleted && (t.state != StateType.COMPLETED))
            continue;
          else if (onlyError && (t.state != StateType.ERROR))
            continue;
                    
          let starterPplTask = PipelineTasks.get(t.starterPplTaskId);
          let starterName = starterPplTask ? starterPplTask.name : Events.get(t.starterEventId).name;
        
          let offsTime = new Date().getTimezoneOffset(),
              startTime = new Date(t.startTime),
              stopTime = new Date(t.stopTime);
          startTime.setMinutes(startTime.getMinutes() - offsTime);
          stopTime.setMinutes(stopTime.getMinutes() - offsTime);
          
          this.m_tasksWidget.push(<tr key={t.id} onContextMenu={(e)=>e.preventDefault()}
                                                 onMouseDown={this.hShowContextMenu}>
            <td style={bodyStyle}>{t.id}</td>
            <td style={bodyStyle}>{starterName}</td>
            <td style={bodyStyle}>{stateToString(t.state)}</td>
            <td style={bodyStyle}>{t.progress}</td>
            <td style={bodyStyle}>{t.startTime ? dateFormat(startTime, "yyyy-mm-dd hh:ii:ss.ms") : ""}</td>
            <td style={bodyStyle}>{t.stopTime ? dateFormat(stopTime, "yyyy-mm-dd hh:ii:ss.ms") : ""}</td>
            <td style={bodyStyle}>{t.result}</td>
          </tr>)
        }      
      }        
      return <tbody>
              {this.m_tasksWidget}
             </tbody> 
    });

    if (this.state.selTaskId == 0)
      this.m_hasHideContextMenu = false;
    else
      this.m_hasShowContextMenu = false;

    this.m_hasFilterCheckbox = false;

    const TaskName = observer(() => {
      const selPplTask = PipelineTasks.get(Tasks.getPplTaskId());    
      return <a style={{color:"gray"}}>{ selPplTask ? selPplTask.name : "" }</a>        
    });

    return  <div className="d-flex flex-row p-0 m-0" onClick={this.hHideContextMenu} >
              <div className="p-0 m-1" style={{minWidth: "200px"}}>
                <TaskName/>     
                <div className="d-flex flex-column align-items-end p-0 mr-2" >
                  <p className="m-0 p-0" >running
                    <input className="ml-2 p-0" 
                      ref={(input) => {this.m_refObj["onlyRunningTasks"] = input }}
                      type="checkbox"
                      onChange={()=>this.m_hasFilterCheckbox = true}  
                      title="Show only running tasks"/>
                  </p>
                  <p className="m-0 p-0" >completed
                    <input className="ml-2 p-0" 
                      ref={(input) => {this.m_refObj["onlyCompletedTasks"] = input }}
                      type="checkbox"    
                      onChange={()=>this.m_hasFilterCheckbox = true}                 
                      title="Show only completed tasks"/>  
                  </p>   
                  <p className="m-0 p-0" >error
                    <input className="ml-2 p-0" 
                      ref={(input) => {this.m_refObj["onlyErrorTasks"] = input }}
                      type="checkbox"        
                      onChange={()=>this.m_hasFilterCheckbox = true}               
                      title="Show only error tasks"/>  
                  </p>     
                </div>                    
              </div>
              <div className="p-0 m-0" 
                   style={{ overflow: "auto", whiteSpace: "nowrap", position:"relative", maxHeight: "20vh"}}>
                <Table striped bordered hover style={{width: "100vw", border: "none"}} ref={el => this.m_table = el}>
                  <thead >
                    <tr >
                      <th style={{...headStyle, width:"0px"}}>Id</th>
                      <th style={{...headStyle, width:"0px"}}>Start from</th>
                      <th style={{...headStyle, width:"0px"}}>State</th>
                      <th style={{...headStyle, width:"0px"}}>Progress</th>
                      <th style={{...headStyle, width:"0px"}}>Begin time</th>
                      <th style={{...headStyle, width:"0px"}}>End time</th>
                      <th style={{...headStyle}}>Result</th>                 
                    </tr>
                  </thead>
                  <TasksRet/>
                </Table>
                <TaskStatusContextMenu id = {this.state.selTaskId}
                                       mpos={this.state.mpos}
                                       hHide={this.hHideContextMenu} />
              </div>              
            </div>
  }
}
