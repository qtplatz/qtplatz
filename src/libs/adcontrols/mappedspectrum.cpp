/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC
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

#include "mappedspectrum.hpp"

#include <adcontrols/idaudit.hpp>
#include <adportable/float.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MappedSpectrum::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & data_;
    }

    template<> void
    MappedSpectrum::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ar & data_;            
    }

    ///////// XML archive ////////
    template<> void
    MappedSpectrum::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & BOOST_SERIALIZATION_NVP(data_);                        
    }

    template<> void
    MappedSpectrum::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ar & BOOST_SERIALIZATION_NVP(data_);            
    }
}

using namespace adcontrols;

MappedSpectrum::~MappedSpectrum()
{
}

MappedSpectrum::MappedSpectrum()
{
}

MappedSpectrum::MappedSpectrum( const MappedSpectrum& t ) : data_( t.data_ )
{
}

MappedSpectrum&
MappedSpectrum::operator = ( const MappedSpectrum& rhs )
{
    data_ = rhs.data_;
    return *this;
}

size_t
MappedSpectrum::size() const
{
    return data_.size();
}

const MappedSpectrum::datum_type&
MappedSpectrum::operator []( size_t idx ) const
{
    return data_[ idx ];
}
            
MappedSpectrum::iterator
MappedSpectrum::begin()
{
    return data_.begin();
}

MappedSpectrum::iterator
MappedSpectrum::end()
{
    return data_.end();    
}

MappedSpectrum::const_iterator
MappedSpectrum::begin() const
{
    return data_.begin();
}

MappedSpectrum::const_iterator
MappedSpectrum::end() const
{
    return data_.end();        
}

MappedSpectrum::iterator
MappedSpectrum::erase( iterator first, iterator last )
{
    return data_.erase( first, last );
}


double
MappedSpectrum::tic() const
{
    double sum(0);
    for ( const auto& x: data_ )
        sum += x.second;
    return sum;
}

MappedSpectrum&
MappedSpectrum::operator << ( const datum_type& t )
{
    if ( data_.empty() ) {

        data_.push_back( t );

    } else {

        auto it = std::lower_bound( data_.begin(), data_.end(), t.first
                                    , [] ( const datum_type& a, const double& b ) { return a.first < b; } );

        if ( it != data_.end() ) {

            if ( adportable::compare< decltype( datum_type::first ) >::approximatelyEqual( it->first, t.first ) )
                it->second += t.second;
            else
                data_.insert( it, t );

        } else {

            data_.push_back( t );                

        }
    }
    return *this;
}

MappedSpectrum&
MappedSpectrum::operator += ( const MappedSpectrum& t )
{
    if ( data_.empty() ) {

        data_ = t.data_;

    } else {

        for ( auto inIt = t.data_.begin(); inIt != t.data_.end(); ++inIt ) {

            auto it = std::lower_bound( data_.begin(), data_.end(), inIt->first, [] ( const datum_type& a, const double& b ) { return a.first < b; } );
 
            if ( it != data_.end() ) {
        
                if ( adportable::compare< decltype( datum_type::first ) >::approximatelyEqual( it->first, inIt->first ) )
                    it->second += inIt->second;
                else
                    data_.insert( it, *inIt );
        
            } else {

                while ( inIt != t.data_.end() )
                    data_.push_back( *inIt++ );
                break;

            }
        }
    }
    return *this;
}
