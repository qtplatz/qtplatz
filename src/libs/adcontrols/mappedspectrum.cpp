/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2016 MS-Cheminformatics LLC
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
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include "samplinginfo.hpp"
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

    template<typename T>
    class MappedSpectrum::serializer {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.data_ );
            ar & BOOST_SERIALIZATION_NVP( _.num_average_ );
            ar & BOOST_SERIALIZATION_NVP( _.trig_number_ );
            ar & BOOST_SERIALIZATION_NVP( _.trig_number_origin_ );
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( _.timeSinceEpoch_ );
                ar & BOOST_SERIALIZATION_NVP( _.sampInterval_ );
                ar & BOOST_SERIALIZATION_NVP( _.delay_ );
                ar & BOOST_SERIALIZATION_NVP( _.nSamples_ );
            }
        }

    };
    
    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MappedSpectrum::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        if ( version >= 2 )
            serializer<const MappedSpectrum>().serialize( ar, *this, version );
        else
            ar & data_;
    }

    template<> void
    MappedSpectrum::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        if ( version >= 2 )
            serializer<MappedSpectrum>().serialize( ar, *this, version );
        else
            ar & data_;
    }

    ///////// XML archive ////////
    template<> void
    MappedSpectrum::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        if ( version >= 2 )
            serializer<const MappedSpectrum>().serialize( ar, *this, version );
        else
            ar & BOOST_SERIALIZATION_NVP( data_ );            
    }

    template<> void
    MappedSpectrum::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        if ( version >= 2 )
            serializer<MappedSpectrum>().serialize( ar, *this, version );
        else
            ar & BOOST_SERIALIZATION_NVP( data_ );
    }
}

using namespace adcontrols;

MappedSpectrum::~MappedSpectrum()
{
}

MappedSpectrum::MappedSpectrum() : num_average_( 0 )
                                 , trig_number_( 0 )
                                 , trig_number_origin_( 0 )
                                 , timeSinceEpoch_( std::make_pair( 0, 0 ) )
                                 , sampInterval_( 20.0e-9 / 16 ) // malpix as default
                                 , delay_( 0 )
                                 , nSamples_( 4096 )             // malpix as default (5ns)
{
}

MappedSpectrum::MappedSpectrum( const MappedSpectrum& t ) : data_( t.data_ )
                                                          , num_average_( t.num_average_ )
                                                          , trig_number_( t.trig_number_ )
                                                          , trig_number_origin_( t.trig_number_origin_ )
                                                          , timeSinceEpoch_( t.timeSinceEpoch_ )
                                                          , sampInterval_( t.sampInterval_ )
                                                          , delay_( t.delay_ )
                                                          , nSamples_( t.nSamples_ )
{
}

MappedSpectrum&
MappedSpectrum::operator = ( const MappedSpectrum& rhs )
{
    data_ = rhs.data_;

    num_average_ = rhs.num_average_;
    trig_number_ = rhs.trig_number_;
    trig_number_origin_ = rhs.trig_number_origin_;
    timeSinceEpoch_ = rhs.timeSinceEpoch_;
    sampInterval_ = rhs.sampInterval_;
    delay_ = rhs.delay_;
    nSamples_ = rhs.nSamples_;

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

void
MappedSpectrum::setNumAverage( uint32_t value )
{
    num_average_ = value;
}

uint32_t
MappedSpectrum::numAverage() const
{
    return num_average_;
}

void
MappedSpectrum::setTrigNumber( uint32_t value, uint32_t origin )
{
    trig_number_ = value;
    trig_number_origin_ = origin;
}

uint32_t
MappedSpectrum::trigNumber( bool sinceOrigin ) const
{
    return sinceOrigin ? trig_number_ - trig_number_origin_ : trig_number_;
}

uint32_t
MappedSpectrum::trigNumberOrigin() const
{
    return trig_number_origin_;
}

std::pair<uint64_t, uint64_t>& 
MappedSpectrum::timeSinceEpoch()
{
    return timeSinceEpoch_;
}

const std::pair<uint64_t, uint64_t>&
MappedSpectrum::timeSinceEpoch() const
{
    return timeSinceEpoch_;
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
    if ( timeSinceEpoch_.first == 0 )
        timeSinceEpoch_.first = t.timeSinceEpoch_.first;

    timeSinceEpoch_.second = t.timeSinceEpoch_.second;

    if ( trig_number_origin_ == 0 )
        trig_number_origin_ = t.trig_number_origin_;

    trig_number_ = t.trig_number_;

    num_average_ += t.numAverage() ? t.numAverage() : 1;

    if ( data_.empty() ) {
        
        data_ = t.data_;

        sampInterval_ = t.sampInterval_;
        delay_ = t.delay_;
        nSamples_ = t.nSamples_;

    } else {

        for ( auto inIt = t.data_.begin(); inIt != t.data_.end(); ++inIt ) {

            auto it = std::lower_bound( data_.begin(), data_.end(), inIt->first
                                        , [] ( const datum_type& a, const double& b ) { return a.first < b; } );
 
            if ( it != data_.end() ) {
        
                if ( adportable::compare< decltype( datum_type::first ) >::approximatelyEqual( it->first, inIt->first ) )
                    it->second += inIt->second;
                else
                    data_.insert( it, *inIt );
        
            } else {

                while ( inIt != t.data_.end() )
                    data_.emplace_back( *inIt++ );
                break;

            }
        }
    }
    return *this;
}

void
MappedSpectrum::setSamplingInfo( double sampInterval, double delay, uint32_t nSamples )
{
    sampInterval_ = sampInterval;
    delay_ = delay;
    nSamples_ = nSamples;
}

bool
MappedSpectrum::transform( adcontrols::MassSpectrum& ms )
{
    auto& prop = ms.getMSProperty();

    int32_t nDelay = int32_t( ( delay_ / sampInterval_ ) + 0.5 );
    auto si = SamplingInfo( sampInterval_, delay_, nDelay, nSamples_, num_average_, 0 /* mode */ );
    si.fSampInterval( sampInterval_ );
    prop.setSamplingInfo( si );
    prop.setNumAverage( num_average_ );
    prop.setTrigNumber( trig_number_, trig_number_origin_ );
    prop.setTimeSinceEpoch( timeSinceEpoch_.second );

    ms.resize( data_.size() );
    ms.setCentroid( adcontrols::CentroidNative );
    //auto scanlaw = prop.scanLaw();
    
    for ( size_t idx = 0; idx < data_.size(); ++idx ) {
        double tof = this->time( idx );
        ms.setTime( idx, tof );
        //if ( scanlaw )
        //    ms.setMass( idx, ms.compute_mass( data_[ idx ].first ) ); 
        ms.setIntensity( idx, data_[ idx ].second );
    }
    return true;
}

