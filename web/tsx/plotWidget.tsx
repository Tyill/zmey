/* eslint-disable no-unused-vars */

import React from "react";
import { connect } from "react-redux";
import { Col, Button} from "react-bootstrap";
 
import * as Action from "./redux/actions"; 
import { IUser, IPipeline, IGroup, ITaskTemplate, ITask } from "./types";

import "bootstrap/dist/css/bootstrap.min.css";

interface IProps {
  user : IUser;                                // | Store
  pipelines : Map<number, IPipeline>;          // | 
  groups : Map<number, IGroup>;                // |
  taskTemplates : Map<number, ITaskTemplate>;  // |
  tasks : Map<number, ITask>;                  // |
  
  onAddTask : (task : ITask) => any;           // | Actions 
  onDelTask : (task : ITask) => any;           // |
};

interface IState {
}; 

class PlotWidget extends React.Component<IProps, IState>{
  
  constructor(props : IProps){
    super(props);
    
    this.state = { 
      statusMess : "" 
    }; 
  }
  
  render(){  
    
    return (
      
    )
  } 
}

////////////////////////////////////////////////////

const mapStoreToProps = (store) => {
  return store;
};

const mapDispatchToProps = (dispatch) => {
  return {
    onAddTask : Action.addTask(dispatch),
    onDelTask : Action.delTask(dispatch),
  }
};

let PlotWidgetRedux = connect(mapStoreToProps, mapDispatchToProps)(PlotWidget);

export default PlotWidgetRedux;