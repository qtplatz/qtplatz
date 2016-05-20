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
#include "samplinginfo.hpp"

namespace adcontrols {

    class SamplingInfo;
    class TofProtocol;
    
    template< typename T > class MSProperty_archive;

    class ADCONTROLSSHARED_EXPORT MSProperty {
    public:
        MSProperty();
        MSProperty( const MSProperty& );
        MSProperty& operator = ( const MSProperty& );

        double acceleratorVoltage() const;
        void acceleratorVoltage( double );
        double tDelay() const;
        void tDelay( double );

        // analyzer mode, ex. Linear/Refrectron for MALDI, number of turns for Multi-turn instrument
        int mode() const;

        double time( size_t pos ); // return flight time for data[pos] in seconds

        double timeSinceInjection() const;
        void setTimeSinceInjection( int64_t, metric::prefix pfx = metric::micro ); // for previous compatibility
        void setTimeSinceInjection( double );

        uint64_t timeSinceEpoch() const;
        void setTimeSinceEpoch( uint64_t );

        uint32_t trigNumberOrigin() const;        
        uint32_t trigNumber( bool sinceOrigin = true ) const;
        void setTrigNumber( uint32_t, uint32_t origin = 0 );

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

        void setTofProtocol( const TofProtocol& );
        std::shared_ptr< const TofProtocol > tofProtocol();

        static double toSeconds( size_t idx, const SamplingInfo& info );
        static size_t toIndex( double seconds, const SamplingInfo& info );
        static size_t compute_profile_time_array( double * p, size_t, const SamplingInfo& segments, metric::prefix pfx );

    private:
        uint64_t time_since_injection_; // nanoseconds, (used be, (befor v8) this was microseconds
        uint64_t time_since_epoch_;     // nanoseconds since 1970 Jan-1 UTC
        double instAccelVoltage_;       // for scan law
        double instTDelay_;             // for scan law
        uint32_t trig_number_;          // trigger number ( a.k.a. 'pos' in the code)
        uint32_t trig_number_origin_;   // trigger number at 'inject' event
        std::string dataInterpreterClsid_;
        std::string deviceData_;
        std::pair< double, double > instMassRange_;
        std::unique_ptr< SamplingInfo > samplingData_;
        std::shared_ptr< const TofProtocol > tofProtocol_;
        
        friend class MSProperty_archive< MSProperty >;
        friend class MSProperty_archive< const MSProperty >;
        friend class boost::serialization::access;
        template<class Archive>  void serialize( Archive& ar, const unsigned int version );
    };
}

BOOST_CLASS_VERSION( adcontrols::MSProperty, 10 )

