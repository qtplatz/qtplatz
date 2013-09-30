// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "adcontrols_global.h"
#include "metric/prefix.hpp"
#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/tuple/tuple.hpp>

//#include <boost/fusion/container.hpp>
//#include <boost/fusion/algorithm.hpp>

#include "mscalibration.hpp"

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSProperty {
    public:
        MSProperty();
        MSProperty( const MSProperty& );
        class SamplingInfo;

        double accelerateVoltage() const;
        void setAccelerateVoltage( double );

        // number of average for waveform
        size_t numAverage() const;
        void setNumAverage( size_t );

        double time( size_t pos ); // return flight time for data[pos] in seconds

        uint32_t instSamplingInterval() const; // ps
        void setInstSamplingInterval( uint32_t ); // ps

        uint32_t instSamplingStartDelay() const;  // number of data points before record waveform in array
        void setInstSamplingStartDelay( uint32_t );

        uint32_t timeSinceInjection() const;
        void setTimeSinceInjection( uint32_t );

        const SamplingInfo& getSamplingInfo() const;
        void setSamplingInfo( const SamplingInfo& );

        // acquisition mass range, usually it is from user parameter based on theoretical calibration
        const std::pair<double, double>& instMassRange() const;
        void setInstMassRange( const std::pair<double, double>& );

        class ADCONTROLSSHARED_EXPORT SamplingInfo {
        public:
            uint32_t sampInterval; // ps
            uint32_t nSamplingDelay;
            uint32_t nSamples;
            uint32_t nAverage;
            uint32_t mode;  // number of turns for InfiTOF, lenear|reflectron for MALDI etc
            SamplingInfo();
            SamplingInfo( uint32_t sampInterval, uint32_t nDelay, uint32_t nCount, uint32_t nAvg, uint32_t mode );
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive& ar, const unsigned int version ) {
                ar & BOOST_SERIALIZATION_NVP(sampInterval);
                ar & BOOST_SERIALIZATION_NVP(nSamplingDelay);
                ar & BOOST_SERIALIZATION_NVP(nSamples);
                ar & BOOST_SERIALIZATION_NVP(nAverage);
                if ( version >= 3 )
                    ar & BOOST_SERIALIZATION_NVP(mode);
            };

        };

        // static std::vector<SamplingInfo>::const_iterator findSamplingInfo( size_t idx, const std::vector<SamplingInfo>& segments );
        // static double toSeconds( size_t idx, const std::vector<SamplingInfo>& segments );
        static double toSeconds( size_t idx, const SamplingInfo& info );
        static size_t compute_profile_time_array( double * p, size_t, const SamplingInfo& segments, metric::prefix pfx );

    private:
        uint32_t time_since_injection_; // msec
        double instAccelVoltage_;
        uint32_t instNumAvrg_;
        uint32_t instSamplingStartDelay_;
        uint32_t instSamplingInterval_; // ps
#if defined _MSC_VER
# pragma warning( disable: 4251 )
#endif
        SamplingInfo samplingData_;
        std::pair< double, double > instMassRange_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(time_since_injection_);
            ar & BOOST_SERIALIZATION_NVP(instAccelVoltage_);
            ar & BOOST_SERIALIZATION_NVP(instNumAvrg_);
            ar & BOOST_SERIALIZATION_NVP(instSamplingStartDelay_);
            ar & BOOST_SERIALIZATION_NVP(instSamplingInterval_);
            ar & BOOST_SERIALIZATION_NVP(instMassRange_.first);
            ar & BOOST_SERIALIZATION_NVP(instMassRange_.second);
            if ( version == 2 ) {
                std::vector< SamplingInfo > data;
                ar & BOOST_SERIALIZATION_NVP( data );
                if ( ! data.empty() )
                    samplingData_ = data[ 0 ];
            } else if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( samplingData_ );
            }
        }

    };

}

BOOST_CLASS_VERSION(adcontrols::MSProperty, 3)
BOOST_CLASS_VERSION(adcontrols::MSProperty::SamplingInfo, 3)

