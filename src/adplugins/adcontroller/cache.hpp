// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#pragma once

#include <adinterface/signalobserverS.h>
#include <mutex>

#include <deque>

namespace adcontroller {

    class Cache {
    public:
        ~Cache();
        Cache();

#if defined _DEBUG || defined DEBUG
	enum { CACHE_SIZE = 512 };
#else
	enum { CACHE_SIZE = 1024 };
#endif

        bool write( long pos, SignalObserver::DataReadBuffer_var& );
        bool read( long pos, SignalObserver::DataReadBuffer_out );
        long posFromTime( unsigned long long usec );
        void uptime_range( unsigned long long& oldest, unsigned long long& newest );

        struct CacheItem {
			~CacheItem();
            CacheItem( long pos, SignalObserver::DataReadBuffer_var& );
            CacheItem( const CacheItem& );
            inline operator long () const { return pos_; }
            long pos_;
            SignalObserver::DataReadBuffer_var rdbuf_;
        };
    private:
        std::deque< CacheItem > fifo_;
        std::mutex mutex_;
    };

}

