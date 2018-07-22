/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/spectrogram.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#include <adprocessor/mschromatogramextractor.hpp>
#include <adutils/acquiredconf.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adwidgets/datareaderchoicedialog.hpp>
#include <adwidgets/mslockdialog.hpp>
#include <coreplugin/icore.h>
#include <QCoreApplication>
#include <QMessageBox>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <thread>

using namespace dataproc;

DataprocessWorker::DataprocessWorker() : work_( io_service_ )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( threads_.empty() )
        threads_.push_back( adportable::asio::thread( [=] { io_service_.run(); } ) );
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
// DataprocessWorker::createChromatograms( Dataprocessor* processor
//                                         , adcontrols::hor_axis axis
//                                         , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges
//                                         , const boost::uuids::uuid& dataReaderUuid )
// {
//     if ( auto rawfile = processor->rawdata() ) {
//         if ( rawfile->dataformat_version() <= 2 ) {
//             createChromatogramsV2( processor, axis, ranges );
//         }
//     }
// }


void
DataprocessWorker::createChromatogramsByPeakInfo3( Dataprocessor* processor
                                                   , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                                   , adcontrols::hor_axis axis
                                                   , std::shared_ptr< const adcontrols::MSPeakInfo > pkinfo
                                                   , const adcontrols::DataReader * reader )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    threads_.emplace_back( adportable::asio::thread( [=] {
                handleChromatogramsByPeakInfo3( processor, pm, pkinfo, reader->shared_from_this(), p );
            } ) );
}

// [3]
void
DataprocessWorker::createChromatogramByAxisRange3( Dataprocessor * processor
                                                   , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                                   , adcontrols::hor_axis axis
                                                   , const std::pair<double, double >& range
                                                   , const adcontrols::DataReader * reader )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    if ( auto rawfile = processor->rawdata() ) {
        if ( auto tm = pm->find< adcontrols::MSChromatogramMethod >() ) {
            if ( rawfile->dataformat_version() >= 3 ) {
                threads_.emplace_back( adportable::asio::thread( [=] {
                            handleChromatogramByAxisRange3( processor, pm, axis, range, reader->shared_from_this(), -1, p );
                        } ) );
            } else {
                QMessageBox::information( 0, "QtPlatz", "Create Chromatograms -- file format(v2) is not supported." );
                return;
                // threads_.push_back( adportable::asio::thread( [=] { handleCreateChromatogramsV2( processor, *tm, pm, p ); } ) );
            }
        }
    }
}

// [0]
void
DataprocessWorker::createChromatogramsByMethod( Dataprocessor* processor, std::shared_ptr< const adcontrols::ProcessMethod > pm, const QString& origin )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    if ( auto rawfile = processor->rawdata() ) {
        if ( auto tm = pm->find< adcontrols::MSChromatogramMethod >() ) {
            if ( rawfile->dataformat_version() >= 3 ) {
                adwidgets::DataReaderChoiceDialog dlg( rawfile->dataReaders() );
                dlg.setProtocolHidden( true );
                if ( dlg.exec() == QDialog::Accepted ) {
                    //int fcn = dlg.fcn();
                    if ( auto reader = rawfile->dataReaders().at( dlg.currentSelection() ) )
                        threads_.push_back( adportable::asio::thread( [=] { handleChromatogramsByMethod3( processor, *tm, pm, reader, p ); } ) );
                }

            } else {
                threads_.push_back( adportable::asio::thread( [=] { handleCreateChromatogramsV2( processor, *tm, pm, p ); } ) );
            }
        }
    }
}

void
DataprocessWorker::createChromatogramsV2( Dataprocessor * processor
                                          , adcontrols::hor_axis axis
                                          , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges )
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
DataprocessWorker::createContour( Dataprocessor* processor )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( threads_.empty() )
            threads_.push_back( adportable::asio::thread( [=] { io_service_.run(); } ) );
    } while ( 0 );

    adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
    MainWindow::instance()->getProcessMethod( *pm );
    
    if ( auto rawfile = processor->rawdata() ) {
        if ( rawfile->dataformat_version() >= 3 ) {
            adwidgets::DataReaderChoiceDialog dlg( rawfile->dataReaders() );
            if ( dlg.exec() == QDialog::Accepted ) {
                int fcn = dlg.fcn();
                if ( auto reader = rawfile->dataReaders().at( dlg.currentSelection() ) )
                    threads_.push_back( adportable::asio::thread( [=] { handleCreateSpectrogram3( processor, pm, reader.get(), fcn, p ); } ) );
            }
        } else {
            threads_.push_back( adportable::asio::thread( [=] { handleCreateSpectrogram( processor, pm, p ); } ) );
        }
    }
}

