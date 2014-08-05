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

#include "quancalibration.hpp"
#include <adportable/uuid.hpp>
#include <adportable/date_string.hpp>
#include <adportable/polfit.hpp>
#include <chrono>
#include <algorithm>
#include <numeric>

using namespace adcontrols;

QuanCalibration::~QuanCalibration()
{
}

QuanCalibration::QuanCalibration() : chisqr_(0)
{
}

QuanCalibration::QuanCalibration( const QuanCalibration& t ) : idCompound_( t.idCompound_ )
                                                             , idTable_( t.idTable_ )
                                                             , formula_( t.formula_ )
                                                             , x_( t.x_ )
                                                             , y_( t.y_ )
                                                             , chisqr_( t.chisqr_ )
                                                             , coefficients_( t.coefficients_ )
{
}

const boost::uuids::uuid&
QuanCalibration::uuid_cmpd_table() const
{
    return idTable_;
}

void
QuanCalibration::uuid_cmpd_table( const boost::uuids::uuid& u )
{
    idTable_ = u;
}

const boost::uuids::uuid&
QuanCalibration::uuid_cmpd() const
{
    return idCompound_;
}

void
QuanCalibration::uuid_cmpd( const boost::uuids::uuid& u )
{
    idCompound_ = u;
}

void
QuanCalibration::formula( const char * value )
{
    formula_ = value;
}

const char *
QuanCalibration::formula() const
{
    return formula_.c_str();
}

void
QuanCalibration::operator << ( const std::pair<double, double>& amount_intensity_pair )
{
    x_.push_back( amount_intensity_pair.second );
    y_.push_back( amount_intensity_pair.first );
}

size_t
QuanCalibration::size() const 
{
    return x_.size();
}

const double * 
QuanCalibration::x() const
{
    return x_.data();
}

const double *
QuanCalibration::y() const
{
    return y_.data();
}

double
QuanCalibration::min_x() const
{
    return *std::min_element( x_.begin(), x_.end() );
}

double
QuanCalibration::max_x() const
{
    return *std::max_element( x_.begin(), x_.end() );
}

bool
QuanCalibration::fit( int nTerm, bool forceOrigin, WEIGHTING wType )
{
    coefficients_.clear();
    if ( nTerm <= 1 ) {
        double sum_x = std::accumulate( x_.begin(), x_.end(), 0.0, [](double a, double b) { return a + b; } );
        double sum_y = std::accumulate( y_.begin(), y_.end(), 0.0, [](double a, double b) { return a + b; } );
        coefficients_.push_back( sum_y / sum_x );
    }
    if ( adportable::polfit::fit( x_.data(), y_.data(), x_.size(), nTerm, coefficients_, chisqr_, adportable::polfit::WeightingType( wType ) ) ) {
        if ( forceOrigin )
            coefficients_[ 0 ] = 0;
        return true;
    }
    return false;
}

double
QuanCalibration::chisqr() const
{
    return chisqr_;
}

double
QuanCalibration::estimate_y( double x ) const
{
    return adportable::polfit::estimate_y( coefficients_, x );
}

const double *
QuanCalibration::coefficients() const
{
    return coefficients_.data();
}

const size_t
QuanCalibration::nTerms() const
{
    return coefficients_.size();
}

