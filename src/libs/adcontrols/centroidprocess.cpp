// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>

#include "centroidprocess.hpp"
#include "centroidmethod.hpp"
#include "description.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include "mspeakinfoitem.hpp"
#include "mspeakinfo.hpp"
#include "samplinginfo.hpp"
#include "waveform_filter.hpp"
#include <adportable/histogram_processor.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/moment.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <compiler/diagnostic_pop.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <memory>

using namespace adcontrols;

namespace adcontrols {

    class MSPeakInfoItem;
    
    namespace internal {
        
        class CentroidProcessImpl : boost::noncopyable {
        public:
            CentroidProcessImpl() {
            }
            void clear();
            void setup( const CentroidMethod& );
            void setup( const MassSpectrum& );
            void copy( MassSpectrum& );
            const CentroidMethod& method() const { return method_; }
            void findpeaks( const MassSpectrum& profile );
            void findpeaks_by_time( const MassSpectrum& profile );
            void findCluster( const MassSpectrum& );

            friend class CentroidProcess;
            MSPeakInfo info_;
            MassSpectrum clone_;
            CentroidMethod method_;
            description desc_;
        };

        struct timeFunctor {

            const adcontrols::SamplingInfo& info;
            
            timeFunctor( const adcontrols::MassSpectrum& profile )
                : info( profile.getMSProperty().samplingInfo() ) {
            }

            double operator ()( int pos ) { 
                return adcontrols::MSProperty::toSeconds( pos, info );
            }
        };
        
    }
}

CentroidProcess::~CentroidProcess(void)
{
    delete pImpl_;
}

CentroidProcess::CentroidProcess(void) : pImpl_( new internal::CentroidProcessImpl() )
{
}

CentroidProcess::CentroidProcess( const CentroidMethod& method )
    : pImpl_( new internal::CentroidProcessImpl() )
{
	pImpl_->setup( method );
}

bool
CentroidProcess::operator()( const CentroidMethod& method, const MassSpectrum& profile )
{
	pImpl_->setup( method );
	//adcontrols::MassSpectrum ms( profile );
	return (*this)( profile );
}

bool
CentroidProcess::operator()( const MassSpectrum& profile )
{
    pImpl_->clear();
	pImpl_->setup( profile );

    if ( profile.size() == 0 )
        return false;

    if ( profile.isCentroid() ) {
        // assume this is a histogram by counting (discreate time,mass,count array)
        pImpl_->findCluster( profile );
        
    } else {
    
        if ( pImpl_->method_.processOnTimeAxis() && std::abs( profile.getMass( 0 ) - profile.getMass( profile.size() - 1 ) ) < 0.001 ) {
            pImpl_->findpeaks_by_time( profile );
        } else {
            pImpl_->findpeaks( profile );
        }

    }
    return true;
}

const MSPeakInfo&
CentroidProcess::getPeakInfo() const
{
    return pImpl_->info_;
}

bool
CentroidProcess::getCentroidSpectrum( MassSpectrum& ms )
{
	pImpl_->copy( ms );

	size_t nSize;
	if ( pImpl_ && ( nSize = pImpl_->info_.size() ) ) {

		ms.resize( nSize );
        ms.setCentroid( adcontrols::CentroidPeakAreaWaitedMass );
		bool is_area = pImpl_->method().centroidAreaIntensity();

        size_t idx = 0;
        for ( auto& info: pImpl_->info_ ) {
			ms.setIntensity( idx, is_area ? info.area() : info.height() );
            ms.setMass( idx, info.mass() );
            ms.setTime( idx, info.time() );
            idx++;
        }
        return true;
    }
    return false;
}

/////////////////////////

using namespace adcontrols::internal;

void
CentroidProcessImpl::clear()
{
	info_.clear();
}

void
CentroidProcessImpl::setup( const CentroidMethod& method )
{
    method_ = method;
	desc_ = adcontrols::description( L"CentroidProcess", L"Centroid" );

	std::ostringstream o;
	boost::archive::xml_oarchive ar( o );
	ar << boost::serialization::make_nvp("CentroidMethod", method);
	desc_.xml( o.str().c_str() );
}

void
CentroidProcessImpl::setup( const MassSpectrum& profile )
{
    clone_.clone( profile, false ); // keep meta data
    info_.setProtocol( profile.protocolId(), profile.nProtocols() );
}

void
CentroidProcessImpl::copy( MassSpectrum& ms )
{
	ms.clone( clone_, false );
	ms.addDescription( desc_ );
}

