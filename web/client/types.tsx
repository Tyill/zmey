
export
enum StateType{
  UNDEFINED = -1,
  READY,
  START,
  RUNNING,
  PAUSE,
  STOP,    
  COMPLETED,
  ERROR,
  CANCEL,
  NOT_RESPONDING,
};

export
interface IUser {
  id : number; 
  name: string;  
  description: string;
}

export
interface IPipeline{
  id : number;     
  name : string;
  description : string;
};

export
interface ITaskTemplate{
  id : number;
  name : string;            
  script : string;  
  averDurationSec : number;    ///< estimated lead time 
  maxDurationSec : number;     ///< maximum lead time
  description : string;  
};   

export
interface IPipelineTask{       
  id : number;
  pplId : number;              ///< pipeline id
  ttId : number;               ///< task template id    
  prevTasksId : Array<number>; ///< pipeline task id of previous tasks
  nextTasksId : Array<number>; ///< pipeline task id of next tasks  
  name : string;
  description : string;  
};

export
interface ITask{               ///< task object
  id : number;
  tpplId : number;             ///< task pipeline id
  priority : number;           ///< [1..3] 
  params : Array<string>;      ///< CLI params for script
};

export
interface ITaskState{
  id : number;                 ///< task id
  state : StateType;  
  progress : number;  
  result : string;
};

export
interface ITaskTime{
  id : number;                 ///< task id
  createTime : string;
  takeInWorkTime : string;
  startTime : string;
  stopTime : string;
};

export
interface IStoreType{
  user : IUser;
  pipelines : Map<number, IPipeline>;         ///< key - pipeline id
  taskTemplates : Map<number, ITaskTemplate>; ///< key - taskTemplate id
  pipelineTask : Map<number, IPipelineTask>;  ///< key - pipelineTask id
};