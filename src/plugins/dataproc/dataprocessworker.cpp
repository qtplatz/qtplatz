/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "document.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/genchromatogram.hpp>
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
#include <adcontrols/peaks.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/spectrogram.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
//#include <adportable/semaphore.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#include <adprocessor/mschromatogramextractor.hpp>
#include <adprocessor/jcb2009_processor.hpp>
#include <adutils/acquiredconf.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adwidgets/datareaderchoicedialog.hpp>
#include <adwidgets/mslockdialog.hpp>
#include <adprocessor/noise_filter.hpp>
#include <coreplugin/icore.h>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <thread>

Q_DECLARE_METATYPE( portfolio::Folium )

using namespace dataproc;


DataprocessWorker::DataprocessWorker() : work_( io_service_ )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( threads_.empty() )
        threads_.emplace_back( adportable::asio::thread( [this] { io_service_.run(); } ) );
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
DataprocessWorker::createChromatogramsByPeakInfo3( Dataprocessor* processor
                                                   , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                                   , adcontrols::hor_axis axis
                                                   , std::shared_ptr< const adcontrols::MSPeakInfo > pkinfo
                                                   , const adcontrols::DataReader * reader )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    threads_.emplace_back( adportable::asio::thread( [=,this] {
        handleChromatogramsByPeakInfo3( processor, pm, pkinfo, reader->shared_from_this(), p );
    }));
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
                threads_.emplace_back( adportable::asio::thread( [=,this] {
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

void
DataprocessWorker::genChromatograms( Dataprocessor * processor
                                     , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                     , const QByteArray& json )
{
    std::vector< std::shared_ptr< adwidgets::Progress > > progresses;

    if ( auto rawfile = processor->rawdata() ) {

        if ( rawfile->dataformat_version() < 3 )
            return;

        // ADDEBUG() << "################ " << json.toStdString();

        adwidgets::DataReaderChoiceDialog dlg( rawfile->dataReaders() );
        dlg.setProtocolHidden( true );
        if ( auto tm = pm->find< adcontrols::MSChromatogramMethod >() ) {
            dlg.setMassWidth( tm->width( tm->widthMethod() ) );
            dlg.setTimeWidth( 4e-9 ); // 4ns
        }

        if ( dlg.exec() == QDialog::Accepted ) {
            auto reader_params = dlg.toJson();
            for ( auto& sel: dlg.selection() ) {
                auto progress( adwidgets::ProgressWnd::instance()->addbar() );
                progresses.emplace_back( progress );

                auto rdpara = QJsonDocument::fromJson( reader_params.at( sel.first ) ).object();
                auto enableTime = rdpara[ "enableTime" ].toBool();
                double massWidth = rdpara[ "massWidth" ].toDouble();
                double timeWidth = rdpara[ "timeWidth" ].toDouble();

                if ( auto reader = rawfile->dataReaders().at( sel.first ) ) {
                    double width = enableTime ? timeWidth : massWidth;
                    threads_.emplace_back(
                        adportable::asio::thread(
                            [=,this]{
                                handleGenChromatogram( processor, pm, reader, json.toStdString(), width, enableTime, progress );
                            })
                        );
                }
            }
        }

    }
}


// [0]
void
DataprocessWorker::createChromatogramsByMethod( Dataprocessor* processor
                                                , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                                , const QString& origin )
{
    // ADDEBUG() << "----------------------- " << __FUNCTION__ << " ------------------------";
    // if ( auto cm = pm->find< adcontrols::MSChromatogramMethod >() ) {
    //     for ( const auto& value: cm->molecules().data() ) {
    //         ADDEBUG() << boost::json::object{ { "formula", value.formula() }
    //                 , { "enable", value.enable() }, {"adducts", value.adducts() }, { "mass", value.mass() } };
    //     }
    // }
    // ADDEBUG() << "----------------------- " << __FUNCTION__ << " ------------------------";

    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    if ( auto rawfile = processor->rawdata() ) {
        if ( auto tm = pm->find< adcontrols::MSChromatogramMethod >() ) {
            if ( rawfile->dataformat_version() >= 3 ) {
                // v3
                // auto datasource = tm->dataSource(); // MSChromatogramMethod::Profile | Centroid
                auto readers = rawfile->dataReaders();
                auto it = std::find_if( readers.begin(), readers.end(), [&](const auto& r){ return r->objtext() == tm->dataReader(); } );
                if ( tm->dataReader().empty() || it == readers.end() ) {
                    adwidgets::DataReaderChoiceDialog dlg( rawfile->dataReaders() );
                    dlg.setProtocolHidden( true );
                    dlg.setMassWidth( tm->width( adcontrols::MSChromatogramMethod::widthInDa ) );
                    dlg.setTimeWidth( tm->width( adcontrols::MSChromatogramMethod::widthTime ) );

                    if ( dlg.exec() == QDialog::Accepted ) {
                        it = readers.begin() + dlg.currentSelection();

                        adcontrols::ProcessMethod tmp( *pm );
                        adcontrols::MSChromatogramMethod m( *tm );
                        m.setDataReader( (*it)->objtext() );
                        tmp *= m;
                        document::instance()->setProcessMethod( tmp );

                        if ( auto reader = rawfile->dataReaders().at( dlg.currentSelection() ) )
                            threads_.emplace_back( adportable::asio::thread( [=,this] {
                                handleChromatogramsByMethod3( processor, *tm, pm, reader, p ); } ) );
                    }
                } else {
                    auto readers = rawfile->dataReaders();
                    auto it = std::find_if( readers.begin(), readers.end(), [&](const auto& r){ return r->objtext() == tm->dataReader(); } );
                    if ( it != readers.end() ) {
                        auto reader = (*it);
                        threads_.emplace_back( adportable::asio::thread( [=,this] {
                            handleChromatogramsByMethod3( processor, *tm, pm, reader, p ); } ) );
                    }
                }

            } else {
                // v2
                threads_.emplace_back( adportable::asio::thread( [=,this] { handleCreateChromatogramsV2( processor, *tm, pm, p ); } ) );
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
        threads_.push_back( adportable::asio::thread( [&] { io_service_.run(); } ) );

	adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
	MainWindow::instance()->getProcessMethod( *pm );

    threads_.push_back( adportable::asio::thread( [=,this] { handleCreateChromatogramsV2( processor, pm, axis, ranges, p ); } ) );
}

void
DataprocessWorker::createContour( Dataprocessor* processor )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( threads_.empty() )
            threads_.push_back( adportable::asio::thread( [=,this] { io_service_.run(); } ) );
    } while ( 0 );

    adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
    MainWindow::instance()->getProcessMethod( *pm );

    if ( auto rawfile = processor->rawdata() ) {
        if ( rawfile->dataformat_version() >= 3 ) {
            adwidgets::DataReaderChoiceDialog dlg( rawfile->dataReaders() );
            dlg.setFormHidden( true );
            if ( dlg.exec() == QDialog::Accepted ) {
                int fcn = dlg.fcn();
                if ( auto reader = rawfile->dataReaders().at( dlg.currentSelection() ) )
                    threads_.push_back( adportable::asio::thread( [=,this] {
                        handleCreateSpectrogram3( processor, pm, reader.get(), fcn, p ); } ) );
            }
        } else {
            threads_.push_back( adportable::asio::thread( [=,this] { handleCreateSpectrogram( processor, pm, p ); } ) );
        }
    }
}

void
DataprocessWorker::clusterContour( Dataprocessor * processor )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    std::lock_guard< std::mutex > lock( mutex_ );
	if ( threads_.empty() )
		threads_.push_back( adportable::asio::thread( [=,this] { io_service_.run(); } ) );

	adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
	MainWindow::instance()->getProcessMethod( *pm );

    threads_.push_back( adportable::asio::thread( [=,this] { handleClusterSpectrogram( processor, pm, p ); } ) );
}

void
DataprocessWorker::findPeptide( Dataprocessor * processor, const adprot::digestedPeptides& /*peptides*/ )
{
    auto p( adwidgets::ProgressWnd::instance()->addbar() );

    std::lock_guard< std::mutex > lock( mutex_ );
	if ( threads_.empty() )
		threads_.push_back( adportable::asio::thread( [=,this] { io_service_.run(); } ) );

	adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
	MainWindow::instance()->getProcessMethod( *pm );

    threads_.push_back( adportable::asio::thread( [=,this] { handleFindPeptide( processor, pm, p ); } ) );
}

void
DataprocessWorker::mslock( Dataprocessor * processor, std::shared_ptr< adcontrols::MassSpectra > spectra, const adcontrols::MSLockMethod& lockm )
{
    if ( spectra->size() == 0 )
        return;

    std::lock_guard< std::mutex > lock( mutex_ );
	if ( threads_.empty() )
		threads_.push_back( adportable::asio::thread( [=,this] { io_service_.run(); } ) );

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
DataprocessWorker::doIt( std::shared_ptr< adprocessor::JCB2009_Processor > proc
                         , std::shared_ptr< const adcontrols::DataReader > reader )
{
    auto progress( adwidgets::ProgressWnd::instance()->addbar() );
    auto future = std::async( std::launch::async, [&](){
        (*proc)( reader, [progress](size_t curr, size_t total){ return (*progress)( curr, total ); } );
    } );

    while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
        QCoreApplication::instance()->processEvents();
}

void
DataprocessWorker::join( adportable::asio::thread::id id )
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
                progress->setRange( 0, int( total ) );
            return ( *progress )( int( curr ) );
        } );
    }

    portfolio::Folium folium;
    for ( auto c: vec ) {
        folium = processor->addChromatogram( c, *pm, nullptr );
    }

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
                    progress->setRange( 0, int( total ) );
                return ( *progress )( int( curr ) );
            } );
    }

    portfolio::Folium folium;
    for ( auto c: vec ) {
        folium = processor->addChromatogram( c, *method, nullptr );
    }
	SessionManager::instance()->folderChanged( processor, folium.parentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

// data format v3 (read chrmatograms from a protocol)
void
DataprocessWorker::handleChromatogramsByMethod3( Dataprocessor * processor
                                                 , const adcontrols::MSChromatogramMethod& cm
                                                 , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                                 , std::shared_ptr< const adcontrols::DataReader > reader
                                                 , std::shared_ptr<adwidgets::Progress> progress )
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;

    if ( cm.enableAutoTargeting() ) {
        adcontrols::ProcessMethod tmp;
        adcontrols::TargetingMethod tgtm;
        if ( auto cm = pm->find< adcontrols::CentroidMethod >() )
            tmp.appendMethod( *cm );
        if ( auto tm = pm->find< adcontrols::TargetingMethod >() )
            tgtm = *tm;

        // ADDEBUG() << boost::json::object{ { "auto_targeting", true }, { "tolerance", tgtm.tolerance( tgtm.toleranceMethod() ) } };
        std::vector< adcontrols::GenChromatogram > genChromatograms;

        for ( auto mol: cm.molecules().data() ) {
            if ( mol.tR() && mol.enable() ) {
                double tR = *mol.tR();
                adcontrols::moltable mtab;
                mtab << mol;
                tgtm.setMolecules( mtab, mol.adducts( mtab.polarity() ) );
                tmp *= tgtm; // add/replace target method.
                double pkw = cm.peakWidthForChromatogram();

                if ( auto ms = reader->coaddSpectrum( reader->findPos( tR - pkw/2.0 ), reader->findPos( tR + pkw/2.0 ) ) ) {
                    auto desc = ( boost::format( "%s %.2f(%.3fs)%s" ) % mol.formula() % mol.mass() % tR % reader->display_name() ).str();
                    ms->addDescription( adcontrols::description( { "create", desc } ) );
                    portfolio::Folium folium = processor->addSpectrum( ms, adcontrols::ProcessMethod() );
                    processor->applyProcess( folium, tmp, CentroidProcess ); // + targeting
                    bool found( false );
                    if ( auto fCentroid = portfolio::find_first_of( folium.attachments()
                                                                    , []( const auto& f ) {
                                                                        return f.name() == Constants::F_CENTROID_SPECTRUM; } ) ) {
                        if ( auto f = portfolio::find_first_of( fCentroid.attachments()
                                                                , []( const auto& a ) {
                                                                    return a.name() == Constants::F_TARGETING; } ) ) {
                            if ( auto targeting = portfolio::get< std::shared_ptr< adcontrols::Targeting > >( f ) ) {
                                found = true;

                                for ( const auto& c : targeting->candidates() ) {
                                    genChromatograms.emplace_back( adcontrols::GenChromatogram( c, true ) );
                                }
                            }
                        }
                    }
                    if ( !found ) {
                        ADDEBUG() << "###### no candidate found " << mol.formula()
                                  << ", " << mol.adducts() << ", " << mol.mass() << ", enable=" << mol.enable();
                    }
                }
            }
        }

        auto jv = boost::json::value_from( boost::json::object{{ "formulae", genChromatograms }} );
        ADDEBUG() << jv;
        auto json = boost::json::serialize( jv );

        double width = cm.width( cm.widthMethod() );
        if ( auto dset = processor->rawdata() ) {
            adprocessor::v3::MSChromatogramExtractor extract( dset, processor );
            extract.extract_by_json( vec, *pm, reader, json, width
                                     , adcontrols::hor_axis_mass, [progress]( size_t curr, size_t total ){
                                         return (*progress)( curr, total ); } );
        }
    } else { // !autoTargeting
        if ( auto dset = processor->rawdata() ) {
            adprocessor::v3::MSChromatogramExtractor extract( dset, processor );
            extract.extract_by_mols( vec, *pm, reader, [progress]( size_t curr, size_t total ){
                return (*progress)( curr, total ); } );
        }
    }

    auto noise_filter = std::make_shared< adprocessor::noise_filter >();
    portfolio::Folium folium;
    for ( auto c: vec ) {
        folium = processor->addChromatogram( c, *pm, noise_filter );
    }

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
        adprocessor::v3::MSChromatogramExtractor ex( dset, processor );
        ex.extract_by_axis_range( vec, *pm, reader, fcn, axis, range
                                  , [progress]( size_t curr, size_t total ){ return (*progress)( curr, total ); } );
    }

    auto noise_filter = std::make_shared< adprocessor::noise_filter >();
    portfolio::Folium folium;
    for ( auto c: vec ) {
        folium = processor->addChromatogram( c, *pm, noise_filter );
    }
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
        adprocessor::v3::MSChromatogramExtractor extract( dset, processor );

        ADDEBUG() << "## " << __FUNCTION__ << " ## reader: " << std::make_pair( reader->display_name(), reader->objtext() );

        extract.extract_by_peak_info( vec
                                      , *pm
                                      , pkinfo
                                      , reader
                                      , [progress]( size_t curr, size_t total ){ return (*progress)( curr, total ); } );
    }

    portfolio::Folium folium;
    auto noise_filter = std::make_shared< adprocessor::noise_filter >();
    for ( auto c: vec ) {
        folium = processor->addChromatogram( c, *pm, noise_filter );
        if ( auto pchr = folium.get< std::shared_ptr< adcontrols::Chromatogram > >() ) {
            if ( (*pchr)->peaks().size() > 0 ) {
                folium.setAttribute( L"isChecked", L"true" );
            }
        }
    }
	SessionManager::instance()->folderChanged( processor, folium.parentFolder().name() );

    io_service_.post( std::bind(&DataprocessWorker::join, this, adportable::this_thread::get_id() ) );
}

