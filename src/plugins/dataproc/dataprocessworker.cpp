/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "dataprocessworker.hpp"
#include "dataprocessor.hpp"
#include "dataprochandler.hpp"
#include "sessionmanager.hpp"
#include "mainwindow.hpp"
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/mschromatogramextractor.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/spectrogram.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adwidgets/datareaderchoicedialog.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#include <coreplugin/icore.h>
#include <QMessageBox>
#include <boost/format.hpp>
#include <functional>
#include <chrono>

using namespace dataproc;

DataprocessWorker::DataprocessWorker() : work_( io_service_ )
{
}

DataprocessWorker::~DataprocessWorker()
{
    std::lock_guard< std::mutex > lock( mutex_ );
	io_service_.stop();
    for ( auto& t: threads_ )
        t.join();
}

DataprocessWorker *
DataprocessWorker::instance()
{
    static DataprocessWorker __instance;
    return &__instance;
}

// void
// DataprocessWorker::createChromatograms( Dataprocessor *, std::shared_ptr< adcontrols::MassSpectrum >&, double lMass, double hMass )
// {
// 	(void)lMass;
// 	(void)hMass;
// }

void
DataprocessWorker::createChromatograms( Dataprocessor* processor,  std::shared_ptr< const adcontrols::ProcessMethod > pm, const QString& origin )
{
}

void
DataprocessWorker::createChromatogramsV3( Dataprocessor* processor
                                          , adcontrols::hor_axis axis
                                          , const std::vector< std::tuple< int, double, double > >& ranges
                                          , const adcontrols::DataReader * reader )
{
    // Time axis can only be accepted
    if ( axis == adcontrols::hor_axis_mass )
        return;

    // copied from msprocessingwnd::make_chromatograms
    for ( auto& range: ranges ) {

        std::pair< double, double > display_value = std::make_pair( std::get<1>( range ), std::get<2>( range ) );
        if ( axis == adcontrols::hor_axis_time )
            display_value = std::make_pair( adcontrols::metric::scale_to_micro( display_value.first ), adcontrols::metric::scale_to_micro( display_value.second ) );

        int fcn = std::get<0>( range );
        double time = std::get<1>( range );
        double width = std::get<2>( range );

        // mass|time,width pair
        if ( auto pChr = reader->getChromatogram( std::get<0>( range ) /*fcn*/, std::get<1>( range ), std::get<2>( range ) ) ) {

            portfolio::Portfolio portfolio = processor->getPortfolio();
            portfolio::Folder folder = portfolio.findFolder( L"Chromatograms" );

            std::wostringstream o;
            if ( axis == adcontrols::hor_axis_time ) {
                o << boost::wformat( L"%s %.3lf(us)(w=%.2lf(ns))" )
                    % adportable::utf::to_wstring( reader->display_name() ) % ( std::get<1>( range ) * 1.0e6 ) % ( std::get<2>( range ) * 1.0e9 );
            } else {
                o << boost::wformat( L"%s %.3lf(w=%.2lf(mDa))" )
                    % adportable::utf::to_wstring( reader->display_name() ) % ( std::get<1>( range ) ) % ( std::get<2>( range ) * 1000 );
            }

            auto folium = folder.findFoliumByName( o.str() );
            if ( folium.nil() ) {
                pChr->addDescription( adcontrols::description( L"acquire.title", o.str() ) );
                adcontrols::ProcessMethod m;
                MainWindow::instance()->getProcessMethod( m );
                portfolio::Folium folium = processor->addChromatogram( *pChr, m, true );
            }
            processor->setCurrentSelection( folium );
        }
    }

	adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
	MainWindow::instance()->getProcessMethod( *pm );
}


void
DataprocessWorker::createChromatogramsV2( Dataprocessor* processor,  std::shared_ptr< const adcontrols::ProcessMethod > pm, const QString& origin )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( threads_.empty() )
            threads_.push_back( adportable::asio::thread( [=] { io_service_.run(); } ) );
    } while( 0 );

    if ( auto rawfile = processor->getLCMSDataset() ) {
        if ( auto tm = pm->find< adcontrols::MSChromatogramMethod >() ) {
            if ( rawfile->dataformat_version() >= 3 ) {
                adwidgets::DataReaderChoiceDialog dlg( rawfile->dataReaders() );
                if ( dlg.exec() == QDialog::Accepted ) {
                    int fcn = dlg.fcn();
                    if ( auto reader = rawfile->dataReaders().at( dlg.currentSelection() ) )
                        threads_.push_back( adportable::asio::thread( [=] { handleCreateChromatogramsV3( processor, *tm, pm, reader, fcn, p ); } ) );
                }
            } else {
                threads_.push_back( adportable::asio::thread( [=] { handleCreateChromatogramsV2( processor, *tm, pm, p ); } ) );
            }
        }
    }
}

