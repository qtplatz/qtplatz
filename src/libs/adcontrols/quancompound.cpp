/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quancompound.hpp"
#include <adportable/uuid.hpp>

using namespace adcontrols;

QuanCompound::~QuanCompound()
{
}

QuanCompound::QuanCompound( const boost::uuids::uuid& uuid )
    : uuid_( uuid )
    , amounts_( 1 )
    , tR_( 0 )
    , mass_( 0 )
    , isISTD_( false )
    , isLKMSRef_( false )
    , isTimeRef_( false )
    , idISTD_( -1 )
    , criteria_( std::make_pair( 0, 0 ) )
{
}

QuanCompound::QuanCompound() : QuanCompound( adportable::uuid()() )
{
}

QuanCompound::QuanCompound( const QuanCompound& t ) : uuid_( t.uuid_ )
                                                    , display_name_( t.display_name_ )
                                                    , formula_( t.formula_ )
                                                    , amounts_( t.amounts_ )
                                                    , description_( t.description_ )
                                                    , tR_( t.tR_ )
                                                    , mass_( t.mass_ )
                                                    , isISTD_( t.isISTD_ )
                                                    , isLKMSRef_( t.isLKMSRef_ )
                                                    , isTimeRef_( t.isTimeRef_ )
                                                    , idISTD_( t.idISTD_ )
                                                    , criteria_( t.criteria_ )
{
}

QuanCompound
QuanCompound::null()
{
    static boost::uuids::uuid uuid; // all zero
    return QuanCompound( uuid );
}

uint32_t
QuanCompound::row() const
{
    return row_;
}

void
QuanCompound::row( uint32_t t )
{
    row_ = t;
}

const wchar_t *
QuanCompound::display_name() const
{
    return display_name_.c_str();
}

void
QuanCompound::displya_name( const wchar_t * v )
{
    display_name_ = v;
}

const wchar_t *
QuanCompound::description() const
{
    return description_.c_str();
}

void
QuanCompound::description( const wchar_t * v )
{
    description_ = v;
}

const char *
QuanCompound::formula() const
{
    return formula_.c_str();
}

void
QuanCompound::formula( const char * v )
{
    formula_ = v;
}

bool
QuanCompound::isLKMSRef() const
{
    return isLKMSRef_;
}

void
QuanCompound::isLKMSRef( bool f )
{
    isLKMSRef_ = f;
}

bool
QuanCompound::isTimeRef() const
{
    return isTimeRef_;
}

void
QuanCompound::isTimeRef( bool f )
{
    isTimeRef_ = f;
}

bool
QuanCompound::isISTD() const
{
    return isISTD_;
}

void
QuanCompound::isISTD( bool f )
{
    isISTD_ = f;
}

int32_t
QuanCompound::idISTD() const
{
    return idISTD_;
}

void
QuanCompound::idISTD( int32_t v )
{
    idISTD_ = v;
}

void
QuanCompound::levels( size_t v )
{
    amounts_.resize(v);
}

size_t
QuanCompound::levels() const
{
    return amounts_.size();
}

const double *
QuanCompound::amounts() const
{
    return amounts_.data();
}

void
QuanCompound::amounts( const double * d, size_t size )
{
    if ( size != amounts_.size() )
        amounts_.resize( size );
    std::copy( d, d + size, amounts_.begin() );
}

double
QuanCompound::criteria( bool second ) const
{
    return second ? criteria_.second : criteria_.first;
}

void
QuanCompound::criteria( double v, bool second )
{
    if ( second )
        criteria_.second = v;
    else
        criteria_.first = v;
}

