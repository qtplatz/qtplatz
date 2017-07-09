// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
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
#include "annotations.hpp"
#include "descriptions.hpp"
#include "mscalibration.hpp"
#include "msproperty.hpp"
#include "massspectra.hpp"
#include "samplinginfo.hpp"

#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

#include <algorithm>
#include <sstream>
#include <tuple>
#include <vector>
#include <map>

namespace adcontrols {
    namespace internal {

        struct datum {
            size_t idx;
            double mass;
            double time;
            double intensity;
            unsigned char color;
            datum( size_t _idx, double _mass, double _tof, double _intensity, unsigned char _color ) :
                idx( _idx ), mass( _mass ), time( _tof ), intensity( _intensity ), color( _color ) {
            }

            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, const unsigned int ) {
                ar & BOOST_SERIALIZATION_NVP( idx );
                ar & BOOST_SERIALIZATION_NVP( time );
                ar & BOOST_SERIALIZATION_NVP( mass );
                ar & BOOST_SERIALIZATION_NVP( intensity );
                ar & BOOST_SERIALIZATION_NVP( color );
            }
        };

       class MassSpectrumImpl {
       public:
           ~MassSpectrumImpl();
           MassSpectrumImpl();
           MassSpectrumImpl( const MassSpectrumImpl& );
           void clone( const MassSpectrumImpl&, bool deep = false );
	    
           inline const double * getTimeArray() const { return tofArray_.size() ? tofArray_.data() : 0; }
           inline const double * getMassArray() const { return massArray_.size() ? massArray_.data() : 0; }
           inline const double * getIntensityArray() const { return intsArray_.size() ? intsArray_.data() : 0; }
           inline const unsigned char * getColorArray() const { return colArray_.size() ? colArray_.data() : 0; }
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

           void addDescription( const description& );
           const descriptions& getDescriptions() const;

           void setCalibration( const MSCalibration& );
           const MSCalibration& calibration() const;

           void setMSProperty( const MSProperty& );
           const MSProperty& getMSProperty() const;

           void setPolarity( MS_POLARITY polarity );
           MS_POLARITY polarity() const;

           inline void set_annotations( const annotations& a ) { annotations_ = a; }
           inline const annotations& get_annotations() const {  return annotations_;  }
		   inline annotations& get_annotations() {  return annotations_;  }
	    
	  // private:
           static std::wstring empty_string_;  // for error return as reference

           CentroidAlgorithm algo_;
           MS_POLARITY polarity_;	    
           descriptions descriptions_;
           MSCalibration calibration_;
           MSProperty property_;
           annotations annotations_;

           std::vector< double > tofArray_;
           std::vector< double > massArray_;
           std::vector< double > intsArray_;
           std::vector< unsigned char > colArray_;

           std::pair<double, double> acqRange_;
           int64_t timeSinceInjTrigger_; // usec
           int64_t timeSinceFirmwareUp_; // usec
           uint32_t numSpectrumSinceInjTrigger_;

           std::vector< std::shared_ptr< MassSpectrum > > vec_;
           int32_t protocolId_;
           int32_t nProtocols_;
           boost::uuids::uuid dataReaderUuid_;
           int64_t rowid_; // SQLite's rowid for raw data record reference

           // exclude from archive
           std::string uuid_; // for instance equality check; out of serialization scope
           std::tuple< bool, double, double > minmax_;
	    
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
                   & BOOST_SERIALIZATION_NVP( annotations_ )
                   ;
               if ( version >= 2 ) {
                   if ( version >= 5 ) {
                       ar & BOOST_SERIALIZATION_NVP( vec_ );
                   } else {
                       vec_.clear();                       
                       std::vector< MassSpectrum > v;
                       ar & BOOST_SERIALIZATION_NVP( v );
                       for ( const auto& a: v ) { vec_.emplace_back( std::make_shared< MassSpectrum >( a ) ); }
                   }
               }
               if ( version >= 3 ) {
                   ar & BOOST_SERIALIZATION_NVP( protocolId_ );
                   ar & BOOST_SERIALIZATION_NVP( nProtocols_ );
               }
               if ( version >= 4 ) {
                   ar & BOOST_SERIALIZATION_NVP( dataReaderUuid_ );
                   ar & BOOST_SERIALIZATION_NVP( rowid_ );
               }

