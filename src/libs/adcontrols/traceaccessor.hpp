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

#include <adcontrols/timeutil.hpp>
#include <vector>
#include "adcontrols_global.h"
#include <compiler/disable_dll_interface.h>
#include <cstdint>

namespace adcontrols {

    class Trace;
    class Chromatogram;

    class ADCONTROLSSHARED_EXPORT TraceAccessor {
    public:
        ~TraceAccessor();
        TraceAccessor();
        TraceAccessor( const TraceAccessor& );
        TraceAccessor& operator += ( const TraceAccessor& );
        inline bool empty() const { return trace_.empty(); }
        inline size_t size() const { return trace_.size(); }

        struct fcnData {
            int fcn;
            uint32_t npos;
            uint32_t events;
            seconds_t x;
            double y;
        };

        void clear();
        void push_back( int fcn, uint32_t pos, const seconds_t&, double value, unsigned long events );
        size_t operator >> ( Trace& ) const;
        void copy_to( Trace&, int fcn );
        void copy_to( Chromatogram&, int fcn );
        
        const std::vector< fcnData >& trace() const { return trace_; }
        size_t nfcn() const { return size_t( maxfcn_ ) + 1; }

    private:
		std::vector< fcnData > trace_;
        int maxfcn_;
    };

}

