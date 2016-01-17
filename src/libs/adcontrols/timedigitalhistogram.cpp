/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC
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

#include "timedigitalhistogram.hpp"
#include "serializer.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <algorithm>
#include <limits>
#include <utility>
#include <numeric>

namespace adcontrols {

    template< typename T = TimeDigitalHistogram >
    class TimeDigitalHistogram_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( _.initialXTimeSeconds_ );
            ar & BOOST_SERIALIZATION_NVP( _.initialXOffset_ );
            ar & BOOST_SERIALIZATION_NVP( _.xIncrement_ );
            ar & BOOST_SERIALIZATION_NVP( _.actualPoints_ );
            ar & BOOST_SERIALIZATION_NVP( _.trigger_count_ );
            ar & BOOST_SERIALIZATION_NVP( _.serialnumber_ );
            ar & BOOST_SERIALIZATION_NVP( _.timeSinceEpoch_ );
            ar & BOOST_SERIALIZATION_NVP( _.wellKnownEvents_ );
            ar & BOOST_SERIALIZATION_NVP( _.histogram_ );
            if ( version >= 1 )
                ar & BOOST_SERIALIZATION_NVP( _.this_protocol_ );
        }
    };
    
    ///////// Portable binary archive ////////    
    template<> ADCONTROLSSHARED_EXPORT void
    TimeDigitalHistogram::serialize( portable_binary_oarchive& ar, const unsigned int version ) {
        TimeDigitalHistogram_archive<>().serialize( ar, *this, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void
    TimeDigitalHistogram::serialize( portable_binary_iarchive& ar, const unsigned int version ) {
        TimeDigitalHistogram_archive<>().serialize( ar, *this, version );
    }
    
    ///////// XML archive ////////
    template<> void
    TimeDigitalHistogram::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        TimeDigitalHistogram_archive<>().serialize( ar, *this, version );
    }

    template<> void
    TimeDigitalHistogram::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        TimeDigitalHistogram_archive<>().serialize( ar, *this, version );        
    }
}

using namespace adcontrols;

TimeDigitalHistogram::TimeDigitalHistogram() : initialXTimeSeconds_( 0 )
                                             , initialXOffset_( 0 )
                                             , xIncrement_( 0 )
                                             , trigger_count_( 0 )
                                             , actualPoints_( 0 )
                                             , serialnumber_( { 0, 0 } )
                                             , timeSinceEpoch_( { 0, 0 } )
                                             , wellKnownEvents_( 0 )
{
}

TimeDigitalHistogram::TimeDigitalHistogram( const TimeDigitalHistogram& t ) : initialXTimeSeconds_( t.initialXTimeSeconds_ )
                                                                            , initialXOffset_( t.initialXOffset_ )
                                                                            , xIncrement_( t.xIncrement_ )
                                                                            , actualPoints_( t.actualPoints_ )
                                                                            , serialnumber_( t.serialnumber_ )
                                                                            , timeSinceEpoch_( t.timeSinceEpoch_ )
                                                                            , trigger_count_( t.trigger_count_ )
                                                                            , wellKnownEvents_( t.wellKnownEvents_ )
                                                                            , histogram_( t.histogram_ )
                                                                            , this_protocol_( t.this_protocol_ )
{
}

double&
TimeDigitalHistogram::initialXTimeSeconds()
{
    return initialXTimeSeconds_;
}

double&
TimeDigitalHistogram::initialXOffset()
{
    return initialXOffset_;
}

double&
TimeDigitalHistogram::xIncrement()
{
    return xIncrement_;
}

uint64_t&
TimeDigitalHistogram::actualPoints()
{
    return actualPoints_;
}

uint64_t&
TimeDigitalHistogram::trigger_count()
{
    return trigger_count_;
}

uint32_t&
TimeDigitalHistogram::wellKnownEvents()
{
    return wellKnownEvents_;
}

double
TimeDigitalHistogram::initialXTimeSeconds() const
{
    return initialXTimeSeconds_;
}

double
TimeDigitalHistogram::initialXOffset() const
{
    return initialXOffset_;
}

double
TimeDigitalHistogram::xIncrement() const
{
    return xIncrement_;
}

uint64_t
TimeDigitalHistogram::actualPoints() const
{
    return actualPoints_;
}

uint64_t
TimeDigitalHistogram::trigger_count() const
{
    return trigger_count_;
}

