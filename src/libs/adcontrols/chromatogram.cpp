// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "chromatogram.hpp"

#include "baseline.hpp"
#include "baselines.hpp"
#include "descriptions.hpp"
#include "peak.hpp"
#include "peakresult.hpp"
#include "peaks.hpp"
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adportable/debug.hpp>
#include <adportable/date_time.hpp>
#include <adportable/iso8601.hpp>
#include <adportable/sgfilter.hpp>
#include <adportable/utf.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/json.hpp>
#include <boost/property_tree/ptree.hpp>                 // -- used in Archive V4
#include <boost/property_tree/ptree_serialization.hpp>   // -- used in Archive V4
#include <boost/property_tree/json_parser.hpp>           // -- used in Archive V4
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/system/error_code.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>

#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <adportable/float.hpp>
#include <chrono>
#include <numeric>
#include <regex>
#include <sstream>
#include <vector>

using namespace adcontrols;
using namespace adcontrols::internal;

namespace adcontrols {

    const size_t Chromatogram::npos;

    class Chromatogram::impl {
    public:
        ~impl() {
        }

        impl() : isConstantSampling_(true)
               , isCounting_( false )
               , dataDelayPoints_(0)
               , samplingInterval_(0)
               , proto_(0)
               , dataReaderUuid_( { {0} } )
               , dataGuid_( boost::uuids::random_generator()() ) // random generator
               , yAxisUnit_( { plot::Arbitrary, 0 } )
               , sfe_injection_delay_( 0 ) { // for SFC inject delay since SFE start
        }

        // impl( const Chromatogram::impl& );
        impl( const impl& t ) : isConstantSampling_( t.isConstantSampling_ )
                              , isCounting_( t.isCounting_ )
                              , peaks_( t.peaks_ )
                              , baselines_( t.baselines_ )
                              , dataArray_( t.dataArray_ )
                              , timeArray_( t.timeArray_ )
                              , evntVec_( t.evntVec_ )
                              , timeRange_( t.timeRange_)
                              , dataDelayPoints_ ( t.dataDelayPoints_ )
                              , samplingInterval_( t.samplingInterval_ )
                              , axisLabels_( t.axisLabels_ )
                              , proto_( t.proto_ )
                              , dataReaderUuid_( t.dataReaderUuid_ )
                              , dataGuid_( t.dataGuid_ )
                              , generator_property_( t.generator_property_ )
                              , tofArray_( t.tofArray_ )
                              , massArray_( t.massArray_ )
                              , time_of_injection_( t.time_of_injection_ )
                              , yAxisUnit_( t.yAxisUnit_ )
                              , display_name_( t.display_name_ )
                              , sfe_injection_delay_( t.sfe_injection_delay_ )
                              , srcTimeArray_( t.srcTimeArray_ ){
            descriptions_ = t.descriptions_;
        }

        inline const std::vector< double >& timeArray() const { return timeArray_; }
        inline std::vector< double >& timeArray()             { return timeArray_; }
        inline const std::vector< double >& dataArray() const { return dataArray_; }

        inline const std::vector<Chromatogram::Event>& getEventVec() const { return evntVec_; }
        inline std::vector<Chromatogram::Event>& getEventVec() { return evntVec_; }

        inline size_t size() const { return dataArray_.size(); }
        inline const std::pair<double, double>& getAcquisitionTimeRange() const { return timeRange_; }
        inline double samplingInterval() const { return samplingInterval_; /* seconds */ }
        void setSamplingInterval( double v ) { samplingInterval_ = v; }

        bool isConstantSampledData() const { return isConstantSampling_; }
        void setIsConstantSampledData( bool b ) { isConstantSampling_ = b; }

        void setTime( size_t idx, const double& t ) {
            timeArray_.resize( dataArray_.size() );
            timeArray_[ idx ] = t;
        }
        void setData( size_t idx, const double& d ) { dataArray_[ idx ] = d;    }
        void setTimeArray( const double *, size_t );
        void setDataArray( const double *, size_t );
        void setEventArray( const unsigned long * );
        void resize( size_t );
        void addDescription( const description& );
        const descriptions& getDescriptions() const;

        void minTime( double v ) { timeRange_.first = v; }
        void maxTime( double v ) { timeRange_.second = v; }
        void dataDelayPoints( size_t n ) { dataDelayPoints_ = n; }
        size_t dataDelayPoints() const { return dataDelayPoints_; }

        friend class Chromatogram;
        static std::wstring empty_string_;  // for error return as reference
        bool isConstantSampling_;
        bool isCounting_;

        descriptions descriptions_;
        Peaks peaks_;
        Baselines baselines_;

