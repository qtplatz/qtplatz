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
#include "quansampleprocessor.hpp"
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/utf.hpp>
#include <chromatogr/chromatography.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>

using namespace quan;

QuanChromatograms::~QuanChromatograms()
{
}

QuanChromatograms::QuanChromatograms( std::shared_ptr< const adcontrols::ProcessMethod > pm ) : pm_( pm )
                                                                                              , tolerance_( 0.010 )
                                                                                              , compounds_( 0 )
                                                                                              , uptime_( 0 )
{
    if ( auto qm = pm_->find< adcontrols::QuanMethod >() ) {
    }

    if ( auto tm = pm_->find< adcontrols::TargetingMethod >() ) {
        tolerance_ = tm->tolerance( adcontrols::idToleranceDaltons );
    }
    
    if ( auto pCompounds = pm_->find< adcontrols::QuanCompounds >() ) {
        if ( auto lkm = pm_->find< adcontrols::MSLockMethod >() ) {

            if ( lkm->enabled() ) {
                
                mslockm_ = std::make_shared< adcontrols::MSLockMethod >( *lkm );
                mslock_ = std::make_shared< adcontrols::lockmass >();

            }
        }
    }
    
    if ( auto compounds_ = pm_->find< adcontrols::QuanCompounds >() ) {
        
        adcontrols::ChemicalFormula parser;
        
        for ( auto& comp : *compounds_ ) {
            
            std::string formula( comp.formula() );
            double exactMass = parser.getMonoIsotopicMass( formula );
            
            if ( ! formula.empty() ) {
                
                targets_.push_back( std::make_tuple( formula, exactMass, std::make_shared< adcontrols::Chromatogram >()
                    , nullptr, nullptr, nullptr, nullptr, nullptr, std::make_pair( 0, 0 ), 0.0, std::make_pair( 0.0, 0.0 ) ) );

                if ( comp.isLKMSRef() )
                    references_.push_back( exactMass );
                
            }
        }

        std::sort( targets_.begin(), targets_.end(), []( const target_t& a, const target_t& b ){ return std::get<idMass>(a) < std::get<idMass>(b); } );
    }
    
}

QuanChromatograms::QuanChromatograms( const QuanChromatograms& t ) : targets_( t.targets_ )
                                                                   , pm_( t.pm_ )
                                                                   , tolerance_( t.tolerance_ )
                                                                   , compounds_( t.compounds_ )
{
}

bool
QuanChromatograms::process1st( size_t pos, adcontrols::MassSpectrum& ms, QuanSampleProcessor& processor )
{
    if ( auto pCentroidMethod = pm_->find< adcontrols::CentroidMethod >() ) {
    }
    
    if ( lockmass_enabled() ) {
        bool locked = false;
        if ( auto raw = processor.getLCMSDataset() ) {
            adcontrols::lockmass lkms;
            if ( raw->mslocker( lkms ) )
                locked = lkms( ms, true );
        }
        if ( !locked )
            doMSLock( ms );
    }

    adcontrols::segment_wrapper<> segments( ms );
    
    int fcn = 0;
    for ( auto& fms: segments ) {

        double rms, base;
        double tic = adportable::spectrum_processor::tic( ms.size(), ms.getIntensityArray(), base, rms );
        double time = fms.getMSProperty().timeSinceInjection();
        if ( time >= 4000 ) // workaround for negative time value at the begining of time-event function caused delay
            time = 0;
        
        for ( auto& t: targets_ ) {

            double mass = std::get< idMass >( t );
            double lMass = mass - tolerance_ / 2;
            double uMass = mass + tolerance_ / 2;

            if ( fms.getMass( 0 ) < lMass && uMass < fms.getMass( fms.size() - 1 ) ) {
                
                if ( auto pChro = std::get< idChromatogram >( t ) ) {

                    adportable::spectrum_processor::areaFraction fraction;
                    adportable::spectrum_processor::getFraction( fraction, fms.getMassArray(), fms.size(), lMass, uMass );
                    
                    double d = adportable::spectrum_processor::area( fraction, base, fms.getIntensityArray(), fms.size() );
                    
                    *pChro << std::make_pair( time, d );
                }
                
            }
            
        }
        ++fcn;
    }
    return true;
}