uint32_t
TimeDigitalHistogram::wellKnownEvents() const
{
    return wellKnownEvents_;
}

std::pair< uint64_t, uint64_t >&
TimeDigitalHistogram::serialnumber()
{
    return serialnumber_;
}

std::pair< uint64_t, uint64_t >&
TimeDigitalHistogram::timeSinceEpoch()
{
    return timeSinceEpoch_;
}

const std::pair< uint64_t, uint64_t >&
TimeDigitalHistogram::serialnumber() const
{
    return serialnumber_;    
}

const std::pair< uint64_t, uint64_t >&
TimeDigitalHistogram::timeSinceEpoch() const
{
    return timeSinceEpoch_;    
}

std::vector< std::pair< double, uint32_t > >&
TimeDigitalHistogram::histogram()
{
    return histogram_;
}

const std::vector< std::pair< double, uint32_t > >&
TimeDigitalHistogram::histogram() const
{
    return histogram_;
}

TofProtocol&
TimeDigitalHistogram::this_protocol()
{
    return this_protocol_;
}

const TofProtocol&
TimeDigitalHistogram::this_protocol() const
{
    return this_protocol_;
}

size_t
TimeDigitalHistogram::size() const
{
    return histogram_.size();
}

const TimeDigitalHistogram::value_type&
TimeDigitalHistogram::operator []( size_t idx ) const
{
    return histogram_[ idx ];
}
            
TimeDigitalHistogram::iterator
TimeDigitalHistogram::begin()
{
    return histogram_.begin();
}

TimeDigitalHistogram::iterator
TimeDigitalHistogram::end()
{
    return histogram_.end();    
}

TimeDigitalHistogram::const_iterator
TimeDigitalHistogram::begin() const
{
    return histogram_.begin();
}

TimeDigitalHistogram::const_iterator
TimeDigitalHistogram::end() const
{
    return histogram_.end();        
}

uint32_t
TimeDigitalHistogram::accumulate( double tof, double window ) const
{
    if ( ! histogram_.empty() ) {

        if ( std::abs( tof ) <= std::numeric_limits< double >::epsilon() ) {
            return std::accumulate( histogram_.begin(), histogram_.end(), uint32_t(0)
                                    , []( const uint32_t& a, const std::pair<double, uint32_t>& b ){ return a + b.second; });
        }

        auto lower = std::lower_bound( histogram_.begin(), histogram_.end(), (tof - window / 2.0)
                                       , []( const std::pair<double, uint32_t>& a, const double& b){ return a.first < b; } );
        
        if ( lower != histogram_.end() ) {
            
            auto upper = std::upper_bound( histogram_.begin(), histogram_.end(), ( tof + window / 2.0 )
                                           , []( const double& a, const std::pair<double, uint32_t>& b){ return a < b.first; } );                
            
            return std::accumulate( lower, upper, uint32_t(0)
                                    , []( const uint32_t& a, const std::pair<double, uint32_t>& b ){ return a + b.second; });        
        }
    }
    return 0;
}

bool
TimeDigitalHistogram::translate( adcontrols::MassSpectrum& sp, const TimeDigitalHistogram& hgrm )
{
    sp.setCentroid( adcontrols::CentroidNative );

    using namespace adcontrols::metric;
    
    adcontrols::MSProperty prop;
    adcontrols::MSProperty::SamplingInfo
        info( 0 /* int interval (must be zero) */
              , uint32_t( hgrm.initialXOffset() / hgrm.xIncrement() + 0.5 )  // delay
              , uint32_t( hgrm.actualPoints() ) // this is for acq. time range calculation
              , uint32_t( hgrm.trigger_count() )
              , 0 /* mode */);
    
    info.fSampInterval( hgrm.xIncrement() );

    prop.acceleratorVoltage( 3000 ); // nominal
    prop.setSamplingInfo( info );
        
    prop.setTimeSinceInjection( hgrm.initialXTimeSeconds() );
    prop.setTimeSinceEpoch( hgrm.timeSinceEpoch().first );
    prop.setNumAverage( uint32_t( hgrm.trigger_count() ) );
    prop.setTrigNumber( uint32_t( hgrm.serialnumber().first ) );

    prop.setDataInterpreterClsid( "u5303a" );
        
    // {
    //     acqrscontrols::u5303a::device_data data;
    //     data.meta_ = meta;
    //     std::string ar;
    //     adportable::binary::serialize<>()( data, ar );
    //     prop.setDeviceData( ar.data(), ar.size() );
    // }
        
    sp.setMSProperty( prop );

    // if ( resolution > meta.xIncrement ) {

    //     std::vector< double > times, intens;
    //     acqrscontrols::u5303a::histogram::average( hist, resolution, times, intens );
    //     sp->resize( times.size() );
    //     sp->setTimeArray( times.data() );
    //     sp->setIntensityArray( intens.data() );

    // } else {

    sp.resize( hgrm.size() );
    size_t idx = 0;
    for ( auto it = hgrm.begin(); it != hgrm.end(); ++it, ++idx ) {
        sp.setTime( idx, it->first );
        sp.setIntensity( idx, it->second );
    }
    
    return true;
}

