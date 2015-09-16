// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include "massspectrometer.hpp"
#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSProperty {
    public:
        MSProperty();
        MSProperty( const MSProperty& );
        class SamplingInfo;

        double acceleratorVoltage() const;
        void acceleratorVoltage( double );
        double tDelay() const;
        void tDelay( double );

        // analyzer mode, ex. Linear/Refrectron for MALDI, number of turns for Multi-turn instrument
        int mode() const;

        // helper method for quick access to spectrometer class
        const MassSpectrometer& spectrometer() const;
        std::shared_ptr< ScanLaw > scanLaw() const;

        double time( size_t pos ); // return flight time for data[pos] in seconds

        double timeSinceInjection() const;
        void setTimeSinceInjection( int64_t, metric::prefix pfx = metric::micro ); // for previous compatibility
        void setTimeSinceInjection( double );

        uint64_t timeSinceEpoch() const;
        void setTimeSinceEpoch( uint64_t );

        uint32_t trigNumberOrigin() const;        
        uint32_t trigNumber( bool sinceOrigin = true ) const;
        void setTrigNumber( uint32_t, uint32_t origin = 0 );

        const SamplingInfo& getSamplingInfo() const; // deprecated
        const SamplingInfo& samplingInfo() const;        
        void setSamplingInfo( const SamplingInfo& );

        // acquisition mass range, usually it is from user parameter based on theoretical calibration
        const std::pair<double, double>& instMassRange() const;
        void setInstMassRange( const std::pair<double, double>& );

        std::pair<double, double> instTimeRange() const;

        // Device specific parameters

        void setDataInterpreterClsid( const char * utf8 );
        const char * dataInterpreterClsid() const;
        void setDeviceData( const char * device, size_t size );
        const char * deviceData() const;
        size_t deviceDataSize() const;

        uint32_t numAverage() const;
        void setNumAverage( uint32_t );
        void setSamplingDelay( uint32_t );
        void setSamplingInterval( uint32_t ); // ps
        void setfSamplingInterval( double ); // seconds

        class ADCONTROLSSHARED_EXPORT SamplingInfo {
        public:
            uint32_t sampInterval;  // ps
            uint32_t nSamplingDelay;
            uint32_t nSamples;
            uint32_t nAverage;
            uint32_t mode;  // number of turns for InfiTOF, lenear|reflectron for MALDI etc
            uint32_t padding;

            SamplingInfo();
            SamplingInfo( uint32_t sampInterval, uint32_t nDelay, uint32_t nCount, uint32_t nAvg, uint32_t mode );
            void fSampInterval( double );
            double fSampInterval() const;
			double fSampDelay() const;
            void horPos( double );
            double horPos() const;
            void setDelayTime( double );
            double delayTime() const;
            size_t numberOfTriggers() const;

        private:
            double fsampInterval; // seconds
            double horPos_;       // seconds
            double delayTime_;    // digitizer delay time (seconds), this can be negative!

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
                if ( version >= 4 ) {
                    ar & BOOST_SERIALIZATION_NVP(padding)
                        & BOOST_SERIALIZATION_NVP(fsampInterval)
                        ;
                }
                if ( version >= 5 ) {
                    ar & BOOST_SERIALIZATION_NVP( horPos_ );
                    ar & BOOST_SERIALIZATION_NVP( delayTime_ );
                }
            };
        };

        static double toSeconds( size_t idx, const SamplingInfo& info );
        static size_t compute_profile_time_array( double * p, size_t, const SamplingInfo& segments, metric::prefix pfx );

    private:
        uint64_t time_since_injection_; // nanoseconds, (used be, (befor v8) this was microseconds
        uint64_t time_since_epoch_;     // nanoseconds since 1970 Jan-1 UTC
        double instAccelVoltage_;       // for scan law
        double instTDelay_;             // for scan law
        uint32_t trig_number_;          // trigger number ( a.k.a. 'pos' in the code)
        uint32_t trig_number_origin_;   // trigger number at 'inject' event
        uint32_t deprecated_instSamplingInterval_;   // deprecated
        std::string dataInterpreterClsid_;
        std::string deviceData_;
        std::vector< double > deprecated_coeffs_; // deprecated

        std::string encode( const std::string& );
        std::string decode( const std::string& );
#if defined _MSC_VER
# pragma warning( push )        
# pragma warning( disable: 4251 )
#endif
        SamplingInfo samplingData_;
        std::pair< double, double > instMassRange_;
#if defined _MSC_VER
# pragma warning( pop )        
#endif
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            if ( version >= 6 ) {
                if ( version >= 8 ) {
                    ar & BOOST_SERIALIZATION_NVP( time_since_injection_ );
                    ar & BOOST_SERIALIZATION_NVP( time_since_epoch_ );
                } else {                    
                    uint32_t time_since_injection;
                    ar & boost::serialization::make_nvp( "time_since_injection_", time_since_injection );
                    time_since_injection_ = time_since_injection;
                }
                ar & BOOST_SERIALIZATION_NVP( instAccelVoltage_);
                ar & BOOST_SERIALIZATION_NVP( instTDelay_ );
                ar & BOOST_SERIALIZATION_NVP( instMassRange_.first );
                ar & BOOST_SERIALIZATION_NVP( instMassRange_.second );
                ar & BOOST_SERIALIZATION_NVP( samplingData_ );
                ar & BOOST_SERIALIZATION_NVP( dataInterpreterClsid_ );
                if ( Archive::is_saving::value ) {
                    std::string data = encode( deviceData_ );
                    ar & boost::serialization::make_nvp( "deviceData_", data );  // for xml (u8 codecvt) safety
                }  else {
                    ar & BOOST_SERIALIZATION_NVP( deviceData_ );
                    if ( version >= 7 ) // v6 data has no encoded data
                        deviceData_ = decode( deviceData_ );
                }
            } else {
                uint32_t time_since_injection;
                ar & boost::serialization::make_nvp( "time_since_injection_", time_since_injection );
                time_since_injection_ = time_since_injection;
                ar & BOOST_SERIALIZATION_NVP(instAccelVoltage_);
                ar & BOOST_SERIALIZATION_NVP(trig_number_);            // same data is in sampleData_ below
                ar & BOOST_SERIALIZATION_NVP(trig_number_origin_);     // same data is in sampleData_ below
                ar & BOOST_SERIALIZATION_NVP(deprecated_instSamplingInterval_);   // same data is in sampleData_ below
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
                if ( version >= 5 )
                    ar & BOOST_SERIALIZATION_NVP( instTDelay_ );
                if ( version >= 4 ) {
                    ar & BOOST_SERIALIZATION_NVP( dataInterpreterClsid_ );
                    ar & BOOST_SERIALIZATION_NVP( deviceData_ );
                    ar & BOOST_SERIALIZATION_NVP( deprecated_coeffs_ );
                }
            }
            if ( Archive::is_loading::value && version <= 8 ) {
                time_since_injection_ *= 1000;  // us -> ns
            }
        }
    };
}

BOOST_CLASS_VERSION(adcontrols::MSProperty, 9)
BOOST_CLASS_VERSION(adcontrols::MSProperty::SamplingInfo, 5)

