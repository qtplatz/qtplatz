/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "tofstaticsetpts.hpp"
#include "tofstaticacts.hpp"
#include "tofacqmethod.hpp"
#include "cstdint.hpp"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

namespace tofinterface {

    struct SpectrumPeakInfo {
        float monitor_range_lower;
        float monitor_range_upper;
        double peakMass;
        float peakHeight;
        float peakArea;
        float resolvingPower;
        float resolvingPowerX1;
        float resolvingPowerX2;
        float resolvingPowerHH;
        SpectrumPeakInfo();
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & monitor_range_lower
                & monitor_range_upper
                & peakMass
                & peakHeight
                & peakArea
                & resolvingPower
                & resolvingPowerX1
                & resolvingPowerX2
                & resolvingPowerHH;
        }
    };

    class tofProcessedData {
    public:
        tofProcessedData();
        tofProcessedData( const tofProcessedData& );

        float tic;
        float spectralBaselineLevel;
        unsigned long long uptime;
        std::vector< SpectrumPeakInfo > info;
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & tic
                & spectralBaselineLevel
                & uptime
                & info;
        }
    };

    typedef std::vector< tofProcessedData > tofProcessedDataArray;

}


