/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <boost/variant.hpp>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace adcontrols { class MassSpectrum; }
namespace adicontroller { namespace SignalObserver { class DataReadBuffer; } }
namespace acqrscontrols { namespace u5303a { class method; class threshold_result; } }

namespace adportable { template<typename T> class mblock; }

namespace acqrscontrols {
    namespace u5303a {

        class method;
        class ident;
        class metadata;

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
            waveform( std::shared_ptr< const identify > id, uint32_t pos, uint32_t events = 0, uint64_t tp = 0 );
            
            waveform( const waveform&, std::unique_ptr< int32_t [] >& data, size_t size, bool invert ); // software averager support

            const int32_t * trim( metadata&, uint32_t& ) const;

            method method_;
            metadata meta_;
            uint32_t serialnumber_;
            uint32_t wellKnownEvents_;
            uint64_t timeSinceEpoch_;
            uint64_t firstValidPoint_;

            size_t size() const; // number of samples
            int dataType() const; // 2 = int16_t, 4 = int32_t
            
            // typedef int32_t value_type;
            
            void setData( const std::shared_ptr< adportable::mblock<int32_t> >&, size_t firstValidPoint );
            void setData( const std::shared_ptr< adportable::mblock<int16_t> >&, size_t firstValidPoint );

            int operator [] ( size_t idx ) const;
            std::pair<double, int> xy( size_t idx ) const;
            double toVolts( int ) const;
            double toVolts( double ) const;
            bool isDEAD() const;            

            const identify* ident() const { return ident_.get(); }

            const std::shared_ptr< const identify >& ident_ptr() const { return ident_; }

            template< typename value_type > const value_type* begin() const;
            template< typename value_type > const value_type* end() const;
            template< typename value_type > value_type* begin();
            template< typename value_type > value_type* end();

			template< typename value_type > value_type* data();

            static bool
                serialize( adicontroller::SignalObserver::DataReadBuffer&, std::shared_ptr< const waveform >, std::shared_ptr< const waveform > );

            static std::array< std::shared_ptr< const waveform >, 2 >
                deserialize( const adicontroller::SignalObserver::DataReadBuffer * );

            static bool apply_filter( std::vector<double>&, const waveform&, const adcontrols::threshold_method& );

            static bool transform( std::vector<double>&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
            
            static bool translate( adcontrols::MassSpectrum&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...

            static bool translate( adcontrols::MassSpectrum&, const threshold_result&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...

        private:

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
            std::shared_ptr< const identify > ident_;

            boost::variant < std::shared_ptr< adportable::mblock<int32_t> >
                             , std::shared_ptr< adportable::mblock<int16_t> > > mblock_;
            
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
