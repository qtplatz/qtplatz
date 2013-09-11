// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "msassignedmass.hpp"
#include <adportable/float.hpp>
#include <boost/bind.hpp>


using namespace adcontrols;

MSAssignedMass::MSAssignedMass() : idReferences_(-1)
                                 , peak_index_( peak_index_type( -1, -1 ) ) 
                                 , exactMass_( 0 ) 
                                 , time_( 0 )
                                 , mass_( 0 )  
                                 , enable_( true ) 
                                 , flags_( 0 )
                                 , mode_( 0 )
{
}

MSAssignedMass::MSAssignedMass( const MSAssignedMass& t ) : formula_( t.formula_ )
                                                          , idReferences_( t.idReferences_ )
                                                          , peak_index_( t.peak_index_ )
                                                          , exactMass_( t.exactMass_ )
                                                          , time_( t.time_ )
                                                          , mass_( t.mass_ )   
                                                          , enable_( t.enable_ )
                                                          , flags_( t.flags_ )
                                                          , mode_( t.mode_ )
{
}

MSAssignedMass::MSAssignedMass( unsigned int idReferences
                                , unsigned int idMassSpectrum
                                , const std::wstring& formula
                                , double exactMass
                                , double time
                                , double mass
                                , bool enable
                                , unsigned int flags
                                , unsigned int mode ) : formula_( formula )
                                                        , idReferences_( idReferences )
                                                        , peak_index_( peak_index_type( 0, idMassSpectrum ) )
                                                        , exactMass_( exactMass )
                                                        , time_( time )   
                                                        , mass_( mass )
                                                        , enable_( enable ) 
                                                        , flags_( flags )
                                                        , mode_( mode )
{
}

MSAssignedMass::MSAssignedMass( unsigned int idReferences
                                , const peak_index_type& idx
                                , const std::wstring& formula
                                , double exactMass
                                , double time
                                , double mass
                                , bool enable
                                , unsigned int flags
                                , unsigned int mode ) : formula_( formula )
                                                      , idReferences_( idReferences )
                                                      , peak_index_( idx )
                                                      , exactMass_( exactMass )
                                                      , time_( time )   
                                                      , mass_( mass )
                                                      , enable_( enable ) 
                                                      , flags_( flags )
                                                      , mode_( mode )
{
}

unsigned int
MSAssignedMass::idReferences() const
{
    return idReferences_;
}

void
MSAssignedMass::idReferences( unsigned int value )
{
    idReferences_ = value;
}

unsigned int
MSAssignedMass::idMassSpectrum() const
{
    return peak_index_.second;
}

void
MSAssignedMass::idMassSpectrum( unsigned int value )
{
	peak_index_.second = value;
	peak_index_.first = 0; // fcn
}

const peak_index_type&
MSAssignedMass::peak_index() const
{
    return peak_index_;
}

void
MSAssignedMass::peak_index( const peak_index_type& idx )
{
    peak_index_ = idx;
}

double
MSAssignedMass::exactMass() const
{
    return exactMass_;
}

void
MSAssignedMass::exactMass( double value )
{
    exactMass_ = value;
}

double
MSAssignedMass::time() const
{
    return time_;
}

void
MSAssignedMass::time( double value )
{
    time_ = value;
}

double
MSAssignedMass::mass() const
{
    return mass_;
}

void
MSAssignedMass::mass( double value )
{
    mass_ = value;
}

bool
MSAssignedMass::enable() const
{
    return enable_;
}

void
MSAssignedMass::enable( bool value )
{
    enable_ = value;
}

unsigned int
MSAssignedMass::flags() const
{
    return flags_;
}

unsigned int
MSAssignedMass::mode() const
{
    return mode_;
}

void
MSAssignedMass::flags( unsigned int value )
{
    flags_ = value;
}

void
MSAssignedMass::mode( unsigned int value )
{
    mode_ = value;
}

const std::wstring&
MSAssignedMass::formula() const
{
    return formula_;
}

void
MSAssignedMass::formula( const std::wstring& value )
{
    formula_ = value;
}

/////////////////////

MSAssignedMasses::MSAssignedMasses()
{
}

MSAssignedMasses::MSAssignedMasses( const MSAssignedMasses& t ) : vec_( t.vec_ )
{
}

MSAssignedMasses&
MSAssignedMasses::operator << ( const MSAssignedMass& t )
{
    vec_.push_back( t );
    return *this;
}

namespace adcontrols { namespace internal {
        struct pred {
            const MSAssignedMass& t_;
            pred( const MSAssignedMass& t ) : t_( t ) {}
            bool operator () ( const MSAssignedMass& a ) const { 
                return t_.mode() == a.mode() &&
                    adportable::compare<double>::essentiallyEqual( t_.exactMass(), a.exactMass() );
            }
        };
    }
}

bool
MSAssignedMasses::operator += ( const MSAssignedMass& t )
{
    if ( std::find_if( vec_.begin(), vec_.end(), internal::pred( t ) ) == vec_.end() ) {
        vec_.push_back( t );
        return true;
    }
    return false;
}

MSAssignedMasses&
MSAssignedMasses::operator += ( const MSAssignedMasses& t )
{
    for ( vector_type::const_iterator it = t.begin(); it != t.end(); ++it )
        *this += *it;
    return *this;
}

MSAssignedMasses::vector_type::iterator
MSAssignedMasses::begin()
{
    return vec_.begin();
}

MSAssignedMasses::vector_type::iterator
MSAssignedMasses::end()
{
    return vec_.end();
}

MSAssignedMasses::vector_type::const_iterator
MSAssignedMasses::begin() const
{
    return vec_.begin();
}

MSAssignedMasses::vector_type::const_iterator
MSAssignedMasses::end() const
{
    return vec_.end();
}
