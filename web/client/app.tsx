import React from "react";
import ReactDOM from "react-dom";
import { connect, Provider } from "react-redux";
import CentralWidget from "./central_widget/central_widget";
import {ServerAPI} from "./server_api"

import * as Action from "./redux/actions";
import Store from "./redux/store"; 
import { IPipeline, ITaskTemplate, ITaskPipeline } from "./types";

import "./css/app.css";

interface IPropsApp {
  onFillTaskTemplates : (taskTemplates : Array<ITaskTemplate>) => any; // | Actions
  onFillPipelines : (pipelines : Array<IPipeline>) => any;             // |
  onFillTaskPipeline : (tasks : Array<ITaskPipeline>) => any;          // |
};

interface IStateApp {
};

class App extends React.Component<IPropsApp, IStateApp>{
   
  constructor(props : IPropsApp){
    super(props);
    
    this.state  = { };   
  }
    
  componentDidMount() {    
    ServerAPI.getAllPipelines(this.props.onFillPipelines);
    ServerAPI.getAllTaskTemplates(this.props.onFillTaskTemplates);
    ServerAPI.getAllTaskPipeline(this.props.onFillTaskPipeline);
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
    onFillTaskPipeline : Action.fillTaskPipeline(dispatch),
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