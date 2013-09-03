// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

    class ADCONTROLSSHARED_EXPORT TraceAccessor {
    public:
        ~TraceAccessor();
        TraceAccessor();
        TraceAccessor( const TraceAccessor& );

		struct ADCONTROLSSHARED_EXPORT fcnTrace {
            std::vector< uint32_t > pos_;
            std::vector< double > traceX_;
            std::vector< double > traceY_;
            fcnTrace() {}
            fcnTrace( const fcnTrace& t ) 
                : pos_( t.pos_ )
                , traceX_( t.traceX_ )
                , traceY_( t.traceY_ ) {
            }
		};

        void clear();

        void push_back( int fcn, uint32_t pos, const seconds_t&, double value, unsigned long events );

        const std::vector< fcnTrace >& traces() const { return traces_; }
        const std::vector< std::pair< seconds_t, uint32_t > >& events() const { return events_; }
    private:
        std::vector< std::pair< seconds_t, uint32_t > > events_;
		std::vector< fcnTrace > traces_;
    };

}