bool
QuanChromatograms::process2nd( size_t pos, const adcontrols::MassSpectrum& ms )
{
    adcontrols::segment_wrapper<const adcontrols::MassSpectrum> segments( ms );
    
    int fcn = 0;
    for ( auto& fms: segments ) {

        double rms, base;
        double tic = adportable::spectrum_processor::tic( ms.size(), ms.getIntensityArray(), base, rms );
        double time = fms.getMSProperty().timeSinceInjection();
        if ( time >= 4000 ) // workaround for negative time value at the begining of time-event function caused delay
            time = 0;
        
        for ( auto& t: targets_ ) {

            double lMass = std::get< idMSWidth >( t ).first;
            double uMass = std::get< idMSWidth >( t ).second;

            if ( lMass < uMass ) {

                if ( fms.getMass( 0 ) < lMass && uMass < fms.getMass( fms.size() - 1 ) ) {
                    
                    if ( auto pChro = std::get< idChromatogram >( t ) ) {
                        
                        adportable::spectrum_processor::areaFraction fraction;
                        adportable::spectrum_processor::getFraction( fraction, fms.getMassArray(), fms.size(), lMass, uMass );
                        
                        double d = adportable::spectrum_processor::area( fraction, base, fms.getIntensityArray(), fms.size() );
                        
                        *pChro << std::make_pair( time, d );
                    }
                    
                }
                
            }
        }
        ++fcn;
    }
    return true;
}

bool
QuanChromatograms::doMSLock( adcontrols::MassSpectrum& profile )
{
    //--------- centroid --------
    auto centroid = std::make_shared< adcontrols::MassSpectrum >();

    if ( auto m = pm_->find< adcontrols::CentroidMethod >() ) {

        adcontrols::CentroidProcess peak_detector;

        adcontrols::segment_wrapper<> segments( profile );
        
        centroid->clone( profile, false );

        if ( peak_detector( *m, profile ) ) {
            peak_detector.getCentroidSpectrum( *centroid );

            for ( auto fcn = 0; fcn < profile.numSegments(); ++fcn ) {
                adcontrols::MassSpectrum cseg;
                peak_detector( profile.getSegment( fcn ) );
                peak_detector.getCentroidSpectrum( cseg );
                centroid->addSegment( cseg );
            }

        }
    }
    
    //--------- lockmass --------
    // this does not find reference from segments attached to 'centroid'
    if ( centroid ) {
        
        adcontrols::MSFinder find( mslockm_->tolerance( mslockm_->toleranceMethod() ), mslockm_->algorithm(), mslockm_->toleranceMethod() );

        adcontrols::segment_wrapper<> segments( *centroid );
        
        if ( auto pCompounds = pm_->find< adcontrols::QuanCompounds >() ) {
            for ( auto& compound: *pCompounds ) {

                if ( compound.isLKMSRef() ) {
                    double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( compound.formula() );                    

                    for ( auto& fms: segments ) {
                        if ( fms.getMass( 0 ) < exactMass && exactMass < fms.getMass( fms.size() - 1 ) ) {
                            auto idx = find( fms, exactMass );
                            if ( idx != adcontrols::MSFinder::npos ) {
                                *mslock_ << adcontrols::lockmass::reference( compound.formula(), exactMass, fms.getMass( idx ), fms.getTime( idx ) );
                            }
                        }
                    }
                }
            }
        }
    }
    if ( mslock_ )
        mslock_->fit();
    ( *mslock_ )( profile, true );
    return true;
}

void
QuanChromatograms::commit( QuanSampleProcessor& processor )
{
    if ( auto peakm = pm_->find< adcontrols::PeakMethod >() ) {
        chromatogr::Chromatography peakfinder( *peakm );

        for ( auto& t: targets_ ) {
            if ( auto pChro = std::get< idChromatogram >( t ) ) {
                if ( peakfinder( *pChro ) ) {
                    auto result = std::make_shared< adcontrols::PeakResult >();
                    result->setBaselines( peakfinder.getBaselines() );
                    result->setPeaks( peakfinder.getPeaks() );
                    std::get< idPeakResult >( t ) = result;
                }
            }
        }
    }

    if ( auto portfolio = processor.portfolio() ) {

        auto folder = portfolio->addFolder( L"Chromatograms" );

        for ( auto& t : targets_ ) {
            std::wstring wformula = adportable::utf::to_wstring( std::get<idFormula>( t ) );
            auto pChro = std::get<idChromatogram>( t );
            
            while ( auto folium = folder.findFoliumByName( wformula ) ) {
                processor.fetch( folium );
                folder.removeFolium( folium );
            }
            
            if ( auto folium = folder.addFolium( wformula ) ) {
                folium.assign( pChro, pChro->dataClass() );

                portfolio::Folium att = folium.addAttachment( L"Peak Result" );
                if ( auto pResult = std::get< idPeakResult >( t ) )
                    att.assign( pResult, pResult->dataClass() );
                att.addAttachment( L"Process Method" ).assign( pm_, pm_->dataClass() );
            }
        }
    }

}

bool
QuanChromatograms::lockmass_enabled() const
{
    return mslockm_ && mslock_;
}