TimeDigitalHistogram&
TimeDigitalHistogram::operator += ( const TimeDigitalHistogram& t )
{
    if ( trigger_count_ == 0 ) {
        *this = t;
        return *this;
    }

    trigger_count_ += t.trigger_count();
    wellKnownEvents_ |= t.wellKnownEvents();

    if ( serialnumber_.first > t.serialnumber().first ) 
        serialnumber_.first = t.serialnumber().first;

    if ( serialnumber_.second < t.serialnumber().second ) 
        serialnumber_.second = t.serialnumber().second;

    if ( timeSinceEpoch_.first > t.timeSinceEpoch().first )
        timeSinceEpoch_.first = t.timeSinceEpoch().first;

    if ( timeSinceEpoch_.second < t.timeSinceEpoch().second )
        timeSinceEpoch_.second = t.timeSinceEpoch().second;

    if ( t.histogram().empty() )
        return *this;

    if ( histogram_.empty() ) {
        histogram_ = t.histogram();
        return *this;
    }

    typedef std::pair< double, uint32_t > value_type;

    if ( histogram_.back().first < t.histogram().front().first ) {
        histogram_.reserve( histogram_.size() + t.histogram().size() );
        histogram_.insert( histogram_.end(), t.histogram().begin(), t.histogram().end() );
        return *this;
    }

    if ( histogram_.front().first > t.histogram().back().first ) {
        histogram_.reserve( histogram_.size() + t.histogram().size() );
        histogram_.insert( histogram_.begin(), t.histogram().begin(), t.histogram().end() );
        return *this;
    }

    struct make_binary {
        double xIncrement;
        make_binary( double _1 ) : xIncrement( _1 ) {}
        inline uint64_t operator()( const double& t ){ return t / xIncrement + 0.5; }
    };

    // const double resolution = xIncrement_ / 2.0;
    std::vector< value_type > summed;
    summed.reserve( histogram_.size() + t.histogram().size() );

    make_binary to_binary( xIncrement_ );

    auto lhs = histogram_.begin();

    for ( auto rhs = t.histogram().begin(); rhs != t.histogram().end() && lhs != histogram_.end(); ++rhs ) {

        while ( lhs != histogram_.end() && to_binary( lhs->first ) < to_binary( rhs->first ) ) {
            summed.emplace_back( lhs->first, lhs->second );
            ++lhs;
        }
        
        if ( lhs != histogram_.end() && to_binary( lhs->first ) == to_binary( rhs->first ) ) {
            double t = ( lhs->first * lhs->second + rhs->first * rhs->second ) / ( lhs->second + rhs->second );
            summed.emplace_back( t, ( lhs->second + rhs->second ) );
            ++lhs;
        } else {
            summed.emplace_back( *rhs );            
        }
    }

    if ( lhs != histogram_.end() )
        summed.insert( summed.end(), lhs, histogram_.end() );

    histogram_ = std::move( summed );

    return *this;
}

double
TimeDigitalHistogram::triggers_per_second() const
{
    return trigger_count_ / double( timeSinceEpoch_.second - timeSinceEpoch_.first ) * 1.0e9;
}

bool
TimeDigitalHistogram::archive( std::ostream& os, const TimeDigitalHistogram& t )
{
    return internal::binSerializer().archive( os, t );
}

bool
TimeDigitalHistogram::restore( std::istream& is, TimeDigitalHistogram& t )
{
    return internal::binSerializer().restore( is, t );
}
