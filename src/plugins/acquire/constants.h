
#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Acquire {

  namespace Constants {
    const char * const MODE_ACQUIRE = "Acquire.Mode";
    const int P_MODE_ACQUIRE        = 99;
    
    // common actions
    const char * const CONNECT              = "Acquire.Connect";
    const char * const RESET                = "Acquire.Reset";
    const char * const INITIALRUN           = "Acquire.InitialRun";
    const char * const RUN                  = "Acquire.Run";
    const char * const STOP                 = "Acquire.Stop";
    const char * const ACQUISITION          = "Acquire.Acquisition";

    // icon
    const char * const ICON_CONNECT         = ":/acquire/images/debugger_start.png";
    const char * const ICON_CONNECT_SMALL   = ":/acquire/images/debugger_continue_small.png";
    const char * const ICON_START           = ":/acquire/images/debugger_start.png";
    const char * const ICON_START_SMALL     = ":/acquire/images/debugger_start_small.png";
    const char * const ICON_RUN             = ":/acquire/images/debugger_start.png";
    const char * const ICON_RUN_SMALL       = ":/acquire/images/debugger_start_small.png";
    const char * const ICON_INTERRUPT       = ":/acquire/images/debugger_interrupt.png";
    const char * const ICON_INTERRUPT_SMALL = ":/acquire/images/debugger_interrupt_small.png";
    const char * const ICON_STOP            = ":/acquire/images/debugger_stop.png";
    const char * const ICON_STOP_SMALL      = ":/acquire/images/debugger_stop_small.png";
  }

}

#endif // CONSTANTS_H
