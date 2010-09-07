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
#include <adportable/array_wrapper.hpp>

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
		void clone( const MassSpectrumImpl&, bool deep = false );
	    
	    inline const double * getTimeArray() const { return tofArray_.empty() ? 0 : &tofArray_[0]; }
	    inline const double * getMassArray() const { return massArray_.empty() ? 0 : &massArray_[0]; }
	    inline const double * getIntensityArray() const { return intsArray_.empty() ? 0 : &intsArray_[0]; }
	    inline const unsigned char * getColorArray() const { return colArray_.empty() ? 0 : &colArray_[0];}
	    inline size_t size() const { return massArray_.size(); }
	    inline bool isCentroid() const { return algo_ != CentroidNone; }
        inline void setCentroid( CentroidAlgorithm algo ) { algo_ = algo; }
	    inline CentroidAlgorithm getCentroidAlgorithm() const { return algo_; }
	    inline const std::pair<double, double>& getAcquisitionMassRange() const { return acqRange_; }
        void setAcquisitionMassRange( double min, double max ) { acqRange_.first = min; acqRange_.second = max; }
	    void setTimeArray( const double * );
	    void setMassArray( const double *, bool setrange );
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
	if ( t.pImpl_ != pImpl_ ) {
		delete pImpl_;
		pImpl_ = new MassSpectrumImpl( *t.pImpl_ );
	}
	return *this;
}

void
MassSpectrum::clone( const MassSpectrum& t, bool deep )
{
	pImpl_->clone( *t.pImpl_, deep );
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

bool
MassSpectrum::isCentroid() const
{
    return pImpl_->isCentroid();
}

void
MassSpectrum::setCentroid( CentroidAlgorithm algo )
{
    return pImpl_->setCentroid( algo );
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

void
MassSpectrum::setMass( size_t idx, double mass )
{
    if ( idx < pImpl_->size() )
        const_cast<double *>( pImpl_->getMassArray() )[idx] = mass;
}

void
MassSpectrum::setIntensity( size_t idx, double intensity )
{
    if ( idx < pImpl_->size() )
        const_cast<double *>( pImpl_->getIntensityArray() )[idx] = intensity;
}

const double *
MassSpectrum::getTimeArray()
{
    return pImpl_->getTimeArray();
}

void
MassSpectrum::setAcquisitionMassRange( double min, double max )
{
    pImpl_->setAcquisitionMassRange( min, max );
}

void
MassSpectrum::setMassArray( const double * values, bool setRange )
{
    pImpl_->setMassArray( values, setRange );
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

void
MassSpectrum::addDescription( const Description& t )
{
  pImpl_->addDescription( t );
}

const Descriptions&
MassSpectrum::getDescriptions() const
{
  return pImpl_->getDescriptions();
}

template<class T> void
MassSpectrum::set( const T& t )
{
}

template<class T> const T&
MassSpectrum::get()
{
}

std::pair<double, double>
MassSpectrum::getAcquisitionMassRange() const
{
    return pImpl_->getAcquisitionMassRange();
}

double
MassSpectrum::getMinIntensity() const
{
	adportable::array_wrapper<const double> y( pImpl_->getIntensityArray(), size() );
	return *std::min_element( y.begin(), y.end() );
}

double
MassSpectrum::getMaxIntensity() const
{
	adportable::array_wrapper<const double> y( pImpl_->getIntensityArray(), size() );
	return *std::max_element( y.begin(), y.end() );
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

MassSpectrumImpl::MassSpectrumImpl( const MassSpectrumImpl& t )
{
	clone( t, true );
}

void
MassSpectrumImpl::clone( const MassSpectrumImpl& t, bool deep )
{
	algo_ = t.algo_;
	polarity_ = t.polarity_;
	acqRange_ = t.acqRange_;
	descriptions_ = t.descriptions_;
	if ( deep ) {
		tofArray_ = t.tofArray_;
		massArray_ = t.massArray_;
		intsArray_ = t.intsArray_;
		colArray_ = t.colArray_;
	} else {
		tofArray_.clear();
		massArray_.clear();
		intsArray_.clear();
		colArray_.clear();
	}
}

void
MassSpectrumImpl::setTimeArray( const double * p )
{
	if ( tofArray_.size() != size() )
		tofArray_.resize( size() );
	memcpy(&tofArray_[0], p, sizeof(double) * size() );
}

void
MassSpectrumImpl::setMassArray( const double * p, bool setrange )
{
    memcpy(&massArray_[0], p, sizeof(double) * size() );
    if ( setrange )
        setAcquisitionMassRange( massArray_[0], massArray_[ size() - 1 ] );
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
	if ( ! tofArray_.empty() )
       tofArray_.resize( size );
    if ( ! colArray_.empty() )
       colArray_.resize( size );
}

void
MassSpectrumImpl::addDescription( const Description& t )
{
	descriptions_.append( t );
}

const Descriptions&
MassSpectrumImpl::getDescriptions() const
{
	return descriptions_;
}
