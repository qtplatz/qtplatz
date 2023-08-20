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

    namespace internal {

        class ChromatogramImpl {
        public:
            ~ChromatogramImpl();
            ChromatogramImpl();
            ChromatogramImpl( const ChromatogramImpl& );

            inline const double * getTimeArray() const { return timeArray_.empty() ? 0 : &timeArray_[0]; }
            inline const double * getDataArray() const { return dataArray_.empty() ? 0 : &dataArray_[0]; }

            inline const std::vector<Chromatogram::Event>& getEventVec() const { return evntVec_; }
            inline std::vector<Chromatogram::Event>& getEventVec() { return evntVec_; }

            inline size_t size() const { return dataArray_.size(); }
            inline const std::pair<double, double>& getAcquisitionTimeRange() const;
            inline double samplingInterval() const { return samplingInterval_; /* seconds */ }
            void setSamplingInterval( double v ) { samplingInterval_ = v; }

            bool isConstantSampledData() const { return isConstantSampling_; }
            void setIsConstantSampledData( bool b ) { isConstantSampling_ = b; }

            void setTime( size_t idx, const double& );
            void setData( size_t idx, const double& );
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
                }
            }
        };
    }
}

BOOST_CLASS_VERSION( adcontrols::internal::ChromatogramImpl, 10 )

namespace {

