// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

using namespace adcontrols;

MSAssignedMass::MSAssignedMass() : idReferences_(-1)
                                 , idMassSpectrum_(-1) 
                                 , exactMass_( 0 ) 
                                 , time_( 0 )
                                 , mass_( 0 )  
{
}

MSAssignedMass::MSAssignedMass( const MSAssignedMass& t ) : formula_( t.formula_ )
                                                          , idReferences_( t.idReferences_ )
                                                          , idMassSpectrum_( t.idMassSpectrum_ )
                                                          , exactMass_( t.exactMass_ )
                                                          , time_( t.time_ )
                                                          , mass_( t.mass_ )   
{
}

MSAssignedMass::MSAssignedMass( unsigned int idReferences
                               , unsigned int idMassSpectrum
                               , const std::wstring& formula
                               , double exactMass
                               , double time
                               , double mass ) : formula_( formula )
                                               , idReferences_( idReferences )
                                               , idMassSpectrum_( idMassSpectrum )
                                               , exactMass_( exactMass )
                                               , time_( time )   
                                               , mass_( mass )
{
}

const std::wstring&
MSAssignedMass::formula() const
{
    return formula_;
}

unsigned int
MSAssignedMass::idReferences() const
{
    return idReferences_;
}

unsigned int
MSAssignedMass::idMassSpectrum() const
{
    return idMassSpectrum_;
}

double
MSAssignedMass::exactMass() const
{
    return exactMass_;
}

double
MSAssignedMass::time() const
{
    return time_;
}

double
MSAssignedMass::mass() const
{
    return mass_;
}

void
MSAssignedMass::formula( const std::wstring& value )
{
    formula_ = value;
}

void
MSAssignedMass::idReferences( unsigned int value )
{
    idReferences_ = value;
}

void
MSAssignedMass::idMassSpectrum( unsigned int value )
{
    idMassSpectrum_ = value;
}

void
MSAssignedMass::exactMass( double value )
{
    exactMass_ = value;
}

void
MSAssignedMass::time( double value )
{
    time_ = value;
}

void
MSAssignedMass::mass( double value )
{
    mass_ = value;
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
