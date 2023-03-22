/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "chromatography.hpp"
#include "integrator.hpp"
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adplot/constants.hpp>
#include <adportable/debug.hpp>

namespace chromatogr {

    class Chromatography::impl {
    public:
        impl() : tp_( method_.end() ) {
        }

        bool setup( const adcontrols::PeakMethod& );
        bool findPeaks( const adcontrols::Chromatogram& );
        void clear();

        void progress( double seconds, Integrator& );

        inline const adcontrols::Peaks & getPeaks() const { return peaks_; }
        inline const adcontrols::Baselines & getBaselines() const { return baselines_; }
    private:
        adcontrols::PeakMethod method_;
        adcontrols::Peaks peaks_;
        adcontrols::Baselines baselines_;
        adcontrols::PeakMethod::const_iterator_type tp_;
    };
}

using namespace chromatogr;

Chromatography::~Chromatography()
{
    delete pImpl_;
}

Chromatography::Chromatography() : pImpl_( new impl() )
{
}

Chromatography::Chromatography( const adcontrols::PeakMethod& method ) : pImpl_( new impl() )
{
	pImpl_->setup( method );
}

bool
Chromatography::operator()( const adcontrols::PeakMethod& method, const adcontrols::Chromatogram& c )
{
    pImpl_->setup( method );
    pImpl_->clear();
    return pImpl_->findPeaks( c );
}

bool
Chromatography::operator()( const adcontrols::Chromatogram& c )
{
    pImpl_->clear();
    return pImpl_->findPeaks( c );
}

const adcontrols::Peaks&
Chromatography::getPeaks() const
{
    return pImpl_->getPeaks();
}

const adcontrols::Baselines&
Chromatography::getBaselines() const
{
    return pImpl_->getBaselines();
}

////
using namespace chromatogr::internal;
void
Chromatography::impl::clear()
{
}

bool
Chromatography::impl::setup( const adcontrols::PeakMethod& m )
{
    using adcontrols::PeakMethod;

    method_ = m;
    method_.sort();

    return true;
}

bool
Chromatography::impl::findPeaks( const adcontrols::Chromatogram& c )
{
	using adcontrols::Peaks;
	using adcontrols::Baselines;

    tp_ = method_.begin();

	Integrator integrator( c.isCounting() );

    if ( adplot::constants::default_chromatogram_time == adplot::constants::chromatogram_time_seconds ) {
        integrator.minimum_width( method_.minimumWidth() ); // sec
    } else {
        integrator.minimum_width( method_.minimumWidth() * 60.0 ); // min -> sec
    }

    integrator.slope_sensitivity( method_.slope() );  // uV/sec -> uV/sec
	integrator.drift( method_.drift() );  // uV/min -> uV/sec

	// integrator.timeOffset( c.minimumTime() );
	const size_t nSize = c.size();

    // ADDEBUG() << "findPeaks size: " << nSize << ", constant sampled: " << c.isConstantSampledData();

    if ( c.isConstantSampledData() ) {
        double t = 0;
        integrator.sampling_interval( c.sampInterval() ); // sec
        const double * y = c.getIntensityArray();
        for ( size_t i = 0; i < nSize; ++i ) {
            progress( t + i * c.sampInterval(), integrator );
            integrator << std::make_pair( t + i * c.sampInterval(), *y++ );
        }

    } else {

        const double * y = c.getIntensityArray();
        const double * x = c.getTimeArray();
        for ( size_t i = 0; i < c.size(); ++i ) {
            progress( *x, integrator );
            integrator << std::make_pair( *x++, *y++ );
        }
    }

	integrator.close( method_, peaks_, baselines_ );

    //size_t N = 0;
    //double sd = 0;

    double heightTotal = 0;
	double areaTotal = 0;

	for ( Peaks::vector_type::const_iterator it = peaks_.begin(); it != peaks_.end(); ++it ) {
		heightTotal += it->peakHeight();
		areaTotal += it->peakArea();
	}
    peaks_.heightTotal( heightTotal );
    peaks_.areaTotal( areaTotal );

	for ( Peaks::vector_type::iterator it = peaks_.begin(); it != peaks_.end(); ++it ) {
		it->setPercentArea(  ( it->peakArea() / areaTotal ) * 100 );
		it->setPercentHeight( ( it->peakHeight() / heightTotal ) * 100 );
	}

    for ( Peaks::vector_type::iterator it = peaks_.begin(); it != peaks_.end(); ++it ) {
        if ( auto display_name = c.display_name() ){
            it->setName( *display_name + " (" + std::to_string( 1 + std::distance( peaks_.begin(), it ) ) + ")");
        } else {
            it->setName( std::to_string( 1 + std::distance( peaks_.begin(), it ) ) );
        }
    }

    return true;
}

void
Chromatography::impl::progress( double seconds, Integrator& integrator )
{
    using namespace adcontrols::chromatography;

    while ( ( tp_ != method_.end() ) && ( tp_->time( false ) < seconds ) ) {

        if ( tp_->peakEvent() == adcontrols::chromatography::ePeakEvent_Off ) {
            integrator.offIntegration( tp_->boolValue() );
        }

        ++tp_;
    }
}
