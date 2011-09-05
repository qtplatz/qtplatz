// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "adcontrols_global.h"
#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/algorithm.hpp>

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

        unsigned long instSamplingInterval() const; // ps
        void setInstSamplingInterval( unsigned long ); // ps

        unsigned long instSamplingStartDelay() const;  // number of data points before record waveform in array
        void setInstSamplingStartDelay( unsigned long );

        unsigned long timeSinceInjection() const;
        void setTimeSinceInjection( unsigned long );

        const std::vector< SamplingInfo >& getSamplingInfo() const;
        void setSamplingInfo( const std::vector< SamplingInfo >& );
        void addSamplingInfo( const SamplingInfo& );

        // acquisition mass range, usually it is from user parameter based on theoretical calibration
        const std::pair<double, double>& instMassRange() const;
        void setInstMassRange( const std::pair<double, double>& );

        class ADCONTROLSSHARED_EXPORT SamplingInfo {
        public:
            unsigned long sampInterval; // ps
            unsigned long nSamplingDelay;
            unsigned long nSamples;
            unsigned long nAverage;
            SamplingInfo();
            SamplingInfo( unsigned long sampInterval, unsigned long nDelay, unsigned long nCount, unsigned long nAvg );
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive& ar, const unsigned int version) {
                if ( version >= 0 ) {
                    ar & BOOST_SERIALIZATION_NVP(sampInterval);
                    ar & BOOST_SERIALIZATION_NVP(nSamplingDelay);
                    ar & BOOST_SERIALIZATION_NVP(nSamples);
                    ar & BOOST_SERIALIZATION_NVP(nAverage);
                }
            };

        };

        static std::vector<SamplingInfo>::const_iterator findSamplingInfo( size_t idx, const std::vector<SamplingInfo>& segments );
        static double toSeconds( size_t idx, const std::vector<SamplingInfo>& segments );
        static size_t compute_profile_time_array( double * p, size_t, const std::vector<SamplingInfo>& segments );

    private:
        unsigned long time_since_injection_; // msec
        double instAccelVoltage_;
        unsigned long instNumAvrg_;
        unsigned long instSamplingStartDelay_;
        unsigned long instSamplingInterval_; // ps

        std::vector< SamplingInfo > samplingData_;
        std::pair< double, double > instMassRange_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP(time_since_injection_);
                ar & BOOST_SERIALIZATION_NVP(instAccelVoltage_);
                ar & BOOST_SERIALIZATION_NVP(instNumAvrg_);
                ar & BOOST_SERIALIZATION_NVP(instSamplingStartDelay_);
                ar & BOOST_SERIALIZATION_NVP(instSamplingInterval_);
                ar & BOOST_SERIALIZATION_NVP(instMassRange_.first);
                ar & BOOST_SERIALIZATION_NVP(instMassRange_.second);
            }
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( samplingData_ );
            }
        }

    };

}

BOOST_CLASS_VERSION(adcontrols::MSProperty, 2)
BOOST_CLASS_VERSION(adcontrols::MSProperty::SamplingInfo, 2)


