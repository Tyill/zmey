
export
enum EnumState{
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
interface ITaskGroup{
  id : number;
  pplId : number;              ///< pipeline id
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
interface ITask{
  id : number;
  pplId : number;              ///< pipeline id
  gId : number;                ///< group id
  ttId : number;               ///< task template id
  priority : number;           ///< [1..3]
  state : EnumState;
  progress : number;
  result : string;
  prevTasksId : Array<number>; ///< pipeline task id of previous tasks
  nextTasksId : Array<number>; ///< pipeline task id of next tasks
  params : Array<string>;      ///< CLI params for script
  createTime : string;
  takeInWorkTime : string;
  startTime : string;
  stopTime : string;
};

export
interface IStoreType{
  user : IUser;
  pipelines : Map<number, IPipeline>;         ///< key - pipeline id
  taskGroups : Map<number, ITaskGroup>;       ///< key - group id
  taskTemplates : Map<number, ITaskTemplate>; ///< key - taskTemplate id
  tasks : Map<number, ITask>;                 ///< key - task id
};