void
DataprocessWorker::createChromatogramsV2( Dataprocessor* processor
                                          , adcontrols::hor_axis axis
                                          , const std::vector< std::tuple< int, double, double > >& ranges )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );
    
    std::lock_guard< std::mutex > lock( mutex_ );
	if ( threads_.empty() )
        threads_.push_back( adportable::asio::thread( [=] { io_service_.run(); } ) );

	adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
	MainWindow::instance()->getProcessMethod( *pm );
    
    threads_.push_back( adportable::asio::thread( [=] { handleCreateChromatogramsV2( processor, pm, axis, ranges, p ); } ) );
}

void
DataprocessWorker::createSpectrogram( Dataprocessor* processor )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( threads_.empty() )
            threads_.push_back( adportable::asio::thread( [=] { io_service_.run(); } ) );
    } while ( 0 );

    adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
    MainWindow::instance()->getProcessMethod( *pm );
    
    if ( auto rawfile = processor->getLCMSDataset() ) {
        if ( rawfile->dataformat_version() >= 3 ) {
            adwidgets::DataReaderChoiceDialog dlg( rawfile->dataReaders() );
            if ( dlg.exec() == QDialog::Accepted ) {
                int fcn = dlg.fcn();
                if ( auto reader = rawfile->dataReaders().at( dlg.currentSelection() ) )
                    threads_.push_back( adportable::asio::thread( [=] { handleCreateSpectrogram( processor, pm, reader, fcn, p ); } ) );
            }
        } else {
            threads_.push_back( adportable::asio::thread( [=] { handleCreateSpectrogram( processor, pm, p ); } ) );
        }
    }
}

void
DataprocessWorker::clusterSpectrogram( Dataprocessor * processor )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    std::lock_guard< std::mutex > lock( mutex_ );
	if ( threads_.empty() )
		threads_.push_back( adportable::asio::thread( [=] { io_service_.run(); } ) );

	adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
	MainWindow::instance()->getProcessMethod( *pm );

    threads_.push_back( adportable::asio::thread( [=] { handleClusterSpectrogram( processor, pm, p ); } ) );
}

void
DataprocessWorker::findPeptide( Dataprocessor * processor, const adprot::digestedPeptides& /*peptides*/ )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    std::lock_guard< std::mutex > lock( mutex_ );
	if ( threads_.empty() )
		threads_.push_back( adportable::asio::thread( [=] { io_service_.run(); } ) );

	adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
	MainWindow::instance()->getProcessMethod( *pm );

    threads_.push_back( adportable::asio::thread( [=] { handleFindPeptide( processor, pm, p ); } ) );
}

void
DataprocessWorker::join( const adportable::asio::thread::id& id )
{
    std::lock_guard< std::mutex > lock( mutex_ );

	auto it = std::find_if( threads_.begin(), threads_.end(), [=]( adportable::asio::thread& t ){ return t.get_id() == id; });
    if ( it != threads_.end() ) {
		it->join();
        threads_.erase( it );
	}
}

void
DataprocessWorker::handleCreateChromatogramsV2( Dataprocessor * processor
                                            , const adcontrols::MSChromatogramMethod& cm
                                            , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                            , std::shared_ptr<adwidgets::Progress> progress )
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;

    if ( auto dset = processor->getLCMSDataset() ) {
        adcontrols::v2::MSChromatogramExtractor extract( dset );

        extract( vec, *pm, [progress] ( size_t curr, size_t total ) { if ( curr == 0 ) progress->setRange( 0, int( total ) ); return ( *progress )( int( curr ) ); } );

    }

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( *c, *pm );

	SessionManager::instance()->folderChanged( processor, folium.getParentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );

}

