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

#include "waveform.hpp"
#include <typeinfo>
#include <cstring>

waveform::waveform( int32_t dataType ) : serialnumber_( 0 )
                                       , wellKnownEvents_( 0 )
                                       , dataType_( dataType )
{
    std::memset( &dataDesc_, 0, sizeof( dataDesc_ ) );
    std::memset( &segDesc_, 0, sizeof( segDesc_ ) );
}

size_t
waveform::size() const
{
    return dataDesc_.returnedSamplesPerSeg;
}

template<> const int8_t *
waveform::begin() const
{
    if ( dataType_ != sizeof(int8_t) )
        throw std::bad_cast();
    return reinterpret_cast< const int8_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
}

template<> const int8_t *
waveform::end() const
{
    return reinterpret_cast< const int8_t* >( d_.data() ) + dataDesc_.indexFirstPoint + dataDesc_.returnedSamplesPerSeg;
}

template<> const int16_t *
waveform::begin() const
{
    if ( dataType_ != sizeof(int16_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int16_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
}

template<> const int16_t *
waveform::end() const
{
    if ( dataType_ != sizeof(int16_t) )
        throw std::bad_cast();
    return reinterpret_cast< const int16_t* >( d_.data() ) + dataDesc_.indexFirstPoint + dataDesc_.returnedSamplesPerSeg;
}

template<> const int32_t *
waveform::begin() const
{
    if ( dataType_ != sizeof(int32_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int32_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
}

template<> const int32_t *
waveform::end() const
{
    if ( dataType_ != sizeof(int32_t) )
        throw std::bad_cast();
    return reinterpret_cast< const int32_t* >( d_.data() ) + dataDesc_.indexFirstPoint + dataDesc_.returnedSamplesPerSeg;
}

waveform::value_type *
waveform::data( size_t size )
{
    d_.resize( size );
    return d_.data();
}

const waveform::value_type *
waveform::data() const
{
    return d_.data();
}

size_t
waveform::data_size() const
{
    return d_.size();
}

template<> const int8_t *
waveform::data() const
{
    if ( dataType_ != sizeof(int8_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int8_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
}

template<> int8_t *
waveform::data()
{
    if ( dataType_ != sizeof(int8_t) )
        throw std::bad_cast();        
    return reinterpret_cast< int8_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
}

template<> const int16_t *
waveform::data() const
{
    if ( dataType_ != sizeof(int16_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int16_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
}

template<> int16_t *
waveform::data()
{
    if ( dataType_ != sizeof(int16_t) )
        throw std::bad_cast();        
    return reinterpret_cast< int16_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
}

template<> const int32_t *
waveform::data() const
{
    if ( dataType_ != sizeof(int32_t) )
        throw std::bad_cast();        
    return reinterpret_cast< const int32_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
}

template<> int32_t *
waveform::data()
{
    if ( dataType_ != sizeof(int32_t) )
        throw std::bad_cast();        
    return reinterpret_cast< int32_t* >( d_.data() ) + dataDesc_.indexFirstPoint;
}

int
waveform::dataType() const
{
    return dataType_;
}

int
waveform::operator [] ( size_t idx ) const
{
    switch( dataType_ ) {
    case 1: return *(begin<int8_t>()  + idx);
    case 2: return *(begin<int16_t>() + idx);
    case 4: return *(begin<int32_t>() + idx);
    }
}