               // exclude
               // scanLaw_.reset();
               minmax_ = std::make_tuple( false, 0.0, 0.0 );
           }
       };

        template<> void MassSpectrumImpl::serialize( boost::archive::xml_woarchive& ar, const unsigned int ) {

            ar & BOOST_SERIALIZATION_NVP( algo_ );
            ar & BOOST_SERIALIZATION_NVP( polarity_ );
            ar & BOOST_SERIALIZATION_NVP( acqRange_.first );
            ar & BOOST_SERIALIZATION_NVP( acqRange_.second );
            ar & BOOST_SERIALIZATION_NVP( descriptions_ );
            ar & BOOST_SERIALIZATION_NVP( calibration_ );
            ar & BOOST_SERIALIZATION_NVP( property_ );
            ar & BOOST_SERIALIZATION_NVP( annotations_ );
            ar & BOOST_SERIALIZATION_NVP( protocolId_ );
            ar & BOOST_SERIALIZATION_NVP( nProtocols_ );
            ar & BOOST_SERIALIZATION_NVP( dataReaderUuid_ );
            ar & BOOST_SERIALIZATION_NVP( rowid_ );

            std::vector< datum > data;
            for ( size_t i = 0; i < massArray_.size(); ++i )
                data.emplace_back( datum( i
                                          , massArray_[ i ]
                                          , (tofArray_.empty() ? 0 : tofArray_[ i ])
                                          , intsArray_[i]
                                          , (colArray_.empty() ? 0 : colArray_[ i ]) ) );

            ar & BOOST_SERIALIZATION_NVP( data );

            // exclude
            // scanLaw_.reset();
            minmax_ = std::make_tuple( false, 0.0, 0.0 );
        }

        template<> void MassSpectrumImpl::serialize( boost::archive::xml_wiarchive&, const unsigned int ) {
               // not supported
        }

    }
}

BOOST_CLASS_VERSION( adcontrols::internal::MassSpectrumImpl, 5 ) 
// V4 -> V5: change std::vector< MassSpectrum > --> std::vector< std::shared_ptr< MassSpectrum > >

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
#if defined _DEBUG || defined DEBUG
    if ( protocolId() != t.protocolId() ) {
        ADDEBUG() << "add unmached spectrum " << protocolId() << " + " << t.protocolId();
        assert( 0 );
    }
#endif
    if ( isCentroid() || t.isCentroid() )
        throw std::runtime_error( "unsupported spectrum type" );

    const double * rhs = getIntensityArray();
    const double * lhs = t.getIntensityArray();
    
    for ( size_t i = 0; i < size() && i < t.size(); ++i )
        setIntensity( i, *rhs++ + *lhs++ );

    // update sampleInfo.numAverage
    getMSProperty().setNumAverage( getMSProperty().numAverage() + t.getMSProperty().numAverage() );

	return *this;
}

