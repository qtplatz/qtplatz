/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "waveform.hpp"
#include <adportable/debug.hpp>
#include <adportable/serializer.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

using namespace ap240spectrometer::ap240;

identify::identify() : bus_number_( 0 )
                     , slot_number_( 0 )
                     , serial_number_( 0 )
{
}

identify::identify( const identify& t ) : bus_number_( t.bus_number_ )
                                        , slot_number_( t.slot_number_ )
                                        , serial_number_( t.serial_number_ )
{
}

size_t
waveform::size() const
{
    return method_.hor_.nbrSamples;
}

template<> const int8_t *
waveform::begin() const
{
    if ( meta_.dataType != sizeof(int8_t) )
        throw std::bad_cast();
    return reinterpret_cast< const int8_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int8_t *
waveform::end() const
{
    if ( meta_.dataType != sizeof(int8_t) )
        throw std::bad_cast();    
    return reinterpret_cast< const int8_t* >( d_.data() ) + meta_.indexFirstPoint + method_.hor_.nbrSamples;
}

template<> const int16_t *
waveform::begin() const
{
    if ( meta_.dataType != sizeof(int16_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int16_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int16_t *
waveform::end() const
{
    if ( meta_.dataType != sizeof(int16_t) )
        throw std::bad_cast();
    return reinterpret_cast< const int16_t* >( d_.data() ) + meta_.indexFirstPoint + method_.hor_.nbrSamples;
}

template<> const int32_t *
waveform::begin() const
{
    if ( meta_.dataType != sizeof(int32_t) )
        throw std::bad_cast();                
    return reinterpret_cast< const int32_t* >( d_.data() ) + meta_.indexFirstPoint;
}

template<> const int32_t *
waveform::end() const
{
    if ( meta_.dataType != sizeof(int32_t) )
        throw std::bad_cast();                    
    return reinterpret_cast< const int32_t* >( d_.data() ) + meta_.indexFirstPoint + method_.hor_.nbrSamples;
}

std::pair<double, int>
waveform::operator [] ( size_t idx ) const
{
    double time = idx * meta_.xIncrement + meta_.horPos;

    if ( method_.hor_.mode == 0 )
        time += method_.hor_.delay;
    else
        time += method_.hor_.nStartDelay * meta_.xIncrement;
    
    switch( meta_.dataType ) {
    case 1: return std::make_pair( time, *(begin<int8_t>()  + idx) );
    case 2: return std::make_pair( time, *(begin<int16_t>() + idx) );
    case 4: return std::make_pair( time, *(begin<int32_t>() + idx) );
    }
    throw std::exception();
}

double
waveform::toVolts( int d ) const
{
    if ( meta_.actualAverages == 0 )
        return meta_.scaleFactor * d - meta_.scaleOffset;
    else
        return double( meta_.scaleFactor * d ) / meta_.actualAverages - ( meta_.scaleOffset * meta_.actualAverages );
}

double
waveform::toVolts( double d ) const
{
    return d * meta_.scaleFactor /  meta_.actualAverages;
}

