// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "isotopemethod.hpp"

using namespace adcontrols;

IsotopeMethod::Formula::Formula() : chargeState(0)
{
}

IsotopeMethod::Formula::Formula( const Formula& t ) : description( t.description )
                                                    , formula( t.formula )
                                                    , adduct( t.adduct )  
                                                    , chargeState( t.chargeState )
                                                    , relativeAmounts( t.relativeAmounts )
                                                    , positive( t.positive )
{
}

IsotopeMethod::Formula::Formula( const std::wstring& desc
								 , const std::wstring& _formula
                                 , const std::wstring& _adduct
                                 , size_t _chargeState
                                 , double _relativeAmounts
                                 , bool _positive ) : description( desc )
								                    , formula( _formula )
                                                    , adduct( _adduct )
                                                    , chargeState( _chargeState )
                                                    , relativeAmounts( _relativeAmounts )
                                                    , positive( _positive )
{
}

/////////////////////////////////////////////////////////

IsotopeMethod::~IsotopeMethod()
{
}

IsotopeMethod::IsotopeMethod() : polarityPositive_( true )
                               , useElectronMass_( true )
                               , threshold_( 0.1 )
                               , resolution_( 0.05 )   
{
}

IsotopeMethod::IsotopeMethod( const IsotopeMethod& t ) : polarityPositive_( t.polarityPositive_ )
                                                       , useElectronMass_( t.useElectronMass_ )
                                                       , threshold_( t.threshold_ )
                                                       , resolution_( t.resolution_ )
                                                       , formulae_(t.formulae_)
{
}    

IsotopeMethod&
IsotopeMethod::operator = ( const IsotopeMethod& t )
{
    formulae_ = t.formulae_;
    polarityPositive_ = t.polarityPositive_;
    useElectronMass_ = t.useElectronMass_;
    threshold_ = t.threshold_;
    resolution_ = t.resolution_;
    return *this;
}

void
IsotopeMethod::clear()
{
    formulae_.clear();
}

size_t
IsotopeMethod::size() const
{
    return formulae_.size();
}

void
IsotopeMethod::addFormula( const Formula& t )
{
    formulae_.push_back( t );
}


IsotopeMethod::vector_type::const_iterator
IsotopeMethod::begin() const
{
    return formulae_.begin();
}

IsotopeMethod::vector_type::const_iterator
IsotopeMethod::end() const
{
    return formulae_.end();
}

IsotopeMethod::vector_type::iterator
IsotopeMethod::begin()
{
    return formulae_.begin();
}

IsotopeMethod::vector_type::iterator
IsotopeMethod::end()
{
    return formulae_.end();
}
 
bool
IsotopeMethod::polarityPositive() const
{
    return polarityPositive_;
}

void
IsotopeMethod::polarityPositive( bool value )
{
    polarityPositive_ = value;
}

bool
IsotopeMethod::useElectronMass() const
{
    return useElectronMass_;
}

void
IsotopeMethod::useElectronMass( bool value )
{
    useElectronMass_ = value;
}

double
IsotopeMethod::threshold() const
{
    return threshold_;
}

void
IsotopeMethod::threshold( double value )
{
    threshold_ = value;
}

double
IsotopeMethod::resolution() const
{
    return resolution_;
}

void
IsotopeMethod::resolution( double value )
{
    resolution_ = value;
}

