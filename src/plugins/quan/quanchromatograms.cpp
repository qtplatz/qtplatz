/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "quanchromatograms.hpp"
#include "quanchromatogram.hpp"
#include "quansampleprocessor.hpp"
#include "quantarget.hpp"
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/utf.hpp>
#include <chromatogr/chromatography.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <numeric>

namespace quan {
    
    template< typename It > struct accumulate {
        size_t size_;
        const It xbeg_;
        const It xend_;        
        const It y_;

        accumulate( It x, It y, size_t size ) : size_( size )
                                              , xbeg_( x )
                                              , xend_( x + size )
                                              , y_( y ) {
        }
        
        double operator()( double lMass, double uMass ) const {
            if ( size_ ) {
                auto lit = std::lower_bound( xbeg_, xend_, lMass );
                if ( lit != xend_ ) {
                    auto bpos = std::distance( xbeg_, lit );
                    auto uit = std::lower_bound( xbeg_, xend_, uMass );
                    if ( uit == xend_ )
                        uit--;
                    while ( uMass < *uit )
                        --uit;
                    auto epos = std::distance( xbeg_, uit );
                    if ( bpos > epos )
                        epos = bpos;

                    return std::accumulate( y_ + bpos, y_ + epos, 0.0 );
                }
            }
            return 0.0;
        }
    };
    
}

using namespace quan;

QuanChromatograms::~QuanChromatograms()
{
}

QuanChromatograms::QuanChromatograms( const std::string& formula
                                      , const std::vector< computed_target_value >& values) : formula_( formula )
                                                                                            , target_values_( values )
{
}

void
QuanChromatograms::append_to_chromatogram( size_t pos, std::shared_ptr<const adcontrols::MassSpectrum> ms )
{
    adcontrols::segment_wrapper<const adcontrols::MassSpectrum> segments( *ms );

    uint32_t fcn = 0;

    for ( auto& fms: segments ) {
        
        double time = fms.getMSProperty().timeSinceInjection();

        uint32_t candidate_index = 0; //  index to masses_

        for ( auto& m : target_values_ ) {

            const double exactMass = target_exactMass( m );
            const double matchedMass = target_matchedMass( m );
            const double width = target_width( m );
            
            double lMass = matchedMass - width / 2;
            double uMass = matchedMass + width / 2;

            if ( fms.getMass( 0 ) <= lMass && uMass < fms.getMass( fms.size() - 1 ) ) {
                double y( 0 );
                if ( fms.isCentroid() ) {
                    y = accumulate<const double *>( fms.getMassArray(), fms.getIntensityArray(), fms.size() )( lMass, uMass );
                } else {
                    double base, rms;
                    double tic = adportable::spectrum_processor::tic( fms.size(), fms.getIntensityArray(), base, rms );
                    adportable::spectrum_processor::areaFraction fraction;
                    adportable::spectrum_processor::getFraction( fraction, fms.getMassArray(), fms.size(), lMass, uMass );

                    y = adportable::spectrum_processor::area( fraction, base, fms.getIntensityArray(), fms.size() );
                }

                auto chro = std::find_if( begin(), end()
                                          , [=] ( std::shared_ptr<QuanChromatogram>& c ) { return c->fcn_ == fcn && c->candidate_index_ == candidate_index; } );
                if ( chro == end() ) {
                    candidates_.push_back( std::make_shared< QuanChromatogram >( fcn, candidate_index, formula_, exactMass, matchedMass, std::make_pair( lMass, uMass) ) );
                    chro = candidates_.end() - 1;
                }

                *( *chro )->chromatogram_ << std::make_pair( time, y );
            }
                        
            candidate_index++;
        }
        ++fcn;    
    }
}

void
QuanChromatograms::process_chromatograms( std::shared_ptr< const adcontrols::ProcessMethod > pm )
{
    if ( pm ) {
        for ( auto& c: candidates_ )
            c->peakinfo_ = std::make_shared< adcontrols::PeakResult >(); // reset all
        
        if ( auto peakm = pm->find< adcontrols::PeakMethod >() ) {
            
            chromatogr::Chromatography peakfinder( *peakm );
            
            for ( auto& c: candidates_ ) {
                
                if ( auto chro = c->chromatogram_ ) {
                    
                    if ( peakfinder( *chro ) ) {
                        
                        c->peakinfo_->setBaselines( peakfinder.getBaselines() );
                        c->peakinfo_->setPeaks( peakfinder.getPeaks() );
                        
                    }
                }
            }
        }
    }

}

void
QuanChromatograms::identify_cpeak( const adcontrols::QuanCompounds * cmpds, std::shared_ptr< const adcontrols::ProcessMethod > )
{
    for ( auto& c: candidates_ )
        c->identify( *cmpds );
}
