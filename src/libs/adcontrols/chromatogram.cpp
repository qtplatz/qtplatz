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
	  inline const unsigned short * getEventArray() const { return evntArray_.empty() ? 0 : &evntArray_[0];}
	  inline size_t size() const { return dataArray_.size(); }
	  inline const std::pair<double, double>& getAcquisitionTimeRange() const { return timeRange_; }
	  inline double samplingInterval() const { return samplingInterval_; /* seconds */ }

	  void setTimeArray( const double * );
	  void setDataArray( const double * );
	  void setEventArray( const unsigned short * );
	  void resize( size_t );
	  void addDescription( const Description& );
	  const Descriptions& getDescriptions() const;
	   
	 private:
	  static std::wstring empty_string_;  // for error return as reference
	   
	  Descriptions descriptions_;
	   
	  std::vector< double > dataArray_;
	  std::vector< double > timeArray_;
	  std::vector< unsigned short > evntArray_;
	  std::pair<double, double> timeRange_;

	  double samplingInterval_;
	   
	  friend class boost::serialization::access;
	  template<class Archive> void serialize(Archive& ar, const unsigned int version) {
		if ( version >= 0 ) {
		  ar & BOOST_SERIALIZATION_NVP(samplingInterval_)
			& BOOST_SERIALIZATION_NVP(timeRange_.first) 
			& BOOST_SERIALIZATION_NVP(timeRange_.second) 
			& BOOST_SERIALIZATION_NVP(descriptions_)
			& BOOST_SERIALIZATION_NVP(dataArray_) 
			& BOOST_SERIALIZATION_NVP(timeArray_) 
			& BOOST_SERIALIZATION_NVP(evntArray_) 
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

const double *
Chromatogram::getDataArray() const
{
    return pImpl_->getDataArray();
}

const double *
Chromatogram::getTimeArray() const
{
    return pImpl_->getTimeArray();
}

const unsigned short *
Chromatogram::getEventArray() const
{
    return pImpl_->getEventArray();
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

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

std::wstring ChromatogramImpl::empty_string_ = L"";

ChromatogramImpl::~ChromatogramImpl()
{
}

ChromatogramImpl::ChromatogramImpl()
{
}

ChromatogramImpl::ChromatogramImpl( const ChromatogramImpl& t ) : dataArray_(t.dataArray_)
																, timeArray_(t.timeArray_)
																, evntArray_(t.evntArray_)
																, timeRange_(t.timeRange_)
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
ChromatogramImpl::setEventArray( const unsigned short * p )
{
  if ( evntArray_.size() != size() )
	evntArray_.resize( size() );
  memcpy(&evntArray_[0], p, sizeof( unsigned short ) * size() );
}

void
ChromatogramImpl::resize( size_t size )
{
  dataArray_.resize( size );
}