void
DataprocessWorker::handleGenChromatogram( Dataprocessor * processor
                                          , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                          , std::shared_ptr< const adcontrols::DataReader > reader
                                          , const std::string& peaks_json
                                          , double width
                                          , bool enableTime
                                          , std::shared_ptr<adwidgets::Progress> progress )
{
    std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec;

    if ( auto dset = processor->rawdata() ) {
        adprocessor::v3::MSChromatogramExtractor ex( dset, processor );

        auto axis = enableTime ? adcontrols::hor_axis_time : adcontrols::hor_axis_mass;
        ex.extract_by_json( vec, *pm, reader, peaks_json, width, axis, [progress]( size_t curr, size_t total ){ return (*progress)( curr, total ); } );
    }

    auto noise_filter = std::make_shared< adprocessor::noise_filter >();
    portfolio::Folium folium;
    for ( auto c: vec ) {
        folium = processor->addChromatogram( c, *pm, noise_filter );
    }
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
                    lkms << adcontrols::lockmass::reference( mol.formula(), mol.mass(), ms->mass( idx ), ms->time( idx ) );
                    ms->addAnnotation( adcontrols::annotation( mol.formula(), mol.mass(), ms->intensity( idx )
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

    auto noise_filter = std::make_shared< adprocessor::noise_filter >();
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
                    masses.push_back( ms->mass( idx ) );
                }
            }

            auto drift = std::make_shared< adcontrols::Chromatogram >();
            drift->resize( masses.size() );

            drift->setIntensityArray( masses.data(), masses.size() );
            drift->setTimeArray( times.data(), times.size() );

            for ( size_t i = 0; i < drift->size(); ++i )
            outf << std::fixed << std::setprecision( 14 ) << times[i] << "," << masses[i] << std::endl;

            drift->addDescription( adcontrols::description( L"create", adportable::utf::to_wstring( mol.formula() ) ) );
            for ( auto& desc: spectra->getDescriptions() )
                drift->addDescription( desc );

            auto folium = processor->addChromatogram( drift, m, noise_filter );
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
    if ( auto spectra = processor->createSpectrogram( pm
                                                      , reader
                                                      , fcn
                                                      , [progress](size_t curr, size_t total){
                                                            if ( curr == 0 )
                                                                progress->setRange( 0, int(total) );
                                                            (*progress)( curr );
                                                            return false;
                                                        } ) ) {
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