        std::vector< double > dataArray_;
        std::vector< double > timeArray_;
        std::optional< std::vector< double > > srcTimeArray_; // original backup for SFE delay
        std::vector< Chromatogram::Event > evntVec_;
        std::pair<double, double> timeRange_;
        size_t dataDelayPoints_;
        double samplingInterval_;
        std::map< plot::axis, std::string > axisLabels_;
        int32_t proto_;
        boost::uuids::uuid dataReaderUuid_;
        boost::uuids::uuid dataGuid_;
        boost::optional< std::string > generator_property_;
        std::vector< double > tofArray_;
        std::vector< double > massArray_;
        std::string time_of_injection_; // iso8601 extended
        std::pair< plot::unit, size_t > yAxisUnit_;
        std::string display_name_;
        //
        int64_t sfe_injection_delay_; // ns

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version) {
            if ( version < 9 ) {
                ar & BOOST_SERIALIZATION_NVP(samplingInterval_)
                    & BOOST_SERIALIZATION_NVP(isConstantSampling_)
                    & BOOST_SERIALIZATION_NVP(timeRange_.first)
                    & BOOST_SERIALIZATION_NVP(timeRange_.second)
                    & BOOST_SERIALIZATION_NVP(dataDelayPoints_)
                    & BOOST_SERIALIZATION_NVP(descriptions_);
                std::wstring axisLabelHorizontal, axisLabelVertical;
                ar  & BOOST_SERIALIZATION_NVP(axisLabelHorizontal)
                    & BOOST_SERIALIZATION_NVP(axisLabelVertical);
                ar  & BOOST_SERIALIZATION_NVP(dataArray_)
                    & BOOST_SERIALIZATION_NVP(timeArray_)
                    & BOOST_SERIALIZATION_NVP(evntVec_)
                    & BOOST_SERIALIZATION_NVP(peaks_)
                    ;
                if ( version >= 2 )
                    ar & BOOST_SERIALIZATION_NVP( proto_ );
                if ( version >= 3 )
                    ar & BOOST_SERIALIZATION_NVP( dataReaderUuid_ );
                if ( version >= 4 ) {
                    boost::property_tree::ptree ptree;
                    ar & BOOST_SERIALIZATION_NVP( dataGuid_ );
                    ar & BOOST_SERIALIZATION_NVP( ptree );
                    if ( Archive::is_loading::value ) {
                        if ( ! (ptree.empty() && ptree.data().empty() )) {
                            std::ostringstream o;
                            boost::property_tree::write_json( o, ptree );
                            generator_property_ = o.str();
                        }
                    }
                }
                if ( version >= 5 ) {
                    ar & BOOST_SERIALIZATION_NVP( tofArray_ );
                    ar & BOOST_SERIALIZATION_NVP( massArray_ );
                }
                if ( version >= 6 )
                    ar & BOOST_SERIALIZATION_NVP( isCounting_ );
                if ( version >= 7 )
                    ar & BOOST_SERIALIZATION_NVP( time_of_injection_ );
                if ( version >= 8 ) {
                    ar & BOOST_SERIALIZATION_NVP( axisLabels_ );
                    ar & BOOST_SERIALIZATION_NVP( yAxisUnit_ );
                }
            } else if ( version >= 9 ) {
                ar & BOOST_SERIALIZATION_NVP( samplingInterval_ );
                ar & BOOST_SERIALIZATION_NVP( isConstantSampling_ );
                ar & BOOST_SERIALIZATION_NVP( timeRange_ );
                ar & BOOST_SERIALIZATION_NVP( dataDelayPoints_ );
                ar & BOOST_SERIALIZATION_NVP( descriptions_ );
                ar & BOOST_SERIALIZATION_NVP( dataArray_ );
                ar & BOOST_SERIALIZATION_NVP( timeArray_ );
                ar & BOOST_SERIALIZATION_NVP( evntVec_ );
                ar & BOOST_SERIALIZATION_NVP( peaks_ );
                ar & BOOST_SERIALIZATION_NVP( dataReaderUuid_ );
                ar & BOOST_SERIALIZATION_NVP( dataGuid_ );
                ar & BOOST_SERIALIZATION_NVP( tofArray_ );
                ar & BOOST_SERIALIZATION_NVP( massArray_ );
                ar & BOOST_SERIALIZATION_NVP( isCounting_ );
                ar & BOOST_SERIALIZATION_NVP( time_of_injection_ );
                ar & BOOST_SERIALIZATION_NVP( axisLabels_ );
                ar & BOOST_SERIALIZATION_NVP( yAxisUnit_ );
                ar & BOOST_SERIALIZATION_NVP( generator_property_ );
                if ( version >= 10 ) {
                    ar & BOOST_SERIALIZATION_NVP( display_name_ );
                }
                if ( version >= 11 ) {
                    ar & BOOST_SERIALIZATION_NVP( sfe_injection_delay_ );
                }
            }
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::Chromatogram::impl, 11 )

namespace {

