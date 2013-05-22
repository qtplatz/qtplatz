/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

namespace chromatogr { namespace internal {

        class ChromatographyImpl {
        public:
            bool setup( const adcontrols::PeakMethod& );
            bool findPeaks( const adcontrols::Chromatogram& );
            void clear();
            inline const adcontrols::Peaks & getPeaks() const { return peaks_; }
            inline const adcontrols::Baselines & getBaselines() const { return baselines_; }
        private:
            adcontrols::PeakMethod method_;
            adcontrols::Peaks peaks_;
            adcontrols::Baselines baselines_;
        };
    }
}

using namespace chromatogr;

Chromatography::~Chromatography()
{
    delete pImpl_;
}

Chromatography::Chromatography() : pImpl_( new internal::ChromatographyImpl() )
{
}

Chromatography::Chromatography( const adcontrols::PeakMethod& method ) : pImpl_( new internal::ChromatographyImpl() )
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
ChromatographyImpl::clear()
{
}

bool
ChromatographyImpl::setup( const adcontrols::PeakMethod& m )
{
    method_ = m;
    return false;
}

bool
ChromatographyImpl::findPeaks( const adcontrols::Chromatogram& c )
{
	using adcontrols::Peaks;
	using adcontrols::Baselines;

	Integrator integrator;

	integrator.minimum_width( method_.minimumWidth() * 60.0 ); // min -> sec
    integrator.slope_sensitivity( method_.slope() );  // uV/sec -> uV/sec
	integrator.drift( method_.drift() / 60.0 );  // uV/min -> uV/sec

	integrator.timeOffset( c.minimumTime() );
    integrator.samping_interval( c.sampInterval() ); // sec
	const size_t nSize = c.size();
	const double * y = c.getIntensityArray();

	for ( size_t i = 0; i < nSize; ++i )
		integrator << *y++;

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
		it->percentArea(  ( it->peakArea() / areaTotal ) * 100 );
		it->percentHeight( ( it->peakHeight() / heightTotal ) * 100 );
	}

    return true;
}

