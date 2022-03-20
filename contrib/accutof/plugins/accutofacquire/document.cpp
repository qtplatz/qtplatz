/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "constants.hpp"
#include "document.hpp"
#include "iu5303afacade.hpp"
#include "mainwindow.hpp"
#include "pkdavgwriter.hpp"
#include "resultwriter.hpp"
#include "task.hpp"

#include <accutofcontrols/constants.hpp>
#include <acqrscontrols/constants.hpp>
#include <acqrscontrols/pkd_counting_data_writer.hpp>
#include <acqrscontrols/u5303a/histogram.hpp>
#include <acqrscontrols/u5303a/metadata.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrscontrols/u5303a/tdcdoc.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <adacquire/masterobserver.hpp>
#include <adacquire/sampleprocessor.hpp>
#include <adacquire/samplesequence.hpp>
#include <adacquire/simpleobserver.hpp>
#include <adacquire/task.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adcontrols/timedigitalmethod.hpp>
#include <adcontrols/trace.hpp>
#include <adextension/icontrollerimpl.hpp>
#include <adextension/isequenceimpl.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/countrate_calculator.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
#include <adurl/ajax.hpp>
#include <adurl/blob.hpp>
#include <adurl/sse.hpp>
#include <adutils/fsio.hpp>
#include <adutils/mscalibio.hpp>
#include <app/app_version.h>
#include <coreplugin/documentmanager.h>
#include <date/date.h>
#include <extensionsystem/pluginmanager.h>
#include <qtwrapper/settings.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/at.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMetaType>
#include <QSettings>
#include <QStandardPaths>
#include <chrono>
#include <future>
#include <fstream>
#include <set>
#include <string>

Q_DECLARE_METATYPE( boost::uuids::uuid );

using namespace accutof::acquire;

namespace accutof { namespace acquire {

        struct user_preference {
            static boost::filesystem::path path( QSettings * settings ) {
                boost::filesystem::path dir( settings->fileName().toStdWString() );
                return dir.remove_filename() / "accutof";
            }
        };

        template< typename T > struct xmlWriter {
            void operator()( const adcontrols::ControlMethod::MethodItem& mi, const boost::filesystem::path& dir ) const {
                T x;
                if ( mi.get<>( mi, x ) ) {
                    boost::filesystem::path fname( dir / mi.modelname() );
                    fname.replace_extension( ".cmth.xml" );
                    std::wofstream outf( fname.string() );
                    T::xml_archive( outf, x );
                }
            }
        };

        namespace so = adacquire::SignalObserver;

        struct ObserverData {
            const char * objtext;
            const char * dataInterpreterClsid;
            const so::Description desc;
        };

        static ObserverData observers [] = {
            // pkd+avg observer
            { acqrscontrols::u5303a::pkd_observer_name
              , acqrscontrols::u5303a::softavgr_datainterpreter
              , { acqrscontrols::u5303a::softavgr_observer_name
                    , so::eTRACE_SPECTRA
                    , so::eMassSpectrometer
                    , L"Time", L"Count", 3, 0
                }
            }
            , { acqrscontrols::u5303a::timecount_observer_name  // objtext; must contains unitid
              , acqrscontrols::u5303a::timecount_datainterpreter // dataInterpreter; doesn't need unitid
              , { acqrscontrols::u5303a::timecount_observer_name // desc.traceId := data name on adfs file
                  , so::eTRACE_SPECTRA
                  , so::eMassSpectrometer
                  , L"Time", L"Count", 3, 0
                }
            }
            , { acqrscontrols::u5303a::histogram_observer_name  // objtext; must contains unitid
                , acqrscontrols::u5303a::histogram_datainterpreter // dataInterpreter; doesn't need unitid
                , { acqrscontrols::u5303a::histogram_observer_name // desc.traceId := data name on adfs file
                    , so::eTRACE_SPECTRA
                    , so::eMassSpectrometer
                    , L"Time", L"Count", 3, 0
                }
            }
            , { acqrscontrols::u5303a::softavgr_observer_name
                , acqrscontrols::u5303a::softavgr_datainterpreter
                , { acqrscontrols::u5303a::softavgr_observer_name
                    , so::eTRACE_SPECTRA
                    , so::eMassSpectrometer
                    , L"Time", L"mV", 3, 0
                }
            }
        };


        struct exec_fsm_stop {
            void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
                task::instance()->sample_stopped();
                for ( auto& iController : iControllers ) {
                    if ( auto session = iController->getInstrumentSession() )
                        session->stop_run();
                }
            }
        };

        struct exec_fsm_start {
            void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
                for ( auto inst : iControllers ) {
                    if ( auto session = inst->getInstrumentSession() )
                        session->start_run();
                }

