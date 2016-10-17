/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "../acqrscontrols_global.hpp"
#include "metadata.hpp"
#include "method.hpp"
#include <boost/serialization/version.hpp>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace adcontrols { class MassSpectrum; }
namespace adicontroller { namespace SignalObserver { class DataReadBuffer; } }
namespace ap240 { namespace detail { struct device_ap240; } }
namespace adportable { template<typename T> class mblock; }
namespace acqrscontrols {
    namespace ap240 { class method; class threshold_result; }
    namespace aqdrv4 { class waveform; }
}

namespace acqrscontrols {
    namespace ap240 {

        class ACQRSCONTROLSSHARED_EXPORT identify {
        public:
            identify();
            identify( const identify& );
            uint32_t bus_number_;
            uint32_t slot_number_;
            uint32_t serial_number_;

        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
        };

        class ACQRSCONTROLSSHARED_EXPORT device_data {
        public:
            identify ident_;
            metadata meta_;
            device_data( const identify& ident, const metadata& meta ) : ident_( ident ), meta_( meta ) {
            }
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int );
        };

#if defined _MSC_VER
        class waveform;
        ACQRSCONTROLSSHARED_TEMPLATE_EXPORT template class ACQRSCONTROLSSHARED_EXPORT std::weak_ptr < waveform > ;
#endif

        template< typename T > class waveform_xdata_archive_t;

        //////////////////
        
        class ACQRSCONTROLSSHARED_EXPORT waveform : public std::enable_shared_from_this < waveform > {

            waveform( const waveform& ) = delete;
            void operator = ( const waveform& ) = delete;

        public:
            waveform();
            
            waveform( const identify& id, uint32_t pos, uint32_t events = 0, uint64_t tp = 0, uint32_t posorg = 0 );

            waveform( std::shared_ptr< const identify > id, uint32_t pos, uint32_t events, uint64_t tp );

            waveform( const method&
                      , const metadata&
                      , uint32_t serialnumber
                      , uint32_t wellKnownEvents
                      , uint64_t timeSinceEpoch
                      , uint64_t firstValidPoint
                      , double timeSinceInject
                      , const std::shared_ptr< const identify >& id
                      , std::unique_ptr< int32_t [] >& data, size_t size, bool invert ); // software averager support

            typedef int32_t value_type; // (internal data holder type) referenced from archiver in WaveformObserver
            
            template< typename value_t > const value_t* begin() const;
            template< typename value_t > const value_t* end() const;
            template< typename value_t > const value_t* data() const;
			template< typename value_t > value_t* data();

            template<typename T> void advance( const T*& it, size_t distance ) const {
                it = ( distance && distance < size_t( std::distance( it, end<T>() ) ) ? it + distance : end<T>() );
            }

            waveform& operator += ( const waveform& );
            
            int64_t operator [] ( size_t ) const;
            std::pair<double, int> xy( size_t idx ) const;
            
            double toVolts( int32_t ) const;
            double toVolts( int64_t ) const;
            double toVolts( double ) const;

            acqrscontrols::ap240::method method_;
            metadata meta_;
            uint32_t serialnumber_origin_;
            uint32_t serialnumber_;
            uint32_t wellKnownEvents_;
            uint32_t firstValidPoint_;
            uint64_t timeSinceEpoch_;
            double timeSinceInject_;
            identify ident_;

            void move( std::shared_ptr< acqrscontrols::aqdrv4::waveform >&& );
            
            size_t size() const; // number of samples (octet size is depend on meta_.dataType)

            int dataType() const; // 1 - int8_t, 2 = int16_t, 4 = int32_t

            value_type * data( size_t size );
            const value_type * data() const;
            size_t data_size() const;

            // reused in threshold_result archive
            bool serialize_xmeta( std::string& ) const;
            bool deserialize_xmeta( const char *, size_t );

            // data serialization for waveform_accessor
            bool serialize_xdata( std::string& ) const;
            bool deserialize_xdata( const char *, size_t );
            bool deserialize( const char * xdata, size_t dsize, const char * xmeta, size_t msize );

            double accumulate( double tof, double window ) const;

            static bool apply_filter( std::vector<double>&, const waveform&, const adcontrols::threshold_method& );
            
            static std::array< std::shared_ptr< const waveform >, 2 >
                deserialize( const adicontroller::SignalObserver::DataReadBuffer * );

            static bool
                serialize( adicontroller::SignalObserver::DataReadBuffer&
                           , std::shared_ptr< const waveform >, std::shared_ptr< const waveform > );

            typedef double( mass_assign_t )( double time, int mode );
            typedef std::function< mass_assign_t > mass_assignor_t;

            static bool transform( std::vector<double>&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
            
            static bool translate( adcontrols::MassSpectrum&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
            static bool translate( adcontrols::MassSpectrum&, const waveform&, mass_assignor_t, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
            static bool translate( adcontrols::MassSpectrum&, const threshold_result&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
            static bool translate( adcontrols::MassSpectrum&, const threshold_result&, mass_assignor_t, int scale = 1000 );

        private:
            static bool translate_property( adcontrols::MassSpectrum&, const waveform& );
            void lvalue_cast();

            std::vector< value_type > d_;
            
            friend struct ::ap240::detail::device_ap240;
            friend class waveform_xdata_archive_t< waveform >;
            friend class waveform_xdata_archive_t< const waveform >;
        };

        template<> ACQRSCONTROLSSHARED_EXPORT const int8_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int8_t * waveform::end() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::end() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::end() const;

		template<> ACQRSCONTROLSSHARED_EXPORT int8_t * waveform::data();
		template<> ACQRSCONTROLSSHARED_EXPORT const int8_t * waveform::data() const;        
		template<> ACQRSCONTROLSSHARED_EXPORT int16_t * waveform::data();
		template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::data() const;
        template<> ACQRSCONTROLSSHARED_EXPORT int32_t * waveform::data();
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::data() const;

    }
}
