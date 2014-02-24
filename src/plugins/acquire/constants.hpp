// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace acquire {

  namespace constants {
    const char * const MODE_ACQUIRE = "Acquire.Mode";
    const int P_MODE_ACQUIRE        = 99;
    
    // common actions
    const char * const CONNECT              = "Acquire.Connect";
    const char * const RESET                = "Acquire.Reset";
    const char * const INITIALRUN           = "Acquire.InitialRun";
    const char * const RUN                  = "Acquire.Run";
    const char * const STOP                 = "Acquire.Stop";
    const char * const ACQUISITION          = "Acquire.Acquisition";
    const char * const SNAPSHOT             = "Acquire.Snapshot";

    // icon
    const char * const ICON_CONNECT         = ":/acquire/images/debugger_continue.png";
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