    struct find_first_cross_up {
        std::pair< size_t, bool > operator()( double th
                                              , const double * values
                                              , size_t size
                                              , size_t begin = 0 ) const {
            // ADDEBUG() << "---------- find_first_cross_up ------------- ";
            adportable::SGFilter filter( 5 );
            for ( size_t i = begin + 2; i < size - 2; ++i ) {
                if ( filter( values + i ) < th && filter( values + i + 1 ) >= th ) {
                    return {i, true};
                }
            }
            return { begin, false };
        };
    };

    struct find_last_cross_down {
        std::pair< size_t, bool > operator()( double th
                                              , const double * values
                                              , size_t size
                                              , size_t begin = 0 ) const {
            adportable::SGFilter filter( 5 );
            for ( size_t i = size - 3; i > begin + 1; --i ) {
                if ( filter( values + i ) > th && filter( values + i + 1 ) >= th )
                    return {i, true};
            }
            if ( filter( values + size - 3 ) > th ) { // last data is higher than th
                return { size - 1, false };
            }
            return { begin, false };
        }
    };


}

///////////////////////////////////////////

Chromatogram::~Chromatogram()
{
    delete impl_;
}

Chromatogram::Chromatogram() : impl_{ new impl{} }
{
}

Chromatogram::Chromatogram( const Chromatogram& c ) : impl_{ new impl( *c.impl_ ) }
{
}

Chromatogram&
Chromatogram::operator = ( const Chromatogram& t )
{
    if ( t.impl_ != impl_ ) {
        delete impl_;
        impl_ = new Chromatogram::impl( *t.impl_ );
    }
    return *this;
}

/////////  static functions
Chromatogram::seconds_t
Chromatogram::toSeconds( const Chromatogram::minutes_t& m )
{
    return m.minutes * 60.0;
}

Chromatogram::minutes_t
Chromatogram::toMinutes( const Chromatogram::seconds_t& s )
{
    return s.seconds / 60.0;
}

std::pair<double, double>
Chromatogram::toMinutes( const std::pair<seconds_t, seconds_t>& pair )
{
    return std::make_pair( pair.first.seconds / 60.0, pair.second.seconds / 60.0 );
}

/////////////////

void
Chromatogram::setIsCounting( bool counting )
{
    impl_->isCounting_ = counting;
}

bool
Chromatogram::isCounting() const
{
    return impl_->isCounting_;
}

void
Chromatogram::setProtocol( int fcn )
{
    impl_->proto_ = fcn;
}

int
Chromatogram::protocol() const
{
    return impl_->proto_;
}

size_t
Chromatogram::size() const
{
    return impl_->size();
}

void
Chromatogram::resize( size_t n )
{
    impl_->resize( n );
}

const Peaks&
Chromatogram::peaks() const
{
    return impl_->peaks_;
}

const Baselines&
Chromatogram::baselines() const
{
    return impl_->baselines_;
}

void
Chromatogram::setBaselines( const Baselines& baselines )
{
    impl_->baselines_ = baselines;
}

void
Chromatogram::setPeaks( const Peaks& peaks )
{
    impl_->peaks_ = peaks;
}

void
Chromatogram::setSinglePeak( std::pair< std::shared_ptr< Peak >, std::shared_ptr< Baseline > >&& t )
{
#if __cplusplus >= 201703L
    auto [ pk, bs ] = t;
#else
    std::shared_ptr< Peak > pk;
    std::shared_ptr< Baseline > bs;
    std::tie( pk, bs ) = t;
#endif
    if ( pk && bs ) {
        impl_->baselines_ = {};
        impl_->peaks_ = {};
        impl_->baselines_.add( *bs );
        impl_->peaks_.add( *pk );
    }
}

bool
Chromatogram::isConstantSampledData() const
{
    return impl_->isConstantSampling_;
}

void
Chromatogram::setIsConstantSampledData( bool b )
{

    impl_->isConstantSampling_ = b;
}

double
Chromatogram::timeFromSampleIndex( size_t sampleIndex ) const
{
    return sampleIndex * impl_->samplingInterval();
}

double
Chromatogram::timeFromDataIndex( size_t index ) const
{
    return ( index + impl_->dataDelayPoints() ) * impl_->samplingInterval();
}

size_t
Chromatogram::toSampleIndex( double time, bool closest ) const
{
    size_t lower = unsigned( time / impl_->samplingInterval() );
    if ( ! closest )
        return lower;
    double ltime = lower * impl_->samplingInterval();
    double utime = ( lower + 1 ) * impl_->samplingInterval();
    if ( std::abs( time - ltime ) > std::abs( utime - ltime ) )
        return lower + 1;
    return lower;
}

size_t
Chromatogram::toDataIndex( double time, bool closest ) const
{
    return toSampleIndex( time, closest ) - impl_->dataDelayPoints();
}

std::pair<size_t,size_t>
Chromatogram::toIndexRange( const std::pair< double, double >& range ) const
{
    if ( impl_->dataArray_.empty() )
        return { npos, npos };
    if ( impl_->timeArray_.empty() ) {
        return std::make_pair( toDataIndex( range.first ), toDataIndex( range.second ) );
    } else {
        auto its = std::lower_bound( impl_->timeArray().begin(), impl_->timeArray().end(), range.first );
        auto ite = std::lower_bound( impl_->timeArray().begin(), impl_->timeArray().end(), range.second );
        // if ( ite != impl_->timeArray().end() ) {
        //     --ite;
        // }
        return std::make_pair( std::distance( impl_->timeArray().begin(), its )
                               , std::distance( impl_->timeArray().begin(), ite ) );
    }
}

std::vector< double >&
Chromatogram::intensVector()
{
     return impl_->dataArray_;
}

const double *
Chromatogram::getIntensityArray() const
{
    return impl_->dataArray().data();
}

void
Chromatogram::operator << ( const std::pair<double, double>& data )
{
    impl_->isConstantSampling_ = false;
    impl_->timeArray_.emplace_back( data.first );
    impl_->dataArray_.emplace_back( data.second );
    if ( impl_->timeArray_.size() == 1 )
        impl_->timeRange_.first = impl_->timeArray_.front();
    impl_->timeRange_.second = impl_->timeArray_.back();
}

void
Chromatogram::operator << ( std::pair<double, double>&& data )
{
    impl_->isConstantSampling_ = false;

    if ( impl_->timeArray_.empty() )
        impl_->timeRange_.first = data.first;
    impl_->timeRange_.second = data.first;

    impl_->timeArray_.emplace_back( data.first );
    impl_->dataArray_.emplace_back( data.second );
}

void
Chromatogram::operator << ( std::tuple<double, double, double, double >&& data )
{
    impl_->isConstantSampling_ = false;

    if ( impl_->timeArray_.empty() )
        impl_->timeRange_.first = std::get<0>(data);
    impl_->timeRange_.second = std::get<0>(data);

    impl_->timeArray_.emplace_back( std::get<0>( data ) );
    impl_->dataArray_.emplace_back( std::get<1>( data ) );

    impl_->tofArray_.emplace_back( std::get<2>( data ) );
    impl_->massArray_.emplace_back( std::get<3>( data ) );
}

const std::vector< double >&
Chromatogram::tofArray() const
{
    return impl_->tofArray_;
}

const std::vector< double >&
Chromatogram::massArray() const
{
    return impl_->massArray_;
}

double
Chromatogram::tof( size_t idx ) const
{
    if ( impl_->tofArray_.size() > idx )
        return impl_->tofArray_.at( idx );
    return 0;
}

double
Chromatogram::mass( size_t idx ) const
{
    if ( impl_->massArray_.size() > idx )
        return impl_->massArray_.at( idx );
    return 0;
}

double
Chromatogram::time( size_t idx ) const
{
    return impl_->timeArray_[ idx ];
}

double
Chromatogram::intensity( size_t idx ) const
{
    return impl_->dataArray_[ idx ];
}

std::pair< double, double >
Chromatogram::datum( size_t idx ) const
{
    return { impl_->timeArray_[ idx ], impl_->dataArray_[ idx ] };
}

void
Chromatogram::setIntensity( size_t idx, double d )
{
    impl_->setData( idx, d );
}

void
Chromatogram::setDatum( size_t idx, std::pair< double, double >&& d )
{
    impl_->setTime( idx, std::get<0>(d) );
    impl_->setData( idx, std::get<1>(d) );
}

void
Chromatogram::setIntensityArray( const double * p, size_t sz )
{
    impl_->setDataArray( p, sz );
}

void
Chromatogram::setTime( size_t idx, double t )
{
    impl_->setTime( idx, t );
}

void
Chromatogram::setTimeArray( const double * p, size_t sz )
{
    impl_->setTimeArray( p, sz );
}

void
Chromatogram::addEvent( const Chromatogram::Event& e )
{
    impl_->getEventVec().push_back( e );
}

const double *
Chromatogram::getTimeArray() const
{
    if ( impl_->timeArray().empty() && isConstantSampledData() ) {
        impl_->timeArray().resize( impl_->dataArray().size() );
        auto [t0,t1] = impl_->getAcquisitionTimeRange();
        size_t n{0};
        std::generate( impl_->timeArray().begin(), impl_->timeArray().end(), [&]{return t1 + impl_->samplingInterval() * n++; } );
    }
    return impl_->timeArray().data();
}

const std::vector< double >&
Chromatogram::timeArray() const
{
    return impl_->timeArray();
}

std::vector< double >&
Chromatogram::timeArray()
{
    return impl_->timeArray();
}

const Chromatogram::Event&
Chromatogram::getEvent( size_t idx ) const
{
    return impl_->getEventVec()[ idx ];
}

Chromatogram::seconds_t
Chromatogram::sampInterval() const
{
    auto value = impl_->samplingInterval();
    if ( adportable::compare<double>::essentiallyEqual( value, 0 ) ) {
        return ( impl_->timeRange_.second - impl_->timeRange_.first ) / ( size() - 1 );
    }
    return value;
}

void
Chromatogram::setSampInterval( const seconds_t& v )
{
    impl_->samplingInterval_ = v;
}

boost::optional< std::string >
Chromatogram::axisLabel( plot::axis axis ) const
{
    auto it = impl_->axisLabels_.find( axis );
    if ( it != impl_->axisLabels_.end() ) {
        return it->second;
    }
    return {};
}

void
Chromatogram::setAxisLabel( plot::axis axis, const std::string& value )
{
    impl_->axisLabels_[ axis ] = value;
}

std::pair< plot::unit, size_t >
Chromatogram::axisUnit() const
{
    return impl_->yAxisUnit_;
}

void
Chromatogram::setAxisUnit( plot::unit unit, size_t den )
{
    impl_->yAxisUnit_ = { unit, den };
}


void
Chromatogram::setMinimumTime( const seconds_t& min )
{
    impl_->minTime( min );
}

void
Chromatogram::setMaximumTime( const seconds_t& min )
{
    impl_->maxTime( min );
}

void
Chromatogram::addDescription( const adcontrols::description& desc )
{
    impl_->addDescription( desc );
}

void
Chromatogram::addDescription( adcontrols::description&& desc )
{
    impl_->descriptions_.append( desc );
}

void
Chromatogram::addDescription( std::pair< std::string, std::string >&& t )
{
    impl_->descriptions_.append( std::move( t ) );
}

const descriptions&
Chromatogram::getDescriptions() const
{
    return impl_->getDescriptions();
}

Chromatogram::seconds_t
Chromatogram::minimumTime() const
{
    return impl_->getAcquisitionTimeRange().first;
}

Chromatogram::seconds_t
Chromatogram::maximumTime() const
{
    return impl_->getAcquisitionTimeRange().second;
}

std::pair<Chromatogram::seconds_t, Chromatogram::seconds_t>
Chromatogram::timeRange() const
{
    const impl& d = *impl_;
    if ( d.timeRange_.second == 0 )
        return std::pair<seconds_t, seconds_t>( d.timeRange_.first, d.timeRange_.first + ( d.size() - 1 ) * d.samplingInterval() );
    return std::pair<seconds_t, seconds_t>( d.timeRange_.first, d.timeRange_.second);
}

size_t  // data index
Chromatogram::min_element( size_t beg, size_t end ) const
{
    const impl& d = *impl_;

    if ( end >= d.size() )
        end = d.size() - 1;
    return std::distance( d.dataArray_.begin(), std::min_element( d.dataArray_.begin() + beg, d.dataArray_.begin() + end ) );
}

size_t   // data index
Chromatogram::max_element( size_t beg, size_t end ) const
{
    const impl& d = *impl_;

    if ( end >= d.size() )
        end = d.size() - 1;
    return std::distance( d.dataArray_.begin(), std::max_element( d.dataArray_.begin() + beg, d.dataArray_.begin() + end ) );
}

double
Chromatogram::getMaxIntensity() const
{
    const impl& d = *impl_;
    return *std::max_element( d.dataArray_.begin(), d.dataArray_.end() );
}

double
Chromatogram::getMinIntensity() const
{
    const impl& d = *impl_;
    return *std::min_element( d.dataArray_.begin(), d.dataArray_.end() );
}

void
Chromatogram::set_time_of_injection( std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds> && t )
{
    impl_->time_of_injection_ = adportable::date_time::to_iso< std::chrono::nanoseconds >( t );
}

void
Chromatogram::set_time_of_injection( const std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds>& t )
{
    impl_->time_of_injection_ = adportable::date_time::to_iso< std::chrono::nanoseconds >( t );
}

std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >
Chromatogram::time_of_injection() const
{
    if ( auto tp = adportable::iso8601::parse( impl_->time_of_injection_.begin(), impl_->time_of_injection_.end() ) )
        return *tp;
    return {};
}

void
Chromatogram::set_time_of_injection_iso8601( const std::string& t )
{
    impl_->time_of_injection_ = t;
}

std::string
Chromatogram::time_of_injection_iso8601() const
{
    if ( impl_->time_of_injection_.empty() ) {
        return adportable::date_time::to_iso< std::chrono::nanoseconds >(
            std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >{} );
    }
    return impl_->time_of_injection_;
}

// specialized template<> for boost::serialization
// template<class Archiver> void serialize(Archiver& ar, const unsigned int version);

namespace adcontrols {
    template<> void
    Chromatogram::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp("Chromatogram", impl_ );
    }

