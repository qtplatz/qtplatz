// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "massspectrum.hpp"
#include "descriptions.hpp"
#include "mscalibration.hpp"
#include "msproperty.hpp"
#include "annotations.hpp"
#include "massspectra.hpp"
#include <adportable/array_wrapper.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

#include <compiler/diagnostic_pop.h>

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

           void setCalibration( const MSCalibration& );
           const MSCalibration& calibration() const;

           void setMSProperty( const MSProperty& );
           const MSProperty& getMSProperty() const;

           void setPolarity( MS_POLARITY polarity );
           MS_POLARITY polarity() const;

           void setAnnotations( const annotations& a ) {
               annotations_ = a;
           }
           const annotations& getAnnotations() const {
               return annotations_;
           }
	    
	  // private:
           static std::wstring empty_string_;  // for error return as reference

           CentroidAlgorithm algo_;
           MS_POLARITY polarity_;	    
           Descriptions descriptions_;
           MSCalibration calibration_;
           MSProperty property_;
           annotations annotations_;

           std::vector< double > tofArray_;
           std::vector< double > massArray_;
           std::vector< double > intsArray_;
           std::vector< unsigned char > colArray_;

           std::pair<double, double> acqRange_;
           long long timeSinceInjTrigger_; // usec
           long long timeSinceFirmwareUp_; // usec
           unsigned long numSpectrumSinceInjTrigger_;

           std::vector< MassSpectrum > vec_;
	    
           friend class MassSpectrum;

           friend class boost::serialization::access;
           template<class Archive> void serialize(Archive& ar, const unsigned int version) {
               ar & BOOST_SERIALIZATION_NVP(algo_)
                   & BOOST_SERIALIZATION_NVP(polarity_)
                   & BOOST_SERIALIZATION_NVP(acqRange_.first)
                   & BOOST_SERIALIZATION_NVP(acqRange_.second)
                   & BOOST_SERIALIZATION_NVP(descriptions_)
                   & BOOST_SERIALIZATION_NVP(calibration_)
                   & BOOST_SERIALIZATION_NVP(property_)
                   & BOOST_SERIALIZATION_NVP(massArray_)
                   & BOOST_SERIALIZATION_NVP(intsArray_)
                   & BOOST_SERIALIZATION_NVP(tofArray_)
                   & BOOST_SERIALIZATION_NVP(colArray_)
                   & BOOST_SERIALIZATION_NVP( annotations_ );
               if ( version >= 2 ) {
                   ar & BOOST_SERIALIZATION_NVP( vec_ );
               }
           }
       };
    }
}

BOOST_CLASS_VERSION( adcontrols::internal::MassSpectrumImpl, 2 )

///////////////////////////////////////////

using namespace adcontrols;
using namespace adcontrols::internal;

MassSpectrum::~MassSpectrum()
{
    delete pImpl_;
}

MassSpectrum::MassSpectrum() : pImpl_( new MassSpectrumImpl )
{
}

MassSpectrum::MassSpectrum( const MassSpectrum& t ) : pImpl_( new MassSpectrumImpl( *t.pImpl_ ) )
{
}

MassSpectrum&
MassSpectrum::operator = ( const MassSpectrum& t )
{
	if ( t.pImpl_ != pImpl_ ) {
		delete pImpl_;
		pImpl_ = new MassSpectrumImpl( *t.pImpl_ );
	}
	return *this;
}