void
CentroidProcessImpl::findpeaks( const MassSpectrum& profile )
{
    using adportable::spectrum_processor;
    using adportable::array_wrapper;

    adportable::spectrum_peakfinder finder;
    if ( method_.peakWidthMethod() == CentroidMethod::ePeakWidthConstant ) {
        finder.width_method_ = adportable::spectrum_peakfinder::Constant;
        finder.peakwidth_ = method_.rsConstInDa();
    } else if ( method_.peakWidthMethod() == CentroidMethod::ePeakWidthProportional ) {
        finder.width_method_ = adportable::spectrum_peakfinder::Proportional;
        finder.peakwidth_ = method_.rsConstInDa();
    } else if ( method_.peakWidthMethod() == CentroidMethod::ePeakWidthTOF ) {
        finder.width_method_ = adportable::spectrum_peakfinder::TOF;
        finder.peakwidth_ = method_.rsTofInDa();
        finder.atmz_ = method_.rsTofAtMz();
    }

    finder( profile.size(), profile.getMassArray(), profile.getIntensityArray() );

    array_wrapper<const double> intens( profile.getIntensityArray(), profile.size() );
    array_wrapper<const double> masses( profile.getMassArray(), profile.size() );

    double toferror = 0;
    double toferror_weight = 0;

	info_.setMode( profile.mode() );  // copy analyzer mode a.k.a. laps for multi-turn mass spectrometer
    info_.setProtocol( profile.protocolId(), profile.nProtocols() );

    for ( adportable::peakinfo& pk: finder.results_ ) {

        adportable::array_wrapper<const double>::iterator it = 
            std::max_element( intens.begin() + pk.first, intens.begin() + pk.second );
        
        double h = *it - pk.base;
        double a = adportable::spectrum_processor::area( intens.begin() + pk.first, intens.begin() + pk.second, pk.base );

        size_t idx = std::distance( intens.begin(), it );

        double threshold = pk.base + h * method_.peakCentroidFraction();
        do {
            // centroid by mass
            adportable::massArrayFunctor mass_array( profile.getMassArray(), profile.size() );
            adportable::Moment< adportable::massArrayFunctor > moment( mass_array );
            double mass = moment.centreX( profile.getIntensityArray(), threshold, uint32_t(pk.first), uint32_t(idx), pk.second );

            // if centroid mass is outside of peak start - end, it should not added into result
            if ( masses[ pk.first ] < mass && mass < masses[ pk.second ] ) {

                MSPeakInfoItem item;

                item.peak_start_index_ = static_cast<uint32_t>( pk.first );
                item.peak_end_index_ = static_cast<uint32_t>( pk.second );
                item.base_height_ = pk.base;
                item.area_ = a;  // this will override with HH area
                item.height_ = h;
                item.mass_ = mass;
                item.centroid_left_mass_ = moment.xLeft();
                item.centroid_right_mass_ = moment.xRight();
                item.centroid_threshold_ = threshold;
                
                // half-height width
                moment.width( profile.getIntensityArray(), pk.base + h * 0.5, uint32_t(pk.first), uint32_t(idx), pk.second );
                item.HH_left_mass_ = moment.xLeft();
                item.HH_right_mass_ = moment.xRight();

                // area in HH range
                adportable::spectrum_processor::areaFraction fraction;
                adportable::spectrum_processor::getFraction( fraction, profile.getMassArray(), profile.size(), moment.xLeft(), moment.xRight() );

				double area = adportable::spectrum_processor::area( fraction, pk.base, intens.begin(), intens.size() );

                array_wrapper<const double>::const_iterator pos = std::lower_bound( masses.begin() + pk.first, masses.begin() + pk.second, mass );
                size_t index = std::distance( masses.begin(), --pos );

                // assert( masses[ index ] < mass && mass < masses[ index + 1 ] );

                double t0 = profile.getTime( index );
				double td = profile.getMSProperty().samplingInfo().fSampInterval();
                item.time_from_mass_ = t0 + td * ( mass - masses[ index ] ) / ( masses[ index + 1 ] - masses[ index ] );

                // centroid by time
                timeFunctor functor( profile );
                adportable::Moment< timeFunctor > time_moment( functor );
                double time = time_moment.centreX( profile.getIntensityArray(), threshold, uint32_t(pk.first), uint32_t(idx), pk.second );
                item.time_from_time_ = time;
                item.centroid_left_time_ = time_moment.xLeft();
                item.centroid_right_time_ = time_moment.xRight();
				time_moment.width( profile.getIntensityArray(), pk.base + h * 0.5, uint32_t(pk.first), uint32_t(idx), pk.second );
				item.HH_left_time_ = time_moment.xLeft();
				item.HH_right_time_ = time_moment.xRight();

                // double width = moment.xRight() - moment.xLeft(); // m/z
                if ( method_.areaMethod() == CentroidMethod::eAreaDa )
                    item.area_ = area * ( moment.xRight() - moment.xLeft() ) * 1000; // Intens. x mDa
                else if ( method_.areaMethod() == CentroidMethod::eAreaTime )
                    item.area_ = area * adcontrols::metric::scale_to_nano( time_moment.xRight() - time_moment.xLeft() ); // Intens. x ns
                else if ( method_.areaMethod() == CentroidMethod::eWidthNormalized )
                    item.area_ = area / ((fraction.uPos - fraction.lPos + 1) + fraction.lFrac + fraction.uFrac); // width of unit of sample interval
                else if ( method_.areaMethod() == CentroidMethod::eAreaPoint )
                    item.area_ = area; // Intens x ns that assumes data is always 1ns interval

                double difference = std::abs( item.time_from_time_ - item.time_from_mass_ );
                
                toferror += difference * item.height_;
                toferror_weight += item.height_;

                // prepare result
                // MSPeakInfoItem item( idx, pk.mass, a, h, pk.width, pk.time );
                item.set_peak_start_index( uint32_t(pk.first) );
                item.set_peak_end_index( uint32_t(pk.second) );
				
				if ( ( masses[pk.second] - masses[pk.first]) >= finder.peakwidth_ )
					info_ << item;
            }
        } while(0);
    }

    toferror /= toferror_weight;
    if ( toferror >= 20.0e-12 ) // warning if error was 20ps or larger
        adportable::debug(__FILE__, __LINE__ ) << "centroid tof interporation error: " << toferror * 1e12 << "ps";
}

