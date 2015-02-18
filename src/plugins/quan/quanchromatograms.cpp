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
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/utf.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>

using namespace quan;

QuanChromatograms::~QuanChromatograms()
{
}

QuanChromatograms::QuanChromatograms( const std::shared_ptr< adcontrols::ProcessMethod > pm ) : pm_( pm )
                                                                                              , tolerance_( 0.010 )
                                                                                              , compounds_( 0 )
                                                                                              , uptime_( 0 )
{
    if ( auto qm = pm_->find< adcontrols::QuanMethod >() ) {
        
    }
    
    if ( auto compounds_ = pm_->find< adcontrols::QuanCompounds >() ) {
        adcontrols::ChemicalFormula parser;

        for ( auto& comp : *compounds_ ) {

            std::string formula( comp.formula() );
            double exactMass = parser.getMonoIsotopicMass( formula );

            if ( !formula.empty() ) {
                targets_.push_back( std::make_pair( exactMass, formula ) );
                if ( comp.isLKMSRef() )
                    references_.push_back( exactMass );
            }
        }
    }
    
    if ( auto tm = pm_->find< adcontrols::TargetingMethod >() ) {
        tolerance_ = tm->tolerance( adcontrols::idToleranceDaltons );
    }
}

QuanChromatograms::QuanChromatograms( const QuanChromatograms& t ) : targets_( t.targets_ )
                                                                   , pm_( t.pm_ )
                                                                   , tolerance_( t.tolerance_ )
                                                                   , compounds_( t.compounds_ )
{
}

bool
QuanChromatograms::processIt( size_t pos, adcontrols::MassSpectrum& ms )
{
    adcontrols::segment_wrapper<> segments( ms );
    
    if ( auto pCentroidMethod = pm_->find< adcontrols::CentroidMethod >() ) {
    }
    
    if ( auto pCompounds = pm_->find< adcontrols::QuanCompounds >() ) {
        if ( auto lkm = pm_->find< adcontrols::MSLockMethod >() ) {
            if ( lkm->enabled() ) {
                // todo: copy from acquisition lockmass process
            }
        }
    }
    int fcn = 0;
    for ( auto& fms: segments ) {

        double rms, base;
        double tic = adportable::spectrum_processor::tic( ms.size(), ms.getIntensityArray(), base, rms );
        double time = fms.getMSProperty().timeSinceInjection();
        
        auto mrange = std::make_pair( fms.getMass( 0 ), fms.getMass( fms.size() - 1 ) );

        for ( auto& target: targets_ ) {

            double mass = target.first;

            if ( mrange.first < mass && mass < mrange.second ) {
                auto chro = chromatograms_[ target.second ];
                if ( !chro ) {
                    chro = std::make_shared< adcontrols::Chromatogram >();
                    chromatograms_[ target.second ] = chro;
                }

                adportable::spectrum_processor::areaFraction fraction;
                adportable::spectrum_processor::getFraction( fraction, fms.getMassArray(), fms.size(), mass - tolerance_, mass + tolerance_ );
                double i = adportable::spectrum_processor::area( fraction, base, ms.getIntensityArray(), ms.size() );
                double elapsed = chro->size() ? chro->time( chro->size() - 1 ) : 0;
                if ( time < 4000 )
                    *chro << std::make_pair( time, i );
#if defined _DEBUG
                if ( target.second == "N2" )
                    ADDEBUG() << "fcn:" << fcn << " pos:" << pos << " mass:" << mass << " time:" << time << " i:" << i;
#endif
            }
        }
        ++fcn;
    }
    return true;
}

void
QuanChromatograms::save( portfolio::Portfolio& portfolio )
{
    auto folder = portfolio.addFolder( L"Chromatograms" );
    for ( auto& chro: chromatograms_ ) {
        std::wstring wformula = adportable::utf::to_wstring( chro.first );
        auto folium = folder.addFolium( wformula );
        folium.assign( chro.second, chro.second->dataClass() );
    }
}

