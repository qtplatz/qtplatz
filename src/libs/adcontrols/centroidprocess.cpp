// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "centroidprocess.hpp"
#include "centroidmethod.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include "mspeakinfoitem.hpp"
#include "description.hpp"
#include "waveform.hpp"
#include <adportable/spectrum_processor.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/moment.hpp>
#include <adportable/differential.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>

#include <vector>
#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <sstream>
#include <cmath>

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/foreach.hpp>

// #define DEBUG_CENTROID_PROCESS

using namespace adcontrols;

namespace adcontrols {

    class MSPeakInfoItem;
    
    namespace internal {
        
        class CentroidProcessImpl {
        public:
            CentroidProcessImpl() {}
            void clear();
            void setup( const CentroidMethod& );
            void setup( const MassSpectrum& );
            void copy( MassSpectrum& );
            const CentroidMethod& method() const { return method_; }
            void findpeaks( const MassSpectrum& profile );

            // result
            std::vector< MSPeakInfoItem > info_;
        private:
            MassSpectrum clone_;
            CentroidMethod method_;
            Description desc_;
        public:
            MassSpectrum debug_profile_;
        };

        struct timeFunctor {
            const std::vector< adcontrols::MSProperty::SamplingInfo>& info;
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

CentroidProcess::CentroidProcess( const CentroidMethod& method)
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

        // std::pair<double, double> mrange = ms.getAcquisitionMassRange();

        for ( size_t i = 0; i < nSize; ++i ) {
			ms.setIntensity( i, is_area ? pImpl_->info_[i].area() : pImpl_->info_[i].height() );
            ms.setMass( i, pImpl_->info_[i].mass() );
            ms.setTime( i, pImpl_->info_[i].time() );
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

	std::wostringstream o;
	boost::archive::xml_woarchive ar( o );
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
    boost::scoped_array< double > pY( new double [ profile.size() ] );
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

    BOOST_FOREACH( adportable::peakinfo& pk, finder.results_ ) {
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
                pk.mass = mass;
                pk.width = moment.width( profile.getIntensityArray(), pk.base + h * 0.5, pk.first, idx, pk.second ); // half-height

                // time interporate from mass
                array_wrapper<const double>::const_iterator pos = std::lower_bound( masses.begin() + pk.first, masses.begin() + pk.second, mass );
                size_t index = std::distance( masses.begin(), --pos );

                // assert( masses[ index ] < mass && mass < masses[ index + 1 ] );

                double t0 = profile.getTime( index );
                double td = profile.getMSProperty().instSamplingInterval() * 1e-12;
                pk.time = t0 + td * ( mass - masses[ index ] ) / ( masses[ index + 1 ] - masses[ index ] );

#if defined _DEBUG
                // centroid by time
                timeFunctor functor( profile );
                adportable::Moment< timeFunctor > time_moment( functor );
                double time = time_moment.centerX( profile.getIntensityArray(), threshold, pk.first, idx, pk.second );

                double difference = std::abs( time - pk.time );
                if ( toferror < difference )
                    toferror = difference;
#endif
                // prepare resutl
                MSPeakInfoItem item( idx, pk.mass, a, h, pk.width, pk.time );
                item.peak_start_index( pk.first );
                item.peak_end_index( pk.second );
                item.base_height( pk.base );
                info_.push_back( item );
            }
        } while(0);

#if defined DEBUG_CENTROID_PROCESS
        debug_profile_.setIntensity( pair.first, 50000 );
        debug_profile_.setIntensity( pair.second, 25000 );
#endif
    }
    adportable::debug() << "centroid tof interporation error: " << toferror * 1e12 << "ps";
}
