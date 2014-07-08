/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <compiler/disable_unused_parameter.h>
#include "targetingmethod.hpp"

using namespace adcontrols;

TargetingMethod::TargetingMethod( idTarget id ) : idTarget_( id )
                                                , is_use_resolving_power_( false )
                                                , resolving_power_( 10000 )
                                                , peak_width_( 1.0 ) // mDa
                                                , chargeStateMin_( 1 )
                                                , chargeStateMax_( 3 )
                                                , isLowMassLimitEnabled_( false ) // auto
                                                , isHighMassLimitEnabled_( false )
    , lowMassLimit_( 1 )
    , highMassLimit_( 1000 )
    , tolerance_( 10.0 )
{
    // reference, 
    // http://fiehnlab.ucdavis.edu/staff/kind/Metabolomics/MS-Adduct-Calculator/
    pos_adducts_.push_back( std::make_pair( false, "H" ) );
    pos_adducts_.push_back( std::make_pair( false, "Na" ) );
    pos_adducts_.push_back( std::make_pair( false, "NH4" ) );
    pos_adducts_.push_back( std::make_pair( false, "K" ) );
    pos_adducts_.push_back( std::make_pair( false, "CH3CN+H" ) );
    pos_adducts_.push_back( std::make_pair( false, "CH3CN+Na" ) );
    pos_adducts_.push_back( std::make_pair( false, "CH3OH+H" ) );
    pos_adducts_.push_back( std::make_pair( false, "NH4" ) );
    pos_adducts_.push_back( std::make_pair( false, "(CH3)2SO+H" ) ); // DMSO+H
    pos_adducts_.push_back( std::make_pair( false, "C3H8O+H" ) );    // IPA+H
    pos_adducts_.push_back( std::make_pair( false, "C3H8O+Na" ) );   // IPA+Na

    neg_adducts_.push_back( std::make_pair( false, "-H" ) );
    neg_adducts_.push_back( std::make_pair( false, "-H2O-H" ) );
    neg_adducts_.push_back( std::make_pair( false, "+Na-H2" ) );
    neg_adducts_.push_back( std::make_pair( false, "Cl" ) );
    neg_adducts_.push_back( std::make_pair( false, "+K-H2" ) );
    neg_adducts_.push_back( std::make_pair( false, "COOH-H" ) );
}

TargetingMethod::TargetingMethod( const TargetingMethod& t )
{
    operator = ( t );
}

TargetingMethod&
TargetingMethod::operator = ( const TargetingMethod& rhs )
{
    idTarget_ = rhs.idTarget_;

    is_use_resolving_power_ = rhs.is_use_resolving_power_;
    resolving_power_        = rhs.resolving_power_;
    peak_width_             = rhs.peak_width_;
    chargeStateMin_         = rhs.chargeStateMin_;
    chargeStateMax_         = rhs.chargeStateMax_;
    isLowMassLimitEnabled_  = rhs.isLowMassLimitEnabled_;
    isHighMassLimitEnabled_ = rhs.isHighMassLimitEnabled_;
    lowMassLimit_           = rhs.lowMassLimit_;
    highMassLimit_          = rhs.highMassLimit_;
    tolerance_              = rhs.tolerance_;

    formulae_               = rhs.formulae_;
    peptides_               = rhs.peptides_;
    pos_adducts_            = rhs.pos_adducts_;
    neg_adducts_            = rhs.neg_adducts_;

	return *this;
}

void
TargetingMethod::targetId( TargetingMethod::idTarget target )
{
    idTarget_ = target;
}

TargetingMethod::idTarget
TargetingMethod::targetId() const
{
    return idTarget_;
}

std::vector< std::pair< bool, std::string > >&
TargetingMethod::adducts( bool positive )
{
    return positive ? pos_adducts_ : neg_adducts_;
}

const std::vector< std::pair< bool, std::string > >&
TargetingMethod::adducts( bool positive ) const
{
    return positive ? pos_adducts_ : neg_adducts_;
}

std::pair< uint32_t, uint32_t >
TargetingMethod::chargeState() const
{
    return std::pair< uint32_t, uint32_t >( chargeStateMin_, chargeStateMax_ );
}

void
TargetingMethod::chargeState( uint32_t min, uint32_t max )
{
    chargeStateMin_ = min;
    chargeStateMax_ = max;
}

std::vector< TargetingMethod::formula_type >&
TargetingMethod::formulae()
{
    return formulae_;
}

const std::vector< TargetingMethod::formula_type >&
TargetingMethod::formulae() const
{
    return formulae_;
}

std::vector< TargetingMethod::peptide_type >&
TargetingMethod::peptides()
{
    return peptides_;
}

const std::vector< TargetingMethod::peptide_type >&
TargetingMethod::peptides() const
{
    return peptides_;
}

bool
TargetingMethod::is_use_resolving_power() const
{
    return is_use_resolving_power_;
}

void
TargetingMethod::is_use_resolving_power( bool value )
{
    is_use_resolving_power_ = value;
}

double
TargetingMethod::resolving_power() const
{
    return resolving_power_;
}

void
TargetingMethod::resolving_power( double value )
{
    resolving_power_ = value;
}


double
TargetingMethod::peak_width() const
{
    return peak_width_;
}

void
TargetingMethod::peak_width( double value )
{
    peak_width_ = value;
}

std::pair< bool, bool >
TargetingMethod::isMassLimitsEnabled() const
{
    return std::pair<bool, bool>( isLowMassLimitEnabled_, isHighMassLimitEnabled_ );
}

void
TargetingMethod::isLowMassLimitEnabled( bool value )
{
    isLowMassLimitEnabled_ = value;
}

void
TargetingMethod::isHighMassLimitEnabled( bool value )
{
    isHighMassLimitEnabled_ = value;
}
        
double
TargetingMethod::lowMassLimit() const
{
    return lowMassLimit_;
}

void
TargetingMethod::lowMassLimit( double value )
{
    lowMassLimit_ = value;
}

double
TargetingMethod::highMassLimit() const
{
    return highMassLimit_;
}

void
TargetingMethod::highMassLimit( double value )
{
    highMassLimit_ = value;
}


double
TargetingMethod::tolerance() const
{
    return tolerance_;
}

void
TargetingMethod::tolerance( double value )
{
    tolerance_ = value;
}