    template<> void
    Chromatogram::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        (void)version;
        ar >> boost::serialization::make_nvp("Chromatogram", impl_);
    }

    template<> void
    Chromatogram::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp( "Chromatogram", impl_ );
    }

    template<> void
    Chromatogram::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        (void)version;
        ar >> boost::serialization::make_nvp( "Chromatogram", impl_ );
    }
}; // namespace adcontrols


/////////////
namespace adcontrols {

    template<> void
    Chromatogram::Event::serialize( boost::archive::xml_woarchive& ar, const unsigned int)
    {
	ar & BOOST_SERIALIZATION_NVP(index) & BOOST_SERIALIZATION_NVP(value);
    }

    template<> void
    Chromatogram::Event::serialize( boost::archive::xml_wiarchive& ar, const unsigned int)
    {
	ar & BOOST_SERIALIZATION_NVP(index) & BOOST_SERIALIZATION_NVP(value);
    }

    template<> void
    Chromatogram::Event::serialize( portable_binary_oarchive& ar, const unsigned int)
    {
	ar & BOOST_SERIALIZATION_NVP(index) & BOOST_SERIALIZATION_NVP(value);
    }

    template<> void
    Chromatogram::Event::serialize( portable_binary_iarchive& ar, const unsigned int)
    {
	ar & BOOST_SERIALIZATION_NVP(index) & BOOST_SERIALIZATION_NVP(value);
    }
}; // namespace adcontrols

