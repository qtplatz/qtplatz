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

#include <adcontrols/timeutil.h>
#include <vector>
#include "adcontrols_global.h"

namespace adcontrols {

    class TraceAccessor;

    class ADCONTROLSSHARED_EXPORT Trace {
    public:
        ~Trace();
        Trace();
        Trace( const Trace& );

        void operator += ( const TraceAccessor& );

        void clear();
        size_t size() const;
        void resize( size_t size );

        const double * getIntensityArray() const;
        const double * getTimeArray() const;   // array of miniutes
        const unsigned long * getEventsArray() const;
        std::pair<double, double> range_y() const;

    private:
        size_t pos_;
        double minY_;
        double maxY_;

#pragma warning(disable:4251)
        std::vector< double > traceX_;
        std::vector< double > traceY_;
        std::vector< unsigned long > events_;
// #pragma warning(default:4251)
    };

}
