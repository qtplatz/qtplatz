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
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
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

    struct peak_score_type {
        uint32_t candidate_index;
        uint32_t fcn;
        adcontrols::Peak * peak;
        uint32_t score;
        double error;
        peak_score_type( uint32_t _candidaet_index, uint32_t _fcn, adcontrols::Peak * _peak )
            : candidate_index( _candidaet_index ), fcn( _fcn ), peak( _peak ), score( 0 ), error(0) {
        }
    };

}

using namespace quan;

QuanChromatograms::~QuanChromatograms()
{
}

QuanChromatograms::QuanChromatograms( const std::string& formula
                                      , const std::vector< QuanTarget::target_value >& values
                                      , const std::string& reader_objtext ) : reader_objtext_( reader_objtext )
                                                                            , formula_( formula )
                                                                            , target_values_( values )
                                                                            , identified_( false )
{
}

QuanChromatograms::QuanChromatograms( const std::string& formula
                                      , const QuanCandidate& c ) : formula_( formula )
                                                                 , target_values_( { QuanTarget::target_value( c.formula(), c.exactMass(), 1, c.matchedMass(), c.width() ) } )
                                                                 , candidate_( std::make_shared< QuanCandidate >( c ) )
                                                                 , identified_( false )
{
}

void
QuanChromatograms::append_to_chromatogram( size_t pos, std::shared_ptr<const adcontrols::MassSpectrum> ms, const std::string& reader_objtext )
{
    //if ( reader_objtext != reader_objtext_ )
    //    ADDEBUG() << reader_objtext  << " != " << reader_objtext_;
    
    adcontrols::segment_wrapper<const adcontrols::MassSpectrum> segments( *ms );

    uint32_t fcn = 0;
    
    for ( auto& fms: segments ) {
        
        double time = fms.getMSProperty().timeSinceInjection();

        uint32_t candidate_index = 0; //  index to masses_

        for ( auto& m : target_values_ ) {

            double lMass = m.matchedMass - m.width / 2;
            double uMass = m.matchedMass + m.width / 2;

            auto range = fms.getAcquisitionMassRange();
            if ( range.first <= lMass && uMass < range.second ) {

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
                                          , [=] ( std::shared_ptr<QuanChromatogram>& c ) { return c->fcn() == fcn && c->candidate_index() == candidate_index; } );
                if ( chro == end() ) {
                    qchro_.push_back( std::make_shared< QuanChromatogram >( fcn, candidate_index, formula_, m.exactMass, m.matchedMass, std::make_pair( lMass, uMass ) ) );
                    chro = qchro_.end() - 1;
                }
                // ADDEBUG() << "append_to_chromatogram fcn=" << (*chro)->fcn() << ", " << (*chro)->formula() << " (time,y) = " << time << ", " << y << " (" << (void *)(ms.get() ) << ")";
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
        for ( auto& c : qchro_ )
            c->peakResult()->clear();
        
        if ( auto peakm = pm->find< adcontrols::PeakMethod >() ) {
            
            chromatogr::Chromatography peakfinder( *peakm );
            
            for ( auto& c: qchro_ ) {
                
                if ( auto chro = c->chromatogram() ) {
                    
                    if ( peakfinder( *chro ) ) {
                        
                        c->peakResult()->setBaselines( peakfinder.getBaselines() );
                        c->peakResult()->setPeaks( peakfinder.getPeaks() );
                        
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
    
    for ( auto& c : qchro_ )
        identified_ |= c->identify( *cmpds, c->formula() );  // identified by retention time, if formula match

    return identified_;

}

void
QuanChromatograms::refactor()
{
    auto it = std::remove_if( begin(), end(), []( const std::shared_ptr< QuanChromatogram >& p ){ return ! p->is_identified(); } );
    if ( it != end() )
        qchro_.erase( it, end() );
}


void
QuanChromatograms::refine_chromatograms( const std::string& reader_objtext
                                         , std::vector< QuanCandidate >& refined, std::function<spectra_type(uint32_t)> read )
{
    if ( qchro_.empty() )
        return;

    // all chromatograms in qchro_ are the candidates for 'formula' (single species of mol, charge and adduct)

    adcontrols::ChemicalFormula parser;

    std::map< std::string, double > formulae;
    for ( auto& tgt : target_values_ )
        formulae[ tgt.formula ] = parser.getMonoIsotopicMass( tgt.formula ); // this should be one item

    std::vector< peak_score_type > peaks;
    
    if ( identified_ ) {
        refine_identified_chromatograms( read, peaks );
    } else {
        refine_unidentified_chromatograms( read, peaks );        
    }
    
    if ( peaks.empty() )
        return;
    
    std::vector < std::vector< peak_score_type > > partitioned;
    if ( peaks.size() >= 2 ) {

        // order of retention time
        std::sort( peaks.begin(), peaks.end(), [] ( const peak_score_type& a, const peak_score_type& b ) { return a.peak->peakTime() < b.peak->peakTime(); } );

        auto it = peaks.begin();
        while ( it != peaks.end() ) {
            double width = it->peak->peakWidth();
            std::pair< double, double > range = std::make_pair( it->peak->peakTime() - width / 2, it->peak->peakTime() + width / 2 );
                                     
            auto p = it;
            while ( p != peaks.end() && ( range.first < p->peak->peakTime() && p->peak->peakTime() < range.second ) )
                ++p;
            
            partitioned.push_back( std::vector< peak_score_type >( it, p ) );
            it = p;
        };
    }

    for ( auto& p: partitioned ) {
        if ( identified_ ) {
            std::sort( p.begin(), p.end(), [] ( const peak_score_type& a, const peak_score_type& b ) { return a.candidate_index < b.candidate_index; } );
            p.begin()->score = int( p.size() );
        } else {
            std::sort( p.begin(), p.end(), [] ( const peak_score_type& a, const peak_score_type& b ) { return std::abs( a.error ) < std::abs( b.error ); } );
        }
    }

    std::map < uint32_t, std::pair< std::shared_ptr< QuanChromatogram>, std::shared_ptr< QuanCandidate > > > positive_candidate;

    for ( auto& p: partitioned ) {

        auto& pk = *p.begin();

        auto itChro = std::find_if( qchro_.begin(), qchro_.end(), [ pk ]( const std::shared_ptr< QuanChromatogram >& c ){
                return c->candidate_index() == pk.candidate_index; } );

        if ( itChro != qchro_.end() ) {

            double width = std::abs( ( *itChro )->matchedMass() - ( *itChro )->exactMass() ) * 2.01;
            if ( width < 0.002 )
                width = 0.002;

            adcontrols::MSFinder find( width, adcontrols::idFindClosest );
            
            uint32_t pos = ( *itChro )->pos_from_peak( *pk.peak );
            auto sp = read( pos );
            
            if ( auto centroid = sp.centroid ) {
                if ( auto pkinfo = sp.mspkinfo ) {
                    
                    auto& fms = adcontrols::segment_wrapper<>( *centroid )[ pk.fcn ];
                    auto& fpkinf = adcontrols::segment_wrapper<adcontrols::MSPeakInfo>( *pkinfo )[ pk.fcn ];
                    
                    auto idx = find( fms, ( *itChro )->exactMass() );
                    
                    // assign & annotate on mass spectrum
                    if ( idx != adcontrols::MSFinder::npos && idx < fpkinf.size() ) {
                        
                        auto mspk = fpkinf.begin() + idx;
                        mspk->formula( ( *itChro )->formula() );
                        
                        adcontrols::annotation anno( mspk->formula(), mspk->mass(), mspk->height(), int( idx ), int( mspk->height() ), adcontrols::annotation::dataFormula );
                        fms.get_annotations() << anno;

                        auto qc = std::make_shared< QuanCandidate >( ( *itChro )->formula()
                                                                     , reader_objtext_
                                                                     , ( *itChro )->exactMass()
                                                                     , mspk->mass()
                                                                     , std::make_pair( mspk->centroid_left(), mspk->centroid_right() )
                                                                     , pk.peak->peakTime()
                                                                     , pk.fcn
                                                                     , uint32_t( idx )
                                                                     , sp.profile
                                                                     , sp.centroid
                                                                     , sp.filtered
                                                                     , sp.mspkinfo );
                        positive_candidate[ pk.candidate_index ] = std::make_pair( *itChro, qc );
                    }
                }
            }
        }
    } // for peaks

    // remained chrmatograms will be stored as 'phase-2'
    qchro_.clear();

    for ( auto& candidate : positive_candidate ) {
        qchro_.push_back( candidate.second.first );
        refined.push_back( *candidate.second.second );
    }

}

void
QuanChromatograms::refine_identified_chromatograms( std::function<spectra_type( uint32_t pos )> reader, std::vector< peak_score_type >& peaks )
{
    for ( auto& c : qchro_ ) {
        for ( auto& ppk : c->peaks() ) {
            if ( ! std::string( ppk->formula() ).empty() )
                peaks.push_back( peak_score_type( c->candidate_index(), c->fcn(), ppk ) );
        }
    }
}

void
QuanChromatograms::refine_unidentified_chromatograms( std::function<spectra_type( uint32_t pos )> reader, std::vector< peak_score_type >& peaks )
{

    for ( auto& c : qchro_ ) {

        double width = std::abs( c->matchedMass() - c->exactMass() ) * 2.01;
        if ( width < 0.002 )
            width = 0.002;
        adcontrols::MSFinder find( width, adcontrols::idFindClosest );

        double h = 0;
        for ( auto& ppk : c->peaks() ) {

            uint32_t pos = c->pos_from_peak( *ppk );
            auto sp = reader( pos );
            if ( auto centroid = sp.centroid ) {
                auto & fms = adcontrols::segment_wrapper<>( *centroid )[ c->fcn() ];
                auto idx = find( fms, c->exactMass() );
                if ( idx != adcontrols::MSFinder::npos ) {
                    peak_score_type x( c->candidate_index(), c->fcn(), ppk );
                    x.error = fms.getMass( idx ) - c->exactMass();
                    peaks.push_back( x );
                }
            }
        }
    }
}

void
QuanChromatograms::finalize( std::function<spectra_type(uint32_t)> read )
{
    for ( auto& c : qchro_ ) {

        auto xpeaks = c->peaks();
        if ( xpeaks.empty() )
            continue;

        if ( !identified_ ) {
            auto maxIt = std::max_element( xpeaks.begin(), xpeaks.end()
                                           , []( const adcontrols::Peak * a, const adcontrols::Peak * b ){ return a->peakHeight() < b->peakHeight(); } );
            
            ( *maxIt )->setFormula( c->formula().c_str() );
            ( *maxIt )->setName( adcontrols::ChemicalFormula::formatFormula( c->formula() ) );
        }
    } // for peaks
}

