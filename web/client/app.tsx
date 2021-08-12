import React from "react";
import ReactDOM from "react-dom";
import CentralWidget from "./central_widget/central_widget";
import {ServerAPI} from "./server_api"

import "./css/style.css";

interface IPropsApp {

};

interface IStateApp {
};

class App extends React.Component<IPropsApp, IStateApp>{
   
  constructor(props : IPropsApp){
    super(props);
    
    this.state  = { };   
  }
    
  componentDidMount() {    
    // ServerAPI.getAllPipelines(this.props.onFillPipelines);
    // ServerAPI.getAllTaskTemplates(this.props.onFillTaskTemplates);
    // ServerAPI.getAllTaskPipeline(this.props.onFillTaskPipeline);
  }
 
  render(){           
    return <CentralWidget/>
  } 
}

////////////////////////////////////////////////////

const root = document.getElementById('root')
if (root){
  ReactDOM.render(
    <App/>,
    root
  );
}