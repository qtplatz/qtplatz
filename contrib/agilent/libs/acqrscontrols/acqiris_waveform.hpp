/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include <AcqirisImport.h> 
#include <AcqirisD1Import.h>
#include <vector>
#include <ratio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/serialization/nvp.hpp>
#include "acqrscontrols_global.hpp"

namespace boost {
    namespace serialization { class access; }
}

namespace acqrscontrols {  namespace ap240 { class waveform; } }

namespace acqrscontrols {
namespace aqdrv4 {

    template< typename T > struct waveform_archive;

    class ACQRSCONTROLSSHARED_EXPORT waveform {

        waveform( const waveform& ) = delete;
        void operator = ( const waveform& ) = delete;

    public:
        waveform( int32_t dataType = sizeof( int8_t ) );

        static const boost::uuids::uuid& clsid();

        typedef int32_t value_type; // (internal data holder type) referenced from archiver in WaveformObserver
            
        template< typename value_t > const value_t* begin() const;
        template< typename value_t > const value_t* end() const;
        template< typename value_t > const value_t* data() const;
        template< typename value_t > value_t* data();

        template<typename T> void advance( const T*& it, size_t distance ) const {
            it = ( distance && distance < size_t( std::distance( it, end<T>() ) ) ? it + distance : end<T>() );
        }

        int operator [] ( size_t idx ) const;

        double time( size_t idx ) const {
            return idx * dataDesc_.sampTime + segDesc_.horPos + delayTime_;        
        }

        double toVolts( int d, int scale = std::milli::den ) const {
            return ( dataDesc_.vGain * d - dataDesc_.vOffset ) * scale ;
        }

        size_t size() const;

        int dataType() const; // 1 - int8_t, 2 = int16_t, 4 = int32_t

        value_type * data( size_t size );
        const value_type * data() const;
        size_t data_size() const;

        inline const AqDataDescriptor& dataDesc() const {
            return dataDesc_;
        }

        inline const AqSegmentDescriptor& segDesc() const {
            return segDesc_;
        }

        inline AqDataDescriptor& dataDesc() {
            return dataDesc_;
        }

        inline AqSegmentDescriptor& segDesc() {
            return segDesc_;
        }

        // inline uint64_t serialNumber() const {
        //      return serialnumber_;
        // }
        // inline uint64_t& serialNumber() {
        //      return serialnumber_;
        // }
        inline uint64_t serialNumber() const {
            return serialnumber_;
        }
        inline uint64_t& serialNumber() {
            return serialnumber_;
        }
        inline uint64_t serialNumber0() const {
            return serialnumber0_;
        }
        inline uint64_t& serialNumber0() {
            return serialnumber0_;
        }

        inline uint32_t wellKnownEvents() const {
            return wellKnownEvents_;
        }        
        inline uint32_t& wellKnownEvents() {
            return wellKnownEvents_;
        }

        inline uint64_t timeSinceEpoch() const {
            return timeSinceEpoch_;
        }
        inline uint64_t& timeSinceEpoch() {
            return timeSinceEpoch_;
        }

        inline uint64_t timeSinceInject() const {
            return timeSinceInject_;
        }
        inline uint64_t& timeSinceInject() {
            return timeSinceInject_;
        }

        inline double delayTime() const {
            return delayTime_;
        }

        inline double& delayTime() {
            return delayTime_;
        }

        inline uint64_t timeStamp() const {
            return uint64_t( segDesc_.timeStampHi ) << 32 | segDesc_.timeStampLo;        
        }

        inline double xIncrement() const {
            return dataDesc_.sampTime;
        }

        inline double vOffset() const {
            return dataDesc_.vOffset;
        }

        inline double vGain() const {
            return dataDesc_.vGain;
        }

        inline std::vector< value_type >& d() {
            return d_;
        }

        inline int32_t& dataType() {
            return dataType_;
        }

        inline void setMethodNumber( uint32_t d ) {
            methodNumber_ = d;
        }

        inline uint32_t methodNumber() const {
            return methodNumber_;
        }

        template< typename T > static bool transform( std::vector<double>& v, const waveform& w, int scale ) {
            std::transform( w.begin<T>(), w.end<T>(), v.begin(), [&](auto& y){ return w.toVolts( y, scale ); } );
        };
        
        static bool transform( std::vector<double>& v, const waveform& w, int scale = 1000 ) { // mV default
            v.resize( w.size() );
            switch( w.dataType() ) {
            case 1: return transform< int8_t >( v, w, scale );
            case 2: return transform< int16_t >( v, w, scale );
            case 4: return transform< int32_t >( v, w, scale );
            };
        };

        
    private:
        uint64_t serialnumber_;            // a.k.a. trigger number
        uint64_t serialnumber0_;           // serialnumber at inject
        uint64_t timeSinceEpoch_;          // host computer time point base
        uint64_t timeSinceInject_;         // hardware timestamp base
        uint32_t wellKnownEvents_;
        int32_t  dataType_;                // actual data type (sizeof(int8_t), sizeof(int32_t) ...)
        uint32_t methodNumber_;            // method sequence number for async method identification
        double delayTime_;
        AqDataDescriptor dataDesc_;
        AqSegmentDescriptor segDesc_;
        std::vector< value_type > d_;

        //////////////////////////////
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
        friend struct  waveform_archive<waveform>;
        friend struct waveform_archive<const waveform>;
        friend class ap240::waveform;
    };

    template<> const int8_t * waveform::begin() const;
    template<> const int8_t * waveform::end() const;
    template<> const int16_t * waveform::begin() const;
    template<> const int16_t * waveform::end() const;
    template<> const int32_t * waveform::begin() const;
    template<> const int32_t * waveform::end() const;

    template<> int8_t * waveform::data();
    template<> const int8_t * waveform::data() const;        
    template<> int16_t * waveform::data();
    template<> const int16_t * waveform::data() const;
    template<> int32_t * waveform::data();
    template<> const int32_t * waveform::data() const;

    
    //////////////////////////////////////////////////////////////
    /////////// waveform vector archier //////////////////////////
    //////////////////////////////////////////////////////////////
    
    class waveforms {
    public:
        std::vector< std::shared_ptr< waveform > > data;
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( data );
        }
    };
    
} // namespace aqdrv4
} // namespace acqriscontrols
