
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
  NOT_RESPONDING,
};

export
interface IUser {
  name: string;  
  description?: string;
}

export
interface IPipeline{
  id : number;     
  name : string;
  refTasks : Set<number>;      ///< task id
  refGroups : Set<number>;     ///< group id
  description?: string;
};

export
interface ITaskGroup{
  id : number;
  pplId : number;              ///< pipeline id
  name : string;            
  refTasks : Set<number>;      ///< task id
  description?: string;     
};

export
interface ITaskTemplate{
  id : number;
  averDurationSec : number;    ///< estimated lead time 
  maxDurationSec : number;     ///< maximum lead time
  name : string;            
  script : string;
  refTasks : Set<number>;      ///< task id
  description? : string;  
};   

export
interface ITask{
  pplId : number;              ///< pipeline id
  groupId? : number;           ///< group id
  templId : number;            ///< task template id
  priority : number;           ///< [1..3]
  state : EnumState;
  prevTasksId : Array<number>; ///< pipeline task id of previous tasks
  nextTasksId : Array<number>; ///< pipeline task id of next tasks
  params : Array<number>;      ///< CLI params for script
};

export
interface IStoreType{
  user : IUser;
  pipelines : Map<number, IPipeline>;         ///< key - pipeline id
  taskGroups : Map<number, ITaskGroup>;       ///< key - group id
  taskTemplates : Map<number, ITaskTemplate>; ///< key - taskTemplate id
  tasks : Map<number, ITask>;                 ///< key - task id
};