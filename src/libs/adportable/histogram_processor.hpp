// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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

#include <vector>
#include <cstddef>
#include <cstdint>
#include <functional>

#include "spectrum_processor.hpp"
#include "adportable_global.h"

namespace adportable {

    class ADPORTABLESHARED_EXPORT histogram_peakfinder {
    public:
        histogram_peakfinder( double xInterval = 1.0e-9, uint32_t width = 3 );

        size_t operator()( size_t nbrSamples, const double * pTimes, const double * pCounts );

        double xInterval_;
        uint32_t width_;
        std::vector< peakinfo > results_;
    };

    class ADPORTABLESHARED_EXPORT histogram_merger {
    public:
        histogram_merger( double xInterval, double threshold );

        size_t operator()( std::vector< peakinfo >&, size_t nbrSamples
                           , const double * pTimes, const double * pCounts );

        const double xInterval_;
        const double threshold_;
    };
}

