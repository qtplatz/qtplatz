//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "machinestatecontroller.h"

MachineStateController::MachineStateController() : state_(STATE_NONE)
												 , posted_state_(state_)
{
}


//static
const wchar_t *
MachineStateController::getStateString(MACHINE_STATE st)
{
  switch(st) {
  STATE_NONE:   return "STATE_NONE";
  OFF:          return "OFF";
  INITIALIZING: return "INITIALIZING";
	//INITIALIZED:  return "INITIALIZED";
  DIAGNOSTIC_IN_PROGRESS: return "DIAGNOSTIC_IN_PROGRESS";
  CALIBRATION_IN_PROGRESS: return "CALIBRATION_IN_PROGRESS";
  MAINTENANCE_IN_PROGRESS: return "MAINTENANCE_IN_PROGRESS";
  PREPARING_FOR_RUN:       return "PREPARING_FOR_RUN";
  READY_FOR_RUN:           return "READY_FOR_RUN";
  RUNNING:                 return "RUNNING";
  STOP:                    return "STOP";
	/*
  DORMANT_STOP:            
  DORMANT_INIT:            
    DORMANT_HOlD            
	BUSY                    // next method can not be accepted
	ACQUIRING               // data acquisition in progress
	READY_FOR_DOWNLOAD      // next method can be downloaded
	WARNING                 // instrument is in warning state
	ERROR                   // instrument is in error state, must be set STOP state too
	*/
  }
  return L"";
}