                document::instance()->prepare_for_run();
                task::instance()->sample_started(); // workaround::method start
                // increment sample number <-- increment should be done at inject event
                //if ( auto run = document::instance()->sampleRun() )
                //    ++( *run );
            }
        };

        struct exec_fsm_ready {
            void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
                // make immediate inject
                // adacquire::task::instance()->fsmInject();
            }
        };

        struct exec_fsm_inject {
            void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
                document::tdc()->clear_histogram();
                task::instance()->sample_injected();
                if ( auto run = document::instance()->sampleRun() ) {  // increment sample number
                    ++( *run );
                    emit document::instance()->sampleRunChanged();
                }
                for ( auto& iController : iControllers ) {
                    if ( auto session = iController->getInstrumentSession() )
                        session->event_out( adacquire::Instrument::instEventInjectOut ); // loopback to peripherals
                }
            }
        };

        struct exec_fsm_complete {
            void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
                adacquire::task::instance()->fsmStart();
                adacquire::task::instance()->fsmReady();
            }
        };

        //..........................................
        class document::impl {
        public:
            static document * instance_; // workaround
            static std::mutex mutex_;
            static const std::chrono::system_clock::time_point uptime_;
            static const uint64_t tp0_;
            static std::set<QString> blockedModules_;
            std::shared_ptr< acqrscontrols::u5303a::tdcdoc > tdcdoc_;
            std::chrono::time_point<std::chrono::steady_clock> trace_check_tp_;
            std::shared_ptr< adcontrols::SampleRun > nextSampleRun_;

            std::shared_ptr< iU5303AFacade > iU5303AFacade_;
            std::shared_ptr< adextension::iSequenceImpl > iSequenceImpl_;
            std::vector< std::shared_ptr< adextension::iController > > iControllers_;
            std::vector< std::shared_ptr< adextension::iController > > activeControllers_;
            std::map< boost::uuids::uuid, std::shared_ptr< adacquire::SignalObserver::Observer > > observers_;
            bool isMethodDirty_;

            std::deque< std::shared_ptr< const acqrscontrols::u5303a::waveform > > que_;
            std::shared_ptr< adcontrols::ControlMethod::Method > cm_;
            std::shared_ptr< acqrscontrols::u5303a::method > method_;
            std::shared_ptr< adcontrols::TimeDigitalMethod > tdm_;
            std::unique_ptr< ResultWriter > resultWriter_;
            std::unique_ptr< PKDAVGWriter > pkdavgWriter_;
            std::shared_ptr< const adcontrols::TofChromatogramsMethod > tofChromatogramsMethod_;
            std::shared_ptr< adcontrols::MassSpectrometer > massSpectrometer_;

            std::shared_ptr< QSettings > settings_;  // user scope settings
            QString ctrlmethod_filename_;
            std::map< QString, bool > moduleStates_;

            // display data
            std::array< std::shared_ptr< adcontrols::Trace >, 8 > traces_;
            std::map< boost::uuids::uuid
                      , std::array< std::shared_ptr< adcontrols::MassSpectrum >
                                    , acqrscontrols::u5303a::nchannels > > spectra_;

            // cache for PKD+AVG waveforms for chromatogram generation
            std::vector< pkdavg_waveforms_t > pkdavg_waveforms_;

            std::array< adportable::countrate_calculator< 3000 >, 8 > countrate_calculators_;

            bool pkdSpectrumEnabled_;
            bool longTermHistogramEnabled_;

            boost::asio::io_context io_context_;
            boost::asio::steady_timer timer_;
            std::unique_ptr< adurl::sse_handler > sse_;
            std::unique_ptr< adurl::blob > blob_;
            std::vector< std::thread > threads_;
            bool hasDark_;

            impl() : tdcdoc_( std::make_shared< acqrscontrols::u5303a::tdcdoc >() )
                   , trace_check_tp_( std::chrono::steady_clock::now() )
                   , nextSampleRun_( std::make_shared< adcontrols::SampleRun >() )
                   , iU5303AFacade_( std::make_shared< iU5303AFacade >() )
                   , iSequenceImpl_( std::make_shared< adextension::iSequenceImpl >( "AccuTOF" ) )
                   , isMethodDirty_( true )
                   , cm_( std::make_shared< adcontrols::ControlMethod::Method >() )
                   , method_( std::make_shared< acqrscontrols::u5303a::method >() )
                   , tdm_( std::make_shared< adcontrols::TimeDigitalMethod>() )
                   , resultWriter_( std::make_unique< ResultWriter >() )
                   , pkdavgWriter_( std::make_unique< PKDAVGWriter >() )
                   , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                               , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                               , QLatin1String( "accutof" ) ) )
                   , timer_( io_context_ )
                   , sse_( std::make_unique< adurl::sse_handler >( io_context_ ) )
                   , blob_( std::make_unique< adurl::blob >( io_context_ ) )
                   , hasDark_( false ) {

                adcontrols::TofChromatogramsMethod tofm;
                tofm.setNumberOfTriggers( 1000 );
                tdcdoc_->setTofChromatogramsMethod( tofm );

                uint32_t id(0);
                for ( auto& trace: traces_ )
                    trace = std::make_shared< adcontrols::Trace >( id++, 4096 - 512, 4096 );
            }

            void addInstController( std::shared_ptr< adextension::iController > p );
            void handleConnected( adextension::iController * controller );
            void handleLog( adextension::iController *, const QString& );
            void handleDataEvent( adacquire::SignalObserver::Observer *, unsigned int events, unsigned int pos );
            bool setControllerSettings( const QString&, bool enable );
            void loadControllerState();
            void takeSnapshot();
            std::shared_ptr< adcontrols::MassSpectrum > getHistogram( double resolution ) const;

            bool prepareStorage( const boost::uuids::uuid&, adacquire::SampleProcessor& sp ) const;
            bool closingStorage( const boost::uuids::uuid&, adacquire::SampleProcessor& sp ) const;
            bool initStorage( const boost::uuids::uuid& uuid, adfs::sqlite& db ) const;
            void handle_fsm_state_changed( bool enter, int id_state, adacquire::Instrument::eInstStatus st ) {
                if ( enter )
                    emit document::instance()->instStateChanged( st );
            }

            void handle_fsm_action( adacquire::Instrument::idFSMAction a ) {

                typedef boost::mpl::vector< exec_fsm_stop, exec_fsm_start, exec_fsm_ready, exec_fsm_inject, exec_fsm_complete > actions;

                switch( a ) {
                case adacquire::Instrument::fsmStop:
                    boost::mpl::at_c<actions, adacquire::Instrument::fsmStop>::type()( iControllers_ );
                    break;
                case adacquire::Instrument::fsmStart:
                    boost::mpl::at_c<actions, adacquire::Instrument::fsmStart>::type()( iControllers_ );
                    break;
                case adacquire::Instrument::fsmReady:
                    boost::mpl::at_c<actions, adacquire::Instrument::fsmReady>::type()( iControllers_ );
                    break;
                case adacquire::Instrument::fsmInject:
                    boost::mpl::at_c<actions, adacquire::Instrument::fsmInject>::type()( iControllers_ );
                    break;
                case adacquire::Instrument::fsmComplete:
                    boost::mpl::at_c<actions, adacquire::Instrument::fsmComplete>::type()( iControllers_ );
                    break;
                }
            }
        };

        // static members
        std::mutex document::impl::mutex_;
        document * document::impl::instance_( 0 );
        const std::chrono::system_clock::time_point document::impl::uptime_ = std::chrono::system_clock::now();
        const uint64_t document::impl::tp0_ =
            std::chrono::duration_cast<std::chrono::nanoseconds>( document::impl::uptime_.time_since_epoch() ).count();
        std::set< QString > document::impl::blockedModules_ = { "Acquire" };
    } // namespace acquire
} // namespace accutof

document::document() : impl_( new impl() )
{
    // diagnostic
    QVariant v;
    v.setValue( boost::uuids::uuid() );
}

document::~document()
{
    delete impl_;
    ADDEBUG() << "=====> document dtor";
}

document *
document::instance()
{
    static std::once_flag flag;

    std::call_once( flag, [=] () { impl::instance_ = new document(); } );
    return impl::instance_;
}

