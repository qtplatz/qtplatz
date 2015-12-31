// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>

#include "descriptions.hpp"
#include "peaks.hpp"
#include "peak.hpp"
#include "baselines.hpp"
#include "baseline.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

#include <compiler/diagnostic_pop.h>

#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/float.hpp>

#include <sstream>
#include <vector>

using namespace adcontrols;
using namespace adcontrols::internal;

namespace adcontrols {

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
            void samplingInterval( double v ) { samplingInterval_ = v; }
            bool isConstantSampledData() const { return isConstantSampling_; }
            void isConstantSampledData( bool b ) { isConstantSampling_ = b; }
            void setTime( size_t idx, const double& );
            void setData( size_t idx, const double& );
            void setTimeArray( const double * );
            void setDataArray( const double * );
            void setEventArray( const unsigned long * );
            void resize( size_t );
            void addDescription( const description& );
            const descriptions& getDescriptions() const;

            const std::wstring& axisLabelHorizontal() const { return axisLabelHorizontal_; }
            const std::wstring& axisLabelVertical() const { return axisLabelVertical_; }
            void axisLabelHorizontal( const std::wstring& v ) { axisLabelHorizontal_ = v; }
            void axisLabelVertical( const std::wstring& v ) { axisLabelVertical_ = v; }
            void minTime( double v ) { timeRange_.first = v; }
            void maxTime( double v ) { timeRange_.second = v; }
            void dataDelayPoints( size_t n ) { dataDelayPoints_ = n; }
            size_t dataDelayPoints() const { return dataDelayPoints_; }
	   
	    // private:
            friend class Chromatogram;
            static std::wstring empty_string_;  // for error return as reference
            bool isConstantSampling_;
	   
            descriptions descriptions_;
            Peaks peaks_;
            Baselines baselines_;

            std::vector< double > dataArray_;
            std::vector< double > timeArray_;
            std::vector< Chromatogram::Event > evntVec_;
            std::pair<double, double> timeRange_;
            size_t dataDelayPoints_;
            double samplingInterval_;
            std::wstring axisLabelHorizontal_;
            std::wstring axisLabelVertical_;
            int32_t fcn_;
	   
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, const unsigned int version) {
                ar & BOOST_SERIALIZATION_NVP(samplingInterval_)
                    & BOOST_SERIALIZATION_NVP(isConstantSampling_)
                    & BOOST_SERIALIZATION_NVP(timeRange_.first) 
                    & BOOST_SERIALIZATION_NVP(timeRange_.second) 
                    & BOOST_SERIALIZATION_NVP(dataDelayPoints_) 
                    & BOOST_SERIALIZATION_NVP(descriptions_)
                    & BOOST_SERIALIZATION_NVP(axisLabelHorizontal_)
                    & BOOST_SERIALIZATION_NVP(axisLabelVertical_)
                    & BOOST_SERIALIZATION_NVP(dataArray_) 
                    & BOOST_SERIALIZATION_NVP(timeArray_) 
                    & BOOST_SERIALIZATION_NVP(evntVec_) 
                    & BOOST_SERIALIZATION_NVP(peaks_) 
                    ;
                if ( version >= 2 ) 
                    ar & BOOST_SERIALIZATION_NVP( fcn_ );
            }
        };
    }
}