    struct find_first_cross_up {
        std::pair< size_t, bool > operator()( double th
                                              , const double * values
                                              , size_t size
                                              , size_t begin = 0 ) const {
            ADDEBUG() << "---------- find_first_cross_up ------------- ";
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
  delete pImpl_;
}

Chromatogram::Chromatogram() : pImpl_(0)
{
    pImpl_ = new ChromatogramImpl;
}

Chromatogram::Chromatogram( const Chromatogram& c ) : pImpl_(0)
{
    pImpl_ = new ChromatogramImpl( *c.pImpl_ );
}

Chromatogram&
Chromatogram::operator = ( const Chromatogram& t )
{
    if ( t.pImpl_ != pImpl_ ) {
        delete pImpl_;
        pImpl_ = new ChromatogramImpl( *t.pImpl_ );
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
    pImpl_->isCounting_ = counting;
}

bool
Chromatogram::isCounting() const
{
    return pImpl_->isCounting_;
}

void
Chromatogram::setProtocol( int fcn )
{
    pImpl_->proto_ = fcn;
}

int
Chromatogram::protocol() const
{
    return pImpl_->proto_;
}

size_t
Chromatogram::size() const
{
    return pImpl_->size();
}

void
Chromatogram::resize( size_t n )
{
    pImpl_->resize( n );
}

const Peaks&
Chromatogram::peaks() const
{
    return pImpl_->peaks_;
}

const Baselines&
Chromatogram::baselines() const
{
    return pImpl_->baselines_;
}

void
Chromatogram::setBaselines( const Baselines& baselines )
{
    pImpl_->baselines_ = baselines;
}

void
Chromatogram::setPeaks( const Peaks& peaks )
{
    pImpl_->peaks_ = peaks;
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
        pImpl_->baselines_ = {};
        pImpl_->peaks_ = {};
        pImpl_->baselines_.add( *bs );
        pImpl_->peaks_.add( *pk );
    }
}

bool
Chromatogram::isConstantSampledData() const
{
    return pImpl_->isConstantSampling_;
}

void
Chromatogram::setIsConstantSampledData( bool b )
{

    pImpl_->isConstantSampling_ = b;
}

double
Chromatogram::timeFromSampleIndex( size_t sampleIndex ) const
{
    return sampleIndex * pImpl_->samplingInterval();
}

double
Chromatogram::timeFromDataIndex( size_t index ) const
{
    return ( index + pImpl_->dataDelayPoints() ) * pImpl_->samplingInterval();
}

size_t
Chromatogram::toSampleIndex( double time, bool closest ) const
{
    size_t lower = unsigned( time / pImpl_->samplingInterval() );
    if ( ! closest )
        return lower;
    double ltime = lower * pImpl_->samplingInterval();
    double utime = ( lower + 1 ) * pImpl_->samplingInterval();
    if ( std::abs( time - ltime ) > std::abs( utime - ltime ) )
        return lower + 1;
    return lower;
}

size_t
Chromatogram::toDataIndex( double time, bool closest ) const
{
    return toSampleIndex( time, closest ) - pImpl_->dataDelayPoints();
}

std::pair<size_t,size_t>
Chromatogram::toIndexRange( double s, double e ) const
{
    if ( pImpl_->dataArray_.empty() )
        return { npos, npos };
    if ( pImpl_->timeArray_.empty() ) {
        return std::make_pair( toDataIndex( s ), toDataIndex( e ) );
    } else {
        auto its = std::lower_bound( pImpl_->timeArray_.begin(), pImpl_->timeArray_.end(), s );
        auto ite = std::lower_bound( pImpl_->timeArray_.begin(), pImpl_->timeArray_.end(), e );
        if ( ite != pImpl_->timeArray_.end() ) {
            // ADDEBUG() << "-------- Index returning end of vector -----------";
            --ite;
        }
        return std::make_pair( std::distance( pImpl_->timeArray_.begin(), its )
                               , std::distance( pImpl_->timeArray_.begin(), ite ) );
    }
}

const double *
Chromatogram::getIntensityArray() const
{
    return pImpl_->getDataArray();
}

void
Chromatogram::operator << ( const std::pair<double, double>& data )
{
    pImpl_->isConstantSampling_ = false;
    pImpl_->timeArray_.emplace_back( data.first );
    pImpl_->dataArray_.emplace_back( data.second );
    if ( pImpl_->timeArray_.size() == 1 )
        pImpl_->timeRange_.first = pImpl_->timeArray_.front();
    pImpl_->timeRange_.second = pImpl_->timeArray_.back();
}

void
Chromatogram::operator << ( std::pair<double, double>&& data )
{
    pImpl_->isConstantSampling_ = false;

    if ( pImpl_->timeArray_.empty() )
        pImpl_->timeRange_.first = data.first;
    pImpl_->timeRange_.second = data.first;

    pImpl_->timeArray_.emplace_back( data.first );
    pImpl_->dataArray_.emplace_back( data.second );
}

void
Chromatogram::operator << ( std::tuple<double, double, double, double >&& data )
{
    pImpl_->isConstantSampling_ = false;

    if ( pImpl_->timeArray_.empty() )
        pImpl_->timeRange_.first = std::get<0>(data);
    pImpl_->timeRange_.second = std::get<0>(data);

    pImpl_->timeArray_.emplace_back( std::get<0>( data ) );
    pImpl_->dataArray_.emplace_back( std::get<1>( data ) );

    pImpl_->tofArray_.emplace_back( std::get<2>( data ) );
    pImpl_->massArray_.emplace_back( std::get<3>( data ) );
}

const std::vector< double >&
Chromatogram::tofArray() const
{
    return pImpl_->tofArray_;
}

const std::vector< double >&
Chromatogram::massArray() const
{
    return pImpl_->massArray_;
}

double
Chromatogram::tof( size_t idx ) const
{
    if ( pImpl_->tofArray_.size() > idx )
        return pImpl_->tofArray_.at( idx );
    return 0;
}

double
Chromatogram::mass( size_t idx ) const
{
    if ( pImpl_->massArray_.size() > idx )
        return pImpl_->massArray_.at( idx );
    return 0;
}

double
Chromatogram::time( size_t idx ) const
{
    return pImpl_->timeArray_[ idx ];
}

double
Chromatogram::intensity( size_t idx ) const
{
    return pImpl_->dataArray_[ idx ];
}

void
Chromatogram::setIntensity( size_t idx, double d )
{
    pImpl_->setData( idx, d );
}

void
Chromatogram::setIntensityArray( const double * p, size_t sz )
{
    pImpl_->setDataArray( p, sz );
}

void
Chromatogram::setTime( size_t idx, double t )
{
    pImpl_->setTime( idx, t );
}

void
Chromatogram::setTimeArray( const double * p, size_t sz )
{
    pImpl_->setTimeArray( p, sz );
}

void
Chromatogram::addEvent( const Chromatogram::Event& e )
{
    pImpl_->getEventVec().push_back( e );
}

const double *
Chromatogram::getTimeArray() const
{
	if ( ( pImpl_->getTimeArray() == 0 ) && isConstantSampledData() ) {
		pImpl_->timeArray_.clear();
		const std::pair<double, double>& timerange = pImpl_->getAcquisitionTimeRange();
		const size_t nSize = pImpl_->size();
		for ( size_t i = 0; i < nSize; ++i )
			pImpl_->timeArray_.push_back( timerange.first + i * pImpl_->samplingInterval() );
	}
    return pImpl_->getTimeArray();
}

const Chromatogram::Event&
Chromatogram::getEvent( size_t idx ) const
{
    return pImpl_->getEventVec()[ idx ];
}

Chromatogram::seconds_t
Chromatogram::sampInterval() const
{
    auto value = pImpl_->samplingInterval();
    if ( adportable::compare<double>::essentiallyEqual( value, 0 ) ) {
        return ( pImpl_->timeRange_.second - pImpl_->timeRange_.first ) / ( size() - 1 );
    }
    return value;
}

void
Chromatogram::sampInterval( const seconds_t& v )
{
    pImpl_->samplingInterval_ = v;
}

boost::optional< std::string >
Chromatogram::axisLabel( plot::axis axis ) const
{
    auto it = pImpl_->axisLabels_.find( axis );
    if ( it != pImpl_->axisLabels_.end() ) {
        return it->second;
    }
    return {};
}

void
Chromatogram::setAxisLabel( plot::axis axis, const std::string& value )
{
    pImpl_->axisLabels_[ axis ] = value;
}

std::pair< plot::unit, size_t >
Chromatogram::axisUnit() const
{
    return pImpl_->yAxisUnit_;
}

void
Chromatogram::setAxisUnit( plot::unit unit, size_t den )
{
    pImpl_->yAxisUnit_ = { unit, den };
}


void
Chromatogram::minimumTime( const seconds_t& min )
{
    pImpl_->minTime( min );
}

void
Chromatogram::maximumTime( const seconds_t& min )
{
    pImpl_->maxTime( min );
}

void
Chromatogram::addDescription( const adcontrols::description& desc )
{
    pImpl_->addDescription( desc );
}

void
Chromatogram::addDescription( adcontrols::description&& desc )
{
    pImpl_->descriptions_.append( desc );
}

const descriptions&
Chromatogram::getDescriptions() const
{
    return pImpl_->getDescriptions();
}

// const descriptions&
// Chromatogram::descriptions() const
// {
//     return pImpl_->getDescriptions();
// }

Chromatogram::seconds_t
Chromatogram::minimumTime() const
{
    return pImpl_->getAcquisitionTimeRange().first;
}

Chromatogram::seconds_t
Chromatogram::maximumTime() const
{
    return pImpl_->getAcquisitionTimeRange().second;
}

std::pair<Chromatogram::seconds_t, Chromatogram::seconds_t>
Chromatogram::timeRange() const
{
    const ChromatogramImpl& d = *pImpl_;
    if ( d.timeRange_.second == 0 )
        return std::pair<seconds_t, seconds_t>( d.timeRange_.first, d.timeRange_.first + ( d.size() - 1 ) * d.samplingInterval() );
    return std::pair<seconds_t, seconds_t>( d.timeRange_.first, d.timeRange_.second );
}

size_t  // data index
Chromatogram::min_element( size_t beg, size_t end ) const
{
    const ChromatogramImpl& d = *pImpl_;

    if ( end >= d.size() )
        end = d.size() - 1;
    return std::distance( d.dataArray_.begin(), std::min_element( d.dataArray_.begin() + beg, d.dataArray_.begin() + end ) );
}

size_t   // data index
Chromatogram::max_element( size_t beg, size_t end ) const
{
    const ChromatogramImpl& d = *pImpl_;

    if ( end >= d.size() )
        end = d.size() - 1;
    return std::distance( d.dataArray_.begin(), std::max_element( d.dataArray_.begin() + beg, d.dataArray_.begin() + end ) );
}

double
Chromatogram::getMaxIntensity() const
{
    const ChromatogramImpl& d = *pImpl_;
    return *std::max_element( d.dataArray_.begin(), d.dataArray_.end() );
}

double
Chromatogram::getMinIntensity() const
{
    const ChromatogramImpl& d = *pImpl_;
    return *std::min_element( d.dataArray_.begin(), d.dataArray_.end() );
}

void
Chromatogram::set_time_of_injection( std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds> && t )
{
    pImpl_->time_of_injection_ = adportable::date_time::to_iso< std::chrono::nanoseconds >( t );
}

std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >
Chromatogram::time_of_injection() const
{
    if ( auto tp = adportable::iso8601::parse( pImpl_->time_of_injection_.begin(), pImpl_->time_of_injection_.end() ) )
        return *tp;
    return {};
}

std::string
Chromatogram::time_of_injection_iso8601() const
{
    if ( pImpl_->time_of_injection_.empty() ) {
        return adportable::date_time::to_iso< std::chrono::nanoseconds >(
            std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >{} );
    }
    return pImpl_->time_of_injection_;
}

// specialized template<> for boost::serialization
// template<class Archiver> void serialize(Archiver& ar, const unsigned int version);

namespace adcontrols {
    template<> void
    Chromatogram::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp("Chromatogram", pImpl_ );
    }

