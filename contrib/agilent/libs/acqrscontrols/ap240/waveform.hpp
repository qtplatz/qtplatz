/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
namespace acqrscontrols { namespace ap240 { class method; class threshold_result; } }

namespace acqrscontrols {
    namespace ap240 {

        namespace ap240x = acqrscontrols::ap240;

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

        class ACQRSCONTROLSSHARED_EXPORT metadata {
        public:
            metadata() : actualPoints( 0 )
                , flags( 0 )
                , actualAverages( 0 )
                , indexFirstPoint( 0 )
                , channel( 1 )
                , dataType( 1 )
                , initialXTimeSeconds( 0 )
                , initialXOffset( 0 )
                , scaleFactor( 0 )
                , xIncrement( 0 )
                , scaleOffset( 0 )
                , horPos( 0 )
            { }
            int64_t actualPoints;
            int32_t flags;           // IO pin states
            int32_t actualAverages;  // 0 = digitizer data, 1..n averaged data
            int32_t indexFirstPoint; // firstValidPoint in U5303A
            int16_t channel;         // 1|2
            int16_t dataType;        // 1, 2, 4 := int8_t, int16_t, int32_t
            double initialXTimeSeconds;
            double initialXOffset;
            double xIncrement;
            double scaleFactor;
            double scaleOffset;
            double horPos;
        private:
            friend class boost::serialization::access;
            template<class Archive>  void serialize( Archive& ar, const unsigned int version );
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

        //////////////////
        
        class ACQRSCONTROLSSHARED_EXPORT waveform : public std::enable_shared_from_this < waveform > {
            waveform( const waveform& ) = delete;
            void operator = ( const waveform& ) = delete;
        public:
            waveform( const identify& id, uint32_t pos, uint32_t events = 0, uint64_t tp = 0, uint32_t posorg = 0 );

            template<typename T> const T* begin() const;
            template<typename T> const T* end() const;

            template<typename T> void advance( const T*& it, size_t distance ) const {
                it = ( distance && distance < size_t( std::distance( it, end<T>() ) ) ? it + distance : end<T>() );
            }

            std::pair<double, int> operator [] ( size_t ) const;
            double toVolts( int ) const;
            double toVolts( double ) const;

            acqrscontrols::ap240::method method_;
            metadata meta_;
            uint32_t serialnumber_origin_;
            uint32_t serialnumber_;
            uint32_t wellKnownEvents_;
            uint64_t timeSinceEpoch_;
            identify ident_;

            size_t size() const; // number of samples (octet size is depend on meta_.dataType)

            typedef int32_t value_type; // referenced from archiver in WaveformObserver

            value_type * data( size_t size ) { d_.resize( size ); return d_.data(); }

            const value_type * data() const { return d_.data(); }
            size_t data_size() const { return d_.size(); }  // internal data count

            static std::array< std::shared_ptr< const waveform >, 2 >
                deserialize( const adicontroller::SignalObserver::DataReadBuffer * );

            static bool
                serialize( adicontroller::SignalObserver::DataReadBuffer&, std::shared_ptr< const waveform >, std::shared_ptr< const waveform > );

            static bool translate( adcontrols::MassSpectrum&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
            static bool translate( adcontrols::MassSpectrum&, const threshold_result&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...

        private:
            static bool translate_property( adcontrols::MassSpectrum&, const waveform& );

#if defined _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif
            std::vector< value_type > d_;

#if defined _MSC_VER
# pragma warning( pop )
#endif
            friend struct ::ap240::detail::device_ap240;
        };

        template<> ACQRSCONTROLSSHARED_EXPORT const int8_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int8_t * waveform::end() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::end() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::end() const;


    }
}
