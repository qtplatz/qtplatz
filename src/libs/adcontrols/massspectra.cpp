// This is a -*- C++ -*- header.
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

#include "massspectra.hpp"
#include "massspectrum.hpp"
#include "chromatogram.hpp"
#include "description.hpp"
#include "descriptions.hpp"
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MassSpectra::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        ar & lower_mass_ & upper_mass_ & z_max_ & x_ & vec_;
        ar & *descriptions_;
        ar & mslocked_;
    }

    template<> void
    MassSpectra::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ar & lower_mass_ & upper_mass_ & z_max_ & x_ & vec_;
        if ( version >= 2 )
            ar & *descriptions_;
        if ( version >= 3 )
            ar & mslocked_;
    }

}

using namespace adcontrols;

MassSpectra::~MassSpectra()
{
}

MassSpectra::MassSpectra() : lower_mass_( 0 )
                           , upper_mass_( 0 )
                           , z_max_( 0 )
                           , descriptions_( std::make_unique< descriptions >() )
                           , mslocked_( false )
{
}

MassSpectra::MassSpectra( const MassSpectra& t ) : vec_( t.vec_ )
                                                 , x_( t.x_ )
                                                 , lower_mass_( t.lower_mass_ )
                                                 , upper_mass_( t.upper_mass_ )
                                                 , z_max_( t.z_max_ )
                                                 , mslocked_( t.mslocked_ )
{
}

MassSpectra&
MassSpectra::operator << ( const MassSpectrum& t )
{
    (*this) << ( std::make_shared< MassSpectrum >( t ) ); // create deep copy
	return *this;
}

MassSpectra&
MassSpectra::operator << ( value_type&& v )
{
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
    vec_.emplace_back( v ); // shared object pointer
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

size_t
MassSpectra::size() const
{
    return vec_.size();
}

void
MassSpectra::setChromatogram( const Chromatogram& c )
{
    x_.clear();
    if ( const double * tarray = c.getTimeArray() ) {
        for ( size_t i = 0; i < c.size(); ++i )
            x_.push_back( tarray[i] / 60.0 ); //--> min
    } else {
        for ( size_t i = 0; i < c.size(); ++i )
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

const MassSpectra::value_type
MassSpectra::find( double t, bool closest ) const
{
    auto it = std::lower_bound( x_.begin(), x_.end(), t );
    if ( it != x_.end() ) {
        size_t idx = std::distance( x_.begin(), it );
        if ( closest && it != x_.begin() ) {
            if ( std::abs( *it - t ) > std::abs( *( it - 1 ) - t ) )
                --idx;
        }
        return vec_[ idx ];
    }
    return 0;
}

void
MassSpectra::addDescription( const description& t )
{
	descriptions_->append( t );
}

const descriptions&
MassSpectra::getDescriptions() const
{
	return *descriptions_;
}

void
MassSpectra::setMSLocked( bool f )
{
    mslocked_ = f;
}

bool
MassSpectra::mslocked() const
{
    return mslocked_;
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