void
DataprocessWorker::clusterContour( Dataprocessor * processor )
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
DataprocessWorker::mslock( Dataprocessor * processor, std::shared_ptr< adcontrols::MassSpectra > spectra, const adcontrols::MSLockMethod& lockm )
{
    if ( spectra->size() == 0 )
        return;

    std::lock_guard< std::mutex > lock( mutex_ );
	if ( threads_.empty() )
		threads_.push_back( adportable::asio::thread( [=] { io_service_.run(); } ) );

    if ( spectra->mslocked() ) {
        int result = QMessageBox::question( MainWindow::instance()
                                            , QObject::tr("Already mass locked")
                                            , QObject::tr( "delete assigned masses?" )
                                            , QMessageBox::Yes, QMessageBox::No|QMessageBox::Default|QMessageBox::Escape );
        if ( result == QMessageBox::No ) 
            return;
    }

    auto p( adwidgets::ProgressWnd::instance()->addbar() );
    threads_.push_back( adportable::asio::thread( [=] { handleMSLock( processor, spectra, lockm, p ); } ) );
}

void
DataprocessWorker::exportMatchedMasses( Dataprocessor * processor
                                        , std::shared_ptr< const adcontrols::MassSpectra > spectra
                                        , const std::wstring& foliumId )
{
    if ( spectra->size() == 0 )
        return;

    adcontrols::MSLockMethod lockm;
    lockm.setEnabled( true );
    if ( auto pm = std::make_unique< adcontrols::ProcessMethod >() ) {
        MainWindow::instance()->getProcessMethod( *pm );
        if ( auto cm = pm->find< adcontrols::MSChromatogramMethod >() ) {
            lockm.setMolecules( cm->molecules() );
            lockm.setToleranceMethod( adcontrols::idToleranceDaltons );
            lockm.setTolerance( adcontrols::idToleranceDaltons, cm->tolerance() );
        }
        if ( auto tm = pm->find< adcontrols::TargetingMethod >() ) {
            lockm.setMolecules( tm->molecules() );
            lockm.setTolerance( tm->toleranceMethod(), tm->tolerance( tm->toleranceMethod() ) );
        }
    }

    adwidgets::MSLockDialog dlg;
    dlg.setContents( lockm );
    
    if ( dlg.exec() == QDialog::Accepted ) {

        if ( dlg.getContents( lockm ) && !lockm.molecules().empty() ) {

            auto progress( adwidgets::ProgressWnd::instance()->addbar() );
            auto future = std::async( std::launch::async, [&](){
                    handleExportMatchedMasses( processor, spectra, lockm, progress );
                } );

            while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
                QCoreApplication::instance()->processEvents();
        }
    }
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

    if ( auto dset = processor->rawdata() ) {
        adprocessor::v2::MSChromatogramExtractor extract( dset );

        extract( vec, *pm, [progress] ( size_t curr, size_t total ) {
                if ( curr == 0 )
                    progress->setRange( 0, int( total ) ); return ( *progress )( int( curr ) );
            } );
    }

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( *c, *pm );

	SessionManager::instance()->folderChanged( processor, folium.parentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );

}

