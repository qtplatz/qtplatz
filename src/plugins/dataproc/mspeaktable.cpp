/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "mspeaktable.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <boost/format.hpp>
#include <QMenu>
#include <algorithm>
#include <set>

namespace {

    struct lap_mass_finder {
        std::shared_ptr< const adcontrols::MassSpectrum > ms_;
        std::shared_ptr< const adcontrols::MassSpectrometer > sp_;

        lap_mass_finder( std::shared_ptr< const adcontrols::MassSpectrum > ms
                         , std::shared_ptr< const adcontrols::MassSpectrometer > sp ) : ms_( ms ), sp_( sp ) {
        }

        void find( std::vector< std::pair< double, int > >& candidates ) const {

            std::vector< double > refms;
            double threshold = ms_->getMaxIntensity() / 20;
            for ( size_t i = 0; i < ms_->size(); ++i ) {
                if ( ms_->intensity( i ) > threshold ) {
                    refms.emplace_back( ms_->mass( i ) );
                }
            }
            std::vector< std::tuple< double, int, double > > result_candidates;

            double delta( 0.1 );
            for ( const auto& rmass: refms ) {
                auto it = std::lower_bound( candidates.begin(), candidates.end(), rmass, [](const auto& a, const auto& b){ return a.first < b; });
                if ( it != candidates.end() ) {
                    if ( it != candidates.begin() ) {
                        if ( std::abs( ( it - 1 )->first - rmass ) < std::abs( it->first - rmass ) ) {
                            --it;
                        }
                    }
                    if ( std::abs( it->first - rmass ) < delta ) {
                        delta = std::abs( it->first - rmass );
                        auto cIt = std::lower_bound( result_candidates.begin()
                                                    , result_candidates.end()
                                                    , delta
                                                    , [](const auto& a, const auto& b ){
                                                        return std::get<2>(a) < b;
                                                    });
                        result_candidates.emplace( cIt, it->first, it->second, delta );
                    }
                }
            }

            // ADDEBUG() << "result candidates size: " << result_candidates.size();
            int limit(5);
            for ( auto& r: result_candidates ) {
                auto [ mass, lap, error ] = r;
                ADDEBUG() << "lap-deconvolution candidate:\t" << mass << ", " << lap << ", " << error;
                if ( --limit == 0 )
                    break;
            }
        }

        void operator()( std::vector< std::pair< double, int > >& candidates ) const {
            // size_t idx;
            // std::tie( std::ignore, idx ) = ms_->minmax_element();
            std::vector< std::tuple< double, int, double > > result_candidates;

            for ( size_t i = 0; i < ms_->size(); ++i ) {
                //for ( size_t i = idx; i < idx + 1; ++i ) {
                double rtof = ms_->time( i );
                int n = ms_->mode();
                double rmass = sp_->scanLaw()->getMass( rtof, n );
                double delta(0.1);
                do {
                    auto it = std::lower_bound( candidates.begin(), candidates.end(), rmass, [](const auto& a, const auto& b){ return a.first < b; });
                    if ( it != candidates.end() ) {
                        if ( it != candidates.begin() ) {
                            if ( std::abs( ( it - 1 )->first - rmass ) < std::abs( it->first - rmass ) ) {
                                --it;
                            }
                        }
                        if ( std::abs( it->first - rmass ) < delta ) {
                            delta = std::abs( it->first - rmass );
                            result_candidates.emplace_back( it->first, it->second, std::abs(it->first - rmass) );
                            ADDEBUG() << "\tcandidate mass: " << *it << ", reference mass: " << std::make_pair( rmass, n )
                                      << ", error: " << ( it->first - rmass );
                        }
                    }
                    rmass = sp_->scanLaw()->getMass( rtof, n++ );
                } while ( rmass > 14.0 && n < ms_->mode() + 40 );
            }

            std::sort( result_candidates.begin(), result_candidates.end()
                       , []( const auto& a, const auto& b){ return std::get<2>(a) < std::get<2>(b); });

            int limit(10);
            for ( auto& r: result_candidates ) {
                 auto [ mass, lap, error ] = r;
                 ADDEBUG() << mass << ", " << lap << ", " << error;
                 if ( --limit == 0 )
                     break;
            }
        }
    };

}