    template<> void
    Chromatogram::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        (void)version;
        ar >> boost::serialization::make_nvp("Chromatogram", pImpl_);
    }

    template<> void
    Chromatogram::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp( "Chromatogram", pImpl_ );
    }

    template<> void
    Chromatogram::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        (void)version;
        ar >> boost::serialization::make_nvp( "Chromatogram", pImpl_ );
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

std::wstring ChromatogramImpl::empty_string_ = L"";

ChromatogramImpl::~ChromatogramImpl()
{
}

ChromatogramImpl::ChromatogramImpl() : isConstantSampling_(true)
                                     , isCounting_( false )
                                     , dataDelayPoints_(0)
                                     , samplingInterval_(0)
                                     , proto_(0)
                                     , dataReaderUuid_( { {0} } )
                                     , dataGuid_( boost::uuids::random_generator()() ) // random generator
                                     , yAxisUnit_( { plot::Arbitrary, 0 } )
{
}

ChromatogramImpl::ChromatogramImpl( const ChromatogramImpl& t ) : isConstantSampling_( t.isConstantSampling_ )
                                                                , isCounting_( t.isCounting_ )
                                                                  // , descriptions_( t.descriptions_ )
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
{
    descriptions_ = t.descriptions_;
}

void
ChromatogramImpl::setData( size_t idx, const double& d )
{
    dataArray_[ idx ] = d;
}

