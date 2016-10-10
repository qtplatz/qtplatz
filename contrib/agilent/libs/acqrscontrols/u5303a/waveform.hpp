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
#include <adcontrols/tofprotocol.hpp>
#include <boost/variant.hpp>
#include <boost/serialization/version.hpp>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>
#include <compiler/pragma_warning.hpp>

namespace adcontrols { class MassSpectrum; class ScanLaw; }
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
            device_data( const identify& ident
                         , const metadata& meta ) : ident_( ident ), meta_( meta ) {
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
        template< typename T > class waveform_xmeta_archive;
        template< typename T > class waveform_xdata_archive;

        class ACQRSCONTROLSSHARED_EXPORT waveform : public std::enable_shared_from_this < waveform > {
            
            waveform( const waveform& ); // = delete;
            void operator = ( const waveform& ); // = delete;

        public:
            waveform( std::shared_ptr< const identify > id, uint32_t pos, uint32_t events = 0, uint64_t tp = 0 );
            
            waveform( const method&
                      , const metadata&
                      , uint32_t serialnumber
                      , uint32_t wellKnownEvents
                      , uint64_t timeSinceEpoch
                      , uint64_t firstValidPoint
                      , double timeSinceInject
                      , const std::shared_ptr< const identify >& id
                      , std::unique_ptr< int32_t [] >& data, size_t size, bool invert ); // software averager support

            waveform();

            waveform( const waveform&, int dataType );

            waveform& operator += ( const waveform& );

            method method_;
            metadata meta_;
            uint32_t serialnumber_;
            uint32_t wellKnownEvents_;
            uint64_t timeSinceEpoch_;
            uint64_t firstValidPoint_;
            double timeSinceInject_;

            size_t size() const; // number of samples

            int dataType() const; // 2 = int16_t, 4 = int32_t
            
            // 32bit interface
            void setData( const std::shared_ptr< adportable::mblock<int32_t> >&, size_t firstValidPoint );

            // 16bit interface
            void setData( const std::shared_ptr< adportable::mblock<int16_t> >&, size_t firstValidPoint );

            int64_t operator [] ( size_t idx ) const;
            std::pair<double, int64_t> xy( size_t idx ) const;
            double toVolts( int32_t ) const;
            double toVolts( int64_t ) const;
            double toVolts( double ) const;
            double time( size_t idx ) const;
            bool isDEAD() const;

            const identify* ident() const { return ident_.get(); }

            const std::shared_ptr< const identify >& ident_ptr() const { return ident_; }
            
            template< typename value_type > const value_type* begin() const;
            template< typename value_type > const value_type* end() const;
            template< typename value_type > value_type* begin();
            template< typename value_type > value_type* end();

            template< typename value_type > const value_type* data() const;
			template< typename value_type > value_type* data();

            size_t serialize_xmeta( std::string& ) const;
            size_t serialize_xdata( std::string& ) const;
            bool deserialize_xmeta( const char *, size_t );
            bool deserialize_xdata( const char *, size_t );
            bool deserialize( const char * xdata, size_t dsize, const char * xmeta, size_t msize );

            double accumulate( double tof, double window ) const;

            static bool apply_filter( std::vector<double>&, const waveform&, const adcontrols::threshold_method& );

            static bool transform( std::vector<double>&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
            
            static bool translate( adcontrols::MassSpectrum&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...

            static bool translate( adcontrols::MassSpectrum&, const threshold_result&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...

            typedef double( mass_assign_t )( double time, int mode );
            typedef std::function< mass_assign_t > mass_assignor_t;

            static bool translate( adcontrols::MassSpectrum&, const waveform&, mass_assignor_t, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
            static bool translate( adcontrols::MassSpectrum&, const threshold_result&, mass_assignor_t, int scale = 1000 );
            
        private:
            friend class waveform_xmeta_archive< waveform >;
            friend class waveform_xmeta_archive< const waveform >;
            friend class waveform_xdata_archive< waveform >;
            friend class waveform_xdata_archive< const waveform >;

            std::shared_ptr< const identify > ident_;
            boost::variant < std::shared_ptr< adportable::mblock<int16_t> >
                             , std::shared_ptr< adportable::mblock<int32_t> >
                             , std::shared_ptr< adportable::mblock<int64_t> >
                             > mblock_;

            template< typename lvalue_type
                      , typename rvalue_type > void add( const waveform& t, double dbase ) {
                std::transform( t.begin<rvalue_type>(), t.end<rvalue_type>(), this->data<lvalue_type>(), this->data<lvalue_type>()
                                , [&]( const rvalue_type& a, const lvalue_type& b ){ return lvalue_type( a + b - dbase ); } );                
            }
        };

        template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::end() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::end() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int64_t * waveform::begin() const;
        template<> ACQRSCONTROLSSHARED_EXPORT const int64_t * waveform::end() const;

		template<> ACQRSCONTROLSSHARED_EXPORT int16_t * waveform::data();
		template<> ACQRSCONTROLSSHARED_EXPORT const int16_t * waveform::data() const;
        template<> ACQRSCONTROLSSHARED_EXPORT int32_t * waveform::data();
        template<> ACQRSCONTROLSSHARED_EXPORT const int32_t * waveform::data() const;
        template<> ACQRSCONTROLSSHARED_EXPORT int64_t * waveform::data();
        template<> ACQRSCONTROLSSHARED_EXPORT const int64_t * waveform::data() const;
    }
}

BOOST_CLASS_VERSION( acqrscontrols::u5303a::device_data, 1 )

