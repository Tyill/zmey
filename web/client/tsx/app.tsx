import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import CentralWidget from "./gui/central_widget/central_widget";

import * as Action from "./redux/actions";
import Store from "./redux/store"; 
import { IUser, IPipeline, IGroup, ITaskTemplate, ITask } from "./types";

import "../css/app.css";
import "../css/fontello.css";
import "bootstrap/dist/css/bootstrap.min.css";

interface IPropsApp {
  user : IUser;                                // | Store
  pipelines : Map<number, IPipeline>;          // | 
  groups : Map<number, IGroup>;                // |
  taskTemplates : Map<number, ITaskTemplate>;  // |
  tasks : Map<number, ITask>;                  // |  

  onFillTaskTemplates : (taskTemplates : Array<ITaskTemplate>) => any; // | Actions
  onFillPipelines : (pipelines : Array<IPipeline>) => any;             // |
  onFillTaskGroups : (groups : Array<IGroup>) => any;                  // |
  onFillTasks : (tasks : Array<ITask>) => any;                         // |
};

interface IStateApp {
};

class App extends React.Component<IPropsApp, IStateApp>{
   
  constructor(props : IPropsApp){
    super(props);
    
    this.state  = { };   
  }
    
  componentDidMount() {    
    fetch('api/v1/taskTemplates')
    .then(response => response.json())    
    .then(taskTemplates =>{   
      this.props.onFillTaskTemplates(taskTemplates);           
    })
    .catch(() => console.log('api/v1/taskTemplates error')); 
    
    fetch('api/v1/pipelines')
    .then(response => response.json())    
    .then(pipelines =>{   
      this.props.onFillPipelines(pipelines);           
    })
    .catch(() => console.log('api/v1/pipelines error'));   
    
    fetch('api/v1/taskGroups')
    .then(response => response.json())    
    .then(groups =>{   
      this.props.onFillTaskGroups(groups);           
    })
    .catch(() => console.log('api/v1/taskGroups error'));  
    
    fetch('api/v1/tasks')
    .then(response => response.json())    
    .then(tasks =>{   
      this.props.onFillTasks(tasks);           
    })
    .catch(() => console.log('api/v1/tasks error'));  
  }
 
  render(){           
    return <CentralWidget/>
  } 
}

////////////////////////////////////////////////////

const mapStoreToProps = (store) => {
  return store;
}

const mapDispatchToProps = (dispatch) => {
  return { 
    onFillTaskTemplates : Action.fillTaskTemplates(dispatch), 
    onFillPipelines : Action.fillPipelines(dispatch),   
    onFillTasks : Action.fillTasks(dispatch), 
    onFillTaskGroups : Action.fillTaskGroups(dispatch),
  }
}

let AppRedux = connect(mapStoreToProps, mapDispatchToProps)(App);

const root = document.getElementById('root')

if (root){
  ReactDOM.render(
    <Provider store={Store}>
       <AppRedux/>
    </Provider>,
    root
  );
}