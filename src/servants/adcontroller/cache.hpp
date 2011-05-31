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

#pragma once

#if defined _MSC_VER
#  pragma warning (disable : 4996 )
#endif
# include <adinterface/signalobserverS.h>
# include <ace/Recursive_Thread_Mutex.h>
#if defined _MSC_VER
#  pragma warning (default : 4996 )
#endif

#include <acewrapper/mutex.hpp>
#include <deque>

namespace adcontroller {

    class Cache {
    public:
        ~Cache();
        Cache();

        bool write( long pos, SignalObserver::DataReadBuffer_var& );
        bool read( long pos, SignalObserver::DataReadBuffer_out );
        long posFromTime( unsigned long long usec );

        struct CacheItem {
            CacheItem( long pos, SignalObserver::DataReadBuffer_var& );
            CacheItem( const CacheItem& );
            inline operator long () const { return pos_; }
            long pos_;
            SignalObserver::DataReadBuffer_var rdbuf_;
        };
    private:
        std::deque< CacheItem > fifo_;
        ACE_Recursive_Thread_Mutex mutex_;
    };

}