void
ChromatogramImpl::setTime( size_t idx, const double& d ) // array of second
{
    if ( timeArray_.empty() )
        timeArray_.resize( dataArray_.size() );
    timeArray_[ idx ] = d;
}

void
ChromatogramImpl::setDataArray( const double * p, size_t sz )
{
    if ( p && sz ) {
        size_t end = std::min( sz, size() );
        std::copy( p, p + end, dataArray_.begin() );
    }
}

void
ChromatogramImpl::setTimeArray( const double * p, size_t sz ) // array of second
{
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
ChromatogramImpl::resize( size_t size )
{
    dataArray_.resize( size );
}

void
ChromatogramImpl::addDescription( const adcontrols::description& desc )
{
    descriptions_.append( desc );
}

const descriptions&
ChromatogramImpl::getDescriptions() const
{
    return descriptions_;
}

const std::pair<double, double>&
ChromatogramImpl::getAcquisitionTimeRange() const
{
    return timeRange_;
}

std::string
Chromatogram::make_title() const
{
    return make_folder_name<char>( pImpl_->descriptions_ );
}

//static
template<> std::string
Chromatogram::make_folder_name( const adcontrols::descriptions& descs )
{
    return descs.make_folder_name( "(MSLock)", true );
}

template<> std::wstring
Chromatogram::make_folder_name( const adcontrols::descriptions& descs )
{
    // ADDEBUG() << "make_folder_name: " << boost::json::value_from( descs );
    return descs.make_folder_name( L"(MSLock)", true );
}

// for v3 format datafile support
void
Chromatogram::setDataReaderUuid( const boost::uuids::uuid& uuid )
{
    pImpl_->dataReaderUuid_ = uuid;
}

const boost::uuids::uuid&
Chromatogram::dataReaderUuid() const
{
    return pImpl_->dataReaderUuid_;
}

void
Chromatogram::setGeneratorProperty( std::string&& prop )
{
    pImpl_->generator_property_ = std::move( prop );
}

void
Chromatogram::setGeneratorProperty( const std::string& prop )
{
    pImpl_->generator_property_ = prop;
}

boost::optional< std::string >
Chromatogram::generatorProperty() const
{
    return pImpl_->generator_property_;
}

void
Chromatogram::set_display_name( const std::string& t )
{
    pImpl_->display_name_ = t;
}

boost::optional< std::string >
Chromatogram::display_name() const
{
    if ( !pImpl_->display_name_.empty() )
        return pImpl_->display_name_;
    return {};
}

void
Chromatogram::setDataGuid( const boost::uuids::uuid& uuid )
{
    pImpl_->dataGuid_ = uuid;
}

const boost::uuids::uuid&
Chromatogram::dataGuid() const
{
    return pImpl_->dataGuid_;
}

bool
Chromatogram::add_manual_peak( PeakResult& result, double t0, double t1, bool horizontalBaseline, double baseLevel ) const
{
    Peak pk;
    Baseline bs;

    auto it0 = std::lower_bound( pImpl_->timeArray_.begin(), pImpl_->timeArray_.end(), t0 );
    if ( it0 == pImpl_->timeArray_.end() )
        return false;

    auto it1 = std::lower_bound( pImpl_->timeArray_.begin(), pImpl_->timeArray_.end(), t1 );

    size_t pos0 = std::distance( pImpl_->timeArray_.begin(), it0 );
    size_t pos1 = std::distance( pImpl_->timeArray_.begin(), it1 );

    pk.setStartData({ std::int32_t(pos0), *it0, pImpl_->dataArray_.at( pos0 ) });
    pk.setEndData({ std::int32_t(pos1), *it1, pImpl_->dataArray_.at( pos1 ) });

    double area = std::accumulate( pImpl_->dataArray_.begin() + pos0, pImpl_->dataArray_.begin() + pos1, 0.0 );
    double height = *std::max_element( pImpl_->dataArray_.begin() + pos0, pImpl_->dataArray_.begin() + pos1 );

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
    if ( t0 < pImpl_->timeArray_.front() )
        t0 = pImpl_->timeArray_.front();

    if ( t1 < t0 || t1 > pImpl_->timeArray_.back() )
        t1 = pImpl_->timeArray_.back();

    auto it0 = std::lower_bound( pImpl_->timeArray_.begin(), pImpl_->timeArray_.end(), t0 );
    if ( it0 == pImpl_->timeArray_.end() )
        return {};

    auto it1 = std::lower_bound( pImpl_->timeArray_.begin(), pImpl_->timeArray_.end(), t1 );

    size_t pos0 = std::distance( pImpl_->timeArray_.begin(), it0 );
    size_t pos1 = std::distance( pImpl_->timeArray_.begin(), it1 );

    auto apex = std::distance( pImpl_->dataArray_.begin()
                               , std::max_element( pImpl_->dataArray_.begin() + pos0, pImpl_->dataArray_.begin() + pos1 ) );

    // ADDEBUG() << "---------- find_single_peak ------------- apex: " << apex << ", " << std::make_pair( pos0, pos1 );

    if ( auto pk = std::make_shared< Peak >() ) {
        pk->setTopData( { apex, pImpl_->timeArray_[ apex ], pImpl_->dataArray_[ apex ] } );

        pk->setPeakTime( pImpl_->timeArray_[ apex ] );
        pk->setPeakHeight( pImpl_->dataArray_[ apex ] - baseLevel );

        double h2 = ( pk->peakHeight() - baseLevel ) / 2.0;
        // double h5 = ( pk->peakHeight() - baseLevel ) / 20.0;
#if __cplusplus >= 201703L
        auto [spos, sfound] = find_first_cross_up()( h2, pImpl_->dataArray_.data(), pk->topPos() );
#else
        uint32_t spos;
        bool sfound;
        std::tie( spos, sfound ) = find_first_cross_up()( h2, pImpl_->dataArray_.data(), pk->topPos() );
#endif
        if ( sfound ) {
            double ha = pImpl_->dataArray_[ spos ];
            double hb = pImpl_->dataArray_[ spos + 1 ];
            double ta = pImpl_->timeArray_[ spos ];
            double tb = pImpl_->timeArray_[ spos + 1 ];
            double tt = ta + std::fabs( h2 - ha ) / ( hb - ha ) * ( tb - ta );
            pk->setStartData( { spos, tt, h2 } );
        } else {
            pk->setStartData( { spos, pImpl_->timeArray_[ spos ], h2 } );
        }
#if __cplusplus >= 201703L
        auto [epos, efound] = find_last_cross_down()( h2, pImpl_->dataArray_.data(), pImpl_->dataArray_.size(), pk->topPos() );
#else
        uint32_t epos;
        bool efound;
        std::tie( epos, efound ) = find_last_cross_down()( h2, pImpl_->dataArray_.data(), pImpl_->dataArray_.size(), pk->topPos() );
#endif
        if ( efound ) {
            double ha = pImpl_->dataArray_[ epos ];
            double hb = pImpl_->dataArray_[ epos + 1 ];
            double ta = pImpl_->timeArray_[ epos ];
            double tb = pImpl_->timeArray_[ epos + 1 ];
            double tt = ta + std::fabs( ha - h2 ) / ( ha - hb ) * ( tb - ta );
            pk->setEndData( { epos, tt, h2 } );
        } else {
            pk->setEndData( { epos, pImpl_->timeArray_[ epos ], h2 } );
        }

        double area = std::accumulate( pImpl_->dataArray_.begin() + pk->startPos(), pImpl_->dataArray_.begin() + pk->endPos(), 0.0 );
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
