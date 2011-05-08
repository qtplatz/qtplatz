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

#include <adcontrols/timeutil.hpp>
#include <vector>
#include "adcontrols_global.h"

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT TraceAccessor {
    public:
        ~TraceAccessor();
        TraceAccessor();
        TraceAccessor( const TraceAccessor& );

        void clear();
        size_t size() const;

        long pos() const;
        void pos( long );

        bool isConstantSampleInterval() const;
        void sampInterval( unsigned long );
        unsigned long sampInterval() const;

        const seconds_t& getMinimumTime() const;
        void setMinimumTime( const seconds_t& );
        void push_back( double value, unsigned long events, const seconds_t& );
        void push_back( double value, unsigned long events );

        const double * getIntensityArray() const;
        const double * getTimeArray() const;   // null if isConstantSampleInterval is set
        const unsigned long * getEventsArray() const;

    private:

#pragma warning(disable:4251)
        std::vector< double > traceX_;
        std::vector< double > traceY_;
        std::vector< unsigned long > events_;
//#pragma warning(default:4251)

        unsigned long pos_;   // data address
        seconds_t minTime_;  // time corresponding to pos
        bool isConstantSampleInterval_;
        unsigned long sampInterval_; // microseconds
    };

}

