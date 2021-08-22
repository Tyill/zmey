
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
  setts : {
    isVisible : boolean;
    isSelected : boolean;
  };
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
interface IPoint{
  x: number;
  y: number;
}

export
interface IRect{
  x: number;
  y: number;
  w: number;
  h: number;
}

export
interface IPipelineTask{       
  id : number;
  pplId : number;               ///< pipeline id
  ttId : number;                ///< task template id    
  isEnabled : boolean;
  nextTasksId : Array<number>;  ///< pipeline task id of next tasks  
  prevTasksId : Array<number>;  ///< pipeline task id of prev tasks  
  nextEventsId : Array<number>; ///< event id of next events
  prevEventsId : Array<number>; ///< event id of prev events
  params : string;
  name : string;
  description : string;
  setts : {
    isVisible : boolean;
    positionX : number;
    positionY : number;
    socketInRect : IRect;
    socketInPoint : IPoint;
    socketOutRect : IRect;
    socketOutPoint : IPoint;
  };
};

export
interface IEvent{       
  id : number;
  isEnabled : boolean;
  nextTasksId : Array<number>;  ///< pipeline task id of next tasks  
  prevTasksId : Array<number>;  ///< pipeline task id of prev tasks  
  nextEventsId : Array<number>; ///< event id of next events
  prevEventsId : Array<number>; ///< event id of prev events
  params : string;
  name : string;
  description : string;
  setts : {
    isVisible : boolean;
    positionX : number;
    positionY : number;
    socketInputPositionX : number;
    socketInputPositionY : number;
    socketOutputPositionX : number;
    socketOutputPositionY : number;
  };
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
