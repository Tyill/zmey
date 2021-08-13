import React from "react";
import ReactDOM from "react-dom";
import CentralWidget from "./central_widget/central_widget";
import {ServerAPI} from "./server_api"

import "./css/style.css";
import { IPipeline, IPipelineTask, ITaskTemplate } from "./types";
import { Pipelines, TaskTemplates, PipelineTasks} from "./store/store";

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
    let ppl = new Map<Number, IPipeline>();
    ServerAPI.getAllPipelines((pipelines : Array<IPipeline>)=>{      
      for (let p of pipelines){
        ppl.set(p.id, p);
      }      
    });
    let ttl = new Map<Number, ITaskTemplate>();
    ServerAPI.getAllTaskTemplates((taskTemplates : Array<ITaskTemplate>)=>{
      for (let t of taskTemplates){
        ttl.set(t.id, t);
      }
      
    });
    setTimeout(()=>{
      console.log(ttl);
      Pipelines.setAll(ppl);
      TaskTemplates.setAll(ttl);
    }, 3000);
    //ServerAPI.getAllPipelineTasks((pipelineTasks : Array<IPipelineTask>)=>{
      // let ppt = {} as Map<Number, IPipelineTask>;
      // for (let pt of pipelineTasks){
      //   ppt.set(pt.id, pt);
      // }
      // PipelineTasks.setAll(ppt);
    //});
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