void
MassSpectrum::normalizeIntensities( uint32_t nImaginalAverage )
{
    // no segment handling due to each segment may have different number of average
    auto nAvg = pImpl_->property_.samplingInfo().numberOfTriggers();
    std::transform( pImpl_->intsArray_.begin(), pImpl_->intsArray_.end(), pImpl_->intsArray_.begin(), [=] ( double d ) { return d * nImaginalAverage / nAvg; } );
    pImpl_->property_.setNumAverage( nImaginalAverage );
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

int
MassSpectrum::mode() const
{
    return pImpl_->getMSProperty().mode();
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

size_t
MassSpectrum::operator << ( const std::pair< double, double >& d ) 
{
    std::vector<double>& m = pImpl_->massArray_;
    size_t pos = std::distance( m.begin(), std::lower_bound( m.begin(), m.end(), d.first ) );
    
    if ( pImpl_->isCentroid() ) {
        // preserve indexed annotation
        for ( auto& a: pImpl_->annotations_ ) {
            if ( a.index() != (-1) && a.index() >= int( pos ) )
                a.index( a.index() + 1 );
        }
    }

    pImpl_->massArray_.insert( pImpl_->massArray_.begin() + pos, d.first );
    pImpl_->intsArray_.insert( pImpl_->intsArray_.begin() + pos, d.second );

    if ( ! pImpl_->tofArray_.empty() )
        pImpl_->tofArray_.insert( pImpl_->tofArray_.begin() + pos, 0.0 ); // adjust size

    return pos;
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
    if ( pImpl_->tofArray_.empty() )
        return MSProperty::toSeconds( idx, pImpl_->getMSProperty().samplingInfo() );
    if ( pImpl_->size() > idx )
        return pImpl_->tofArray_[ idx ];
    else 
        return pImpl_->tofArray_[ pImpl_->tofArray_.size() - 1 ];
    return 0;
}

size_t
MassSpectrum::getIndexFromTime( double seconds, bool closest ) const
{
    size_t idx;
    if ( const double * p = pImpl_->getTimeArray() ) {
        idx = std::distance( p, std::lower_bound( p, p + pImpl_->size(), seconds ) );
    } else {
        const SamplingInfo& info = pImpl_->getMSProperty().samplingInfo();
        idx = size_t( ( seconds - info.fSampDelay() ) / info.fSampInterval() );
    }
    if ( closest && idx < pImpl_->size() ) {
        if ( ( ( idx + 1 ) < pImpl_->size() )
             && ( std::abs( seconds - getTime( idx ) ) > std::abs( seconds - getTime( idx + 1 ) ) ) )
            ++idx;
    }
    return idx; // will return size() when 'seconds' does not exist
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
MassSpectrum::setIntensityArray( std::vector< double >&& a )
{
    pImpl_->intsArray_ = std::move( a );
}

void
MassSpectrum::setMassArray( std::vector< double >&& a )
{
    pImpl_->massArray_ = std::move( a );

    if ( ! pImpl_->massArray_.empty() ) {

        if ( pImpl_->acqRange_.first > pImpl_->massArray_.front() )
            pImpl_->acqRange_.first = pImpl_->massArray_.front();

        if ( pImpl_->acqRange_.second < pImpl_->massArray_.back() )
            pImpl_->acqRange_.second = pImpl_->massArray_.back();
    }
}

void
MassSpectrum::setTimeArray( std::vector< double >&& a )
{
    pImpl_->tofArray_ = std::move( a );
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
MassSpectrum::addDescription( const description& t )
{
  pImpl_->addDescription( t );
}

void
MassSpectrum::addDescription( description&& t )
{
  pImpl_->addDescription( t );
}

const descriptions&
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

MSProperty&
MassSpectrum::getMSProperty()
{
    return pImpl_->property_;
}

void
MassSpectrum::setMSProperty( const MSProperty& prop )
{
    pImpl_->setMSProperty( prop );
}

void
MassSpectrum::set_annotations( const annotations& annots )
{
    pImpl_->set_annotations( annots );
}

const annotations&
MassSpectrum::get_annotations() const
{
    return pImpl_->get_annotations();
}

annotations&
MassSpectrum::get_annotations()
{
    return pImpl_->get_annotations();
}

void
MassSpectrum::addAnnotation( annotation&& a, bool uniq )
{
    if ( uniq && a.index() >= 0 ) {
        auto it = std::find_if( pImpl_->annotations_.begin(), pImpl_->annotations_.end()
                                , [&]( const annotation& x ){ return x.index() == a.index() && x.dataFormat() == a.dataFormat(); } );
        if ( it != pImpl_->annotations_.end() )
             pImpl_->annotations_.erase( it );
    }
        
    pImpl_->annotations_ << std::move( a );
}

void
MassSpectrum::uuid( const char * uuid )
{
    pImpl_->uuid_ = uuid;
}

const char *
MassSpectrum::uuid() const
{
    if ( pImpl_->uuid_.empty() )
        return 0;
    return pImpl_->uuid_.c_str();
}

template<class T> void
MassSpectrum::set( const T& t )
{
    (void)t;
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
        if ( !std::get<0>( pImpl_->minmax_ ) )
            getMaxIntensity();
        return std::get<1>( pImpl_->minmax_ );
    }
    return 0;
}

double
MassSpectrum::getMaxIntensity() const
{
    if ( !std::get<0>( pImpl_->minmax_ ) ) {
        std::get<0>( pImpl_->minmax_ ) = true;
        auto it = std::minmax_element( pImpl_->intsArray_.begin(), pImpl_->intsArray_.end() );
        if ( it.first != pImpl_->intsArray_.end() )
            std::get<1>( pImpl_->minmax_ ) = *it.first;
        if ( it.second != pImpl_->intsArray_.end() )
            std::get<2>( pImpl_->minmax_ ) = *it.second;
    }
    return std::get<2>( pImpl_->minmax_ );
}

int32_t
MassSpectrum::protocolId() const
{
    return pImpl_->protocolId_;
}

int32_t
MassSpectrum::nProtocols() const
{
    return pImpl_->nProtocols_;
}

void
MassSpectrum::setProtocol( int32_t proto, int32_t nproto )
{
    pImpl_->protocolId_ = proto;
    pImpl_->nProtocols_ = nproto;
}

// for v3 format datafile support
void
MassSpectrum::setDataReaderUuid( const boost::uuids::uuid& uuid )
{
    pImpl_->dataReaderUuid_ = uuid;
}

const boost::uuids::uuid&
MassSpectrum::dataReaderUuid() const
{
    return pImpl_->dataReaderUuid_;
}

void
MassSpectrum::setRowid( int64_t rowid )
{
    pImpl_->rowid_ = rowid;
}

int64_t
MassSpectrum::rowid() const
{
    return pImpl_->rowid_;
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

    template<> ADCONTROLSSHARED_EXPORT void
    MassSpectrum::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp("MassSpectrum", pImpl_);
    }

    template<> ADCONTROLSSHARED_EXPORT void
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


// size_t
// MassSpectrum::addSegment( const MassSpectrum& sub )
// {
//     pImpl_->vec_.emplace_back( std::make_shared< MassSpectrum >( sub ) );
//     return pImpl_->vec_.size();
// }

MassSpectrum&
MassSpectrum::getSegment( size_t fcn )
{
    if ( pImpl_->vec_.size() > fcn )
        return *pImpl_->vec_[ fcn ];
    throw std::out_of_range( "MassSpectrum fragments subscript out of range" );
}

const MassSpectrum&
MassSpectrum::getSegment( size_t fcn ) const
{
    if ( pImpl_->vec_.size() > fcn )
        return *pImpl_->vec_[ fcn ];
    throw std::out_of_range( "MassSpectrum fragments subscript out of range" );
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

MassSpectrum *
MassSpectrum::findProtocol( int32_t proto )
{
    if ( protocolId() == proto )
        return this;
    auto it = std::find_if( pImpl_->vec_.begin(), pImpl_->vec_.end(), [=]( const std::shared_ptr<const MassSpectrum>& ms ){ return ms->protocolId() == proto; } );
    if ( it != pImpl_->vec_.end() )
        return it->get();// &( *it );
    return 0;
}

const MassSpectrum *
MassSpectrum::findProtocol( int32_t proto ) const
{
    if ( protocolId() == proto )
        return this;
    auto it = std::find_if( pImpl_->vec_.begin(), pImpl_->vec_.end(), [=]( const std::shared_ptr< const MassSpectrum >& ms ){ return ms->protocolId() == proto; } );
    if ( it != pImpl_->vec_.end() )
        return it->get();
    return 0;
}

size_t
MassSpectrum::lower_bound( double value, bool isMass ) const
{
    if ( isMass ) {
        const auto& vec = pImpl_->massArray_;
        auto it = std::lower_bound( vec.begin(), vec.end(), value );
        if ( it != vec.end() )
            return std::distance( vec.begin(), it );
        return npos; // size_t(-1)
    } else {
        if ( !pImpl_->tofArray_.empty() ) {
            const auto& vec = pImpl_->tofArray_;
            auto it = std::lower_bound( vec.begin(), vec.end(), value );
            if ( it != vec.end() )
                return std::distance( vec.begin(), it );
        } else {
            size_t idx = MSProperty::toIndex( value, pImpl_->getMSProperty().samplingInfo() );
            double time = MSProperty::toSeconds( idx, pImpl_->getMSProperty().samplingInfo() );
            //double error = std::abs( value - time );
            assert( value == time );
        }
    }
    return npos; // size_t(-1)
}

size_t
MassSpectrum::find( double mass, double tolerance ) const
{
    size_t pos = lower_bound( mass );
    
    if ( pos != npos ) {
        if ( pos != 0 ) {
            if ( std::abs( getMass( pos ) - mass ) > std::abs( getMass( pos - 1 ) - mass ) )
                --pos;
        }
        
        double error = getMass( pos ) - mass;
        
        if ( std::abs( error ) > tolerance )
            return npos;
    }

    return pos;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

std::wstring MassSpectrumImpl::empty_string_ = L"";

MassSpectrumImpl::~MassSpectrumImpl()
{
}

MassSpectrumImpl::MassSpectrumImpl() : algo_(CentroidNone)
				                     , polarity_(PolarityPositive)
                                     , timeSinceInjTrigger_(0)
                                     , timeSinceFirmwareUp_(0)
                                     , numSpectrumSinceInjTrigger_(0)
                                     , protocolId_(0)
                                     , nProtocols_(0)
                                     , dataReaderUuid_( { {0} } )
                                     , rowid_(0)
                                     , minmax_( std::make_tuple( false, 0.0, 0.0 ) )
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
    protocolId_ = t.protocolId_;
    nProtocols_ = t.nProtocols_;
    dataReaderUuid_ = t.dataReaderUuid_;
    rowid_ = t.rowid_;

	if ( deep ) {
		tofArray_ = t.tofArray_;
		massArray_ = t.massArray_;
		intsArray_ = t.intsArray_;
		colArray_ = t.colArray_;
        annotations_ = t.annotations_;
        uuid_ = t.uuid_;
        vec_.clear();
        for ( const auto& a: t.vec_ )
            vec_.push_back(  std::make_shared< MassSpectrum >( *a ) );
	} else {
		tofArray_.clear();
		massArray_.clear();
		intsArray_.clear();
        colArray_.clear();
        annotations_.clear();
        uuid_.clear();
        vec_.clear();
	}
}

void
MassSpectrumImpl::setTimeArray( const double * p )
{
    if ( p == nullptr ) {
        tofArray_.clear();
    } else {
        if ( tofArray_.size() != size() )
            tofArray_.resize( size() );
        std::copy( p, p + tofArray_.size(), tofArray_.begin() );
    }
}

void
MassSpectrumImpl::setMassArray( const double * p, bool setrange )
{
	std::copy( p, p + massArray_.size(), massArray_.begin() );
    if ( setrange )
		setAcquisitionMassRange( massArray_.front(), massArray_.back() );
}

void
MassSpectrumImpl::setIntensityArray( const double * p )
{
	std::copy( p, p + intsArray_.size(), intsArray_.begin() );
}

void
MassSpectrumImpl::setColorArray( const unsigned char * p )
{
    if ( p ) {
        if ( colArray_.size() != size() )
            colArray_.resize( size() );
		std::copy( p, p + colArray_.size(), colArray_.begin() );
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
MassSpectrumImpl::addDescription( const description& t )
{
	descriptions_.append( t );
}

const descriptions&
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

double
segments_helper::min_intensity( const MassSpectrum& ms )
{
    if ( ms.isCentroid() )
        return 0;
    double y = ms.getMinIntensity();
    for ( size_t i = 0; i < ms.numSegments(); ++i ) {
        double t = ms.getSegment( i ).getMinIntensity();
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

//static
double
segments_helper::get_mass( const MassSpectrum& ms, const std::pair< int, int >& idx )
{
    if ( idx.first < 0 )
        return 0;
    if ( idx.second == 0 )
        return ms.getMass( idx.first );
    return ms.getSegment( idx.second - 1 ).getMass( idx.first );
}

//static
double
segments_helper::get_intensity( const MassSpectrum& ms, const std::pair< int, int >& idx )
{
    if ( idx.first < 0 )
        return 0;
    if ( idx.second == 0 )
        return ms.getIntensity( idx.first );
    return ms.getSegment( idx.second - 1 ).getIntensity( idx.first );
}

//static
std::pair<int, int> // idx, fcn
segments_helper::base_peak_index( const MassSpectrum& ms, double lMass, double hMass )
{
    std::pair< int, int > idx( -1, -1 );

    if ( ! ms.isCentroid() )
        return idx; // error
    
    if ( lMass > hMass ) // just in case
        std::swap( lMass, hMass );
    
    double hMax = 0;
    
    segment_wrapper< const MassSpectrum > segments( ms );
    int fcn = 0;
    for ( auto& fms: segments ) {
        if ( fms.size() &&
             ( lMass < fms.getMass( fms.size() - 1 ) ) && ( hMass > fms.getMass( 0 ) ) ) {
            
            const double * intens = fms.getIntensityArray();
            const double * masses = fms.getMassArray();
            auto lIt = std::lower_bound( masses, masses + fms.size(), lMass );
            auto hIt = std::lower_bound( masses, masses + fms.size(), hMass );
            size_t lIdx = std::distance( masses, lIt );
            size_t hIdx = std::distance( masses, hIt );
            auto maxIt = std::max_element( intens + lIdx, intens + hIdx );

            if ( hMax < *maxIt ) {
                hMax = *maxIt;
                idx.first = static_cast<int>(std::distance( intens, maxIt ));
                idx.second = fcn;
            }
        }
        ++fcn;
    }
    return idx;
}

bool
segments_helper::add( MassSpectrum& lhs, const MassSpectrum& rhs )
{
    if ( lhs.numSegments() != rhs.numSegments() )
        return false;

    lhs += rhs;

    for ( size_t fcn = 0; fcn < lhs.numSegments(); ++fcn )
        lhs.getSegment( fcn ) += rhs.getSegment( fcn );

    return true;
}

// static
bool
segments_helper::normalize( MassSpectrum& vms, uint32_t imaginalNumAverage )
{
    for ( auto& ms: segment_wrapper< MassSpectrum >( vms ) ) {
        ms.normalizeIntensities( imaginalNumAverage );
    }
    return true;
}

//static
std::pair<double, double>
segments_helper::acquisition_time_range( const MassSpectrum& ms )
{
    std::pair< double, double > range( std::numeric_limits<double>::max(), 0 );

    for ( auto& fms: segment_wrapper< const MassSpectrum >( ms ) ) {
        std::pair< double, double > x = fms.getMSProperty().instTimeRange();
        range.first = std::min( range.first, x.first );
        range.second = std::max( range.second, x.second );
    }
    return range;
}

bool
MassSpectrum::trim( adcontrols::MassSpectrum& ms, const std::pair<double, double>& range ) const
{
    ms.clone( *this );
    if ( uuid() )
        ms.uuid( uuid() );
    // tofarray, massarray, colarray, annotations
    // no segements array supported

    auto itFirst = std::lower_bound( pImpl_->massArray_.begin(), pImpl_->massArray_.end(), range.first );
    if ( itFirst == pImpl_->massArray_.end() )
        return false;

    if ( itFirst != pImpl_->massArray_.begin() )
        --itFirst;

    auto itEnd = std::lower_bound( pImpl_->massArray_.begin(), pImpl_->massArray_.end(), range.second );
    size_t size = std::distance( itFirst, itEnd );
    size_t idx = std::distance( pImpl_->massArray_.begin(), itFirst );

    ms.resize( size );

    if ( const double * p = getMassArray() )
        ms.setMassArray( p + idx, true ); // update range

    if ( const double * p = getIntensityArray() )
        ms.setIntensityArray( p + idx );

    if ( const double * p = getTimeArray() )
        ms.setTimeArray( p + idx );

    if ( auto * p = getColorArray() )
        ms.setColorArray( p + idx );

    adcontrols::annotations annots;

    for ( auto a: get_annotations() ) {
        if ( ( size_t( a.index() ) >= idx && size_t( a.index() ) <= idx + size ) ||
             ( a.x() >= range.first && a.x() <= range.second ) ) {
            if ( a.index() >= 0 )
                a.index( a.index() - int( idx ) );
            annots << a;
        }
    }
    ms.set_annotations( annots );
    return true;
}

bool
MassSpectrum::assign_masses( mass_assignee_t assign_mass )
{
    const MSProperty& prop = pImpl_->getMSProperty();
    int mode = prop.mode();

    if ( assign_mass ) {
        
        if ( pImpl_->tofArray_.empty() ) {

            size_t idx(0);
            std::transform( pImpl_->massArray_.begin(), pImpl_->massArray_.end(), pImpl_->massArray_.begin()
                            , [&]( const double& ){
                                return assign_mass( MSProperty::toSeconds( idx++, prop.samplingInfo() ), mode );
                            } );
        
        } else {

            std::transform( pImpl_->tofArray_.begin(), pImpl_->tofArray_.end(), pImpl_->massArray_.begin()
                            , [&]( const double& t ){
                                return assign_mass( t, mode );
                            } );

        }

        auto t_range = prop.instTimeRange();
        this->setAcquisitionMassRange( assign_mass( t_range.first, mode ), assign_mass( t_range.second, mode ) );

        return true;
    }
    return false;
}

MassSpectrum&
MassSpectrum::operator << ( std::shared_ptr< MassSpectrum >&& ms )
{
    auto range = ms->getAcquisitionMassRange();
    pImpl_->vec_.emplace_back( ms );

    pImpl_->acqRange_.first = std::min( range.first, pImpl_->acqRange_.first );
    pImpl_->acqRange_.second = std::max( range.second, pImpl_->acqRange_.second );
    
    return *this;
}

MassSpectrum::iterator
MassSpectrum::begin()
{
    return pImpl_->vec_.begin();
}

MassSpectrum::iterator
MassSpectrum::end()
{
    return pImpl_->vec_.end();
}

MassSpectrum::const_iterator
MassSpectrum::begin() const
{
    return pImpl_->vec_.begin();
}

MassSpectrum::const_iterator
MassSpectrum::end() const
{
    return pImpl_->vec_.end();
}

MassSpectrum::iterator
MassSpectrum::erase( const_iterator first, const_iterator last )
{
    return pImpl_->vec_.erase( first, last );
}
