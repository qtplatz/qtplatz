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

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>
#include "msreference.hpp"
#include "chemicalformula.hpp"
#include <compiler/diagnostic_pop.h>
#include <sstream>

using namespace adcontrols;

MSReference::MSReference() : enable_( true )
                           , exactMass_(0)  
                           , polarityPositive_( true )
						   , chargeCount_( 1 )
{
}

MSReference::MSReference( const MSReference& t ) : enable_( t.enable_ )
                                                 , exactMass_( t.exactMass_ ) 
                                                 , polarityPositive_( t.polarityPositive_ )
												 , chargeCount_( t.chargeCount_ )
                                                 , formula_( t.formula_ )
                                                 , adduct_or_loss_( t.adduct_or_loss_ )
                                                 , description_( t.description_ )    
{
    if ( exactMass_ <= std::numeric_limits<double>::epsilon() )
        compute_mass();
}

MSReference::MSReference( const std::wstring& formula
                          , bool polarityPositive
                          , const std::wstring& adduct_or_loss
                          , bool enable
                          , double exactMass
                          , uint32_t charge
                          , const std::wstring& description ) : enable_( enable )
                                                              , exactMass_( exactMass ) 
                                                              , chargeCount_( charge )
                                                              , polarityPositive_( polarityPositive )
                                                              , formula_( formula )
                                                              , adduct_or_loss_( adduct_or_loss )
                                                              , description_( description )    
{
	if ( exactMass_ <= std::numeric_limits<double>::epsilon() )
        compute_mass();
}

void
MSReference::compute_mass()
{
    ChemicalFormula formula;
    exactMass_ = formula.getMonoIsotopicMass( formula_ );
    if ( ! adduct_or_loss_.empty() ) {
        std::wstring::size_type sign = adduct_or_loss_.find_first_of( L"+-" );
        if ( sign != std::wstring::npos && adduct_or_loss_[ sign ] == L'-' ) {
            double lose = formula.getMonoIsotopicMass( adduct_or_loss_.substr( sign ) );
            exactMass_ -= lose;
        } else {
            double adduct = formula.getMonoIsotopicMass( adduct_or_loss_ );
            exactMass_ += adduct;
        }
        // handle electron mass
        // if ( polarityPositive_ ) subtract electron
            
    }
}

bool
MSReference::enable() const
{
    return enable_;
}

bool
MSReference::operator < ( const MSReference& t ) const
{
    return exactMass_ < t.exactMass_;
}

double
MSReference::exact_mass() const
{
    return exactMass_;
}

bool
MSReference::polarityPositive() const
{
    return polarityPositive_;
}

const std::wstring&
MSReference::formula() const
{
    return formula_;
}

std::wstring
MSReference::display_formula() const
{
	std::wostringstream wo;
	wo << formula_;
	if ( ! adduct_or_loss_.empty() ) {
        std::wstring::size_type sign = adduct_or_loss_.find_first_of( L"+-" );
        if ( sign == std::wstring::npos ) // if no '+' or '-' specified, assume adduct
            wo << '+' << adduct_or_loss_;
        else
            wo << adduct_or_loss_.substr( sign );
    }
	return wo.str();
}

const std::wstring&
MSReference::adduct_or_loss() const
{
    return adduct_or_loss_;
}

const std::wstring&
MSReference::description() const
{
    return description_;
}

void
MSReference::enable( bool value )
{
    enable_ = value;
}

void
MSReference::exact_mass( double value )
{
    exactMass_ = value;
}

void
MSReference::charge_count( uint32_t v )
{
	chargeCount_ = v;
}

uint32_t
MSReference::charge_count() const
{
	return chargeCount_;
}

void
MSReference::polarityPositive( bool value )
{
    polarityPositive_ = value;
}

void
MSReference::formula( const std::wstring& value )
{
    formula_ = value;
}

void
MSReference::adduct_or_loss( const std::wstring& value )
{
    adduct_or_loss_ = value;
}

void
MSReference::description( const std::wstring& value )
{
    description_ = value;
}