void
CentroidProcessImpl::findpeaks_by_time( const MassSpectrum& profile )
{
    using adportable::spectrum_processor;
    using adportable::array_wrapper;

    adportable::spectrum_peakfinder finder;
    finder.width_method_ = adportable::spectrum_peakfinder::Constant;
    finder.peakwidth_ = method_.rsInSeconds();

    std::vector< double > times( profile.size() );
    timeFunctor functor( profile );    
    for ( size_t i = 0; i < times.size(); ++i )
        times[ i ] = functor( int(i) );
    
    finder( profile.size(), times.data(), profile.getIntensityArray() );

    array_wrapper<const double> intens( profile.getIntensityArray(), profile.size() );
    // array_wrapper<const double> times( profile.getTimeArray(), profile.size() );

    double toferror = 0;
    double toferror_weight = 0;

	info_.setMode( profile.mode() );  // copy analyzer mode a.k.a. laps for multi-turn mass spectrometer
    info_.setProtocol( profile.protocolId(), profile.nProtocols() );

    for ( adportable::peakinfo& pk: finder.results_ ) {

        adportable::array_wrapper<const double>::iterator it = 
            std::max_element( intens.begin() + pk.first, intens.begin() + pk.second );
        
        double h = *it - pk.base;
        //double a = adportable::spectrum_processor::area( intens.begin() + pk.first, intens.begin() + pk.second, pk.base );

        size_t idx = std::distance( intens.begin(), it );

        double threshold = pk.base + h * method_.peakCentroidFraction();
        do {
            MSPeakInfoItem item;
            
            // centroid by time
            timeFunctor functor( profile );
            adportable::Moment< timeFunctor > time_moment( functor );
            double time = time_moment.centreX( profile.getIntensityArray(), threshold, uint32_t(pk.first), uint32_t(idx), pk.second );
            item.time_from_time_ = time;
            // workaround
            item.time_from_mass_ = time;
            // end workaround
            item.centroid_left_time_ = time_moment.xLeft();
            item.centroid_right_time_ = time_moment.xRight();
            item.centroid_threshold_ = threshold;
            time_moment.width( profile.getIntensityArray(), pk.base + h * 0.5, uint32_t(pk.first), uint32_t(idx), pk.second );
            item.HH_left_time_ = time_moment.xLeft();
            item.HH_right_time_ = time_moment.xRight();

            item.base_height_ = pk.base;
            item.height_ = h;
            
            // area in HH range
            adportable::spectrum_processor::areaFraction fraction;
            adportable::spectrum_processor::getFraction( fraction, times.data(), profile.size(), time_moment.xLeft(), time_moment.xRight() );
            double area = adportable::spectrum_processor::area( fraction, pk.base, intens.begin(), intens.size() );
            
            if ( method_.areaMethod() == CentroidMethod::eAreaTime || method_.areaMethod() == CentroidMethod::eAreaDa )
                item.area_ = area * adcontrols::metric::scale_to_nano( time_moment.xRight() - time_moment.xLeft() ); // Intens. x ns
            else if ( method_.areaMethod() == CentroidMethod::eWidthNormalized )
                item.area_ = area / ((fraction.uPos - fraction.lPos + 1) + fraction.lFrac + fraction.uFrac); // width of unit of sample interval
            else if ( method_.areaMethod() == CentroidMethod::eAreaPoint )
                item.area_ = area; // Intens x ns that assumes data is always 1ns interval
            
            double difference = std::abs( item.time_from_time_ - item.time_from_mass_ );
            
            toferror += difference * item.height_;
            toferror_weight += item.height_;
            
            // prepare result
            // MSPeakInfoItem item( idx, pk.mass, a, h, pk.width, pk.time );
            item.set_peak_start_index( uint32_t(pk.first) );
            item.set_peak_end_index( uint32_t(pk.second) );
			
            //if ( ( times[pk.second] - times[pk.first]) >= finder.peakwidth_ )
                info_ << item;
        } while(0);

        toferror /= toferror_weight;    
        if ( toferror >= 20.0e-12 ) // warning if error was 20ps or larger
            adportable::debug(__FILE__, __LINE__ ) << "centroid tof interporation error: " << toferror * 1e12 << "ps";
    }
}

