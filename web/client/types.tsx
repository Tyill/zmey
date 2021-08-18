
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
  averDurationSec : number;    ///< estimated lead time 
  maxDurationSec : number;     ///< maximum lead time
  name : string;  
  description : string;
  script : string;  
};   

export
interface IPipelineTask{       
  id : number;
  pplId : number;               ///< pipeline id
  ttId : number;                ///< task template id    
  nextTasksId : Array<number>;  ///< pipeline task id of next tasks  
  nextEventsId : Array<number>; ///< event id of next events
  name : string;
  description : string;  
  params : Array<string>;
};

export
interface IEvent{       
  id : number;
  nextTasksId : Array<number>;  ///< pipeline task id of next tasks  
  nextEventsId : Array<number>; ///< event id of next events
  name : string;
  description : string;  
  params : Array<string>;
};

// export
// interface ITask{               ///< task object
//   id : number;
//   pplTaskId : number;          ///< pipeline task id
//   params : Array<string>;      ///< CLI params for script
// };

// export
// interface ITaskState{
//   id : number;                 ///< task id
//   state : StateType;  
//   progress : number;  
//   result : string;
// };

// export
// interface ITaskTime{
//   id : number;                 ///< task id
//   createTime : string;
//   takeInWorkTime : string;
//   startTime : string;
//   stopTime : string;
// };