BOOST_CLASS_VERSION( adcontrols::internal::ChromatogramImpl, 2 )

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
Chromatogram::operator =( const Chromatogram& t )
{
    if ( t.pImpl_ != pImpl_ ) { // can't assign
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
Chromatogram::setFcn( int fcn )
{
    pImpl_->fcn_ = fcn;
}

int
Chromatogram::fcn() const
{
    return pImpl_->fcn_;
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

Peaks& 
Chromatogram::peaks()
{
    return pImpl_->peaks_;
}

const Peaks& 
Chromatogram::peaks() const
{
    return pImpl_->peaks_;
}

Baselines&
Chromatogram::baselines()
{
    return pImpl_->baselines_;
}

const Baselines&
Chromatogram::baselines() const
{
    return pImpl_->baselines_;
}

bool
Chromatogram::isConstantSampledData() const
{
    return pImpl_->isConstantSampledData();
}

void
Chromatogram::isConstantSampledData( bool b )
{
    pImpl_->isConstantSampledData( b );
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

const double *
Chromatogram::getIntensityArray() const
{
    return pImpl_->getDataArray();
}

void
Chromatogram::operator << ( const std::pair<double, double>& data )
{
    pImpl_->isConstantSampling_ = false;
    pImpl_->timeArray_.push_back( data.first );
    pImpl_->dataArray_.push_back( data.second );
    if ( pImpl_->timeArray_.size() == 1 )
        pImpl_->timeRange_.first = pImpl_->timeArray_.front();
    pImpl_->timeRange_.second = pImpl_->timeArray_.back();
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
Chromatogram::setIntensityArray( const double * p )
{
    pImpl_->setDataArray( p );
}

void
Chromatogram::setTime( size_t idx, double t )
{
    pImpl_->setTime( idx, t );
}

void
Chromatogram::setTimeArray( const double * p )
{
    ChromatogramImpl& d = *pImpl_;
    d.setTimeArray( p );
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
    return pImpl_->samplingInterval();
}

void
Chromatogram::sampInterval( const seconds_t& v )
{
    pImpl_->samplingInterval( v );
}

const std::wstring&
Chromatogram::axisLabelHorizontal() const
{
    return pImpl_->axisLabelHorizontal();
}

const std::wstring&
Chromatogram::axisLabelVertical() const
{
    return pImpl_->axisLabelVertical();
}

void
Chromatogram::axisLabelHorizontal( const std::wstring& v )
{
    pImpl_->axisLabelHorizontal( v );
}

void
Chromatogram::axisLabelVertical( const std::wstring& v )
{
    pImpl_->axisLabelVertical( v );
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

const descriptions&
Chromatogram::getDescriptions() const
{
    return pImpl_->getDescriptions();
}

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

// specialized template<> for boost::serialization
// template<class Archiver> void serialize(Archiver& ar, const unsigned int version);

namespace adcontrols {
    template<> void
    Chromatogram::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
	(void)version;
	ar << boost::serialization::make_nvp("Chromatogram", pImpl_);
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
                                     , dataDelayPoints_(0)
                                     , samplingInterval_(0)
                                     , fcn_(0)
{
}

ChromatogramImpl::ChromatogramImpl( const ChromatogramImpl& t ) : isConstantSampling_( t.isConstantSampling_ )
                                                                , peaks_( t.peaks_ )
                                                                , baselines_( t.baselines_ )
                                                                , dataArray_( t.dataArray_ )
                                                                , timeArray_( t.timeArray_ )
                                                                , evntVec_( t.evntVec_ )
                                                                , timeRange_( t.timeRange_)
                                                                , dataDelayPoints_ ( t.dataDelayPoints_ )   
                                                                , samplingInterval_( t.samplingInterval_ )
                                                                , axisLabelHorizontal_( t.axisLabelHorizontal_ )
                                                                , axisLabelVertical_( t.axisLabelVertical_ )
                                                                , fcn_( t.fcn_ )
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
ChromatogramImpl::setDataArray( const double * p )
{
    if ( p && size() ) {
		std::copy( p, p + size(), dataArray_.begin() );
	}
}

void
ChromatogramImpl::setTimeArray( const double * p ) // array of second
{
    if ( p ) {
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

//static
std::wstring
Chromatogram::make_folder_name( const adcontrols::descriptions& descs )
{
    std::wstring name;
    for ( auto& desc: descs ) {
        if ( ! name.empty() )
            name += L"/";
        name += desc.text();
    }
    return name;
}
