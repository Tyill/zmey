/* eslint-disable no-unused-vars */

import { UPDATE_FROM_SERVER,
         SET_DATA_PARAMS,
         SET_SIGNALS_FROM_SERVER,
         SIGNAL_BUFFER_ENABLE,
         CHANGE_CONFIG } from "./actions.jsx";
import { combineReducers } from 'redux'


function signals(curSignals = {}, action){

  switch (action.type) {
    case UPDATE_FROM_SERVER:{     
     
      let signalsCpy = Object.assign({}, curSignals);// no deep clone

      for (let k in action.newSignData){

        signalsCpy[k].buffVals.push(action.newSignData[k]);
      }      

      return signalsCpy;
    }

    case SET_SIGNALS_FROM_SERVER:     
      return action.signals;

    case SIGNAL_BUFFER_ENABLE:{

      let signalsCpy = Object.assign({}, curSignals),// no deep clone
          sname = action.sname;

      signalsCpy[sname].isBuffEna = action.set;
      
      if (!action.set)
        signalsCpy[sname].buffVals = [];

      return signalsCpy;
    }

     
    default:
      return curSignals;
 }
}

function dataParams(curParams, action){

  switch (action.type) {
    case SET_DATA_PARAMS:     
      return action.dataParams;
    
    default: 
      return curParams ? curParams : { 
          packetSize : 10,
          cycleTimeMs : 100,
        };
 }
}

function config(curConfig, action){

  switch (action.type) {
    case CHANGE_CONFIG:     
      return action.config;
    
    default: 
      return curConfig ? curConfig : { 
        backgroundColor : "white",
      };
 }
}

const ComboReducer = combineReducers({config, signals, dataParams});

export default ComboReducer;