void
document::actionConnect()
{
    using namespace std::literals::chrono_literals;

    if ( !impl_->iControllers_.empty() ) {

        std::vector< std::future<bool> > futures;

        std::vector< std::shared_ptr< adextension::iController > > activeControllers;

        for ( auto& iController : impl_->iControllers_ ) {

            if ( isControllerEnabled( iController->module_name() ) ) {

                activeControllers.emplace_back( iController );

                futures.emplace_back( std::async( [iController] () { return iController->wait_for_connection_ready( 3s ); } ) );

                iController->connect();
            }
        }

        QStringList failed;
        size_t i = 0;
        for ( auto& future : futures ) {
            if ( future.get() )
                impl_->activeControllers_.push_back( activeControllers[ i ] );
            else
                failed << activeControllers[ i ]->module_name();
            ++i;
        }

        emit onModulesFailed( failed );

        auto cm = MainWindow::instance()->getControlMethod();
        setControlMethod( *cm, QString() );
        tdc()->set_threshold_method( 0, impl_->tdm_->threshold( 0 ) );
        tdc()->set_threshold_action( impl_->tdm_->action() );

        futures.clear();
        for ( auto& iController : impl_->iControllers_ ) {
            if ( auto session = iController->getInstrumentSession() ) {
                session->initialize();
                session->prepare_for_run( cm );
            }
        }

        // setup observer hiralchey
        if ( auto masterObserver = adacquire::task::instance()->masterObserver() ) {

            // create local signal observers --->
            for ( auto& o: observers ) {
                auto so = std::make_shared< adacquire::SimpleObserver >( o.objtext, o.dataInterpreterClsid, o.desc );
                const auto uuid = so->objid();
                impl_->observers_[ uuid ] = so;
                so->setPrepareStorage( [ uuid, this ]( adacquire::SampleProcessor& sp ) { return impl_->prepareStorage( uuid, sp ); } );
                so->setClosingStorage( [ uuid, this ]( adacquire::SampleProcessor& sp ) { return impl_->closingStorage( uuid, sp ); } );
            }
            for ( auto& obs: impl_->observers_ )
                masterObserver->addSibling( obs.second.get() );
            ////////// <--- signal observer hiralchey crated /////////

            masterObserver->setPrepareStorage( [&]( adacquire::SampleProcessor& sp ) {
                                                   return impl_->prepareStorage( boost::uuids::uuid{{ 0 }}, sp );
                                               } );

            masterObserver->setClosingStorage( [&]( adacquire::SampleProcessor& sp ) {
                                                   return impl_->closingStorage( boost::uuids::uuid{{ 0 }}, sp );
                                               } );

            for ( auto& iController : impl_->iControllers_ ) {
                if ( auto session = iController->getInstrumentSession() ) {
                    if ( auto observer = session->getObserver() )
                        masterObserver->addSibling( observer );
                }
            }
        }

        // FSM Action
        adacquire::task::instance()->connect_fsm_action( std::bind( &impl::handle_fsm_action, impl_, std::placeholders::_1 ) );

        // FSM State
        adacquire::task::instance()->connect_fsm_state(
            std::bind( &impl::handle_fsm_state_changed, impl_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );

        adacquire::task::instance()->fsmStart();
        adacquire::task::instance()->fsmReady();
    } // namesapce acquire
} // namespace accutof

void
document::actionInject()
{
    ADDEBUG() << "############### actionInject #############";
    adacquire::task::instance()->fsmInject();

    for ( auto& iController : impl_->iControllers_ ) {
        if ( auto session = iController->getInstrumentSession() )
            session->event_out( adacquire::Instrument::instEventInjectOut ); // loopback to peripherals
    }
}

void
document::actionRec( bool onoff )
{
    task::instance()->setRecording( onoff );

    for ( auto inst : impl_->iControllers_ ) {
        if ( auto session = inst->getInstrumentSession() )
            session->recording( onoff );
    }
}

void
document::actionSyncTrig()
{
    tdc()->clear_histogram();
}

void
document::actionRun()
{
    auto run = MainWindow::instance()->getSampleRun();
    setSampleRun( run );

    auto cm = MainWindow::instance()->getControlMethod();
    setControlMethod( cm, QString() );

    adacquire::task::instance()->fsmStart();
}

void
document::actionStop()
{
    adacquire::task::instance()->fsmStop();
    impl_->resultWriter_->fsmStop(); // stop writing .txt data
}

void
document::addInstController( adextension::iController * p )
{
    try {

        if ( auto ptr = p->pThis() )
            impl_->addInstController( p->pThis() );

    } catch ( std::bad_weak_ptr& ) {

        QMessageBox::warning( MainWindow::instance(), "accutofacquire plugin"
                              , QString( tr( "Instrument controller %1 has no shared_ptr; ignored." ) ).arg( p->module_name() ) );

    }
}

void
document::impl::addInstController( std::shared_ptr< adextension::iController > p )
{
    using adextension::iController;
    using adacquire::SignalObserver::Observer;

    if ( document::instance()->isControllerEnabled( p->module_name() ) ) {

        iControllers_.emplace_back( p );

        // switch to UI thread
        connect( p.get(), &iController::message, document::instance()
                 , [] ( iController * p, unsigned int code, unsigned int value ) { document::instance()->handleMessage( p, code, value ); } );

        // non UI thread
        connect( p.get(), &iController::connected, [this] ( iController * p ) { handleConnected( p ); } );
        connect( p.get(), &iController::log, [this] ( iController * p, const QString& log ) { handleLog( p, log ); } );

        p->dataChangedHandler( [] ( Observer *o, unsigned int pos ) { task::instance()->onDataChanged( o, pos ); } );
    }
}

void
document::prepare_next_sample( std::shared_ptr< adcontrols::SampleRun > run, const adcontrols::ControlMethod::Method& cm )
{
    // make empty que
    while( auto sample = adacquire::task::instance()->deque() ) {
        sample->close( true ); // async (detach)
    }

    // push new sample
    adacquire::task::instance()->prepare_next_sample( run, cm );

    // set INJECTION WAITING
    adacquire::task::instance()->fsmReady();

    emit sampleRunChanged();
}



void
document::prepare_for_run()
{
    using adcontrols::ControlMethod::MethodItem;

    save_defaults();

    auto cm = MainWindow::instance()->getControlMethod();

    setControlMethod( *cm, QString() );

    impl_->isMethodDirty_ = false;

    prepare_next_sample( impl_->nextSampleRun_, *impl_->cm_ );

    ADDEBUG() << "### prepare_for_run ### next = " << impl_->nextSampleRun_->runCount();

    std::vector< std::future< bool > > futures;
    for ( auto& iController : impl_->iControllers_ ) {
        if ( auto session = iController->getInstrumentSession() ) {
            futures.push_back( std::async( [=] () { return session->prepare_for_run( cm ); } ) );
        }
    }
    if ( !futures.empty() )
        task::instance()->post( futures );
}

void
document::start_run()
{
    ADDEBUG() << "### start run ###";
    prepare_for_run();
}

void
document::stop()
{
    ADDEBUG() << "### stop ###";

    std::vector< std::future< bool > > futures;

    for ( auto& iController : impl_->iControllers_ ) {
        if ( auto session = iController->getInstrumentSession() ) {
            futures.push_back( std::async( [=] () { return session->stop_run(); } ) );
        }
    }
    if ( !futures.empty() )
        task::instance()->post( futures );

}

std::shared_ptr< const acqrscontrols::u5303a::method >
document::method() const
{
    return impl_->method_;
}

// static
bool
document::appendOnFile( const boost::filesystem::path& path
                        , const QString& title
                        , const adcontrols::MassSpectrum& ms
                        , QString& id )
{
    adfs::filesystem fs;

    if ( ! boost::filesystem::exists( path ) ) {
        if ( ! fs.create( path.c_str() ) )
            return false;
    } else {
        if ( ! fs.mount( path.c_str() ) )
            return false;
    }
    adfs::folder folder = fs.addFolder( L"/Processed/Spectra" );

    if ( folder ) {
        adfs::file file = folder.addFile( adfs::create_uuid(), title.toStdWString() );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            id = QString::fromStdWString( file.id() );
            if ( file.save( ms ) )
                file.commit();
        }
    }
    return true;

}

