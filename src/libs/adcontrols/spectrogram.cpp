/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "spectrogram.hpp"
#include "massspectra.hpp"
#include "massspectrum.hpp"
#include "chromatogram.hpp"
#include <adportable/array_wrapper.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/debug.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <algorithm>
#include <list>

using namespace adcontrols;

Spectrogram::Spectrogram()
{
}

Spectrogram::ClusterData::ClusterData()
{
}

Spectrogram::ClusterData::ClusterData( const ClusterData& t ) : peaks_( t.peaks_ )
{
}

Spectrogram::ClusterData::~ClusterData()
{
    peaks_.clear();
}

Spectrogram::ClusterFinder::ClusterFinder( const Spectrogram::ClusterMethod& m
                                           , std::function<bool (int curr, int total)> progress ) : method_(m)
                                                                                                  , progress_( progress )
{
}

bool
Spectrogram::ClusterFinder::operator()( const MassSpectra& spectra
                                        , std::vector< std::shared_ptr< ClusterData > >& clusters )
{
    progress_( 0, int(spectra.size()) );

    if ( method_.lMassLimit_ < 1.0 )
        method_.lMassLimit_ = spectra.lower_mass();
    
    if ( method_.hMassLimit_ < 1.0 )
        method_.hMassLimit_ = spectra.upper_mass();

    int idx = 0;
    std::vector< peak_type > peaks;
    for ( auto& ms: spectra ) {
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *ms );        
        for ( auto& fms: segs ) {
            for ( size_t i = 0; i < fms.size(); ++i )
                peaks.push_back( peak_type( idx, fms.getMass( i ), fms.getIntensity( i ) ) );
        }
        ++idx;
        progress_( idx, int(spectra.size() ) );
    }

	std::sort( peaks.begin(), peaks.end(), []( const peak_type& a, const peak_type& b ){ return a.mass_ < b.mass_; });

    double mass_window = 0.005;

	size_t npeaks = peaks.size();
    int iprog = 0;
	progress_( iprog, int(npeaks) );

    // std::vector< std::shared_ptr< ClusterData > > pclusters;

    auto it = peaks.begin();
    while ( it != peaks.end() ) {

        work_.push_back( std::make_shared< ClusterData>() );
        ClusterData& cluster = *work_.back();

		it->flag_ = true;
        cluster.push_back( *it );

        for ( auto it2 = it + 1; it2 != peaks.end() && it2->mass_ < it->mass_ + mass_window; ++it2 ) {
			if ( ! it2->flag_ ) {
                auto equalIdx = std::find_if( cluster.begin(), cluster.end(), [=]( const peak_type& a ){ return a.idx_ == it2->idx_; });
                if ( equalIdx == cluster.end() ) {
					uint32_t fidx = cluster.font().idx_ < 5 ? 0 : cluster.font().idx_ - 5;
					uint32_t bidx = cluster.back().idx_ + 5;
                    if ( fidx < it2->idx_ && it2->idx_ < bidx ) {
                        it2->flag_ = true;
                        cluster.insert( std::lower_bound( cluster.begin(), cluster.end()
                                                                 , it2->idx_, []( const peak_type& a, uint32_t i ){ return a.idx_ < i; }), *it2 );
                    }
                }
			}
        }

        while ( it != peaks.end() && it->flag_ )
            ++it;

        size_t count = std::distance( peaks.begin(), it );
        if ( int( count % 2000 ) == 0 ) {
            progress_( int( count ), int(npeaks) );
			ADDEBUG() << "progress: " << count << "/" << npeaks;
        }
    }

    std::for_each( work_.begin(), work_.end(), [&]( std::shared_ptr< ClusterData >& p ){
            if ( p->size() >= 5 )
                clusters.push_back( p );
        });

	return true;
}

Spectrogram::ChromatogramExtractor::ChromatogramExtractor( const MassSpectra& spectra ) : spectra_( spectra )
{
    for ( auto& x: spectra.x() )
        seconds_.push_back( x * 60.0 );
}

void
Spectrogram::ChromatogramExtractor::operator () ( Chromatogram& c, double lMass, double hMass )
{
    int idx = 0;

    c.resize( spectra_.size() );
    c.setTimeArray( seconds_.data() );

    for ( const auto& ms: spectra_ ) {
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *ms );
        double y = 0;
        for ( auto& fms: segs ) {
            adportable::array_wrapper< const double > masses( fms.getMassArray(), fms.size() );
            auto it = std::lower_bound( masses.begin(), masses.end(), lMass );
            while ( *it++ <= hMass )
                y += fms.getIntensity( std::distance( masses.begin(), it ) );
        }
        c.setIntensity( idx++, y );
    }
    
}