void
CentroidProcessImpl::findCluster( const MassSpectrum& histogram )
{
    adportable::histogram_peakfinder finder( histogram.getMSProperty().samplingInfo().fSampInterval() );

    const double * pCounts = histogram.getIntensityArray();
    const double * pTimes = histogram.getTimeArray();
    const double * pMasses = histogram.getMassArray();

    if ( finder( histogram.size(), pTimes, pCounts ) ) {
        
        info_.setMode( histogram.mode() );  // copy analyzer mode a.k.a. laps for multi-turn mass spectrometer
        info_.setProtocol( histogram.protocolId(), histogram.nProtocols() );
        
        for ( adportable::peakinfo& pk: finder.results_ ) {
            
            MSPeakInfoItem item;
            
            auto it = std::max_element( pCounts + pk.first, pCounts + pk.second );
            
            item.height_ = *it;
            size_t idx = std::distance( pCounts, it );
            double threshold = item.height_ * method_.peakCentroidFraction();
            
            struct xfunctor {
                const double * pData_;
                xfunctor( const double * pData ) : pData_( pData ) {}
                double operator () ( int pos ) { return pData_[ pos ]; }
            };
            
            // centroid by mass
            if ( pMasses && pMasses[ 0 ] > 1.0 ) {
                xfunctor functor( pMasses );
                adportable::Moment< xfunctor > moment( functor );
                item.mass_ = moment.centreX( pCounts, threshold, uint32_t(pk.first), uint32_t(idx), pk.second );
                item.peak_start_index_ = pk.first;
                item.peak_end_index_ = pk.second;
                item.base_height_ = 0;
                item.centroid_left_mass_ = moment.xLeft();
                item.centroid_right_mass_ = moment.xRight();
                item.centroid_threshold_ = threshold;
                item.HH_left_mass_ = moment.xLeft();
                item.HH_right_mass_ = moment.xRight();
            }

            // centroid by time
            if ( pTimes && ( ( pTimes[ 1 ] - pTimes[ 0 ] ) > 10.0e-12 /* 10ps */ ) ) {
                xfunctor functor( pTimes );
                adportable::Moment< xfunctor > moment( functor );
                double time = moment.centreX( pCounts, threshold, uint32_t(pk.first), uint32_t(idx), pk.second );
                item.time_from_time_ = time;
                item.time_from_mass_ = time; // workaround since no way to compute time from mass, which is descreate
                item.centroid_left_time_  = moment.xLeft();
                item.centroid_right_time_ = moment.xRight();
                item.centroid_threshold_  = threshold;
                item.HH_left_time_        = moment.xLeft();
                item.HH_right_time_       = moment.xRight();
            }
            
            double counts(0);
            for ( auto i = pk.first; i <= pk.second; ++i ) {
                if ( pTimes[ i ] >= item.centroid_left_time_ && pTimes[ i ] <= item.centroid_right_time_ )
                    counts += pCounts[ i ];
            }
            item.area_ = counts;
            
            item.set_peak_start_index( uint32_t(pk.first) );
            item.set_peak_end_index( uint32_t(pk.second) );
            info_ << item;

        } // for
    }
}