void
document::initialSetup()
{
    boost::filesystem::path dir = user_preference::path( impl_->settings_.get() );

    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "pkdavg::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    QString path = recentFile( Constants::GRP_DATA_FILES, false );
    if ( path.isEmpty() ) {
        path = QString::fromStdWString(
            ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data" ).generic_wstring() );
    } else {
        path = QFileInfo( path ).path();
    }

    if ( auto ptr = std::make_shared< adcontrols::ControlMethod::Method >() ) {
        // always load 'latest', which may not be same with methodName if user did not save with the name
        boost::filesystem::path fname( dir / Constants::LAST_METHOD );
        if ( load( QString::fromStdWString( fname.wstring() ), *ptr ) ) {
            setControlMethod( ptr );
            impl_->tdcdoc_->set_threshold_method( 0, impl_->tdm_->threshold( 0 ) );
            impl_->tdcdoc_->set_threshold_action( impl_->tdm_->action() );
        }
    }

    if ( auto run = std::make_shared< adcontrols::SampleRun >() ) {

        boost::filesystem::path fname( dir / "samplerun.xml" );
        if ( boost::filesystem::exists( fname ) ) {
            std::wifstream inf( fname.string() );
            try {
                adcontrols::SampleRun::xml_restore( inf, *run );

                // replace directory name to 'today'
                run->setDataDirectory( impl_->nextSampleRun_->dataDirectory() ); // reset data directory to ctor default
                adacquire::SampleProcessor::prepare_sample_run( *run, false );

                MainWindow::instance()->setSampleRun( *run );
                document::setSampleRun( run );

            } catch ( std::exception& ex ) {
                ADDEBUG() << ex.what();
            }
        }
    }

    if ( ! impl_->massSpectrometer_  ) {

        if ( ( impl_->massSpectrometer_
               = adcontrols::MassSpectrometerBroker::make_massspectrometer( accutof::spectrometer::iids::uuid_massspectrometer ) ) ) {
            // todo: load mass calibration from settings
        } else {
            QMessageBox::warning( MainWindow::instance(), "accutofacquire plugin", QString( tr( "No AccuTOF Spectrometer installed." ) ) );
        }
    }

    ///////////////////////////////////////////////////////
    // loadControllerSettings
    {
        auto settings( impl_->settings_ );
        settings->beginGroup( Constants::THIS_GROUP );
        int size = settings->beginReadArray( "Controllers" );
        for ( int i = 0; i < size; ++i ) {
            settings->setArrayIndex( i );
            QString module_name = settings->value("module_name").toString();
            bool enable = settings->value( "enable" ).toBool();
            impl_->moduleStates_[ module_name ] = enable;
        }

        settings->endArray();
        settings->endGroup();

        for ( auto& list : impl_->blockedModules_ ) {
            auto it = impl_->moduleStates_.find( list  );
            if ( it == impl_->moduleStates_.end() )
                impl_->moduleStates_[ list ] = false;
        }
    } ///////////// end loadControllerSettings ///////////////

    { // draw settings
        auto settings( impl_->settings_ );
        impl_->pkdSpectrumEnabled_ = settings->value( Constants::THIS_GROUP + QString("/pkdSpectrumEnabled"),  true ).toBool();
        impl_->longTermHistogramEnabled_ = settings->value( Constants::THIS_GROUP + QString("/longTermHistogramEnabled"),  true ).toBool();
    }

    // { // host:port
    //     auto settings( impl_->settings_ );
    //     impl_->http_host_ = settings->value( Constants::THIS_GROUP + QString("/http_host"),  "httpd-map" ).toString().toStdString();
    //     impl_->http_port_ = settings->value( Constants::THIS_GROUP + QString("/http_port"),  "http" ).toString().toStdString();
    // }

    emit on_threshold_action_changed();

    /////////////////////////////////////
    //impl_->connect_httpd_map();
}

void
document::finalClose()
{
    // impl_->disconnect_httpd_map();

    for ( auto iController : impl_->iControllers_ )
        iController->disconnect( true );

    // make empty que
    while( auto sample = adacquire::task::instance()->deque() )
        sample->close( false ); // sync

    task::instance()->finalize();

    save_defaults();
}

void
document::save_defaults()
{
    boost::filesystem::path dir = user_preference::path( impl_->settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "accutof::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }

    if ( auto cm = MainWindow::instance()->getControlMethod() ) {
        boost::filesystem::path fname( dir / Constants::LAST_METHOD );
        save( QString::fromStdWString( fname.wstring() ), *cm );
    }

    if ( auto run = sampleRun() ) {
        boost::filesystem::path fname( dir / "samplerun.xml" );
        std::wofstream outf( fname.string() );
        adcontrols::SampleRun::xml_archive( outf, *run );
    }

    // for debugging convension
#if 0
    for ( auto& mi : *cm ) {
        if ( mi.clsid() == acqrscontrols::u5303a::method::clsid() ) {
            xmlWriter< acqrscontrols::u5303a::method >()( mi, dir );
        } else if ( mi.clsid() == adcontrols::TimeDigitalMethod::clsid() ) {
            xmlWriter< adcontrols::TimeDigitalMethod >()( mi, dir );
        } else if ( mi.clsid() == adcontrols::threshold_method::clsid() ) {
            xmlWriter< adcontrols::threshold_method >()( mi, dir );
        } else if ( mi.clsid() == adcontrols::threshold_action::clsid() ) {
            xmlWriter< adcontrols::threshold_action >()( mi, dir );
        }
    }
#endif
}

void
document::addToRecentFiles( const QString& filename )
{
    qtwrapper::settings( *impl_->settings_ ).addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, filename );
}

QString
document::recentFile( const char * group, bool dir_on_fail )
{
    if ( group == 0 )
        group = Constants::GRP_DATA_FILES;

    QString file = qtwrapper::settings( *impl_->settings_ ).recentFile( group, Constants::KEY_FILES );
    if ( !file.isEmpty() )
        return file;

    if ( dir_on_fail ) {
        file = Core::DocumentManager::currentFile();
        if ( file.isEmpty() )
            file = qtwrapper::settings( *impl_->settings_ ).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );

        if ( !file.isEmpty() ) {
            QFileInfo fi( file );
            return fi.path();
        }
        return QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() );
    }
    return QString();
}

bool
document::load( const QString& filename, acqrscontrols::u5303a::method& m )
{
    try {

        std::wifstream inf( filename.toStdString() );
        boost::archive::xml_wiarchive ar( inf );

        ar >> boost::serialization::make_nvp( "u5303a_method", m );

        return true;

    } catch( ... ) {
        std::cout << "############# acqrscontrols::u5303a::method load failed" << std::endl;
    }
    return false;
}

bool
document::save( const QString& filename, const acqrscontrols::u5303a::method& m )
{
    std::wofstream outf( filename.toStdString() );

    boost::archive::xml_woarchive ar( outf );
    ar << boost::serialization::make_nvp( "u5303a_method", m );
    return true;
}

bool
document::load( const QString& filename, adcontrols::ControlMethod::Method& m )
{
    QFileInfo fi( filename );

    if ( fi.exists() ) {
        adfs::filesystem fs;
        if ( fs.mount( filename.toStdWString().c_str() ) ) {
            adfs::folder folder = fs.findFolder( L"/ControlMethod" );

            auto files = folder.files();
            if ( !files.empty() ) {
                auto file = files.back();
                try {
                    file.fetch( m );
                } catch ( std::exception& ex ) {
                    QMessageBox::information( 0, "acquire -- Open default process method"
                                              , (boost::format( "Failed to open last used process method file: %1% by reason of %2% @ %3% #%4%" )
                                                 % filename.toStdString() % ex.what() % __FILE__ % __LINE__).str().c_str() );
                    return false;
                }
                return true;
            }
        }
    }
    return false;
}

