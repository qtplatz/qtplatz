/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include <compiler/disable_unused_parameter.h>
#include "targetingmethod.hpp"

using namespace adcontrols;

TargetingMethod::TargetingMethod() : isPositiveIonMode_( true )
                                   , is_use_resolving_power_( false )
                                   , resolving_power_( 10000 )
                                   , peak_width_( 1.0 ) // mDa
                                   , chargeStateMin_( 1 )
                                   , chargeStateMax_( 1 )
                                   , isLowMassLimitEnabled_( false ) // auto
                                   , isHighMassLimitEnabled_( false )
                                   , lowMassLimit_( 1 )
                                   , highMassLimit_( 1000 )
                                   , tolerance_( 10.0 )
{
    adductsPos_.push_back( std::pair< std::wstring, bool >( L"+H+", true ) );
    adductsPos_.push_back( std::pair< std::wstring, bool >( L"+Na+", false ) );
    adductsPos_.push_back( std::pair< std::wstring, bool >( L"+K+", false ) );
    adductsPos_.push_back( std::pair< std::wstring, bool >( L"+Li+", false ) );

    adductsNeg_.push_back( std::pair< std::wstring, bool >( L"-H+", true ) );
    adductsNeg_.push_back( std::pair< std::wstring, bool >( L"+COO-", false ) );
    adductsNeg_.push_back( std::pair< std::wstring, bool >( L"+Cl-", false ) );
}

TargetingMethod::TargetingMethod( const TargetingMethod& t )
{
    operator = ( t );
}

TargetingMethod&
TargetingMethod::operator = ( const TargetingMethod& rhs )
{
    isPositiveIonMode_      = rhs.isPositiveIonMode_;
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
    adductsPos_             = rhs.adductsPos_;
    adductsNeg_             = rhs.adductsNeg_;

	return *this;
}


std::vector< TargetingMethod::value_type >&
TargetingMethod::adducts( bool positive )
{
    return positive ? adductsPos_ : adductsNeg_;
}

const std::vector< TargetingMethod::value_type >&
TargetingMethod::adducts( bool positive ) const
{
    return positive ? adductsPos_ : adductsNeg_;
}

std::pair< unsigned int, unsigned int >
TargetingMethod::chargeState() const
{
    return std::pair< unsigned int, unsigned int >( chargeStateMin_, chargeStateMax_ );
}

void
TargetingMethod::chargeState( unsigned int min, unsigned int max )
{
    chargeStateMin_ = min;
    chargeStateMax_ = max;
}

std::vector< TargetingMethod::value_type >&
TargetingMethod::formulae()
{
    return formulae_;
}

const std::vector< TargetingMethod::value_type >&
TargetingMethod::formulae() const
{
    return formulae_;
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