void
DataprocessWorker::handleCreateChromatogramsV2( Dataprocessor* processor
                                              , const std::shared_ptr< adcontrols::ProcessMethod > method
                                              , adcontrols::hor_axis axis
                                              , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges
											  , std::shared_ptr<adwidgets::Progress> progress )
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;

    if ( const adcontrols::LCMSDataset * dset = processor->rawdata() ) {

        adprocessor::v2::MSChromatogramExtractor extract( dset );
        extract( vec, axis, ranges, [progress] ( size_t curr, size_t total ) {
                if ( curr == 0 )
                    progress->setRange( 0, int( total ) ); return ( *progress )( int( curr ) );
            } );
    }

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( *c, *method );
	SessionManager::instance()->folderChanged( processor, folium.parentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

// data format v3 (read chrmatograms from an fcn)
void
DataprocessWorker::handleChromatogramsByMethod3( Dataprocessor * processor
                                                 , const adcontrols::MSChromatogramMethod& cm
                                                 , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                                 , std::shared_ptr< const adcontrols::DataReader > reader
                                                 , std::shared_ptr<adwidgets::Progress> progress )
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;

    if ( auto dset = processor->rawdata() ) {
        adprocessor::v3::MSChromatogramExtractor extract( dset );

        extract.extract_by_mols( vec, *pm, reader, [progress]( size_t curr, size_t total ){ return (*progress)( curr, total ); } );

    }

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( *c, *pm );

	SessionManager::instance()->folderChanged( processor, folium.parentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );

}

