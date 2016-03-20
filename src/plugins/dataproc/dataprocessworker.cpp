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
#include <adcontrols/mschromatogramextractor.hpp>
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
#include <adutils/acquiredconf.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adwidgets/datareaderchoicedialog.hpp>
#include <adwidgets/mslockdialog.hpp>
#include <coreplugin/icore.h>
#include <QMessageBox>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <functional>
#include <chrono>
#include <iomanip>
#include <fstream>

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


void
DataprocessWorker::createChromatograms( Dataprocessor* processor
                                        , adcontrols::hor_axis axis
                                        , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges
                                        , const boost::uuids::uuid& dataReaderUuid )
{
    if ( auto rawfile = processor->getLCMSDataset() ) {
        if ( rawfile->dataformat_version() >= 3 ) {
            if ( auto reader = rawfile->dataReader( dataReaderUuid ) )
                createChromatogramsV3( processor, axis, ranges, reader );
        } else {
            createChromatogramsV2( processor, axis, ranges );
        }
    }
}


void
DataprocessWorker::createChromatogramsV3( Dataprocessor* processor
                                          , adcontrols::hor_axis axis
                                          , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges
                                          , const adcontrols::DataReader * reader )
{
    for ( auto& range: ranges ) {
        
        int fcn = range.first;
        double time = range.second.time();
        double width = range.second.widthHH( true );
        
        // mass|time,width pair
        if ( auto pChr = reader->getChromatogram( fcn, time, width ) ) {
            
            portfolio::Portfolio portfolio = processor->getPortfolio();
            portfolio::Folder folder = portfolio.findFolder( L"Chromatograms" );
            
            std::wostringstream o;
            if ( axis == adcontrols::hor_axis_time ) {
                o << boost::wformat( L"%s %.3lf(us)(w=%.2lf(ns))" )
                    % adportable::utf::to_wstring( reader->display_name() ) % ( time * 1.0e6 ) % ( width * 1.0e9 );
            } else {
                o << boost::wformat( L"%s %.3lf(w=%.2lf(mDa))" )
                    % adportable::utf::to_wstring( reader->display_name() ) % ( range.second.mass() ) % ( range.second.widthHH( false ) * 1000 );
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
DataprocessWorker::createChromatograms( Dataprocessor* processor,  std::shared_ptr< const adcontrols::ProcessMethod > pm, const QString& origin )
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
                    threads_.push_back( adportable::asio::thread( [=] { handleCreateSpectrogram( processor, pm, reader.get(), fcn, p ); } ) );
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
    
    adwidgets::MSLockDialog dlg;
    dlg.setContents( lockm );
    
    if ( dlg.exec() == QDialog::Accepted ) {

        if ( dlg.getContents( lockm ) && !lockm.molecules().empty() ) {
            
            auto p( adwidgets::ProgressWnd::instance()->addbar() );
            threads_.push_back( adportable::asio::thread( [=] { handleExportMatchedMasses( processor, spectra, lockm, p ); } ) );
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

void
DataprocessWorker::handleCreateChromatogramsV2( Dataprocessor* processor
                                              , const std::shared_ptr< adcontrols::ProcessMethod > method
                                              , adcontrols::hor_axis axis
                                              , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges
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
    if ( auto rawfile = processor->getLCMSDataset() ) {
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

    for ( auto ms: *spectra ) {

        if ( ms->isCentroid() ) {

            adcontrols::lockmass::mslock lkms;
            for ( const auto& mol: mols.data() ) {
                auto idx = finder( *ms, mol.mass() );
                if ( idx != adcontrols::MSFinder::npos ) {
                    lkms << adcontrols::lockmass::reference( mol.formula(), mol.mass(), ms->getMass( idx ), ms->getTime( idx ) );
                    ms->addAnnotation( adcontrols::annotation( mol.formula(), mol.mass(), ms->getIntensity( idx ), int(idx), 999, adcontrols::annotation::dataFormula ) );
                }
            }
            if ( lkms.fit() ) {
                lkms( *ms );
                db && adutils::AcquiredConf::insert( *db, objid, fcn, ms->rowid(), lkms );
            }
        }
        (*progress)( pos++ );
    }
    
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

    boost::filesystem::path base = boost::filesystem::path( processor->filename() ).parent_path() / boost::filesystem::path( processor->filename() ).stem();

    int nprog( 0 );
    for ( auto& mol : lockm.molecules().data() ) {

        double t0 = (*spectra->begin())->getMSProperty().timeSinceInjection();
        std::string name = ( boost::format( "%s_%s_%d.txt" ) % base.string() % mol.formula() % (*spectra->begin())->mode() ).str();
        std::ofstream outf( name );
        
        // auto drift = std::make_shared< adcontrols::MassSpectrum >();
        // drift->resize( spectra->size() );
        
        int nprog( 0 );
        std::vector< double > masses, times;
        for ( auto& ms : *spectra ) {
            (*progress)( nprog++ );
            if ( auto idx = finder( *ms, mol.mass() ) ) {
                times.push_back( ms->getMSProperty().timeSinceInjection() - t0 );
                masses.push_back( ms->getMass( idx ) );
            }
        }
        // auto mean = std::accumulate( masses.begin(), masses.end(), double(0) ) / masses.size();
        // auto sdsum = std::accumulate( masses.begin(), masses.end(), double(0), [&]( double a, double b ){ return a + ( b - mean ) * ( b - mean ); } );
        // double sd = std::sqrt( sdsum / ( masses.size() - 1 ) );
        // std::transform( masses.begin(), masses.end(), masses.begin(), [&]( double d ){ return std::abs( d - mean ) > 0.010 ? mean : d ; } );

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
                //ADDEBUG() << pos << "/" << tic->size() << " : " << it->rowid() << " fcn:" << it->fcn() << ", " << double(it->elapsed_time() * 1.0e-9 / 60.0) << "min , " << ptr->size();
                ( *spectra ) << std::move( ptr );

            } else {
                ( *spectra ) << std::move( ms );
            }

        }
        using adportable::utf;
        spectra->addDescription( adcontrols::description( L"Create", ( boost::wformat( L"%s,fcn(%d)" ) % utf::to_wstring( reader->display_name() ) % fcn ).str() ) );
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

