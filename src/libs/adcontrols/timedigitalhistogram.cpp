/**************************************************************************
** Copyright (C) 2015-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2017 MS-Cheminformatics LLC
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
#include <adcontrols/samplinginfo.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/float.hpp>
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
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( _.protocolIndex_ );
                ar & BOOST_SERIALIZATION_NVP( _.nProtocols_ );
            }
        }
    };

    class TimeDigitalHistogram::device_data {
    public:
        const adcontrols::TofProtocol& this_protocol;
        device_data( const adcontrols::TofProtocol& p ) : this_protocol( p ) {
        }

        template<class Archive>
        void serialize( Archive& ar, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( this_protocol );
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
                                             , actualPoints_( 0 )
                                             , trigger_count_( 0 )
                                             , wellKnownEvents_( 0 )
                                             , protocolIndex_( 0 )
                                             , nProtocols_( 1 )
                                             , serialnumber_( { 0, 0 } )
                                             , timeSinceEpoch_( { 0, 0 } )
{
}

TimeDigitalHistogram::TimeDigitalHistogram( const TimeDigitalHistogram& t ) : initialXTimeSeconds_( t.initialXTimeSeconds_ )
                                                                            , initialXOffset_( t.initialXOffset_ )
                                                                            , xIncrement_( t.xIncrement_ )
                                                                            , actualPoints_( t.actualPoints_ )
                                                                            , trigger_count_( t.trigger_count_ )
                                                                            , wellKnownEvents_( t.wellKnownEvents_ )
                                                                            , this_protocol_( t.this_protocol_ )
                                                                            , protocolIndex_ ( t.protocolIndex_ )
                                                                            , nProtocols_ ( t.nProtocols_ )
                                                                            , serialnumber_( t.serialnumber_ )
                                                                            , timeSinceEpoch_( t.timeSinceEpoch_ )
                                                                            , histogram_( t.histogram_ )
{
}

std::shared_ptr< TimeDigitalHistogram >
TimeDigitalHistogram::clone( const std::vector< std::pair<double, uint32_t > >& histogram ) const
{
    auto t = std::make_shared< TimeDigitalHistogram >();
    t->initialXTimeSeconds_ = initialXTimeSeconds_;
    t->initialXOffset_ = initialXOffset_;
    t->xIncrement_ = xIncrement_;
    t->actualPoints_ = actualPoints_;
    t->serialnumber_ = serialnumber_;
    t->timeSinceEpoch_ = timeSinceEpoch_;
    t->trigger_count_ = trigger_count_;
    t->wellKnownEvents_ = wellKnownEvents_;
    t->histogram_ = histogram;                 // <-- replace histogram
    t->this_protocol_ = this_protocol_;
    t->protocolIndex_ = protocolIndex_;
    t->nProtocols_ = nProtocols_;
    return t;
}

std::shared_ptr< TimeDigitalHistogram >
TimeDigitalHistogram::merge_peaks( double resolution ) const
{
    auto t = std::make_shared< TimeDigitalHistogram >();
    t->initialXTimeSeconds_ = initialXTimeSeconds_;
    t->initialXOffset_ = initialXOffset_;
    t->xIncrement_ = xIncrement_;
    t->actualPoints_ = actualPoints_;
    t->serialnumber_ = serialnumber_;
    t->timeSinceEpoch_ = timeSinceEpoch_;
    t->trigger_count_ = trigger_count_;
    t->wellKnownEvents_ = wellKnownEvents_;
    t->this_protocol_ = this_protocol_;
    t->protocolIndex_ = protocolIndex_;
    t->nProtocols_ = nProtocols_;

    average_time( this->histogram(), resolution, t->histogram() );

    return t;
}

void
TimeDigitalHistogram::setInitialXTimeSeconds( double d )
{
    initialXTimeSeconds_ = d;
}

void
TimeDigitalHistogram::setInitialXOffset( double d )
{
    initialXOffset_ = d;
}

void
TimeDigitalHistogram::setXIncrement( double d )
{
    xIncrement_ = d;
}

void
TimeDigitalHistogram::setActualPoints( uint64_t d )
{
    actualPoints_ = d;
}

void
TimeDigitalHistogram::setTrigger_count( uint64_t d )
{
    trigger_count_ = d;
}

void
TimeDigitalHistogram::setWellKnownEvents( uint32_t d )
{
    wellKnownEvents_ = d;
}

void
TimeDigitalHistogram::setSerialnumber( const std::pair< uint64_t, uint64_t >& d )
{
    serialnumber_ = d;
}

void
TimeDigitalHistogram::setTimeSinceEpoch( const std::pair< uint64_t, uint64_t >& d )
{
    timeSinceEpoch_ = d;
}

// double&
// TimeDigitalHistogram::initialXTimeSeconds()
// {
//     return initialXTimeSeconds_;
// }

// double&
// TimeDigitalHistogram::initialXOffset()
// {
//     return initialXOffset_;
// }

// double&
// TimeDigitalHistogram::xIncrement()
// {
//     return xIncrement_;
// }

// uint64_t&
// TimeDigitalHistogram::actualPoints()
// {
//     return actualPoints_;
// }

// uint64_t&
// TimeDigitalHistogram::trigger_count()
// {
//     return trigger_count_;
// }

// uint32_t&
// TimeDigitalHistogram::wellKnownEvents()
// {
//     return wellKnownEvents_;
// }

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

// std::pair< uint64_t, uint64_t >&
// TimeDigitalHistogram::serialnumber()
// {
//     return serialnumber_;
// }

// std::pair< uint64_t, uint64_t >&
// TimeDigitalHistogram::timeSinceEpoch()
// {
//     return timeSinceEpoch_;
// }

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

void
TimeDigitalHistogram::setThis_protocol( const TofProtocol& d )
{
    this_protocol_ = d;
}

const TofProtocol&
TimeDigitalHistogram::this_protocol() const
{
    return this_protocol_;
}

uint32_t
TimeDigitalHistogram::protocolIndex() const
{
    return protocolIndex_;
}

uint32_t
TimeDigitalHistogram::nProtocols() const
{
    return nProtocols_;
}

void
TimeDigitalHistogram::setProtocolIndex( uint32_t idx, uint32_t count )
{
    protocolIndex_ = idx;
    nProtocols_ = count;
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

        // histogram_ is an array of digitizer time, so that adjust external (de0-nano-soc) trigger delay.
        tof -= this_protocol_.delay_pulses().at( adcontrols::TofProtocol::EXT_ADC_TRIG ).first;

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
TimeDigitalHistogram::translate( adcontrols::MassSpectrum& sp
                                 , const TimeDigitalHistogram& hgrm )
{
    sp.setCentroid( adcontrols::CentroidHistogram );

    using namespace adcontrols::metric;

    // ext_trig_delay should be managed before came here.  (ex. histogram::move())

    double ext_trig_delay = hgrm.this_protocol_.delay_pulses().at( adcontrols::TofProtocol::EXT_ADC_TRIG ).first;

    adcontrols::MSProperty prop;
    adcontrols::SamplingInfo info( hgrm.xIncrement()
                                   , hgrm.initialXOffset() + ext_trig_delay
                                   , int32_t( ( hgrm.initialXOffset() + ext_trig_delay ) / hgrm.xIncrement() )  // delay
                                   , uint32_t( hgrm.actualPoints() ) // this is for acq. time range calculation
                                   , uint32_t( hgrm.trigger_count() )
                                   , hgrm.this_protocol_.mode() /* mode */);

    prop.setAcceleratorVoltage( 0 ); // empty

    prop.setSamplingInfo( info );

    prop.setTimeSinceInjection( hgrm.initialXTimeSeconds() );
    prop.setTimeSinceEpoch( hgrm.timeSinceEpoch().first );
    prop.setNumAverage( uint32_t( hgrm.trigger_count() ) );
    prop.setTrigNumber( uint32_t( hgrm.serialnumber().first ) );

    prop.setDataInterpreterClsid( "adcontrols::TimeDigitalHistogram" );

    TimeDigitalHistogram::device_data data( hgrm.this_protocol_ );
    std::string ar;
    adportable::binary::serialize<>()( data, ar );
    prop.setDeviceData( ar.data(), ar.size() );

    sp.setMSProperty( prop );
    sp.setProtocol( hgrm.protocolIndex_, hgrm.nProtocols_ );

    size_t size = hgrm.size();

    sp.resize( size );
    size_t idx = 0;
    for ( auto it = hgrm.begin(); it != hgrm.end(); ++it, ++idx ) {
        sp.setTime( idx, it->first );
        sp.setIntensity( idx, it->second ); // return raw count values
    }

    return true;
}

