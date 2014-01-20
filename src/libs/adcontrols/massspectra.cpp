// This is a -*- C++ -*- header.
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

#include "massspectra.hpp"
#include "massspectrum.hpp"
#include "chromatogram.hpp"
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <fstream>

using namespace adcontrols;

MassSpectra::~MassSpectra()
{
}

MassSpectra::MassSpectra() : upper_mass_( 0 )
                           , lower_mass_( 0 )
                           , z_max_( 0 )
{
}

MassSpectra::MassSpectra( const MassSpectra& t ) : vec_( t.vec_ )
                                                 , x_( t.x_ )
                                                 , upper_mass_( t.upper_mass_ )
                                                 , lower_mass_( t.lower_mass_ )
                                                 , z_max_( t.z_max_ )
{
}

MassSpectra&
MassSpectra::operator << ( const MassSpectrum& t )
{
    (*this) << ( std::make_shared< MassSpectrum >( t ) ); // create deep copy
	return *this;
}

MassSpectra&
MassSpectra::operator << ( value_type v )
{
    vec_.push_back( v ); // keep shared object pointer
    const std::pair< double, double >& range = v->getAcquisitionMassRange();
    double z_max = segments_helper::max_intensity( *v );
    if ( vec_.empty() ) {
        lower_mass_ = range.first;
        upper_mass_ = range.second;
        z_max_ = z_max;
    } else {
        lower_mass_ = std::min( lower_mass_, range.first );
        upper_mass_ = std::max( upper_mass_, range.second );
        z_max_ = std::max( z_max_, z_max );
    }
	return *this;
}

MassSpectrum&
MassSpectra::operator [] ( size_t fcn )
{
    return *vec_[ fcn ];
}

const MassSpectrum&
MassSpectra::operator [] ( size_t fcn ) const
{
    return *vec_[ fcn ];
}

void
MassSpectra::setChromatogram( const Chromatogram& c )
{
    x_.clear();
    if ( const double * tarray = c.getTimeArray() ) {
        for ( int i = 0; i < c.size(); ++i )
            x_.push_back( tarray[i] / 60.0 ); //--> min
    } else {
        for ( int i = 0; i < c.size(); ++i )
            x_.push_back( c.timeFromSampleIndex( i ) / 60.0 ); // --> min
    }
}

void
MassSpectra::lower_mass( double v )
{
    lower_mass_ = v;
}

void
MassSpectra::upper_mass( double v )
{
    upper_mass_ = v;
}

double
MassSpectra::lower_mass() const
{
    return lower_mass_;
}

double
MassSpectra::upper_mass() const
{
    return upper_mass_;
}

double
MassSpectra::x_left() const
{
    return x_.front();
}

double
MassSpectra::x_right() const
{
	return x_.back();
}

double
MassSpectra::z_max() const
{
    return z_max_;
}

void
MassSpectra::z_max( double v )
{
    z_max_ = v;
}

const std::vector<double>&
MassSpectra::x() const
{
    return x_;
}

bool
MassSpectra::archive( std::ostream& os, const MassSpectra& v )
{
    portable_binary_oarchive ar( os );
    ar << v;
    return true;
}

bool
MassSpectra::restore( std::istream& is, MassSpectra& v )
{
    portable_binary_iarchive ar( is );
    ar >> v;
    return true;
}