bool
document::save( const QString& filename, const adcontrols::ControlMethod::Method& m )
{
    adfs::filesystem file;

    if ( !file.create( filename.toStdWString().c_str() ) ) {
        ADTRACE() << "Error: \"" << filename.toStdString() << "\" can't be created";
        return false;
    }

    adfs::folder folder = file.addFolder( L"/ControlMethod" );
    adfs::file adfile = folder.addFile( adfs::create_uuid(), filename.toStdWString() );
    try {
        adfile.dataClass( adcontrols::ControlMethod::Method::dataClass() );
        adfile.save( m );
    } catch ( std::exception& ex ) {
        ADTRACE() << "Exception: " << boost::diagnostic_information( ex );
        return false;
    }
    adfile.commit();

#if 0 // adcontrols can't archive into xml format
    QFileInfo xmlfile( filename + ".xml" );
    if ( xmlfile.exists() )
        QFile::remove( xmlfile.absoluteFilePath() );

    std::wstringstream o;
    try {
        adcontrols::ControlMethod::xml_archive( o, m );
    } catch ( std::exception& ex ) {
        ADDEBUG() << boost::diagnostic_information( ex );
    }
    pugi::xml_document doc;
    doc.load( o );
    doc.save_file( xmlfile.absoluteFilePath().toStdString().c_str() );
#endif

    return true;
}

std::shared_ptr< const adcontrols::ControlMethod::Method >
document::controlMethod() const
{
    return impl_->cm_;
}

void
document::setControlMethod( std::shared_ptr< adcontrols::ControlMethod::Method > ptr, const QString& filename )
{
    do {
        auto it = ptr->find( ptr->begin(), ptr->end(), acqrscontrols::u5303a::method::clsid() );
        if ( it != ptr->end() ) {
            acqrscontrols::u5303a::method m;
            if ( it->get( *it, m ) ) {
                *impl_->method_ = m;
            }
            impl_->cm_ = ptr;
        }
    } while(0);

    do {
        auto it = ptr->find( ptr->begin(), ptr->end(), adcontrols::TimeDigitalMethod::clsid() );
        if ( it != ptr->end() ) {
            adcontrols::TimeDigitalMethod tdm;
            if ( it->get( *it, tdm ) )
                *impl_->tdm_ = tdm;
        }
    } while ( 0 );

    do {
        auto it = ptr->find( ptr->begin(), ptr->end(), adcontrols::TofChromatogramsMethod::clsid() );
        if ( it != ptr->end() ) {
            adcontrols::TofChromatogramsMethod tdcm;
            it->get( *it, tdcm );
            setMethod( tdcm );  // set method to tdc, create traces
        }
    } while ( 0 );

    if ( ! filename.isEmpty() ) {
        impl_->ctrlmethod_filename_ = filename;
        qtwrapper::settings( *impl_->settings_ ).addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, filename );
    }

    emit onControlMethodChanged( filename );
}

void
document::setControlMethod( const adcontrols::ControlMethod::Method& m, const QString& filename )
{
    auto ptr = std::make_shared< adcontrols::ControlMethod::Method >( m );
    setControlMethod( ptr, filename );
}

double
document::triggers_per_second() const
{
    return impl_->tdcdoc_->triggers_per_second();
}

size_t
document::unprocessed_trigger_counts() const
{
    return 0; //impl_->unprocessed_trigger_counts();
}

void
document::save_histogram( size_t tickCount, const adcontrols::MassSpectrum& hist )
{
#if 0
    std::ofstream of( impl_->hist_datafile(), std::ios_base::out | std::ios_base::app );

    const double * times = hist.getTimeArray();
    const double * intens = hist.getIntensityArray();
    const adcontrols::MSProperty& prop = hist.getMSProperty();

    of << boost::format("\n%d, %.8lf, %.14le") % tickCount % prop.timeSinceInjection() % prop.instTimeRange().first;
    for ( size_t i = 0; i < hist.size(); ++i )
        of << boost::format(", %.14le, %d" ) % times[ i ] % uint32_t( intens[ i ] );
#endif
}

void
document::set_threshold_method( int ch, const adcontrols::threshold_method& m )
{
    tdc()->set_threshold_method( ch, m );

    auto mp = MainWindow::instance()->getControlMethod();
    setControlMethod( mp );

    emit on_threshold_method_changed( ch );
}

void
document::set_threshold_action( const adcontrols::threshold_action& m )
{
    tdc()->set_threshold_action( m );

    auto mp = MainWindow::instance()->getControlMethod();
    setControlMethod( mp );

    emit on_threshold_action_changed();
}


void
document::set_method( const acqrscontrols::u5303a::method& )
{
    if ( auto cm = MainWindow::instance()->getControlMethod() ) {
        setControlMethod( cm );

        auto it = std::find_if( impl_->iControllers_.begin(), impl_->iControllers_.end(), [] ( std::shared_ptr<adextension::iController> ic ) {
                return ic->module_name() == "u5303a";
            } );
        if ( it != impl_->iControllers_.end() ) {
            if ( auto session = ( *it )->getInstrumentSession() )
                session->prepare_for_run( cm );
        }
    }
}

std::shared_ptr< const adcontrols::SampleRun >
document::activeSampleRun() const
{
    if ( auto sequence = adacquire::task::instance()->sampleSequence() ) {

        if ( sequence->size() ) {
            if ( auto p = sequence->at( 0 ) ) {
                return p->sampleRun();
            }
        }
    }
    return nullptr;
}

std::shared_ptr< const adcontrols::SampleRun >
document::sampleRun() const
{
    return impl_->nextSampleRun_;
}

std::shared_ptr< adcontrols::SampleRun >
document::sampleRun()
{
    return impl_->nextSampleRun_;
}

void
document::setSampleRun( std::shared_ptr< adcontrols::SampleRun > sr )
{
    impl_->nextSampleRun_ = sr;
}

bool
document::isRecording() const
{
    return task::instance()->isRecording();
}

void
document::impl::handleConnected( adextension::iController * controller )
{
    ADDEBUG() << controller->module_name().toStdString();
    task::instance()->initialize();
}

void
document::handleMessage( adextension::iController * ic, uint32_t code, uint32_t value )
{
    if ( code == adacquire::Receiver::CLIENT_ATTACHED ) {

        // do nothing

    } else if ( code == adacquire::Receiver::STATE_CHANGED ) {

        if ( value & adacquire::Instrument::eErrorFlag ) {
            QMessageBox::warning( MainWindow::instance(), "U5303A Error"
                                  , QString( "Module %1 error with code %2" ).arg( ic->module_name(), QString::number( value, 16 ) ) );
        }
    } else if ( code == adacquire::Receiver::DARK_STARTED ) {
        impl_->hasDark_ = false;
        ADDEBUG() << "DARK_STARTED: " << value;
    } else if ( code == adacquire::Receiver::DARK_ACQUIRED ) {
        impl_->hasDark_ = true;
        emit darkStateChanged( 1 );
        ADDEBUG() << "DARK_ACQUIRED: " << value;
    } else if ( code == adacquire::Receiver::DARK_CANCELED ) {
        impl_->hasDark_ = false;
        emit darkStateChanged( 0 );
        ADDEBUG() << "DARK_CANCELD: " << value;
    }
}

