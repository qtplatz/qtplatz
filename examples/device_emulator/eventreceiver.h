// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include <QObject>
# pragma warning (disable: 4996)
#include <ace/Event_Handler.h>
#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>
# pragma warning (default: 4996)

class ACE_Time_Value;

namespace acewrapper {
  class McastHandler;
  class DgramHandler;
}

class QEventReceiver : public QObject {
      Q_OBJECT
   public:
      explicit QEventReceiver(QObject *parent = 0);
      
      int handle_input(acewrapper::McastHandler&, ACE_HANDLE );  // routed from multicast handler
      // int handle_input(acewrapper::DgramHandler&, ACE_HANDLE );  // routed from dgram handler
      int handle_input( ACE_HANDLE ); // native entry, may not be used
      int handle_timeout( const ACE_Time_Value&, const void * );
      int handle_close( ACE_HANDLE, ACE_Reactor_Mask );
      
  signals:
      // void signal_dgram_input( ACE_Message_Block * mb );
      void signal_mcast_input( ACE_Message_Block * mb );
      // void signal_timeout( unsigned long, long );

  public slots:

};

#endif // EVENTRECEIVER_H
