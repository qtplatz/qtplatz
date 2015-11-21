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
#include "identify.hpp"
#include "metadata.hpp"
#include <boost/serialization/version.hpp>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace adcontrols { class MassSpectrum; }
namespace adicontroller { namespace SignalObserver { class DataReadBuffer; } }
namespace acqrscontrols { namespace u5303a { class method; class threshold_result; } }

namespace acqrscontrols {
    namespace u5303a {

        class method;
        class ident;
        class metadata;
		class mblock;

        class ACQRSCONTROLSSHARED_EXPORT device_data {
            device_data( const device_data& ) = delete;
            device_data& operator = ( const device_data& ) = delete;
        public:
            device_data() {}
            device_data( const identify& ident, const metadata& meta ) : ident_( ident ), meta_( meta ) {
            }
            identify ident_;
            metadata meta_;
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int );
        };

        
#if defined _MSC_VER
        class waveform;
        ACQRSCONTROLSSHARED_TEMPLATE_EXPORT template class ACQRSCONTROLSSHARED_EXPORT std::weak_ptr < waveform > ;
#endif

        /////////////////////////////

        class ACQRSCONTROLSSHARED_EXPORT waveform : public std::enable_shared_from_this < waveform > {

            waveform( const waveform& ); // = delete;
            void operator = ( const waveform& ); // = delete;

        public:
            waveform( std::shared_ptr< identify > id, uint32_t pos, uint32_t events = 0, uint64_t tp = 0 );

            const int32_t * trim( metadata&, uint32_t& ) const;

            method method_;
            metadata meta_;
            uint32_t serialnumber_;
            uint32_t wellKnownEvents_;
            uint64_t timeSinceEpoch_;
            uint64_t firstValidPoint_;

            size_t size() const; // number of samples (octet size is depend on meta_.dataType)

            typedef int32_t value_type;
            
            //value_type * data( size_t size ) { d_.resize( size ); return d_.data(); } // raw data pointer
            //const value_type * data() const { return d_.data() + firstValidPoint_; }  // raw data pointer
            //size_t data_size() const { return d_.size(); }
            //void resize( size_t size ) { d_.resize( size ); }
            void setData( const std::shared_ptr< mblock >&, size_t firstValidPoint );

            std::pair<double, int> operator [] ( size_t idx ) const;

            double toVolts( int ) const;

            double toVolts( double ) const;

            const identify* ident() const { return ident_.get(); }

            template<typename T=int32_t> const T* begin() const;
            template<typename T=int32_t> const T* end() const;
			template<typename T> T* data();

            bool isDEAD() const;
            
            static std::array< std::shared_ptr< const waveform >, 2 >
                deserialize( const adicontroller::SignalObserver::DataReadBuffer * );

            static bool
                serialize( adicontroller::SignalObserver::DataReadBuffer&, std::shared_ptr< const waveform >, std::shared_ptr< const waveform > );

            static bool translate( adcontrols::MassSpectrum&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...

            static bool translate( adcontrols::MassSpectrum&, const threshold_result&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...

        private:

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
            //std::vector< int32_t > d_;
            std::shared_ptr< identify > ident_;
            std::shared_ptr< mblock > mblock_;

#if defined _MSC_VER
#pragma warning(pop)
#endif
        };

        template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::end() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::end() const;
		template<> ACQRSCONTROLSSHARED_EXPORT int16_t * waveform::data();
        template<> ACQRSCONTROLSSHARED_EXPORT int32_t * waveform::data();
    }
}
