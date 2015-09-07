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

#include "ap240spectrometer_global.hpp"
#include "method.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace ap240 { namespace detail { struct device_ap240; } }
namespace ap240spectrometer { namespace ap240 { class method; } }

namespace ap240spectrometer {
    
    class AP240SPECTROMETERSHARED_EXPORT identify {
    public:
        identify();
        identify( const identify& );
        uint32_t bus_number_;
        uint32_t slot_number_;
        uint32_t serial_number_;
        
    private:
        class impl;
        friend class impl;
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
    
    class AP240SPECTROMETERSHARED_EXPORT metadata {
    public:
        metadata() : initialXTimeSeconds( 0 )
            , actualPoints( 0 )
            , flags( 0 )
            , actualAverages( 0 )
            , initialXOffset( 0 )
            , xIncrement( 0 )
            , scaleFactor( 0 )
            , scaleOffset(0)
            , horPos( 0 )
            , indexFirstPoint(0)
            , channel( 1 )
            , dataType( 1 )
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
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( actualPoints );
            ar & BOOST_SERIALIZATION_NVP( flags );            
            ar & BOOST_SERIALIZATION_NVP( actualAverages );
            ar & BOOST_SERIALIZATION_NVP( indexFirstPoint );
            ar & BOOST_SERIALIZATION_NVP( channel );
            ar & BOOST_SERIALIZATION_NVP( dataType );
            ar & BOOST_SERIALIZATION_NVP( initialXTimeSeconds );
            ar & BOOST_SERIALIZATION_NVP( initialXOffset );
            ar & BOOST_SERIALIZATION_NVP( xIncrement );
            ar & BOOST_SERIALIZATION_NVP( scaleFactor );
            ar & BOOST_SERIALIZATION_NVP( scaleOffset );
            ar & BOOST_SERIALIZATION_NVP( horPos );
        }
    };
    
#if defined _MSC_VER
    class waveform;
    AP240SPECTROMETERSHARED_TEMPLATE_EXPORT template class AP240SPECTROMETERSHARED_EXPORT std::weak_ptr < waveform > ;
#endif
    
    class AP240SPECTROMETERSHARED_EXPORT waveform : public std::enable_shared_from_this< waveform > {
        waveform( const waveform& ); // = delete;
        void operator = ( const waveform& ); // = delete;
    public:
        waveform( std::shared_ptr< identify >& id ) : ident_( *id ), wellKnownEvents_( 0 ), serialnumber_( 0 ), timeSinceEpoch_( 0 ) {
        }
        
        size_t size() const;
        template<typename T> const T* begin() const;
        template<typename T> const T* end() const;
        
        std::pair<double,int> operator [] ( size_t ) const;
        double toVolts( int ) const;
        double toVolts( double ) const;
        
        ap240x::method method_;
        metadata meta_;
        uint32_t serialnumber_;
        uint32_t wellKnownEvents_;
        uint64_t timeSinceEpoch_;
        identify ident_;
        typedef int32_t value_type; // referenced from archiver in WaveformObserver
        const value_type * data() const { return d_.data(); }
        value_type * data( size_t size ) { d_.resize( size ); return d_.data(); }
    private:
        
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
    
    template<> AP240SPECTROMETERSHARED_EXPORT const int8_t * waveform::begin() const;
    template<> AP240SPECTROMETERSHARED_EXPORT const int8_t * waveform::end() const;    
    template<> AP240SPECTROMETERSHARED_EXPORT const int16_t * waveform::begin() const;
    template<> AP240SPECTROMETERSHARED_EXPORT const int16_t * waveform::end() const;    
    template<> AP240SPECTROMETERSHARED_EXPORT const int32_t * waveform::begin() const;
    template<> AP240SPECTROMETERSHARED_EXPORT const int32_t * waveform::end() const;    
}