void
DataprocessWorker::handleChromatogramByAxisRange3( Dataprocessor * processor
                                                  , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                                  , adcontrols::hor_axis axis
                                                  , const std::pair<double, double >& range
                                                  , std::shared_ptr< const adcontrols::DataReader > reader
                                                  , int fcn
                                                  , std::shared_ptr<adwidgets::Progress> progress )
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;
    
    if ( auto dset = processor->rawdata() ) {
        adprocessor::v3::MSChromatogramExtractor ex( dset );
        ex.extract_by_axis_range( vec, *pm, reader, fcn, axis, range
                                  , [progress]( size_t curr, size_t total ){ return (*progress)( curr, total ); } );
    }

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( *c, *pm );

	SessionManager::instance()->folderChanged( processor, folium.parentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

void
DataprocessWorker::handleChromatogramsByPeakInfo3( Dataprocessor * processor
                                                   , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                                   , std::shared_ptr< const adcontrols::MSPeakInfo > pkinfo
                                                   , std::shared_ptr< const adcontrols::DataReader > reader
                                                   , std::shared_ptr<adwidgets::Progress> progress )
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;

    if ( auto dset = processor->rawdata() ) {
        adprocessor::v3::MSChromatogramExtractor extract( dset );

        extract.extract_by_peak_info( vec
                                      , *pm
                                      , pkinfo
                                      , reader
                                      , [progress]( size_t curr, size_t total ){ return (*progress)( curr, total ); } );
    }

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( *c, *pm );

	SessionManager::instance()->folderChanged( processor, folium.parentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

void
DataprocessWorker::handleMSLock( Dataprocessor * processor
                                 , std::shared_ptr< adcontrols::MassSpectra > spectra
                                 , const adcontrols::MSLockMethod& lockm
                                 , std::shared_ptr<adwidgets::Progress> progress )                                 
{
    if ( spectra->size() == 0 )
        return;

    progress->setRange( 0, static_cast<int>( spectra->size() ) );

    adcontrols::MSFinder finder( lockm.tolerance( lockm.toleranceMethod() ), lockm.algorithm(), lockm.toleranceMethod() );
    const auto& mols = lockm.molecules();

    int fcn = ( *spectra->begin() )->protocolId();
    const auto& objid = ( *spectra->begin() )->dataReaderUuid();

    adfs::sqlite * db(0);
    if ( auto rawfile = processor->rawdata() ) {
        if ( rawfile->dataformat_version() >= 3 ) {
            if ( ( db = rawfile->db() ) ) {
                adutils::AcquiredConf::create_mslock( *db );
                adutils::AcquiredConf::delete_mslock( *db, objid, fcn );
            }
        }
        if ( spectra->mslocked() ) {
            for ( auto& ms : *spectra ) {
                auto reader = rawfile->dataReader( ms->dataReaderUuid() );
                reader->massSpectrometer()->assignMasses( *ms );
            }
        }
    }

    int pos(0);
    spectra->setMSLocked( true );

    adfs::stmt sql( *db );
    sql.begin();

    for ( auto ms: *spectra ) {

        if ( ms->isCentroid() ) {

            adcontrols::lockmass::mslock lkms;
            for ( const auto& mol: mols.data() ) {
                auto idx = finder( *ms, mol.mass() );
                if ( idx != adcontrols::MSFinder::npos ) {
                    lkms << adcontrols::lockmass::reference( mol.formula(), mol.mass(), ms->getMass( idx ), ms->getTime( idx ) );
                    ms->addAnnotation( adcontrols::annotation( mol.formula(), mol.mass(), ms->getIntensity( idx )
                                                               , int(idx), 999, adcontrols::annotation::dataFormula ) );
                }
            }
            if ( lkms.fit() ) {
                lkms( *ms );
                db && adutils::AcquiredConf::insert( sql, objid, fcn, ms->rowid(), lkms );
            }
        }
        (*progress)( pos++ );
    }

    sql.commit();
    
    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

void
DataprocessWorker::handleExportMatchedMasses( Dataprocessor * processor
                                              , std::shared_ptr< const adcontrols::MassSpectra > spectra
                                              , const adcontrols::MSLockMethod& lockm
                                              , std::shared_ptr<adwidgets::Progress> progress )                                 
{
    auto& mols = lockm.molecules();
    progress->setRange( 0, static_cast<int>( spectra->size() * mols.data().size() ) );

    adcontrols::ProcessMethod m;
    m << lockm;
    
    adcontrols::MSFinder finder( lockm.tolerance( lockm.toleranceMethod() ), lockm.algorithm(), lockm.toleranceMethod() );
    

    boost::filesystem::path base =
        boost::filesystem::path( processor->filename() ).parent_path() / boost::filesystem::path( processor->filename() ).stem();

    for ( auto& mol : lockm.molecules().data() ) {

        if ( mol.enable() ) {

            double t0 = (*spectra->begin())->getMSProperty().timeSinceInjection();
            std::string name = ( boost::format( "%s_%s_%d.txt" ) % base.string() % mol.formula() % (*spectra->begin())->mode() ).str();
            std::ofstream outf( name );
            
            int nprog( 0 );
            std::vector< double > masses, times;
            for ( auto& ms : *spectra ) {
                (*progress)( nprog++ );
                size_t idx = finder( *ms, mol.mass() );
                if ( idx != adcontrols::MSFinder::npos ) {
                    times.push_back( ms->getMSProperty().timeSinceInjection() - t0 );
                    masses.push_back( ms->getMass( idx ) );
                }
            }

            auto drift = std::make_shared< adcontrols::Chromatogram >();
            drift->resize( masses.size() );
            
            drift->setIntensityArray( masses.data() );
            drift->setTimeArray( times.data() );
            
            for ( size_t i = 0; i < drift->size(); ++i )
            outf << std::fixed << std::setprecision( 14 ) << times[i] << "," << masses[i] << std::endl;
            
            drift->addDescription( adcontrols::description( L"create", adportable::utf::to_wstring( mol.formula() ) ) );
            for ( auto& desc: spectra->getDescriptions() )
                drift->addDescription( desc );
            
            auto folium = processor->addChromatogram( *drift, m );
        }
    }
}


void
DataprocessWorker::handleCreateSpectrogram( Dataprocessor* processor
                                            , const std::shared_ptr< adcontrols::ProcessMethod > pm
                                            , std::shared_ptr<adwidgets::Progress> progress )
{
    if ( const adcontrols::LCMSDataset * dset = processor->rawdata() ) {

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
        portfolio::Folium folium = processor->addContour( spectra );
        SessionManager::instance()->folderChanged( processor, folium.parentFolder().name() );
    }
    
    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
    
}

void
DataprocessWorker::handleCreateSpectrogram3( Dataprocessor* processor
                                             , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                             , const adcontrols::DataReader * reader
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
                ( *spectra ) << std::move( ptr );

            } else {
                ( *spectra ) << std::move( ms );
            }

        }
        using adportable::utf;
        spectra->addDescription( adcontrols::description( L"Create", ( boost::wformat( L"%s,fcn(%d)" ) % utf::to_wstring( reader->display_name() ) % fcn ).str() ) );
        portfolio::Folium folium = processor->addContour( spectra );
        SessionManager::instance()->folderChanged( processor, folium.parentFolder().name() );
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
        processor->addContourClusters( clusters );
        // 
        start = std::chrono::steady_clock::now();
        // heap free checking on microsoft takes long time (more than 30 min for deleting 4000k objects)
    }

    ADTRACE() << "destractor spent: " 
              << double( std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - start ).count() / 1000.0 );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