void
document::impl::handleLog( adextension::iController *, const QString& )
{
}

void
document::impl::handleDataEvent( adacquire::SignalObserver::Observer *, unsigned int events, unsigned int pos )
{
}

// static
acqrscontrols::u5303a::tdcdoc *
document::tdc()
{
    return document::instance()->impl_->tdcdoc_.get();
}

void
document::setData( const boost::uuids::uuid& objid, std::shared_ptr< adcontrols::MassSpectrum > ms, unsigned idx )
{
    assert( idx < acqrscontrols::u5303a::nchannels );

    if ( idx >= acqrscontrols::u5303a::nchannels )
        return;

    do {
        std::lock_guard< std::mutex > lock( impl_->mutex_ );
        impl_->spectra_[ objid ][ idx ] = ms;
    } while( 0 );

    emit dataChanged( objid, idx );
}


std::shared_ptr< adcontrols::MassSpectrum >
document::recentSpectrum( const boost::uuids::uuid& uuid, int idx )
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    auto it = impl_->spectra_.find( uuid );
    if ( it != impl_->spectra_.end() ) {
        if ( it->second.size() > idx )
            return it->second.at( idx );
    }
    return 0;
}

QSettings *
document::settings()
{
    return impl_->settings_.get();
}

void
document::takeSnapshot()
{
    impl_->takeSnapshot();
}

std::shared_ptr< adcontrols::MassSpectrum >
document::impl::getHistogram( double resolution ) const
{
    if ( auto hgrm = tdcdoc_->longTermHistogram() ) {

        double resolution = 0;
        if ( auto tm = tdcdoc_->threshold_method( 0 ) )
            resolution = tm->time_resolution;

        if ( resolution > hgrm->xIncrement() ) {
            hgrm = hgrm->merge_peaks( resolution );
        }

        auto ms = std::make_shared< adcontrols::MassSpectrum >();
        adcontrols::TimeDigitalHistogram::translate( *ms, *hgrm );
        return ms;
    }
    return nullptr;
}

void
document::impl::takeSnapshot()
{
    boost::filesystem::path dir( nextSampleRun_->dataDirectory() );
    boost::filesystem::path file( std::wstring( nextSampleRun_->filePrefix() ) + L".adfs~" );

    // debug -->
    // resultWriter_->dump_waveform();
    // <-- debug

    if ( ! boost::filesystem::exists( dir ) ) {
        boost::system::error_code ec;
        boost::filesystem::create_directories( dir, ec );
    }

    boost::filesystem::path path( dir / file );
    if ( ! boost::filesystem::exists( path ) )
        path = dir / ( std::wstring( nextSampleRun_->filePrefix() ) + L"_snapshots.adfs" );

    unsigned idx = 0;

    // get histogram
    double resolution = 0.0;
    if ( auto tm = tdcdoc_->threshold_method( idx ) )
        resolution = tm->time_resolution;

    auto histogram = getHistogram( resolution );

    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::string date = adportable::date_string::logformat( tp );

    // get waveform(s)
    auto spectra = spectra_[ acqrscontrols::u5303a::waveform_observer ];

    int ch = 1;
    for ( auto ms: spectra ) {
        if ( ms ) {
            QString title = QString( "Spectrum %1 CH-%2" ).arg( QString::fromStdString( date ), QString::number( ch ) );
            QString folderId;
            if ( appendOnFile( path, title, *ms, folderId ) ) {
                auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
                for ( auto handler: vec )
                    handler->folium_added( path.string().c_str(), "/Processed/Spectra", folderId );
            }
        }
        ++ch;
    }

    // save histogram
    ch = 1;
    if ( histogram ) {
        QString title = QString( "Histogram %1 CH-%2" ).arg( QString::fromStdString( date ), QString::number( ch ) );
        QString folderId;
        if ( document::appendOnFile( path, title, *histogram, folderId ) ) {
            auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
            for ( auto handler: vec )
                handler->folium_added( path.string().c_str(), "/Processed/Spectra", folderId );
        }
    }

}

std::vector< adextension::iController * >
document::iControllers()
{
    return std::vector< adextension::iController * >({ impl_->iU5303AFacade_.get() });
}

adextension::iSequenceImpl *
document::iSequence()
{
    return impl_->iSequenceImpl_.get();
}

void
document::commitData()
{
    // save time data
    impl_->resultWriter_->commitData();
    impl_->pkdavgWriter_->commitData();
}

void
document::result_to_file( std::shared_ptr< acqrscontrols::u5303a::threshold_result > ch1 )
{
    *( impl_->resultWriter_ ) << ch1;
}

void
document::waveforms_to_file( std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, 2 > wforms )
{
    *( impl_->pkdavgWriter_ ) << std::move( wforms );
}

size_t
document::enqueue( pkdavg_waveforms_t&& q )
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    impl_->pkdavg_waveforms_.emplace_back( q );
    return impl_->pkdavg_waveforms_.size();
}

size_t
document::dequeue( std::vector< pkdavg_waveforms_t >& a )
{
    if ( impl_->pkdavg_waveforms_.size() > 1 ) {
        std::lock_guard< std::mutex > lock( impl_->mutex_ );
        a.reserve( a.size() + impl_->pkdavg_waveforms_.size() );

        std::move( impl_->pkdavg_waveforms_.begin(), impl_->pkdavg_waveforms_.end(), std::back_inserter( a ) );

        impl_->pkdavg_waveforms_.clear();
    }
    return a.size();
}

bool
document::impl::setControllerSettings( const QString& module, bool enable )
{
    bool dirty( false );

    if ( moduleStates_[ module ] != enable )
        dirty = true;

    moduleStates_[ module ] = enable;

    // update settings
    settings_->beginGroup( Constants::THIS_GROUP );

    settings_->beginWriteArray( "Controllers" );

    int i = 0;
    for ( auto& state : moduleStates_ ) {
        settings_->setArrayIndex( i++ );
        settings_->setValue( "module_name", state.first );
        settings_->setValue( "enable", state.second );
    }

    settings_->endArray();
    settings_->endGroup();

    return dirty;
}

void
document::setControllerSettings( const QString& module, bool enable )
{
    if ( impl_->setControllerSettings( module, enable ) )
        emit moduleConfigChanged();
}

bool
document::isControllerEnabled( const QString& module ) const
{
    auto it = impl_->moduleStates_.find( module );

    if ( it != impl_->moduleStates_.end() )
        return it->second;

    return false;
}

bool
document::isControllerBlocked( const QString& module ) const
{
    return impl_->blockedModules_.find( module ) != impl_->blockedModules_.end();
}

QVector< QPair< QString, bool > >
document::controllerSettings() const
{
    QVector< QPair< QString, bool > > modules;
    for ( auto module: impl_->moduleStates_ )
        modules.append( { module.first, module.second } );
    return modules;
}