bool
TimeDigitalHistogram::translate( adcontrols::MassSpectrum& sp
                                 , const TimeDigitalHistogram& hgrm
                                 , mass_assignor_t mass_assignee )
{
    if ( translate( sp, hgrm ) ) {
        const adcontrols::MSProperty& prop = sp.getMSProperty();
        const auto& sinfo = prop.samplingInfo();
        if ( mass_assignee ) {
            double lMass = mass_assignee( sinfo.fSampDelay(), prop.mode() );
            double hMass = mass_assignee( sinfo.fSampDelay() + sinfo.fSampInterval() * sinfo.nSamples(), prop.mode() );
            sp.setAcquisitionMassRange( lMass, hMass );
            return sp.assign_masses( mass_assignee );
        } else {
            return true;
        }
    }
    return false;
}

TimeDigitalHistogram&
TimeDigitalHistogram::operator += ( const TimeDigitalHistogram& t )
{
    if ( trigger_count_ == 0 ||
         !adportable::compare<double>::essentiallyEqual(
             this_protocol_.delay_pulses().at( TofProtocol::EXT_ADC_TRIG ).first
             , t.this_protocol().delay_pulses().at( TofProtocol::EXT_ADC_TRIG ).first ) ) {
        *this = t;
#if !defined NDEBUG
        ADDEBUG() << "########################### " << __FUNCTION__ << " ### just clear it, trigger_count_=" << trigger_count_
                  << ", " << t.trigger_count_
                  << ", " << this_protocol_.delay_pulses().at( TofProtocol::EXT_ADC_TRIG ).first
                  << ", " << t.this_protocol().delay_pulses().at( TofProtocol::EXT_ADC_TRIG ).first;
#endif
        return *this;
    }

    trigger_count_ += t.trigger_count();
    wellKnownEvents_ |= t.wellKnownEvents();

    serialnumber_.first = t.serialnumber().first;

    if ( serialnumber_.second < t.serialnumber().second )
        serialnumber_.second = t.serialnumber().second;

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

//static
bool
TimeDigitalHistogram::average_time( const std::vector< std::pair< double, uint32_t > >& hist
                                    , double resolution
                                    , std::vector< std::pair< double, uint32_t > >& merged )
{
    merged.clear();

    auto it = hist.begin();

    while ( it != hist.end() ) {

        auto tail = it + 1;

        while ( ( tail != hist.end() ) ) {

            double t1 = tail->first;

            if ( std::abs( t1 - it->first ) > resolution )
                break;

            std::advance( tail, 1 );
        }

        std::pair< double, double > sum
            = std::accumulate( it, tail, std::make_pair( 0.0, 0.0 )
                               , []( const std::pair<double, double>& a, const std::pair<double, uint32_t>& b ) {
                                   return std::make_pair( a.first + (b.first * b.second), double(a.second + b.second) );
                               });
        merged.emplace_back( ( sum.first / sum.second ), sum.second );

        it = tail;
    }

    return true;
}
