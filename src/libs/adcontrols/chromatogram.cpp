//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "chromatogram.h"
#include "descriptions.h"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

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
	  inline const std::pair<double, double>& getAcquisitionTimeRange() const { return timeRange_; }
	  inline double samplingInterval() const { return samplingInterval_; /* seconds */ }
      void samplingInterval( double v ) { samplingInterval_ = v; }
      bool isConstantSampledData() const { return isConstantSampling_; }
      void isConstantSampledData( bool b ) { isConstantSampling_ = b; }
	  void setTimeArray( const double * );
	  void setDataArray( const double * );
	  void setEventArray( const unsigned long * );
	  void resize( size_t );
	  void addDescription( const Description& );
	  const Descriptions& getDescriptions() const;

      const std::wstring& axisLabelHorizontal() const { return axisLabelHorizontal_; }
      const std::wstring& axisLabelVertical() const { return axisLabelVertical_; }
      void axisLabelHorizontal( const std::wstring& v ) { axisLabelHorizontal_ = v; }
      void axisLabelVertical( const std::wstring& v ) { axisLabelVertical_ = v; }
      void minTime( double v ) { timeRange_.first = v; }
      void maxTime( double v ) { timeRange_.second = v; }
      void dataDelayPoints( size_t n ) { dataDelayPoints_ = n; }
      size_t dataDelayPoints() const { return dataDelayPoints_; }
	   
    private:
        static std::wstring empty_string_;  // for error return as reference
        bool isConstantSampling_;
	   
        Descriptions descriptions_;

        std::vector< double > dataArray_;
        std::vector< double > timeArray_;
        std::vector< Chromatogram::Event > evntVec_;
        std::pair<double, double> timeRange_;
        size_t dataDelayPoints_;
        double samplingInterval_;
        std::wstring axisLabelHorizontal_;
        std::wstring axisLabelVertical_;
	   
        friend class boost::serialization::access;
	  template<class Archive> void serialize(Archive& ar, const unsigned int version) {
		if ( version >= 0 ) {
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
              ;
		}
	  }
	};
  }
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
Chromatogram::operator =( const Chromatogram& t )
{
    if ( t.pImpl_ != pImpl_ ) { // can't assign
      delete pImpl_;
      pImpl_ = new ChromatogramImpl( *t.pImpl_ );
  }
  return *this;
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
Chromatogram::getDataArray() const
{
    return pImpl_->getDataArray();
}

void
Chromatogram::setDataArray( const double * p )
{
    pImpl_->setDataArray( p );
}

void
Chromatogram::setTimeArray( const double * p )
{
    pImpl_->setTimeArray( p );
}

void
Chromatogram::addEvent( const Chromatogram::Event& e )
{
    pImpl_->getEventVec().push_back( e );
}

const double *
Chromatogram::getTimeArray() const
{
    return pImpl_->getTimeArray();
}

const Chromatogram::Event& 
Chromatogram::getEvent( size_t idx ) const
{
    return pImpl_->getEventVec()[ idx ];
}

double
Chromatogram::sampInterval() const
{
    return pImpl_->samplingInterval();
}

void
Chromatogram::sampInterval( double v )
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
Chromatogram::minTime( double min )
{
    pImpl_->minTime( min );
}

void
Chromatogram::maxTime( double min )
{
    pImpl_->maxTime( min );
}

void
Chromatogram::addDescription( const adcontrols::Description& desc )
{
    pImpl_->addDescription( desc );
}

const Descriptions&
Chromatogram::getDescriptions() const
{
    return pImpl_->getDescriptions();
}

double
Chromatogram::minTime() const
{
    return pImpl_->getAcquisitionTimeRange().first;    
}

double
Chromatogram::maxTime() const
{
    return pImpl_->getAcquisitionTimeRange().second;
}


// specialized template<> for boost::serialization
// template<class Archiver> void serialize(Archiver& ar, const unsigned int version);
template<> void
Chromatogram::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
{
    if ( version >= 0 ) {
        ar << boost::serialization::make_nvp("Chromatogram", pImpl_);
    }
}

template<> void
Chromatogram::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
{
    if ( version >= 0 ) {
	    ar >> boost::serialization::make_nvp("Chromatogram", pImpl_);
    }
}

/////////////
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

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

std::wstring ChromatogramImpl::empty_string_ = L"";

ChromatogramImpl::~ChromatogramImpl()
{
}

ChromatogramImpl::ChromatogramImpl() : dataDelayPoints_(0)
                                     , samplingInterval_(0.5)
                                     , isConstantSampling_(true) 
{
}

ChromatogramImpl::ChromatogramImpl( const ChromatogramImpl& t ) : dataArray_(t.dataArray_)
																, timeArray_(t.timeArray_)
																, evntVec_(t.evntVec_)
																, timeRange_(t.timeRange_)
                                                                , samplingInterval_( t.samplingInterval_ )
                                                                , axisLabelHorizontal_( t.axisLabelHorizontal_ )
                                                                , axisLabelVertical_( t.axisLabelVertical_ )
                                                                , dataDelayPoints_ ( t.dataDelayPoints_ )   
{
    
}

void
ChromatogramImpl::setDataArray( const double * p )
{
    memcpy(&dataArray_[0], p, sizeof(double) * size() );
}

void
ChromatogramImpl::setTimeArray( const double * p )
{
    if ( timeArray_.size() != size() )
        timeArray_.resize( size() );
    memcpy(&timeArray_[0], p, sizeof(double) * size() );
}

void
ChromatogramImpl::resize( size_t size )
{
  dataArray_.resize( size );
}

void
ChromatogramImpl::addDescription( const adcontrols::Description& desc )
{
    descriptions_.append( desc );
}

const Descriptions&
ChromatogramImpl::getDescriptions() const
{
    return descriptions_;
}

