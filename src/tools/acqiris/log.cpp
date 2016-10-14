/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "log.hpp"

#if defined __linux__
# include <syslog.h>
#endif

#include <iostream>
#include <atomic>
#include <mutex>

#if WIN32
static std::atomic_flag _is_syslog_open = ATOMIC_FLAG_INIT;
#else
static std::atomic_flag _is_syslog_open( ATOMIC_FLAG_INIT );
#endif

using namespace acqiris::server;

log::log( priority level ) : level_( level )
{
#ifndef WIN32
    if ( !_is_syslog_open.test_and_set( std::memory_order_acquire ) )
        openlog( "acqirisd", LOG_CONS | LOG_PID, LOG_DAEMON );
#endif
}

log::~log()
{
#if defined __linux__
    syslog( level_, "%s", o_.str().c_str() );
#endif
}

