// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "eventreceiver.h"
#include <acewrapper/mcasthandler.h>
#include <acewrapper/dgramhandler.h>
# pragma warning(disable:4996)
# include <ace/Message_Block.h>
# pragma warning(default:4996)
#include <iostream>
#include <acewrapper/ace_string.h>
#include <acewrapper/outputcdr.h>
#include <adportable/protocollifecycle.h>
#include <acewrapper/lifecycle_frame_serializer.h>
#include <boost/exception/all.hpp>

QEventReceiver::QEventReceiver(QObject *parent) : QObject(parent)
{
}

int
QEventReceiver::handle_input( ACE_HANDLE )
{
    return 0;
}

int
QEventReceiver::handle_input( acewrapper::McastHandler& mcast, ACE_HANDLE )
{
    ACE_Message_Block * mb = new ACE_Message_Block( 2000 );
    ACE_Message_Block * pfrom = new ACE_Message_Block( 512 );
    ACE_INET_Addr * pFromAddr = new (pfrom->wr_ptr()) ACE_INET_Addr();

    int res = mcast.recv( mb->wr_ptr(), 2000, *pFromAddr );
    if (res == (-1)) {
		perror("handle_input mcast.recv");
        ACE_Message_Block::release( mb );
        ACE_Message_Block::release( pfrom );
        return 0;
    }
    mb->length( res );
    mb->cont( pfrom );

    emit signal_mcast_input( mb );
    return 0;
}

int
QEventReceiver::handle_timeout( const ACE_Time_Value& /* tv */, const void * )
{
   return 0;
}

int
QEventReceiver::handle_close( ACE_HANDLE, ACE_Reactor_Mask )
{
    return 0;
}
