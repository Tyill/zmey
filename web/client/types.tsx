
export
enum StateType{
  UNDEFINED = 0,
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
function stateToString(state : StateType){
  switch (state){
    case StateType.UNDEFINED: return 'UNDEFINED';
    case StateType.READY: return 'READY';
    case StateType.START: return 'START';
    case StateType.RUNNING: return 'RUNNING';
    case StateType.PAUSE: return 'PAUSE';
    case StateType.STOP: return 'STOP';   
    case StateType.COMPLETED: return 'COMPLETED';
    case StateType.ERROR: return 'ERROR';
    case StateType.CANCEL: return 'CANCEL';
    case StateType.NOT_RESPONDING: return 'NOT_RESPONDING';
  }
}

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
    hasChangeTask : boolean;
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
  pplId : number;                     ///< pipeline id
  ttId : number;                      ///< task template id    
  isEnabled : boolean;      
  nextTasksId : Array<number>;        ///< pipeline task id of next tasks  
  prevTasksId : Array<number>;        ///< pipeline task id of prev tasks 
  isStartNext : Array<number>;        ///< is start next task?
  isSendResultToNext : Array<number>; ///< is send result to next task?
  conditionStartNext : Array<string>;
  params : string;
  name : string;
  description : string;
  setts : {
    isVisible : boolean;
    isSelected : boolean;
    isMoved : boolean;
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
  timeStartOnceOfDay : Array<string>;
  timeStartEverySec : number;
  tasksForStart : Array<number>;
  name : string;
  description : string;  
};

export
interface ITask{
  id : number;
  pplTaskId : number;
  starterPplTaskId : number;
  starterEventId : number;
  ttlId : number;
  state : StateType | number;
  progress : number;
  params : string;
  result : string;
  createTime : string;
  takeInWorkTime : string;
  startTime : string;
  stopTime : string;
};