
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
enum MessType{
  Error,
  Ok,
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
    isSelected : boolean;
    positionX : number;
    positionY : number;
    width : number;
    height : number;
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

export
interface ITask{
  id : number;
  pplTaskId : number;
  ttlId : number;
  state : StateType | number;
  progress : number;
  result : string;
  createTime : string;
  takeInWorkTime : string;
  startTime : string;
  stopTime : string;
};