//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "massspectrum.h"
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
#include <map>

namespace adcontrols {
   namespace internal {

      class MassSpectrumImpl {
	 public:
	    ~MassSpectrumImpl();
	    MassSpectrumImpl();
	    MassSpectrumImpl( const MassSpectrumImpl& );
	    
	    inline const double * getTimeArray() const { return tofArray_.empty() ? 0 : &tofArray_[0]; }
	    inline const double * getMassArray() const { return massArray_.empty() ? 0 : &massArray_[0]; }
	    inline const double * getIntensityArray() const { return intsArray_.empty() ? 0 : &intsArray_[0]; }
	    inline const unsigned char * getColorArray() const { return colArray_.empty() ? 0 : &colArray_[0];}
	    inline size_t size() const { return massArray_.size(); }
	    inline bool isCentroid() const { return algo_ != CentroidNone; }
	    inline CentroidAlgorithm getCentroidAlgorithm() const { return algo_; }
	    inline const std::pair<double, double>& getAcquisitionMassRange() const { return acqRange_; }
	    void setTimeArray( const double * );
	    void setMassArray( const double * );
	    void setIntensityArray( const double * );
	    void setColorArray( const unsigned char * );
	    void resize( size_t );

	    void addDescription( const Description& );
        const Descriptions& getDescriptions() const;
	    
	 private:
	    static std::wstring empty_string_;  // for error return as reference

	    CentroidAlgorithm algo_;
	    MS_POLARITY polarity_;	    
	    Descriptions descriptions_;

	    std::vector< double > tofArray_;
	    std::vector< double > massArray_;
	    std::vector< double > intsArray_;
	    std::vector< unsigned char > colArray_;

	    std::pair<double, double> acqRange_;
	    long long timeSinceInjTrigger_; // usec
	    long long timeSinceFirmwareUp_; // usec
	    unsigned long numSpectrumSinceInjTrigger_;
	    
	    friend class boost::serialization::access;
	    template<class Archive> void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP(algo_) 
                    & BOOST_SERIALIZATION_NVP(polarity_) 
                    & BOOST_SERIALIZATION_NVP(acqRange_.first) 
                    & BOOST_SERIALIZATION_NVP(acqRange_.second) 
                    & BOOST_SERIALIZATION_NVP(descriptions_)
                    & BOOST_SERIALIZATION_NVP(massArray_) 
                    & BOOST_SERIALIZATION_NVP(intsArray_) 
                    & BOOST_SERIALIZATION_NVP(tofArray_) 
                    & BOOST_SERIALIZATION_NVP(colArray_) 
                    ;
            }
	    }
	};
  }
}
///////////////////////////////////////////

using namespace adcontrols;
using namespace adcontrols::internal;

MassSpectrum::~MassSpectrum()
{
    delete pImpl_;
}

MassSpectrum::MassSpectrum() : pImpl_(0)
{
    pImpl_ = new MassSpectrumImpl;
}

MassSpectrum::MassSpectrum( const MassSpectrum& t ) : pImpl_(0)
{
    pImpl_ = new MassSpectrumImpl( *t.pImpl_ );
}

MassSpectrum&
MassSpectrum::operator =( const MassSpectrum& t )
{
    if ( t.pImpl_ != pImpl_ ) { // can't assign
      delete pImpl_;
      pImpl_ = new MassSpectrumImpl( *t.pImpl_ );
  }
  return *this;
}

size_t
MassSpectrum::size() const
{
    return pImpl_->size();
}

void
MassSpectrum::resize( size_t n )
{
  pImpl_->resize( n );
}

const double *
MassSpectrum::getMassArray() const
{
    return pImpl_->getMassArray();
}

const double *
MassSpectrum::getIntensityArray() const
{
    return pImpl_->getIntensityArray();
}

const double *
MassSpectrum::getTimeArray()
{
    return pImpl_->getTimeArray();
}

void
MassSpectrum::setMassArray( const double * values )
{
    pImpl_->setMassArray( values );
}

void
MassSpectrum::setIntensityArray( const double * values )
{
    pImpl_->setIntensityArray( values );
}

void
MassSpectrum::setTimeArray( const double * values )
{
   pImpl_->setTimeArray( values );
}

const unsigned char *
MassSpectrum::getColorArray() const
{
    return pImpl_->getColorArray();
}

void
MassSpectrum::setColorArray( const unsigned char * values )
{
    pImpl_->setColorArray( values );
}

template<class T> void
MassSpectrum::set( const T& t )
{
}

template<class T> const T&
MassSpectrum::get()
{
}

std::wstring
MassSpectrum::saveXml() const
{
   std::wostringstream o;
   boost::archive::xml_woarchive ar( o );
   ar << boost::serialization::make_nvp("MssSpectrum", pImpl_);
   return o.str();
}

void
MassSpectrum::loadXml( const std::wstring& xml )
{
   std::wistringstream in( xml );
   boost::archive::xml_wiarchive ar( in );
   ar >> boost::serialization::make_nvp("MassSpectrum", pImpl_);
}
      
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

std::wstring MassSpectrumImpl::empty_string_ = L"";

MassSpectrumImpl::~MassSpectrumImpl()
{
}

MassSpectrumImpl::MassSpectrumImpl() : algo_(CentroidNone)
				     , polarity_(PolarityIndeterminate)
{
}

MassSpectrumImpl::MassSpectrumImpl( const MassSpectrumImpl& t ) : algo_(t.algo_)
								, polarity_(t.polarity_)
								, tofArray_(t.tofArray_)
								, massArray_(t.massArray_)
								, intsArray_(t.intsArray_)
								, colArray_(t.colArray_)
								, acqRange_(t.acqRange_)
{
}

void
MassSpectrumImpl::setTimeArray( const double * p )
{
  if ( tofArray_.size() != size() )
	tofArray_.resize( size() );
  memcpy(&tofArray_[0], p, sizeof(double) * size() );
}

void
MassSpectrumImpl::setMassArray( const double * p )
{
  memcpy(&massArray_[0], p, sizeof(double) * size() );
}

void
MassSpectrumImpl::setIntensityArray( const double * p )
{
  memcpy(&intsArray_[0], p, sizeof(double) * size() );
}

void
MassSpectrumImpl::setColorArray( const unsigned char * p )
{
  if ( colArray_.size() != size() )
	colArray_.resize( size() );
  memcpy(&colArray_[0], p, sizeof( unsigned char ) * size() );
}

void
MassSpectrumImpl::resize( size_t size )
{
  massArray_.resize( size );
  intsArray_.resize( size );
}

