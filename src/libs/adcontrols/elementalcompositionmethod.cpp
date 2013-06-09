/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "elementalcompositionmethod.hpp"

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/version.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

#include <compiler/diagnostic_pop.h>

using namespace adcontrols;

ElementalCompositionMethod::ElementalCompositionMethod() : electron_mode_(Even)
                                                         , tolerance_in_ppm_( false )
                                                         , tolerance_mDa_( 5.0 )
                                                         , tolerance_ppm_( 10 )
                                                         , dbe_minimum_( -0.5 )                        
                                                         , dbe_maximum_( 200.0 )
                                                         , numResults_( 100 )
{
    vec_.push_back( CompositionConstraint( "C", 0, 100 ) );
    vec_.push_back( CompositionConstraint( "H", 0, 100 ) );
    vec_.push_back( CompositionConstraint( "N", 0, 100 ) );
    vec_.push_back( CompositionConstraint( "O", 0, 100 ) );
    vec_.push_back( CompositionConstraint( "Na", 0, 100 ) );
    vec_.push_back( CompositionConstraint( "P", 0, 0 ) );
    vec_.push_back( CompositionConstraint( "S", 0, 0 ) );
}

///////////// serialize //////////////////

ElementalCompositionMethod::ElectronMode
ElementalCompositionMethod::electronMode() const
{
    return electron_mode_;
}

void
ElementalCompositionMethod::electronMode( ElectronMode v )
{
    electron_mode_ = v;
}

bool
ElementalCompositionMethod::toleranceInPpm() const
{
    return tolerance_in_ppm_;
}

void
ElementalCompositionMethod::toleranceInPpm( bool v )
{
    tolerance_in_ppm_ = v;
}

double
ElementalCompositionMethod::tolerance( bool ppm ) const
{
    if ( ppm )
        return tolerance_ppm_;
    else
        return tolerance_mDa_;
}

void
ElementalCompositionMethod::tolerance( bool ppm, double v )
{
    if ( ppm )
        tolerance_ppm_ = v;
    else
        tolerance_mDa_ = v;
}

double
ElementalCompositionMethod::dbeMinimum() const
{
    return dbe_minimum_;
}

void
ElementalCompositionMethod::dbeMinimum( double v )
{
    dbe_minimum_ = v;
}

double
ElementalCompositionMethod::dbeMaximum() const
{
    return dbe_maximum_;
}

void
ElementalCompositionMethod::dbeMaximum( double v )
{
    dbe_maximum_ = v;
}

size_t
ElementalCompositionMethod::numResults() const
{
    return numResults_;
}

void
ElementalCompositionMethod::numResults( size_t v )
{
    numResults_ = v;
}

void
ElementalCompositionMethod::addCompositionConstraint( const CompositionConstraint& t )
{
    vec_.push_back( t );
}

ElementalCompositionMethod::vector_type::iterator
ElementalCompositionMethod::erase( vector_type::iterator beg, vector_type::iterator end )
{
    return vec_.erase( beg, end );
}

size_t
ElementalCompositionMethod::size() const
{
    return vec_.size();
}

ElementalCompositionMethod::vector_type::iterator
ElementalCompositionMethod::begin()
{
    return vec_.begin();
}

ElementalCompositionMethod::vector_type::iterator
ElementalCompositionMethod::end()
{
    return vec_.end();
}

ElementalCompositionMethod::vector_type::const_iterator
ElementalCompositionMethod::begin() const
{
    return vec_.begin();
}

ElementalCompositionMethod::vector_type::const_iterator
ElementalCompositionMethod::end() const
{
    return vec_.end();
}

//

ElementalCompositionMethod::CompositionConstraint::CompositionConstraint( 
    const std::string & a
    , size_t minimum
    , size_t maximum ) : atom( a )
                       , numMinimum( minimum )
                       , numMaximum( maximum )
{
}

ElementalCompositionMethod::CompositionConstraint::CompositionConstraint( 
    const CompositionConstraint& t ) : atom( t.atom )
                                     , numMinimum( t.numMinimum )
                                     , numMaximum( t.numMaximum )
{
}
