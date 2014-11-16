/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "retentiontime.hpp"

using namespace adcontrols;

RetentionTime::RetentionTime() : algo_( ParaboraFitting )
{
}

RetentionTime::RetentionTime( const RetentionTime& t ) : algo_( t.algo_ )
                                                       , threshold_( t.threshold_ )
                                                       , boundary_( t.boundary_ )
                                                       , eq_( t.eq_ )
{
}

void
RetentionTime::setAlgorithm( algo a )
{
    algo_ = a;
}

void
RetentionTime::setThreshold( double left, double right )
{
    threshold_ = std::make_pair( left, right );
}

void
RetentionTime::setBoundary( double left, double right )
{
    boundary_ = std::make_pair( left, right );
}


RetentionTime::algo
RetentionTime::algorithm() const
{
    return algo_;
}

double
RetentionTime::threshold( int idx ) const
{
    return ( idx == 0 ) ? threshold_.first : threshold_.second;
        
}

double
RetentionTime::boundary( int idx ) const
{
    return ( idx == 0 ) ? boundary_.first : boundary_.second;
}

void
RetentionTime::setEq( double a, double b, double c )
{
    eq_.clear();
    eq_.push_back( a );
    eq_.push_back( b );
    eq_.push_back( c );
}

bool
RetentionTime::eq( double& a, double& b, double& c ) const
{
    if ( eq_.size() >= 3 ) {
        a = eq_[ 0 ];
        b = eq_[ 1 ];
        c = eq_[ 2 ];
        return true;
    }
    return false;
}

