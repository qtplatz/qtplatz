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
        void nlimits( size_t n );
        size_t nlimits() const;

        const double * getIntensityArray() const;
        const double * getTimeArray() const;   // array of miniutes
        const unsigned long * getEventsArray() const;
        std::pair<double, double> range_y() const;

    private:
        size_t ulimits_;
		size_t fcn_;
        double minY_;
        double maxY_;

        std::vector< double > traceX_;
        std::vector< double > traceY_;
        std::vector< unsigned long > events_;
    };

}
