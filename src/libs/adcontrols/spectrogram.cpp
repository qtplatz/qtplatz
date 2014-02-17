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

namespace adcontrols {
    
    class CountingMap : boost::noncopyable {
    public:
        CountingMap( size_t xCount, size_t yCount ) : m_( xCount, yCount ) {
        }
        void xlimits( double t0, double t1 ) {
            xlimits_ = std::make_pair( t0, t1 );
        }
        void ylimits( double m0, double m1 ) {
            ylimits_ = std::make_pair( m0, m1 );
        }
        size_t dx( double x ) const {
            size_t d = size_t( ((x - xlimits_.first) / ( xlimits_.second - xlimits_.first )) * ( m_.size1() - 1 ) );
			if ( d > m_.size1() - 1 )
				return m_.size1() - 1;
			return d;
        }
        size_t dy( double y ) const {
            size_t d = size_t( ((y - ylimits_.first) / ( ylimits_.second - ylimits_.first )) * ( m_.size2() - 1 ) );
			if ( d > m_.size2() - 1 )
				return m_.size2() - 1;
            return d;
        }
        void operator()( const adcontrols::MassSpectra& spectra, double lMass, double hMass ) {
            ylimits_ = std::make_pair( lMass, hMass );
            m_.clear();
            size_t id = 0;
            for ( auto& ms: spectra ) {
                double t = spectra.x()[id++];
                if ( xlimits_.first <= t && t <= xlimits_.second ) {
                    size_t ix = dx( t );
                    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );
                    for ( auto& fms: segments ) {
                        for ( int i = 0; i < fms.size(); ++i ) {
                            double m = fms.getMass( i );
                            if ( lMass < m && m < hMass ) {
                                size_t iy = dy(m);
                                m_( ix, iy ) += fms.getIntensity( i );
                            }
                        }
                    }
                }
            }
        }
    private:
        boost::numeric::ublas::matrix< double > m_;
        std::pair< double, double > xlimits_, ylimits_;
    };

}

using namespace adcontrols;

Spectrogram::Spectrogram()
{
}

Spectrogram::ClusterFinder::ClusterFinder( const Spectrogram::ClusterMethod& m
                                           , std::function<bool (int curr, int total)> progress ) : method_(m)
                                                                                                  , progress_( progress )
{
}

bool
Spectrogram::ClusterFinder::operator()( const MassSpectra& spectra )
{
    clusters_.clear();
    progress_( 0, int(spectra.size()) );

    if ( method_.lMassLimit_ < 1.0 )
        method_.lMassLimit_ = spectra.lower_mass();
    
    if ( method_.hMassLimit_ < 1.0 )
        method_.hMassLimit_ = spectra.upper_mass();

    // make total peak list
    typedef std::tuple< int, double > peak_type;
    std::vector< peak_type > peaks;
    int idx = 0;
    for ( auto& ms: spectra ) {
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *ms );        
        for ( auto& fms: segs ) {
            for ( size_t i = 0; i < fms.size(); ++i ) {
                double mass = fms.getMass( i );
                peaks.push_back( std::make_tuple( idx, mass ) );
            }
        }
        ++idx;
        progress_( idx, int(spectra.size() ) );
    }
    size_t nCount = peaks.size();
    std::sort( peaks.begin(), peaks.end(), []( const peak_type& a, const peak_type& b ){ return std::get<1>(a) < std::get<1>(b); });

    double mass_window = 0.050; // 10mDa 
    size_t time_window = 30;

    typedef std::vector< peak_type > cluster_type;
    std::vector< cluster_type > clusters;

    size_t iCount = 0;
    progress_( 0, int(nCount) );
    auto itFirst = peaks.begin();
    while ( itFirst != peaks.end() ) {
        double mass = std::get<1>(*itFirst);
        auto lBound = std::lower_bound( peaks.begin(), peaks.end(), mass - 0.0025, [](const peak_type& lhs, double m){ return std::get<1>(lhs) < m; });
        auto hBound = std::lower_bound( peaks.begin(), peaks.end(), mass + 0.0025, [](const peak_type& lhs, double m){ return std::get<1>(lhs) < m; });
        size_t n = std::distance( lBound, hBound );
        if ( n > 10 ) {
            clusters.push_back( cluster_type() );
            auto& cluster = clusters.back();
            for ( auto pk = lBound; pk != hBound; ++pk )
                cluster.push_back( *pk );
        }
        progress_( int(iCount += n), int(nCount) );
        itFirst = hBound;
    }
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
