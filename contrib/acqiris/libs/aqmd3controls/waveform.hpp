/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "aqmd3controls_global.hpp"
#include "meta_data.hpp"
#include "method.hpp"
#include <adportable/basic_waveform.hpp>
#include <adportable/mass_assign_t.hpp>
#include <boost/variant.hpp>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace adcontrols { class MassSpectrum; class ScanLaw; }
namespace adacquire { namespace SignalObserver { class DataReadBuffer; } }
namespace adportable { template<typename T> class mblock; }

namespace aqmd3controls {

    class identify;
    class meta_data;
    class method;
    class threshold_result;

    class AQMD3CONTROLSSHARED_EXPORT waveform;

    //  Digitizer or soft averaged data to volts
    template< typename T, method::DigiMode = method::DigiMode::Digitizer > struct toVolts_ {
        inline double operator()( const meta_data& meta, T d ) const {
            int actualAverages = meta.actualAverages == 0 ? 1 : meta.actualAverages;
            return double( meta.scaleFactor * d ) / actualAverages + meta.scaleOffset;
        }
    };

    //  Averaged data to volts
    template< typename T > struct toVolts_<T, method::DigiMode::Averager > {
        inline double operator()( const meta_data& meta, T d ) const {
            return d * meta.scaleFactor + meta.scaleOffset;
        }
    };


    class waveform : public std::enable_shared_from_this < waveform >
                   , public adportable::basic_waveform< int32_t, meta_data > {

        void operator = ( const waveform& ) = delete;

    public:
        typedef double( mass_assign_t )( double time, int mode );
        typedef std::function< mass_assign_t > mass_assignor_t;

        waveform();
        waveform( const waveform& );
        waveform( uint32_t pos
                  , uint32_t fcn
                  , uint32_t serialnumber
                  , uint32_t wellKnownEvents
                  , uint64_t timepoint
                  , uint64_t elapsed_time
                  , uint64_t epoch_time
            );

        waveform( uint32_t pos, const meta_data& );

        // see u5303a/waveform
        waveform( std::shared_ptr< const aqmd3controls::identify > id, uint32_t pos, uint32_t events = 0, uint64_t tp = 0 );

        void set_is_pkd( bool );
        bool is_pkd() const;

        void set_xmeta( const meta_data& ) override;

        // size_t serialize_xdata( std::string& ar ) const override;

        size_t serialize_xmeta( std::string& ar ) const override;
        bool deserialize_xmeta( const char * data, size_t size ) override;

        size_t serialize_xdata( std::string& d ) const override;
        bool deserialize_xdata( const char * data, size_t size ) override;

        void set_trigger_delay( double );

        void set_elapsed_time( uint64_t value );
        //double trigger_delay() const;
        //double xIncrement() const;
        static double toVolts( int32_t, size_t actual_averages = 0 );
        //static int32_t adCounts( double );

        double time( size_t ) const;
        std::pair< double, uint64_t > xy( uint32_t ) const;

        bool operator += ( const waveform& );

        static bool transform( std::vector<double>&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...

        static bool translate( adcontrols::MassSpectrum&, const waveform&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
        static bool translate( adcontrols::MassSpectrum&, const threshold_result&, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...

        static bool translate( adcontrols::MassSpectrum&, const waveform&, mass_assignor_t, int scale = 1000 ); // 0 := binary, 1 = Volts, 1000 = mV ...
        static bool translate( adcontrols::MassSpectrum&, const threshold_result&, mass_assignor_t, int scale = 1000 );

        ////////////////////////////
        template< typename value_type > const value_type* begin() const;
        template< typename value_type > const value_type* end() const;
        template< typename value_type > value_type* begin();
        template< typename value_type > value_type* end();

        template< typename value_type > const value_type* data() const;
        template< typename value_type > value_type* data();

        // 32bit interface
        void setData( const std::shared_ptr< adportable::mblock<int32_t> >&, size_t firstValidPoint );

        // 16bit interface
        void setData( const std::shared_ptr< adportable::mblock<int16_t> >&, size_t firstValidPoint );

        void set_method( const aqmd3controls::method& );
        const aqmd3controls::method& method() const;

    private:
        boost::variant < std::shared_ptr< adportable::mblock<int16_t> >
                         , std::shared_ptr< adportable::mblock<int32_t> >
                         , std::shared_ptr< adportable::mblock<int64_t> >
                         > mblock_;

        double trigger_delay_;
        bool is_pkd_;
        std::unique_ptr< aqmd3controls::method > method_;

        template< typename lvalue_type
                  , typename rvalue_type > void add( const waveform& t, double dbase ) {
            std::transform( t.begin<rvalue_type>(), t.end<rvalue_type>(), this->data<lvalue_type>(), this->data<lvalue_type>()
                            , [&]( const rvalue_type& a, const lvalue_type& b ){ return lvalue_type( a + b - dbase ); } );
        }

        template< typename lvalue_type
                  , typename rvalue_type > void sub( const waveform& t ) {
            std::transform( t.begin<rvalue_type>(), t.end<rvalue_type>(), this->data<lvalue_type>(), this->data<lvalue_type>()
                            , [&]( const rvalue_type& a, const lvalue_type& b ){ return lvalue_type( b - a ); } );
        }
    };

    template<> AQMD3CONTROLSSHARED_EXPORT const int8_t * waveform::begin() const;
    template<> AQMD3CONTROLSSHARED_EXPORT const int8_t * waveform::end() const;
    template<> AQMD3CONTROLSSHARED_EXPORT const int16_t * waveform::begin() const;
    template<> AQMD3CONTROLSSHARED_EXPORT const int16_t * waveform::end() const;
    template<> AQMD3CONTROLSSHARED_EXPORT const int32_t * waveform::begin() const;
    template<> AQMD3CONTROLSSHARED_EXPORT const int32_t * waveform::end() const;
    template<> AQMD3CONTROLSSHARED_EXPORT const int64_t * waveform::begin() const;
    template<> AQMD3CONTROLSSHARED_EXPORT const int64_t * waveform::end() const;

    template<> AQMD3CONTROLSSHARED_EXPORT int16_t * waveform::data();
    template<> AQMD3CONTROLSSHARED_EXPORT const int16_t * waveform::data() const;
    template<> AQMD3CONTROLSSHARED_EXPORT int32_t * waveform::data();
    template<> AQMD3CONTROLSSHARED_EXPORT const int32_t * waveform::data() const;
    template<> AQMD3CONTROLSSHARED_EXPORT int64_t * waveform::data();
    template<> AQMD3CONTROLSSHARED_EXPORT const int64_t * waveform::data() const;

}
