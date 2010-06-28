// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef MACHINESTATECONTROLLER_H
#define MACHINESTATECONTROLLER_H

#include "admachine_global.h"

class ADMACHINESHARED_EXPORT MachineStateController {
public:
  enum MACHINE_STATE {
	STATE_NONE              = 0x00000000,  // host software not initialized
	OFF                     = 0x00000001,  // hardware software not initialized
	INITIALIZING            = 0x00000002,  // hardware software loaded, initialize communication
	STOP                    = 0x00000003,  // hardware software initialize complete | stopped
	DIAGNOSTIC_IN_PROGRESS  = 0x00000004,  // power on initial diagnostic in progress.  User can't start initital conditioning
	MAINTENANCE_IN_PROGRESS = 0x00000005,  // maintenance in progress (incl. MCP conditioning), User can be stared at initial conditioning
	PREPARING_FOR_RUN       = 0x00000006,  // downloading method, GC oven temp goind down etc.
	READY_FOR_RUN           = 0x00000007,  // initial condition running (pump is on)
	RUNNING                 = 0x00000008,  // run in progress
    DORMANT_INIT            = 0x00000009,  // <reset> to be INITRUN (back to inital condition and keep runngin)
    DORMANT_HOlD            = 0x0000000a,  // <reset> to be HOLD (stay in current hardware condition and keep running)
    DORMANT_STOP            = 0x0000000b,  // <reset> to be STOP (instrument stopped:= pump off, detectors off, HTV on Mass spec is off)
	//--------- following states can be ored with above state --------
	BUSY                    = 0x00002000,  // next method can not be accepted
	ACQUIRING               = 0x00004000,  // data acquisition in progress
	READY_FOR_DOWNLOAD      = 0x00008000,  // next method can be downloaded
	WARNING                 = 0x40000000,  // instrument is in warning state
	ERROR                   = 0x80000000,  // instrument is in error state, must be set STOP state too
  };

  enum MACHINE_COMMAND {
	COMMAND_NONE,
	COMMAND_CONNECT,
	COMMAND_STOP,
	COMMAND_INITRUN,
	COMMAND_RUN,
	COMMAND_PAUSE,
	COMMAND_PAUSE_RERUN,
	COMMAND_HOLD,
	COMMAND_HOLD_RERUN,
	COMMAND_REMOINJ,
	COMMAND_MAX
  };

  enum SEQUENCE_STATE {
	SEQUENCE_STATE_RESET,
	SEQUENCE_STATE_RUNNING,
	SEQUENCE_STATE_SUSPEND,
	SEQUENCE_STATE_DORMANT,
  };

protected:
  MACHINE_STATE state_;
  MACHINE_STATE posted_state_;

  MachineStateController();
};

#endif // MACHINESTATECONTROLLER_H
