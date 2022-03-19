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
#include "lapdeconvdlg.hpp"
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
#include <QStandardItemModel>
#include <algorithm>
#include <set>

namespace {

    struct lap_mass_finder {
        std::shared_ptr< const adcontrols::MassSpectrum > ms_;
        std::shared_ptr< const adcontrols::MassSpectrometer > sp_;

        lap_mass_finder( std::shared_ptr< const adcontrols::MassSpectrum > ms
                         , std::shared_ptr< const adcontrols::MassSpectrometer > sp ) : ms_( ms ), sp_( sp ) {
        }

        template< bool nlapped > std::vector< std::tuple< double, int, double > >
        find( std::vector< std::pair< double, int > >& candidates ) const;

        std::vector< std::tuple< double, int, double > >
        operator()( std::vector< std::pair< double, int > >& candidates ) const;
    };

    template<>
    std::vector< std::tuple< double, int, double > >
    lap_mass_finder::find< false >( std::vector< std::pair< double, int > >& candidates ) const {

        std::vector< double > refms;
        double threshold = ms_->maxIntensity() / 20;
        for ( size_t i = 0; i < ms_->size(); ++i ) {
            if ( ms_->intensity( i ) > threshold ) {
                refms.emplace_back( ms_->mass( i ) );
            }
        }
        std::vector< std::tuple< double, int, double > > result_candidates;

        for ( const auto& rmass: refms ) {
            double delta( 0.1 );
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
        return result_candidates;
    }

    template<>
    std::vector< std::tuple< double, int, double > >
    lap_mass_finder::find< true >( std::vector< std::pair< double, int > >& candidates ) const {
        // operator()( std::vector< std::pair< double, int > >& candidates ) const {

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
                        auto cIt = std::lower_bound( result_candidates.begin()
                                                     , result_candidates.end()
                                                     , delta
                                                     , []( const auto& a, const auto& b ){
                                                         return std::get<2>(a) < b;
                                                     });
                        result_candidates.emplace( cIt, it->first, it->second, delta );
                    }
                }
                rmass = sp_->scanLaw()->getMass( rtof, n++ );
            } while ( rmass > 14.0 && n < ms_->mode() + 40 );
        }
        return result_candidates;
    }
    //------------

    std::vector< std::tuple< double, int, double > >
    lap_mass_finder::operator()( std::vector< std::pair< double, int > >& candidates ) const {
        if ( ms_->mode() == 0 )
            return find<false>( candidates );
        else
            return find<true>( candidates );
    }
    //------------
}

using namespace dataproc;

MSPeakTable::MSPeakTable(QWidget *parent) : adwidgets::MSPeakTable( parent )
{
}

void
MSPeakTable::addContextMenu( QMenu& menu, const QPoint& pos, const QTableView *, const QModelIndexList& list ) const
{
    adwidgets::MSPeakTable::addActionsToContextMenu( menu, pos );

    if ( auto sp = this->massSpectrometer() ) {
        menu.addSeparator();
        menu.addAction( QObject::tr( "lap deconvolution" )
                        , [=,this](){
                            const_cast< MSPeakTable *>(this)->lap_deconvolution( getSelectedPeaks() );
                        });

        menu.addAction( QObject::tr( "list selected peaks" )
                        , [=,this](){
                            if ( auto pks = this->getSelectedPeaks() ) {
                                lap_list( pks );
                            }
                        });
        menu.addSeparator();
    }
}

void
MSPeakTable::lap_deconvolution( std::shared_ptr< adcontrols::MSPeaks > pks )
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
        for ( const auto& pk: *pks ) {
            std::vector< std::pair< double, int > > candidates;
            double mass = pk.mass();
            for ( int n = pk.mode(); n < pk.mode() + 40 && mass >= 13.5; ++n ) {
                mass = sp->assignMass( pk.time(), n );
                candidates.emplace_back( mass, n );
            }
            std::reverse( candidates.begin(), candidates.end() );
            auto result_candidates = lap_mass_finder( ref, sp )( candidates );

            if ( !result_candidates.empty() ) {
                dataproc::lapDeconvDlg dlg;
                dlg.setData( std::move( result_candidates ) );
                if ( dlg.exec() ) {
                    if ( auto select = dlg.getSelection() ) {
#if __cplusplus >= 201703L
                        auto [ mass, lap, error ] = *select;
#else
                        double mass, error; int lap;
                        std::tie( mass, lap, error ) = *select;
#endif
                        auto pk2( pk );
                        pk2.mode( lap );
                        pk2.mass( mass );
                        setMSPeak( std::move( pk2 ) );
                    }
                }
            }
        }
    }

}

void
MSPeakTable::lap_list( std::shared_ptr< adcontrols::MSPeaks > pks ) const
{
    if ( auto sp = this->massSpectrometer() ) {
        std::vector< std::tuple< double, int > > list;
        for ( const auto& pk: *pks ) {
            double mass = pk.mass();
            for ( int n = pk.mode(); n < pk.mode() + 40 && mass >= 10; ++n ) {
                mass = sp->assignMass( pk.time(), n );
                list.emplace_back( mass, n );
            }
        }
        dataproc::lapDeconvDlg dlg;
        dlg.setList( std::move( list ) );
        dlg.exec();
    }
}
