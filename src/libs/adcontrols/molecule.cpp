/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "molecule.hpp"
#include "chemicalformula.hpp"
#include "tableofelement.hpp"
#include <adportable/debug.hpp>
#include <algorithm>

using namespace adcontrols;
using namespace adcontrols::mol;

molecule::molecule() : charge_( 0 )
                     , mass_( 0 )
{
}

molecule::molecule( const molecule& t ) : cluster_( t.cluster_ )
                                        , elements_( t.elements_ )
                                        , charge_( t.charge_ )
                                        , display_formula_( t.display_formula_ )
                                        , mass_( t.mass_ )
{
}

molecule&
molecule::operator << ( const element& t )
{
    elements_.emplace_back( t );
    return *this;
}

molecule&
molecule::operator << ( element&& t )
{
    elements_.emplace_back( t );
    return *this;
}

molecule&
molecule::operator << ( const isotope& t )
{
    cluster_.emplace_back( t );
    return *this;
}

molecule&
molecule::operator << ( isotope&& t )
{
    cluster_.emplace_back( t );
    return *this;
}

molecule&
molecule::operator += ( const molecule& t )
{
    for ( auto it = t.elements_begin(); it != t.elements_end(); ++it ) {
        auto xIt = std::find_if( elements_.begin(), elements_.end(), [&]( const auto& e ){ return e.atomicNumber() == it->atomicNumber(); });
        if ( xIt == elements_.end() ) {
            elements_.emplace_back( *it );
        } else {
            xIt->count( xIt->count() + it->count() );
        }
    }
    charge_ += t.charge();
    return *this;
}

molecule&
molecule::operator *= ( size_t n )
{
    if ( n > 1 ) {
        std::for_each( elements_.begin(), elements_.end(), [n](auto& e){ e.count( e.count() * n ); });
        charge_ *= n;
    }
    return *this;
}

molecule
molecule::operator * ( size_t n ) const
{
    molecule t(*this);
    t *= n;
    return t;
}


int
molecule::charge() const
{
    return charge_;
}

void
molecule::setCharge( int value )
{
    charge_ = value;
}

void
molecule::clear()
{
    charge_ = 0;
    mass_ = 0;
    elements_.clear();
    cluster_.clear();
}

std::vector< isotope >::const_iterator
molecule::max_abundant_isotope() const
{
    if ( cluster_.empty() )
        return cluster_.end();
    return std::max_element( cluster_.begin(), cluster_.end()
                             , []( const auto& a, const auto& b ){ return a.abundance < b.abundance; });
}

std::string
molecule::formula( bool handleCharge ) const
{
    std::ostringstream o;
    if ( charge_ && handleCharge )
        o << "[";
    for ( const auto& e: elements_ ) {
        o << e.symbol();
        if ( e.count() > 1 )
            o << e.count();
    }
    if ( charge_ && handleCharge ) {
        o << "]";
        if ( std::abs( charge_ ) > 1 )
            o << std::abs( charge_ );
        o << ( charge_ > 0 ? '+' : '-' );
    }
    return o.str();
}

void
molecule::set_display_formula( const std::string& s )
{
    display_formula_ = s;
}

const std::string&
molecule::display_formula() const
{
    return display_formula_;
}

void
molecule::setMass( double value )
{
    mass_ = value;
}

double
molecule::mass( bool handleCharge ) const
{
    if ( handleCharge ) {
        if ( charge_ > 0 ) {
            return ( mass_ - ( adcontrols::TableOfElement::instance()->electronMass() * charge_ ) ) / charge_;
        } else if ( charge_ < 0 ) {
            return ( mass_  + ( adcontrols::TableOfElement::instance()->electronMass() * std::abs(charge_) ) ) / std::abs(charge_);
        }
    }
    return mass_;
}
