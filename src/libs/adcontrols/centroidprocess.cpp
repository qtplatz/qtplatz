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

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>

#include "centroidprocess.hpp"
#include "centroidmethod.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include "mspeakinfoitem.hpp"
#include "mspeakinfo.hpp"
#include "description.hpp"
#include "waveform.hpp"
#include <adportable/spectrum_processor.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/moment.hpp>
#include <adportable/differential.hpp>
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

            friend class CentroidProcess;
            MSPeakInfo info_;
            MassSpectrum clone_;
            CentroidMethod method_;
            Description desc_;
        public:
            MassSpectrum debug_profile_;
        };

        struct timeFunctor {
            const adcontrols::MSProperty::SamplingInfo& info;
            timeFunctor( const adcontrols::MassSpectrum& profile )
                : info( profile.getMSProperty().getSamplingInfo() ) {
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
	adcontrols::MassSpectrum ms( profile );
	return (*this)( profile );
}

bool
CentroidProcess::operator()( const MassSpectrum& profile )
{
    pImpl_->clear();
	pImpl_->setup( profile );
    pImpl_->findpeaks( profile );
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

#if defined DEBUG_CENTROID_PROCESS
    ms = pImpl_->debug_profile_;
    return true;
#endif

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
	desc_ = adcontrols::Description( L"CentroidProcess", L"Centroid" );

	std::ostringstream o;
	boost::archive::xml_oarchive ar( o );
	ar << boost::serialization::make_nvp("CentroidMethod", method);
	desc_.xml( o.str() );
}

void
CentroidProcessImpl::setup( const MassSpectrum& profile )
{
    clone_.clone( profile, false ); // keep meta data
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
    using adportable::differential;
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

    // buffer for smoothing
    std::unique_ptr< double [] > pY( new double [ profile.size() ] );
    int nAverage = 3;
	while ( ( profile.getMass( nAverage ) - profile.getMass( 0 ) ) < finder.peakwidth_ )
		++nAverage;
    if ( nAverage >= 26 )
		nAverage = 25;

	spectrum_processor::moving_average( profile.size(), pY.get(), profile.getIntensityArray(), nAverage );
    finder( profile.size(), profile.getMassArray(), pY.get() );

#if defined DEBUG_CENTROID_PROCESS
    debug_profile_ = profile;
    // debug_profile_.setIntensityArray( &finder.pdebug_[0] );
#endif

    array_wrapper<const double> intens( profile.getIntensityArray(), profile.size() );
    array_wrapper<const double> masses( profile.getMassArray(), profile.size() );

    double toferror = 0;
    double toferror_weight = 0;

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
            double mass = moment.centerX( profile.getIntensityArray(), threshold, pk.first, idx, pk.second );

            // if centroid mass is outside of peak start - end, it should not added into result
            if ( masses[ pk.first ] < mass && mass < masses[ pk.second ] ) {

                MSPeakInfoItem item;

                item.peak_start_index_ = pk.first;
                item.peak_end_index_ = pk.second;
                item.base_height_ = pk.base;
                item.area_ = a;
                item.height_ = h;
                item.mass_ = mass;
                item.centroid_left_mass_ = moment.xLeft();
                item.centroid_right_mass_ = moment.xRight();
                item.centroid_threshold_ = threshold;
                
                // pk.mass = mass;
                moment.width( profile.getIntensityArray(), pk.base + h * 0.5, pk.first, idx, pk.second ); // half-height
                item.HH_left_mass_ = moment.xLeft();
                item.HH_right_mass_ = moment.xRight();
                
                // time interporate from mass
                array_wrapper<const double>::const_iterator pos = std::lower_bound( masses.begin() + pk.first, masses.begin() + pk.second, mass );
                size_t index = std::distance( masses.begin(), --pos );

                // assert( masses[ index ] < mass && mass < masses[ index + 1 ] );

                double t0 = profile.getTime( index );
                double td = profile.getMSProperty().instSamplingInterval() * 1e-12;
                item.time_from_mass_ = t0 + td * ( mass - masses[ index ] ) / ( masses[ index + 1 ] - masses[ index ] );

                // centroid by time
                timeFunctor functor( profile );
                adportable::Moment< timeFunctor > time_moment( functor );
                double time = time_moment.centerX( profile.getIntensityArray(), threshold, pk.first, idx, pk.second );
                item.time_from_time_ = time;
                item.centroid_left_time_ = time_moment.xLeft();
                item.centroid_right_time_ = time_moment.xRight();
				time_moment.width( profile.getIntensityArray(), pk.base + h * 0.5, pk.first, idx, pk.second );
				item.HH_left_time_ = time_moment.xLeft();
				item.HH_right_time_ = time_moment.xRight();

                double difference = std::abs( item.time_from_time_ - item.time_from_mass_ );
                
                toferror += difference * item.height_;
                toferror_weight += item.height_;

                // prepare result
                // MSPeakInfoItem item( idx, pk.mass, a, h, pk.width, pk.time );
                item.peak_start_index( pk.first );
                item.peak_end_index( pk.second );
                item.base_height( pk.base );
                info_ << item;
            }
        } while(0);

#if defined DEBUG_CENTROID_PROCESS
        debug_profile_.setIntensity( pair.first, 50000 );
        debug_profile_.setIntensity( pair.second, 25000 );
#endif
    }
    toferror /= toferror_weight;
    if ( toferror >= 1.0e-12 ) // warning if error was 1ps or larger
        adportable::debug(__FILE__, __LINE__ ) << "centroid tof interporation error: " << toferror * 1e12 << "ps";
}