// data format v3 (read chrmatograms from an fcn)
void
DataprocessWorker::handleCreateChromatogramsV3( Dataprocessor * processor
                                              , const adcontrols::MSChromatogramMethod& cm
                                              , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                              , std::shared_ptr< const adcontrols::DataReader > reader
                                              , int fcn                                        
                                              , std::shared_ptr<adwidgets::Progress> progress )
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;

    if ( auto dset = processor->getLCMSDataset() ) {
        adcontrols::v3::MSChromatogramExtractor extract( dset );

        extract( vec, *pm, reader, fcn
                 , [progress] ( size_t curr, size_t total ) { if ( curr == 0 ) progress->setRange( 0, int( total ) ); return ( *progress )( int( curr ) ); } );

    }

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( *c, *pm );

	SessionManager::instance()->folderChanged( processor, folium.getParentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );

}


void
DataprocessWorker::handleCreateChromatogramsV2( Dataprocessor* processor
                                              , const std::shared_ptr< adcontrols::ProcessMethod > method
                                              , adcontrols::hor_axis axis
                                              , const std::vector< std::tuple< int, double, double > >& ranges
											  , std::shared_ptr<adwidgets::Progress> progress )
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;

    if ( const adcontrols::LCMSDataset * dset = processor->getLCMSDataset() ) {

        adcontrols::v2::MSChromatogramExtractor extract( dset );
        extract( vec, axis, ranges, [progress] ( size_t curr, size_t total ) { if ( curr == 0 ) progress->setRange( 0, int( total ) ); return ( *progress )( int( curr ) ); } );

    }

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( *c, *method );
	SessionManager::instance()->folderChanged( processor, folium.getParentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

void
DataprocessWorker::handleCreateSpectrogram( Dataprocessor* processor
                                            , const std::shared_ptr< adcontrols::ProcessMethod > pm
                                            , std::shared_ptr<adwidgets::Progress> progress )
{
    if ( const adcontrols::LCMSDataset * dset = processor->getLCMSDataset() ) {

        bool hasCentroid = dset->hasProcessedSpectrum( 0, 0 );
        const adcontrols::CentroidMethod * centroidMethod = pm->find< adcontrols::CentroidMethod >();
        
        auto spectra = std::make_shared< adcontrols::MassSpectra >();
        auto objId = dset->findObjId( L"MS.CENTROID" );
        
        adcontrols::Chromatogram tic;
        if ( dset->getTIC( 0, tic ) ) {
            
            spectra->setChromatogram( tic );
            
            for ( int pos = 0; pos < int( tic.size() ); ++pos ) {
                if ( auto ptr = std::make_shared< adcontrols::MassSpectrum >() ) {
                    if ( pos == 0 )
                        progress->setRange( 0, static_cast<int>(tic.size()) );
                    (*progress)( pos );
                    
                    if ( hasCentroid ) {
                        if ( dset->getSpectrum( 0, pos, *ptr, objId ) ) {
                            ADTRACE() << "Creating spectrogram from centroid: " << pos << "/" << tic.size();
                            (*spectra) << std::move( ptr );
                        }
                    } else {
                        adcontrols::MassSpectrum profile;
                        if ( dset->getSpectrum( 0, pos, profile, 0 ) ) {
                            adcontrols::MSPeakInfo result;
                            DataprocHandler::doCentroid( result, *ptr, profile, *centroidMethod );
                            (*spectra) << std::move( ptr );
                            ADTRACE() << "Creating spectrogram from profile: " << pos << "/" << tic.size()
                                      << " found peaks: " << result.size();
                        }
                    }
                }
            }
        }
        portfolio::Folium folium = processor->addSpectrogram( spectra );
        SessionManager::instance()->folderChanged( processor, folium.getParentFolder().name() );
    }
    
    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
    
}

void
DataprocessWorker::handleCreateSpectrogram( Dataprocessor* processor
                                            , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                            , std::shared_ptr< const adcontrols::DataReader > reader
                                            , int fcn
                                            , std::shared_ptr<adwidgets::Progress> progress )
{
    const adcontrols::CentroidMethod * centroidMethod = pm->find< adcontrols::CentroidMethod >();
        
    auto spectra = std::make_shared< adcontrols::MassSpectra >();

    // todo: handle fcn
    if ( auto tic = reader->TIC( fcn ) ) {

        spectra->setChromatogram( *tic );

        progress->setRange( 0, static_cast<int>( tic->size() ) );

        int pos( 0 );

        for ( auto it = reader->begin( fcn ); it != reader->end(); ++it ) {

            auto ms = reader->getSpectrum( it->rowid() );
            (*progress)( pos++ );

            if ( !ms->isCentroid() ) {
                adcontrols::MSPeakInfo result;
                auto ptr = std::make_shared< adcontrols::MassSpectrum >();
                DataprocHandler::doCentroid( result, *ptr, *ms, *centroidMethod );
                //ADDEBUG() << pos << "/" << tic->size() << " : " << it->rowid() << " fcn:" << it->fcn() << ", " << double(it->elapsed_time() * 1.0e-9 / 60.0) << "min , " << ptr->size();
                ( *spectra ) << std::move( ptr );

            } else {
                ( *spectra ) << std::move( ms );
            }

        }
        spectra->addDescription( adcontrols::description( L"Create", (boost::wformat( L"fcn(%d)" ) % fcn ).str() ) );
        portfolio::Folium folium = processor->addSpectrogram( spectra );
        SessionManager::instance()->folderChanged( processor, folium.getParentFolder().name() );
    }
    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

void
DataprocessWorker::handleFindPeptide( Dataprocessor* processor
                                      , const std::shared_ptr< adcontrols::ProcessMethod > pm
                                      , std::shared_ptr<adwidgets::Progress> progress )
{
    adcontrols::TargetingMethod tm;
    auto it = std::find_if( pm->begin(), pm->end(), [](const adcontrols::ProcessMethod::value_type& t){
            using adcontrols::TargetingMethod;
            return t.type() == typeid(TargetingMethod) && boost::get<TargetingMethod>(t).targetId() == TargetingMethod::idTargetPeptide;});
    if ( it != pm->end() )
        tm = boost::get<adcontrols::TargetingMethod>( *it );
    
    adcontrols::MassSpectraPtr ptr;

    if ( portfolio::Folder folder = processor->portfolio().findFolder( L"Spectrograms" ) ) {
		if ( auto folium = folder.findFoliumByName( L"Spectrogram" ) )
            portfolio::Folium::get< adcontrols::MassSpectraPtr >( ptr, folium );
    }

    if ( ptr ) {
        // make a list of possible ions

        // find peaks for each item on the list

        // find a cluster for a found peak
    }

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

void
DataprocessWorker::handleClusterSpectrogram( Dataprocessor* processor
                                             , const std::shared_ptr< adcontrols::ProcessMethod > pm
                                             , std::shared_ptr<adwidgets::Progress> progress )
{

    adcontrols::TargetingMethod tm;

    auto it = std::find_if( pm->begin(), pm->end(), [](const adcontrols::ProcessMethod::value_type& t){
            using adcontrols::TargetingMethod;
            return t.type() == typeid(TargetingMethod) && boost::get<TargetingMethod>(t).targetId() == TargetingMethod::idTargetPeptide;});
    if ( it != pm->end() )
        tm = boost::get<adcontrols::TargetingMethod>( *it );
    
    adcontrols::MassSpectraPtr ptr;

    if ( portfolio::Folder folder = processor->portfolio().findFolder( L"Spectrograms" ) ) {
        if ( auto folium = folder.findFoliumByName( L"Spectrogram" ) )
            portfolio::Folium::get< adcontrols::MassSpectraPtr >( ptr, folium );
    }

	std::shared_ptr< adcontrols::SpectrogramClusters > clusters( std::make_shared< adcontrols::SpectrogramClusters >() );
    std::chrono::steady_clock::time_point start;
    if ( ptr ) {
        adcontrols::Spectrogram::ClusterMethod m;
        adcontrols::Spectrogram::ClusterFinder finder( m, [=](int curr, int total)->bool{
                if ( curr == 0 )
                    progress->setRange( 0, total );
                (*progress)( curr );
                return true;
            });
        finder( *ptr, *clusters );
        processor->addSpectrogramClusters( clusters );
        // 
        start = std::chrono::steady_clock::now();
        // heap free checking on microsoft takes long time (more than 30 min for deleting 4000k objects)
    }

    ADTRACE() << "destractor spent: " 
              << double( std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - start ).count() / 1000.0 );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}