bool
Chromatogram::archive( std::ostream& os, const Chromatogram& c )
{
    portable_binary_oarchive ar( os );
    ar << c;
    return true;
}

bool
Chromatogram::restore( std::istream& is, Chromatogram& c )
{
    portable_binary_iarchive ar( is );
    ar >> c;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

std::wstring Chromatogram::impl::empty_string_ = L"";

void
Chromatogram::impl::setDataArray( const double * p, size_t sz )
{
    if ( p && sz ) {
        size_t end = std::min( sz, size() );
        std::copy( p, p + end, dataArray_.begin() );
    }
}

void
Chromatogram::impl::setTimeArray( const double * p, size_t sz ) // array of second
{
    ADDEBUG() << __FUNCTION__;
    if ( p && sz ) {
        if ( timeArray_.size() != size() )
            timeArray_.resize( size() );
        std::copy( p, p + size(), timeArray_.begin() );
        timeRange_.first = p[0];
        timeRange_.second = p[ size() - 1 ];
		if ( adportable::compare<double>::essentiallyEqual( samplingInterval_, 0 ) )
			samplingInterval_ = ( timeRange_.second - timeRange_.first ) / ( size() - 1 );
    } else {
        timeArray_.clear();
    }
}

void
Chromatogram::impl::resize( size_t size )
{
    dataArray_.resize( size );
}

void
Chromatogram::impl::addDescription( const adcontrols::description& desc )
{
    descriptions_.append( desc );
}

const descriptions&
Chromatogram::impl::getDescriptions() const
{
    return descriptions_;
}

std::string
Chromatogram::make_title() const
{
    return make_folder_name<char>( impl_->descriptions_ );
}

//static
template<> std::string
Chromatogram::make_folder_name( const adcontrols::descriptions& descs )
{
    return descs.make_folder_name( "(MSLock)||(_.*)", true );
}

template<> std::wstring
Chromatogram::make_folder_name( const adcontrols::descriptions& descs )
{
    return descs.make_folder_name( L"(MSLock)||(_.*)", true );
}

// for v3 format datafile support
void
Chromatogram::setDataReaderUuid( const boost::uuids::uuid& uuid )
{
    impl_->dataReaderUuid_ = uuid;
}

const boost::uuids::uuid&
Chromatogram::dataReaderUuid() const
{
    return impl_->dataReaderUuid_;
}

void
Chromatogram::setGeneratorProperty( std::string&& prop )
{
    impl_->generator_property_ = std::move( prop );
}

void
Chromatogram::setGeneratorProperty( const std::string& prop )
{
    impl_->generator_property_ = prop;
}

boost::optional< std::string >
Chromatogram::generatorProperty() const
{
    return impl_->generator_property_;
}

void
Chromatogram::set_display_name( const std::string& t )
{
    impl_->display_name_ = t;
}

boost::optional< std::string >
Chromatogram::display_name() const
{
    if ( !impl_->display_name_.empty() )
        return impl_->display_name_;
    return {};
}

void
Chromatogram::setDataGuid( const boost::uuids::uuid& uuid )
{
    impl_->dataGuid_ = uuid;
}

const boost::uuids::uuid&
Chromatogram::dataGuid() const
{
    return impl_->dataGuid_;
}

bool
Chromatogram::set_sfe_injection_delay( bool enable, double s ) // seconds (intanally, ns with int64)
{
    int64_t delay = enable ? ( s * std::nano::den ) : 0;
    if ( impl_->sfe_injection_delay_ && impl_->sfe_injection_delay_ != delay ) {
        // reset time to original value
        impl_->timeRange_.first += double(impl_->sfe_injection_delay_)/std::nano::den;
        impl_->timeRange_.second += double(impl_->sfe_injection_delay_)/std::nano::den;
        std::transform( impl_->timeArray_.begin()
                        , impl_->timeArray_.end()
                        , impl_->timeArray_.begin()
                        , [&]( const auto& a ){ return a + double(impl_->sfe_injection_delay_)/std::nano::den; } );
    }

    if ( impl_->sfe_injection_delay_ != delay ) {
        // set new value
        impl_->sfe_injection_delay_ = delay;
        if ( impl_->sfe_injection_delay_ ) {
            impl_->timeRange_.first -= double(impl_->sfe_injection_delay_)/std::nano::den;
            impl_->timeRange_.second -= double(impl_->sfe_injection_delay_)/std::nano::den;
            std::transform( impl_->timeArray_.begin()
                            , impl_->timeArray_.end()
                            , impl_->timeArray_.begin()
                            , [&]( const auto& a ){ return a - double(impl_->sfe_injection_delay_)/std::nano::den; } );
        }
        return true;
    }
    return false;
}

std::optional< double >
Chromatogram::sfe_injection_delay() const
{
    if ( impl_->sfe_injection_delay_ )
        return double(impl_->sfe_injection_delay_) / std::nano::den;
    return {};
}
bool
Chromatogram::add_manual_peak( PeakResult& result, double t0, double t1, bool horizontalBaseline, double baseLevel ) const
{
    Peak pk;
    Baseline bs;

    auto it0 = std::lower_bound( impl_->timeArray_.begin(), impl_->timeArray_.end(), t0 );
    if ( it0 == impl_->timeArray_.end() )
        return false;

    auto it1 = std::lower_bound( impl_->timeArray_.begin(), impl_->timeArray_.end(), t1 );

    size_t pos0 = std::distance( impl_->timeArray_.begin(), it0 );
    size_t pos1 = std::distance( impl_->timeArray_.begin(), it1 );

    pk.setStartData({ std::int32_t(pos0), *it0, impl_->dataArray_.at( pos0 ) });
    pk.setEndData({ std::int32_t(pos1), *it1, impl_->dataArray_.at( pos1 ) });

    double area = std::accumulate( impl_->dataArray_.begin() + pos0, impl_->dataArray_.begin() + pos1, 0.0 );
    double height = *std::max_element( impl_->dataArray_.begin() + pos0, impl_->dataArray_.begin() + pos1 );

    pk.setStartTime( t0 );
    pk.setEndTime( t1 );
    pk.setPeakTime( t0 + ( t1 - t0 ) / 2.0 );
    pk.setPeakArea( area );
    pk.setPeakHeight( height );
    pk.setName( "added" );

    bs.setStartHeight( 0 );
    bs.setStopHeight( 0 );
    bs.setStartPos( pk.startPos() );
    bs.setStopPos( pk.endPos() );
    bs.setStartTime( t0 );
    bs.setStopTime( t1 );
    bs.setManuallyModified( true );

    int bsid = result.baselines().add( bs );
    pk.setBaseId( bsid );

    result.peaks().add( pk );

    return true;
}

std::pair< std::shared_ptr< Peak >, std::shared_ptr< Baseline > >
Chromatogram::find_single_peak( double t0, double t1, bool horizontalBaseline, double baseLevel ) const
{
    // ADDEBUG() << "---------- find_single_peak ------------- : " << std::make_pair( t0, t1 );
    if ( t0 < impl_->timeArray_.front() )
        t0 = impl_->timeArray_.front();

    if ( t1 < t0 || t1 > impl_->timeArray_.back() )
        t1 = impl_->timeArray_.back();

    auto it0 = std::lower_bound( impl_->timeArray_.begin(), impl_->timeArray_.end(), t0 );
    if ( it0 == impl_->timeArray_.end() )
        return {};

    auto it1 = std::lower_bound( impl_->timeArray_.begin(), impl_->timeArray_.end(), t1 );

    size_t pos0 = std::distance( impl_->timeArray_.begin(), it0 );
    size_t pos1 = std::distance( impl_->timeArray_.begin(), it1 );

    auto apex = std::distance( impl_->dataArray_.begin()
                               , std::max_element( impl_->dataArray_.begin() + pos0, impl_->dataArray_.begin() + pos1 ) );

    // ADDEBUG() << "---------- find_single_peak ------------- apex: " << apex << ", " << std::make_pair( pos0, pos1 );

    if ( auto pk = std::make_shared< Peak >() ) {
        pk->setTopData( { apex, impl_->timeArray_[ apex ], impl_->dataArray_[ apex ] } );

        pk->setPeakTime( impl_->timeArray_[ apex ] );
        pk->setPeakHeight( impl_->dataArray_[ apex ] - baseLevel );

        double h2 = ( pk->peakHeight() - baseLevel ) / 2.0;
        // double h5 = ( pk->peakHeight() - baseLevel ) / 20.0;
#if __cplusplus >= 201703L
        auto [spos, sfound] = find_first_cross_up()( h2, impl_->dataArray_.data(), pk->topPos() );
#else
        uint32_t spos;
        bool sfound;
        std::tie( spos, sfound ) = find_first_cross_up()( h2, impl_->dataArray_.data(), pk->topPos() );
#endif
        if ( sfound ) {
            double ha = impl_->dataArray_[ spos ];
            double hb = impl_->dataArray_[ spos + 1 ];
            double ta = impl_->timeArray_[ spos ];
            double tb = impl_->timeArray_[ spos + 1 ];
            double tt = ta + std::fabs( h2 - ha ) / ( hb - ha ) * ( tb - ta );
            pk->setStartData( { spos, tt, h2 } );
        } else {
            pk->setStartData( { spos, impl_->timeArray_[ spos ], h2 } );
        }
#if __cplusplus >= 201703L
        auto [epos, efound] = find_last_cross_down()( h2, impl_->dataArray_.data(), impl_->dataArray_.size(), pk->topPos() );
#else
        uint32_t epos;
        bool efound;
        std::tie( epos, efound ) = find_last_cross_down()( h2, impl_->dataArray_.data(), impl_->dataArray_.size(), pk->topPos() );
#endif
        if ( efound ) {
            double ha = impl_->dataArray_[ epos ];
            double hb = impl_->dataArray_[ epos + 1 ];
            double ta = impl_->timeArray_[ epos ];
            double tb = impl_->timeArray_[ epos + 1 ];
            double tt = ta + std::fabs( ha - h2 ) / ( ha - hb ) * ( tb - ta );
            pk->setEndData( { epos, tt, h2 } );
        } else {
            pk->setEndData( { epos, impl_->timeArray_[ epos ], h2 } );
        }

        double area = std::accumulate( impl_->dataArray_.begin() + pk->startPos(), impl_->dataArray_.begin() + pk->endPos(), 0.0 );
        pk->setPeakArea( area );
        pk->setName( "single peak" );
        pk->setPeakWidth( pk->endTime() - pk->startTime() );

        if ( auto bs = std::make_shared< Baseline >() ) {
            bs->setStartHeight( 0 );
            bs->setStopHeight( 0 );
            bs->setStartPos( pk->startPos() );
            bs->setStopPos( pk->endPos() );
            bs->setStartTime( pk->startTime() );
            bs->setStopTime( pk->endTime() );
            bs->setManuallyModified( true );
            return { pk, bs };
        }
    }
    return {};
}

Chromatogram::iterator
Chromatogram::begin()
{
    return Chromatogram_iterator( this, 0 );
}

Chromatogram::iterator
Chromatogram::end()
{
    return Chromatogram_iterator( this, size() );
}

Chromatogram::const_iterator
Chromatogram::begin() const
{
    return Chromatogram_iterator( this, 0 );
}

Chromatogram::const_iterator
Chromatogram::end() const
{
    return Chromatogram_iterator( this, size() );
}

/////////
Chromatogram_iterator::Chromatogram_iterator() : chromatogram_( 0 )
                                               , idx_( -1 )
{
}

Chromatogram_iterator::Chromatogram_iterator( const Chromatogram * p, size_t idx ) : chromatogram_( p )
                                                                                   , idx_( idx )
{
}

Chromatogram_iterator::Chromatogram_iterator( const Chromatogram_iterator& t ) : chromatogram_( t.chromatogram_ )
                                                                               , idx_( t.idx_ )
{
}

Chromatogram_iterator&
Chromatogram_iterator::operator = ( const Chromatogram_iterator& t )
{
    chromatogram_ = t.chromatogram_;
    idx_ = t.idx_;
    return *this;
}

const Chromatogram_iterator&
Chromatogram_iterator::operator ++ ()
{
    ++idx_;
    return *this;
}

const Chromatogram_iterator
Chromatogram_iterator::operator ++ ( int )
{
    return Chromatogram_iterator( chromatogram_, idx_++ );
}