MassSpectrum&
MassSpectrum::operator += ( const MassSpectrum& t )
{
    const size_t nSize = size();
	if ( t.size() >= nSize && std::abs( t.getMass(0) - getMass(0) ) <= 1.0e-9 ) {
		const double * src = t.getIntensityArray();
		double * dst = &pImpl_->intsArray_[0];
		for ( size_t i = 0; i < nSize; ++i )
			*dst++ += *src++;
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

void
MassSpectrum::setPolarity( MS_POLARITY polarity )
{
    pImpl_->setPolarity( polarity );
}

MS_POLARITY
MassSpectrum::polarity() const
{
    return pImpl_->polarity();
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

double
MassSpectrum::getMass( size_t idx ) const
{
    if ( idx < pImpl_->size() )
        return pImpl_->getMassArray()[idx];
    return 0;
}

double
MassSpectrum::getIntensity( size_t idx ) const
{
    if ( idx < pImpl_->size() )
        return pImpl_->getIntensityArray()[idx];
    return 0;
}

double
MassSpectrum::getTime( size_t idx ) const
{
    if ( idx < pImpl_->size() ) {
        const double * p = pImpl_->getTimeArray();
        if ( p )
            return p[ idx ];
        return MSProperty::toSeconds( idx, pImpl_->getMSProperty().getSamplingInfo() );
    }
    return 0;
}

void
MassSpectrum::setIntensity( size_t idx, double intensity )
{
    if ( idx < pImpl_->size() )
        const_cast<double *>( pImpl_->getIntensityArray() )[idx] = intensity;
}

void
MassSpectrum::setTime( size_t idx, double time )
{
    if ( pImpl_->tofArray_.empty() )
        pImpl_->tofArray_.resize( pImpl_->size() );
    if ( idx < pImpl_->size() )
        const_cast<double *>( pImpl_->getTimeArray() )[idx] = time;
}

const double *
MassSpectrum::getTimeArray() const
{
    return pImpl_->getTimeArray();
}

size_t
MassSpectrum::compute_profile_time_array( double * p, size_t size ) const
{
    if ( pImpl_->getTimeArray() ) {
        size_t i;
        for ( i = 0; i < size && i < pImpl_->size(); ++i )
            *p++ = getTimeArray()[ i ];
        return i;
    }
    return MSProperty::compute_profile_time_array( p, size, pImpl_->getMSProperty().getSamplingInfo() );
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
MassSpectrum::setColor( size_t idx, unsigned char color )
{
    if ( idx < pImpl_->size() ) {
        if ( pImpl_->getColorArray() == 0 )
            pImpl_->colArray_.resize( pImpl_->size() );
        const_cast< unsigned char *>( pImpl_->getColorArray() )[ idx ] = color;
    }
}

int
MassSpectrum::getColor( size_t idx ) const
{
    if ( idx < pImpl_->size() && pImpl_->getColorArray() )
        return pImpl_->getColorArray()[ idx ];
    return -1;
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

const MSCalibration&
MassSpectrum::calibration() const
{
    return pImpl_->calibration();
}

void
MassSpectrum::setCalibration( const MSCalibration& calib, bool assignMasses )
{
    pImpl_->setCalibration( calib );
    if ( assignMasses ) {
        for ( size_t i = 0; i < pImpl_->size(); ++i ) {
            double tof = getTime( i );
            double mq = MSCalibration::compute( calib.coeffs(), tof );
            if ( mq > 0.0 )
                setMass( i, mq * mq );
        }
    }
}

const MSProperty&
MassSpectrum::getMSProperty() const
{
    return pImpl_->getMSProperty();
}

void
MassSpectrum::setMSProperty( const MSProperty& prop )
{
    pImpl_->setMSProperty( prop );
}

void
MassSpectrum::setAnnotations( const annotations& annots )
{
    pImpl_->setAnnotations( annots );
}

const annotations&
MassSpectrum::getAnnotations() const
{
    return pImpl_->getAnnotations();
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
    if ( ! isCentroid() ) {
        adportable::array_wrapper<const double> y( pImpl_->getIntensityArray(), size() );
        if ( y )
            return *std::min_element( y.begin(), y.end() );
    }
    return 0;
}

double
MassSpectrum::getMaxIntensity() const
{
	adportable::array_wrapper<const double> y( pImpl_->getIntensityArray(), size() );
    if ( y )
        return *std::max_element( y.begin(), y.end() );
    return 0;
}

std::wstring
MassSpectrum::saveXml() const
{
   std::wostringstream o;
   boost::archive::xml_woarchive ar( o );
   ar << boost::serialization::make_nvp("MassSpectrum", pImpl_);
   return o.str();
}

void
MassSpectrum::loadXml( const std::wstring& xml )
{
   std::wistringstream in( xml );
   boost::archive::xml_wiarchive ar( in );
   ar >> boost::serialization::make_nvp("MassSpectrum", pImpl_);
}

bool
MassSpectrum::archive( std::ostream& os, const MassSpectrum& ms )
{
    portable_binary_oarchive ar( os );
    ar << ms;
    return true;
}

bool
MassSpectrum::restore( std::istream& is, MassSpectrum& ms )
{
    portable_binary_iarchive ar( is );
    ar >> ms;
    return true;
}

namespace adcontrols {

    template<> void
    MassSpectrum::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp( "MassSpectrum", pImpl_ );
    }

    template<> void
    MassSpectrum::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        (void)version;
        ar >> boost::serialization::make_nvp("MassSpectrum", pImpl_);
    }

    template<> void
    MassSpectrum::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp("MassSpectrum", pImpl_);
    }

    template<> void
    MassSpectrum::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        (void)version;
        ar >> boost::serialization::make_nvp("MassSpectrum", pImpl_);
    }
}

// Handling of segmented/fragmented spectrum
// Not only InfiTOF but also FASTFLIGHT data acquisition system generate a spectrum in chunk of fragment spectra
// that cause strange spectral draw image since disconnected reagions are draw by strait line
// Here is the trial implementation to help a spectrum that was acquired in chunks
// This also intend to apply separate spectral processing algorithms depend on each fragment

// Parent MassSpectrum class hold full ownership of the fragments memory.
// first fragment 'MassSpectrum' class points to parent MassSpectrum class it-self
// so the MassSpectrum[0] is identical to MassSpectrum if there is subsequent fragment.


size_t
MassSpectrum::addSegment( const MassSpectrum& sub )
{
    pImpl_->vec_.push_back( sub );
    return pImpl_->vec_.size();
}

MassSpectrum&
MassSpectrum::getSegment( size_t fcn )
{
    if ( pImpl_->vec_.size() > fcn )
        return pImpl_->vec_[ fcn ];
    throw std::out_of_range( "MassSpectrum fragments subscript out of range" );
}

const MassSpectrum&
MassSpectrum::getSegment( size_t fcn ) const
{
    if ( pImpl_->vec_.size() > fcn )
        return pImpl_->vec_[ fcn ];
    throw std::out_of_range( "MassSpectrum fragments subscript out of range" );
}

const MassSpectrum&
MassSpectrum::operator [] ( size_t fcn ) const
{
    return getSegment( fcn );
}

MassSpectrum&
MassSpectrum::operator [] ( size_t fcn )
{
    return getSegment( fcn );
}

size_t
MassSpectrum::numSegments() const
{
    return pImpl_->vec_.size();
}

void
MassSpectrum::clearSegments()
{
    pImpl_->vec_.clear();
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
                                     , timeSinceInjTrigger_(0)
                                     , timeSinceFirmwareUp_(0)
                                     , numSpectrumSinceInjTrigger_(0)
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
    calibration_ = t.calibration_;
    property_ = t.property_;

	if ( deep ) {
		tofArray_ = t.tofArray_;
		massArray_ = t.massArray_;
		intsArray_ = t.intsArray_;
		colArray_ = t.colArray_;
        annotations_ = t.annotations_;
        vec_ = t.vec_;
	} else {
		tofArray_.clear();
		massArray_.clear();
		intsArray_.clear();
        colArray_.clear();
        annotations_.clear();
        vec_.clear();
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
    if ( p ) {
        if ( colArray_.size() != size() )
            colArray_.resize( size() );
        memcpy(&colArray_[0], p, sizeof( unsigned char ) * size() );
    } else {
        colArray_.clear();
    }
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

const MSCalibration&
MassSpectrumImpl::calibration() const
{
    return calibration_;
}

void
MassSpectrumImpl::setCalibration( const MSCalibration& calib )
{
    calibration_ = calib;
}

const MSProperty&
MassSpectrumImpl::getMSProperty() const
{
    return property_;
}

void
MassSpectrumImpl::setMSProperty( const MSProperty& prop )
{
    property_ = prop;
}

void
MassSpectrumImpl::setPolarity( MS_POLARITY polarity )
{
    polarity_ = polarity;
}

MS_POLARITY
MassSpectrumImpl::polarity() const
{
    return polarity_;
}

////////////
double
segments_helper::max_intensity( const MassSpectrum& ms )
{
    double y = ms.getMaxIntensity();
    for ( size_t i = 0; i < ms.numSegments(); ++i ) {
        double t = ms.getSegment( i ).getMaxIntensity();
        if ( y < t )
            y = t;
    }
    return y;
}

void
segments_helper::set_color( MassSpectrum& ms, size_t fcn, size_t idx, int color )
{
    if ( fcn == 0 )
        return ms.setColor( idx, color );
    return ms.getSegment( fcn - 1 ).setColor( idx, color );
}

int
segments_helper::get_color( const MassSpectrum& ms, size_t fcn, size_t idx )
{
    if ( fcn == 0 )
        return ms.getColor( idx );
    return ms.getSegment( fcn - 1 ).getColor( idx );
}