using namespace dataproc;

MSPeakTable::MSPeakTable(QWidget *parent) : adwidgets::MSPeakTable( parent )
{
}

// void
// MSPeakTable::addContextMenu( QMenu& menu
//                              , const QPoint& pos
//                              , std::shared_ptr< const adcontrols::MassSpectrum > ms ) const
// {
//     adwidgets::MSPeakTable::addActionsToContextMenu( menu, pos );
//     menu.addSeparator();

//     if ( ms ) {
//         if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
//             dp->addContextMenu( adprocessor::ContextMenuOnMSPeakTable, menu, ms, { 0, 0 }, false );
//         }
//     }
// }

void
MSPeakTable::addContextMenu( QMenu& menu, const QPoint& pos, const QTableView *, const QModelIndexList& list ) const
{
    adwidgets::MSPeakTable::addActionsToContextMenu( menu, pos );

    if ( auto sp = this->massSpectrometer() ) {
        menu.addSeparator();
        menu.addAction( QObject::tr( "lap deconvolution" )
                        , [=](){
                            lap_deconvolution( getSelectedPeaks() );
                        });

        menu.addAction( QObject::tr( "list selected peaks" )
                        , [=](){
                            if ( auto pks = this->getSelectedPeaks() ) {
                                if ( auto scanlaw = sp->scanLaw() ) {
                                    for ( const auto& pk: *pks ) {
                                        ADDEBUG() << "formula: " << pk.formula()
                                                  << ", mass: " << pk.mass()
                                                  << ", time: " << pk.time()
                                                  << ", mode: " << pk.mode()
                                                  << ", proto: " << pk.fcn();
                                        double mass = pk.mass();
                                        for ( int n = pk.mode(); n < pk.mode() + 40 && mass >= 10; ++n ) {
                                            auto m = scanlaw->getMass( pk.time(), n );
                                            ADDEBUG() << "n,mass:\t" << n << "\t" << boost::format( "%.4f" ) % m;
                                            mass = m;
                                        }
                                    }
                                }
                            }
                        });
        menu.addSeparator();
    }
}

void
MSPeakTable::lap_deconvolution( std::shared_ptr< adcontrols::MSPeaks > pks ) const
{
    std::shared_ptr< adcontrols::MassSpectrum > ref;

    if ( auto me = SessionManager::instance()->getActiveDataprocessor() ) {
        for ( auto& session : *SessionManager::instance() ) {
            if ( auto processor = session.processor() ) {
                if ( processor != me ) {
                    auto spectra = processor->getPortfolio().findFolder( L"Spectra" );
                    for ( auto& folium: spectra.folio() ) {
                        if ( folium.attribute( L"isChecked" ) == L"true" ) {
                            if ( folium.empty() )
                                processor->fetch( folium );
                            if ( auto it = portfolio::find_first_of( folium.attachments()
                                                                    , []( const auto& a ){
                                                                        return a.name() == Constants::F_CENTROID_SPECTRUM; } ) ) {
                                if ( auto bar = portfolio::get< adcontrols::MassSpectrumPtr > ( it ) ) {
                                    ref = bar;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    if ( !ref )
        return;

    if ( auto sp = this->massSpectrometer() ) {
        auto scanlaw = sp->scanLaw();
        for ( const auto& pk: *pks ) {
            std::vector< std::pair< double, int > > candidates;
            double mass = pk.mass();
            for ( int n = pk.mode(); n < pk.mode() + 40 && mass >= 13.5; ++n ) {
                mass = scanlaw->getMass( pk.time(), n );
                candidates.emplace_back( mass, n );
            }
            std::reverse( candidates.begin(), candidates.end() );
            if ( ref->mode() == 0 ) {
                lap_mass_finder( ref, sp ).find( candidates );
            } else {
                lap_mass_finder( ref, sp )( candidates );
            }
        }
    }

}
