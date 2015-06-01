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
#include "quancandidate.hpp"
#include "quansampleprocessor.hpp"
#include "quantarget.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
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
#include <algorithm>
#include <numeric>
#include <set>

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
                                                                                            , identified_(false)
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
                ( *chro )->append( uint32_t( pos ), time, y );
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

                        for ( auto& pk: c->peakinfo_->peaks() )
                            pk.userData( c->candidate_index() );
                        
                    }
                }
            }
        }
    }

}

bool
QuanChromatograms::identify( const adcontrols::QuanCompounds * cmpds
                             , std::shared_ptr< const adcontrols::ProcessMethod > )
{
    identified_ = false;

    for ( auto& c : candidates_ ) {

        if ( c->identify( *cmpds ) )  // identified by retention time
            identified_ = true;
    }

    return identified_;

}

void
QuanChromatograms::getPeaks( std::vector< std::pair< uint32_t, adcontrols::Peak * > >& vec, bool identified )
{
    for ( auto& c : candidates_ ) {
        
        for ( auto& pk : c->peaks( identified ) ) {
            uint32_t pos = c->pos_from_peak( *pk );
            vec.push_back( std::make_pair( c->candidate_index(), pk ) );
        }

    }
}

uint32_t
QuanChromatograms::posFromPeak( uint32_t candidate_index, const adcontrols::Peak& peak ) const
{
    auto it = std::find_if( candidates_.begin(), candidates_.end(), [candidate_index](const std::shared_ptr< QuanChromatogram >& c ){
            return c->candidate_index() == candidate_index; });

    if ( it != candidates_.end() )
        return (*it)->pos_from_peak( peak );
    
    return (-1);
}