bool
document::impl::initStorage( const boost::uuids::uuid& uuid, adfs::sqlite& db ) const
{
    std::string objtext;

    if ( uuid == boost::uuids::uuid{{ 0 }} ) {
        objtext = "master.observer";
    } else {
        auto it = observers_.find( uuid );
        if ( it != observers_.end() )
            objtext = it->second->objtext();
        else
            return false;
    }
#ifndef NDEBUG
    ADDEBUG() << "## " << __FUNCTION__ << " " << uuid << ", " << objtext;
#endif
    if ( auto sp = document::instance()->massSpectrometer() ) {
        if ( auto law = sp->scanLaw() ) {
            adfs::stmt sql( db );
            sql.prepare( "\
INSERT OR REPLACE INTO ScanLaw (                                        \
 objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer) \
 VALUES ( ?,?,?,?,?,? )" );
            sql.bind( 1 ) = uuid;
            sql.bind( 2 ) = objtext;
            sql.bind( 3 ) = sp->acceleratorVoltage();
            sql.bind( 4 ) = sp->tDelay();
            sql.bind( 5 ) = std::string( accutof::spectrometer::names::objtext_massspectrometer );
            sql.bind( 6 ) = accutof::spectrometer::iids::uuid_massspectrometer;

            // ADDEBUG() << "initStorage acceleratorVoltage: " << sp->acceleratorVoltage() << ", " << sp->tDelay() << ", " << uuid;

            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << "sqlite error";
        }

        do {
            adfs::stmt sql( db );

            sql.prepare( "INSERT OR REPLACE INTO Spectrometer ( id, scanType, description, fLength ) VALUES ( ?,?,?,? )" );
            sql.bind( 1 ) = accutof::spectrometer::iids::uuid_massspectrometer; // 9568b15d-73b6-48ed-a1c7-ac56a308f712;
            sql.bind( 2 ) = 0;
            sql.bind( 3 ) = std::string( accutof::spectrometer::names::objtext_massspectrometer ); // := 'AccuTOF'
            sql.bind( 4 ) = sp->fLength();

            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << "sqlite error";
        } while ( 0 );
    }

    // Save method
    if ( uuid == boost::uuids::uuid{{ 0 }} ) {
        // only if call for master observer

        adfs::stmt sql( db );
        sql.exec( "CREATE TABLE IF NOT EXISTS MetaData (clsid UUID, attrib TEXT, data BLOB )" ); // check adutils/AcquiredData::create_table_v3

        std::string ar;
        {
            auto cm( cm_ );
            boost::iostreams::back_insert_device< std::string > inserter( ar );
            boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
            adcontrols::ControlMethod::Method::archive( device, *cm );
        }

        sql.prepare( "INSERT OR REPLACE INTO MetaData ( clsid, attrib, data ) VALUES ( ?,?,? )" );
        sql.bind( 1 ) = adcontrols::ControlMethod::Method::clsid();
        sql.bind( 2 ) = std::string( "ControlMethod::Method" );
        sql.bind( 3 ) = adfs::blob( ar.size(), reinterpret_cast< const int8_t * >( ar.data() ) );
        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "sqlite error";
    }

    if ( uuid == boost::uuids::uuid{{ 0 }} ) {
        // default calibration
        boost::filesystem::path path = QStandardPaths::locate( QStandardPaths::ConfigLocation, "QtPlatz", QStandardPaths::LocateDirectory ).toStdString();
        path /= accutof::acquire::Constants::DEFAULT_CALIB_FILE; // default.mscalib

        if ( boost::filesystem::exists( path ) ) {
            ADTRACE() << "Loading calibration from file: " << path.string();
            adfs::filesystem fs;
            if ( fs.mount( path ) ) {
                adcontrols::MSCalibrateResult calibResult;
                if ( adutils::fsio::load_mscalibfile( fs, calibResult ) ) {
                    if ( calibResult.calibration().massSpectrometerClsid() == accutof::spectrometer::iids::uuid_massspectrometer ) {
                        adutils::mscalibio::write( db, calibResult );
                        massSpectrometer_->initialSetup( db, {{0}} );
                    }
                }
            }
        }
    }


    return true;
}

bool
document::impl::prepareStorage( const boost::uuids::uuid& uuid, adacquire::SampleProcessor& sp ) const
{
#ifndef NDEBUG
    ADDEBUG() << "## " << __FUNCTION__ << " " << uuid;
#endif

    document::instance()->progress( 0.0, sp.sampleRun() ); // show data name on top of waveformwnd

    if ( initStorage( uuid, sp.filesystem().db() ) && uuid == boost::uuids::uuid{{ 0 }} ) {

        // counting peaks
        if ( uuid == boost::uuids::uuid{{ 0 }} ) {
            acqrscontrols::pkd_counting_data_writer::prepare_storage( sp.filesystem() );
        }

        return true;
    }

    return false;
}

bool
document::impl::closingStorage( const boost::uuids::uuid& uuid, adacquire::SampleProcessor& sp ) const
{
    // ADDEBUG() << "## " << __FUNCTION__ << " " << uuid;
    if ( uuid == boost::uuids::uuid{{ 0 }} ) {
        ADDEBUG() << "## " << __FUNCTION__ << " " << boost::filesystem::path( sp.filesystem().filename() ).stem().string();
    }
    return true;
}

void
document::applyTriggered()
{
    auto ptr = MainWindow::instance()->getControlMethod();
    setControlMethod( ptr );
    prepare_for_run();
}

void
document::setMethod( const adcontrols::TofChromatogramsMethod& m )
{
    ADDEBUG() << "-------------- chromatograms method value changed -------------------";
    namespace xic = adcontrols::xic;

    tdc()->setTofChromatogramsMethod( m );

    task::instance()->setHistogramClearCycleEnabled( m.refreshHistogram() );

    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    if ( ( impl_->tofChromatogramsMethod_ = tdc()->tofChromatogramsMethod() ) ) {

        for ( size_t idx = 0; idx < impl_->traces_.size(); ++idx ) {
            auto& trace = impl_->traces_[ idx ];
            if ( idx == 0 ) {
                auto [enable, algo] = m.tic();
                trace->setEnable( enable );
                trace->setIsCountingTrace( algo == xic::eCounting );
                trace->setLegend( "TIC" );
            } else if ( ( idx - 1 ) < impl_->tofChromatogramsMethod_->size() ) {
                const auto item = impl_->tofChromatogramsMethod_->begin() + ( idx - 1 );
                trace->setEnable( item->enable() );
                trace->setIsCountingTrace( item->intensityAlgorithm() == xic::eCounting );
                char c = item->intensityAlgorithm() == xic::eCounting ? 'C' : item->intensityAlgorithm() == xic::ePeakAreaOnProfile ? 'A' : 'H';
                auto formula = adcontrols::ChemicalFormula::formatFormula( item->formula() );
                trace->setLegend( ( boost::format( "%d[%c]" ) % idx % c ).str() );
            }
        }
    }

}

