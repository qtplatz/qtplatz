/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

#if defined __GNUC__ && !defined __APPLE__
# pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include <boost/serialization/variant.hpp>
#include <boost/noncopyable.hpp>

namespace infitof {

    class SpectrumPeakInfo {
    public:
        float monitor_range_lower;
        float monitor_range_upper;
        double peakMass;
        float peakHeight;
        float peakArea;
        float resolvingPower;
        float resolvingPowerX1;
        float resolvingPowerX2;
        float resolvingPowerHH;
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
                & resolvingPowerHH
                ;
        }
    };

    typedef std::vector< SpectrumPeakInfo > SpectrumPeakInfoVec;
    
    class SpectrumProcessedData {
    public:
        uint32_t npos;
        uint32_t fcn;
        uint64_t uptime;
        float tic;
        float spectralBaselineLevel;
        std::vector< SpectrumPeakInfo > info;
        SpectrumProcessedData() : npos( 0 ), fcn(0), uptime(0), tic(0), spectralBaselineLevel(0) {
        }
        SpectrumProcessedData( const SpectrumProcessedData& t ) : npos( t.npos )
                                                                , fcn( t.fcn )
                                                                , uptime( t.uptime )
                                                                , tic( t.tic )
                                                                , spectralBaselineLevel( t.spectralBaselineLevel )
                                                                , info( t.info ) {
        }
    private:
        friend class boost::serialization::access;
        template< class Archive >  
        void serialize( Archive& ar, const unsigned int ) {
            ar & npos
                & fcn
                & uptime
                & tic 
                & spectralBaselineLevel
                & info
                ;
        }
    };

    class TraceMetadata {
	public:
        uint32_t wellKnownEvents;
        uint32_t ndata;
        uint64_t uptime;      // usec since clock sync
        uint64_t timeSinceInject;  // usec since most recent INJ trig.
        uint32_t sampInterval;     // usec
        uint32_t dataType;
    private:
        friend class boost::serialization::access;
        template< class Archive >  
        void serialize( Archive& ar, const unsigned int ) {
            ar & wellKnownEvents
                & ndata
                & uptime      // usec since clock sync
                & timeSinceInject  // usec since most recent INJ trig.
                & sampInterval     // usec
                & dataType
                ;
        }
    };

}