void
QuanChromatograms::refine_chromatograms( std::vector< QuanCandidate >& refined, std::function<spectra_type(uint32_t)> read )
{
    if ( candidates_.empty() )
        return;

    adcontrols::ChemicalFormula parser;

    std::map< std::string, double > formulae;
    for ( auto& tgt : target_values_ ) {
        std::string formula = target_formula( tgt );
        formulae[ formula ] = parser.getMonoIsotopicMass( formula );
    }

    struct peak_score_type {
        uint32_t candidate_index;
        uint32_t fcn;
        adcontrols::Peak * peak;
        int score;
        peak_score_type( uint32_t _candidaet_index, uint32_t _fcn, adcontrols::Peak * _peak )
            : candidate_index( _candidaet_index ), fcn( _fcn ), peak( _peak ), score( 0 ) {
        }
    };
    // typedef std::tuple< uint32_t, uint32_t, adcontrols::Peak *, int > peak_score_type;
    
    std::vector< peak_score_type > peaks;

    for ( auto& c : candidates_ ) {
        if ( identified_ ) {
            for ( auto& ppk : c->peaks( false ) )
                peaks.push_back( peak_score_type( c->candidate_index(), c->fcn(), ppk ) );
        } else {
            auto xpeaks = c->peaks( false );
            if ( xpeaks.empty() )
                return;

            auto maxIt = std::max_element( xpeaks.begin(), xpeaks.end(), []( const adcontrols::Peak * a, const adcontrols::Peak * b ){ return a->peakHeight() < b->peakHeight(); } );
            double h = ( *maxIt )->peakHeight();

            ( *maxIt )->formula( c->formula_.c_str() );
            ( *maxIt )->name( adportable::utf::to_wstring( c->formula_ ).c_str() );

            auto it = std::remove_if( xpeaks.begin(), xpeaks.end(), [h]( const adcontrols::Peak * a ){ return a->peakHeight() < h * 0.1; } );
            if ( it != xpeaks.end() )
                xpeaks.erase( it, xpeaks.end() );

            for ( auto& ppk : xpeaks )
                peaks.push_back( peak_score_type( c->candidate_index(), c->fcn(), ppk ) );            
        }
    }
    if ( peaks.empty() )
        return;

    if ( identified_ ) { // if retention-time identified
        auto it = std::remove_if( peaks.begin(), peaks.end(), [] ( const peak_score_type& t ) { return std::string( t.peak->formula() ).empty(); } );
        if ( it != peaks.end() )
            peaks.erase( it, peaks.end() );
    } else {
        
    }

    std::vector < std::vector< peak_score_type > > partitioned;
    if ( peaks.size() >= 2 ) {

        // order of retention time
        std::sort( peaks.begin(), peaks.end(), [] ( const peak_score_type& a, const peak_score_type& b ) { return a.peak->peakTime() < b.peak->peakTime(); } );

        auto it = peaks.begin();
        while ( it != peaks.end() ) {
            double width = identified_ ? it->peak->peakWidth() * 0.2 : it->peak->peakWidth() / 0.4;
            auto p = std::lower_bound( it, peaks.end(), ( *it->peak ), [width] ( const peak_score_type& a, const adcontrols::Peak& pk ) {
                    double tR = a.peak->peakTime();
                    return ( pk.peakTime() - width ) < tR && tR < ( pk.peakTime() + width );
                } );
            partitioned.push_back( std::vector< peak_score_type >( it, p ) );
            it = p;
        };
    }
    for ( auto& p: partitioned ) {
        std::sort( p.begin(), p.end(), [] ( const peak_score_type& a, const peak_score_type& b ) { return a.candidate_index < b.candidate_index; } );
        p.begin()->score = int( p.size() );
    }

    std::map< uint32_t, std::shared_ptr< QuanChromatogram> > positive_candidate;

    for ( auto& p: partitioned ) {

        auto& pk = *p.begin();

        auto itChro = std::find_if( candidates_.begin(), candidates_.end(), [ pk ]( const std::shared_ptr< QuanChromatogram >& c ){
                return c->candidate_index() == pk.candidate_index; } );

        if ( itChro != candidates_.end() ) {

            double width = std::abs( ( *itChro )->matchedMass() - ( *itChro )->exactMass() ) * 2.01;
            if ( width < 0.002 )
                width = 0.002;

            adcontrols::MSFinder find( width, adcontrols::idFindClosest );
            
            uint32_t pos = ( *itChro )->pos_from_peak( *pk.peak );
            auto sp = read( pos );
            
            if ( auto centroid = std::get< idCentroid >( sp ) ) {
                if ( auto pkinfo = std::get< idMSPeakInfo >( sp ) ) {
                    
                    for ( auto& f : formulae ) {
                        
                        double& exactMass = f.second;
                        const std::string& formula = f.first;
                        
                        auto& fms = adcontrols::segment_wrapper<>( *centroid )[ pk.fcn ];
                        auto& fpkinf = adcontrols::segment_wrapper<adcontrols::MSPeakInfo>( *pkinfo )[ pk.fcn ];
                        
                        auto idx = find( fms, exactMass );

                        // assign & annotate on mass spectrum
                        if ( idx != adcontrols::MSFinder::npos && idx < fpkinf.size() ) {

                            auto mspk = fpkinf.begin() + idx;
                            mspk->formula( formula );
                                                    
                            adcontrols::annotation anno( formula, mspk->mass(), mspk->height(), int( idx ), int( mspk->height() ), adcontrols::annotation::dataFormula );
                            fms.get_annotations() << anno;
                            
                            positive_candidate[ pk.candidate_index ] = *itChro;

                            refined.push_back( QuanCandidate( formula, exactMass, mspk->mass(), std::make_pair( mspk->centroid_left(), mspk->centroid_right() ), pk.peak->peakTime(), pk.fcn ) );
                        }
                        
                    }
                }
            }
        }
    } // for peaks

    candidates_.clear();
    for ( auto& candidate : positive_candidate )
        candidates_.push_back( candidate.second );

}
                                         
