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

//#include "avgr_arp.hpp"
#include "avgr_acqiris.hpp"
#include "avgr_u5303a.hpp"
#include "method.hpp"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/version.hpp>
#include <boost/noncopyable.hpp>

namespace infitof {

    namespace acqiris { class AqDescriptors; }
    namespace arp { class ArpDescriptors; }

    enum eAveragerType {
        Averager_NONE
        , Averager_AP240
        , Averager_U5303A
    };

    struct DelayData {
        int64_t delay_; // ps (can be negative)
        uint64_t duration_; // ps
        DelayData() : delay_( 0 ), duration_( 0 ) {}
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & delay_ & duration_;
        }
    };

    class AveragerData {

        AveragerData( const AveragerData& ) = delete;
        AveragerData& operator = ( const AveragerData& ) = delete;

    public:
        AveragerData();
        ~AveragerData();

        int32_t npos;                 // 0
        int64_t  timeSinceInject;     // 1,2
        uint64_t timeSinceEpoch;      // 3,4
        uint64_t uptime;              // 5,6
        uint32_t nbrSamples;          // 7 (total number in waveform) --> will obsolte
        uint32_t nbrAverage;          // 8
        uint32_t nSamplingDelay;      // 9 (point to 1st segment data)
        uint32_t sampInterval;        // 10 (ps)
        uint32_t wellKnownEvents;     // 11
        uint32_t nTurns_deprecated;
        uint32_t protocolId;
        uint32_t nProtocols;
        OrbitProtocol protocol_;
        eAveragerType  avgrType;
        boost::variant< /* arp::ArpDescriptors, */ acqiris::AqDescriptors, u5303a::Descriptors > desc;
        double kAcceleratorVoltage;
        double tDelay;
        uint32_t mark;
        std::vector< int32_t > waveform;

        // don't archive byound this line
        int archive_version_;

        bool operator -= ( const AveragerData& dark ) {
            if ( nbrAverage == dark.nbrAverage
                 && waveform.size() == dark.waveform.size() ) {
                std::transform( waveform.begin(), waveform.end()
                                , dark.waveform.begin(), waveform.begin()
                                , [] ( int32_t a, int32_t b ){ return a - b; } );
                return true;
            }
            return false;
        }

    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int version );
    };

}

BOOST_CLASS_VERSION( infitof::AveragerData, 5 )