void
document::addChromatogramsPoint( const adcontrols::TofChromatogramsMethod& method
                                 , pkdavg_waveforms_t waveforms )
{
    auto avg( waveforms[ 0 ] );
    auto pkd( waveforms[ 1 ] );

    if ( !pkd )
        return;

    // elapsed time since start
    double seconds  = double( avg->timeSinceEpoch_ - task::instance()->upTimeSinceEpoch() ) / std::nano::den;
    double t_inject = double( task::instance()->injectTimeSinceEpoch() - task::instance()->upTimeSinceEpoch() ) / std::nano::den;

    size_t pkd_total_counts = pkd->accumulate( 0, 0 );
    // double tic = avg->accumulate( 0, -1 ); // TIC

    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    impl_->traces_ [ 0 ]->append( avg->serialnumber_, seconds, pkd_total_counts );
    impl_->traces_ [ 0 ]->setIsCountingTrace( true );
    impl_->traces_ [ 0 ]->setInjectTime( t_inject );

    for ( auto item: method ) {
        if ( item.enable() ) {
            int id = item.id() + 1;
            if ( id >= 1 && id < impl_->traces_.size() ) {
                uint32_t pkCounts = pkd->accumulate( item.time(), item.timeWindow() );
                impl_->traces_[ id ]->setInjectTime( t_inject );
                impl_->countrate_calculators_[ item.id() ] << std::make_pair( pkCounts, pkd->meta_.actualAverages );

                if ( item.intensityAlgorithm() == adcontrols::xic::eCounting ) {
                    impl_->traces_[ id ]->append( pkd->serialnumber(), seconds, pkCounts );
                    impl_->traces_[ id ]->setIsCountingTrace( true );
                } else if ( item.intensityAlgorithm() == adcontrols::xic::ePeakAreaOnProfile ) {
                    impl_->traces_[ id ]->append( pkd->serialnumber(), seconds, avg->accumulate( item.time(), item.timeWindow() ) ); // area
                    impl_->traces_[ id ]->setIsCountingTrace( false );
                } else {
                    impl_->traces_[ id ]->append( pkd->serialnumber(), seconds, avg->height( item.time(), item.timeWindow() ) ); // height
                    impl_->traces_[ id ]->setIsCountingTrace( false );
                }
            }
        }
    }

    using namespace std::chrono_literals;
    auto tp = std::chrono::steady_clock::now();
    if ( (impl_->traces_[ 0 ]->size() >= 2 ) && ( tp - impl_->trace_check_tp_ ) > 0.5s ) {
        emit traceChanged( pkd_trace_observer ); // append timed trace
        impl_->trace_check_tp_ = tp;
    }

}

void
document::addCountingChromatogramPoints( const adcontrols::TofChromatogramsMethod& method
                                         , const std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& vec )
{
    double t_inject = double( task::instance()->injectTimeSinceEpoch() - task::instance()->upTimeSinceEpoch() ) / std::nano::den;

    for ( auto& ptr: vec ) {

        double seconds = double( ptr->timeSinceEpoch().first - task::instance()->upTimeSinceEpoch() ) / std::nano::den;
        impl_->traces_[ 0 ]->append( ptr->serialnumber().first, seconds, ptr->accumulate( 0, -1 ) ); // TIC
        impl_->traces_[ 0 ]->setIsCountingTrace( true );
        impl_->traces_[ 0 ]->setInjectTime( t_inject );

        for ( auto item: method ) {
            if ( item.enable() && item.intensityAlgorithm() == adcontrols::xic::eCounting ) {
                int id = item.id() + 1;
                if ( id >= 1 && id < impl_->traces_.size() ) {
                    uint32_t pkCounts = ptr->accumulate( item.time(), item.timeWindow() );
                    impl_->countrate_calculators_[ item.id() ] << std::make_pair( pkCounts, ptr->trigger_count() );
                    impl_->traces_[ id ]->append( ptr->serialnumber().first, seconds, pkCounts );
                    impl_->traces_[ id ]->setIsCountingTrace( true );
                    impl_->traces_[ id ]->setInjectTime( t_inject );
                }
            }
        }
    }

    if ( impl_->traces_[ 0 ]->size() >= 2 )
        emit dataChanged( trace_observer, 0 );
}

double
document::countRate( int idx ) const
{
    if ( idx >= 0 && idx < impl_->countrate_calculators_.size() ) {
        return impl_->countrate_calculators_.at( idx ).rate();
    }
    return 0;
}

void
document::getTraces( std::vector< std::shared_ptr< adcontrols::Trace > >& traces )
{
    traces.clear();

    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    for ( auto& trace: impl_->traces_ )
        traces.emplace_back( trace );
}

std::shared_ptr< const adcontrols::MassSpectrometer >
document::massSpectrometer() const
{
    return impl_->massSpectrometer_;
}

void
document::progress( double elapsed_time, std::shared_ptr< const adcontrols::SampleRun >&& sampleRun )
{
    // double method_time = sampleRun->methodTime();
    // QString runName = QString::fromStdWString( sampleRun->filePrefix() );
    //ADDEBUG() << __FUNCTION__ << " runName: " << runName.toStdString() << ", method time: " << method_time;
}

void
document::appendToChromatograms(std::shared_ptr<const acqrscontrols::u5303a::waveform> waveform )
{
#if 0
    if ( auto pm = tofChromatogramsMethod() ) {
        std::vector< std::pair<uint32_t, double> > results;

        if ( waveform->wellKnownEvents_ & adacquire::SignalObserver::wkEvent_INJECT ) {
            double time = ( waveform->timeSinceEpoch_ - task::instance()->upTimeSinceEpoch() ) / std::nano::den; // uptime (s)
            //impl_->injectTime_ = time;
            //for ( auto& trace: impl_->traces_ )
            //    trace->setInjectTime( time );
            //emit onWellKnownEvent( adacquire::SignalObserver::wkEvent_INJECT, time );
        }

        tdc()->makeChromatogramPoints( waveform, *pm, results );

        if ( results.empty() )
            return;

        double time = ( waveform->timeSinceEpoch_ - task::instance()->upTimeSinceEpoch() ) / std::nano::den; // uptime (s)

        std::lock_guard< std::mutex > lock( impl_->mutex_ );

        for ( auto& data: results ) {
            if ( data.first < impl_->traces_.size() )
                impl_->traces_[ data.first ]->append( waveform->serialnumber_, time, data.second, waveform->wellKnownEvents_ );
        }
    }
#endif
}

void
document::onChromatogramChanged()
{
    emit dataChanged( trace_observer, 0 );
}

void
document::setPKDSpectrumEnabled( bool enable )
{
    impl_->pkdSpectrumEnabled_ = enable;
    impl_->settings_->setValue( Constants::THIS_GROUP + QString("/pkdSpectrumEnabled"), enable );
    emit drawSettingChanged();
}

void
document::setLongTermHistogramEnabled( bool enable )
{
    impl_->longTermHistogramEnabled_ = enable;
    impl_->settings_->setValue( Constants::THIS_GROUP + QString("/longTermHistogramEnabled"), enable );
    emit drawSettingChanged();
}

bool
document::pkdSpectrumEnabled() const
{
    return impl_->pkdSpectrumEnabled_;
}

bool
document::longTermHistogramEnabled() const
{
    return impl_->longTermHistogramEnabled_;
}

bool
document::hasDark() const
{
    return false;
}

void
document::clearDark()
{
    for ( auto& iController : impl_->iControllers_ ) {
        if ( auto session = iController->getInstrumentSession() ) {
            session->dark_run( 0 );
        }
    }
}

void
document::acquireDark()
{
    for ( auto& iController : impl_->iControllers_ ) {
        if ( auto session = iController->getInstrumentSession() )
            session->dark_run( 3 ); // wait 3 averaged waveforms
    }
    emit darkStateChanged( 1 );
}
