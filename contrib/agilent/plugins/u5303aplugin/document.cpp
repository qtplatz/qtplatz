/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include "constants.hpp"
#include "mainwindow.hpp"
#include "resultwriter.hpp"
#include "task.hpp"
#include "icontrollerimpl.hpp"
#include <acqrscontrols/constants.hpp>
#include <acqrscontrols/u5303a/histogram.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrscontrols/u5303a/metadata.hpp>
#include <acqrscontrols/u5303a/tdcdoc.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <adlog/logger.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/timedigitalmethod.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adcontrols/trace.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adextension/icontrollerimpl.hpp>
#include <adextension/isequenceimpl.hpp>
#include <adicontroller/sampleprocessor.hpp>
#include <adicontroller/masterobserver.hpp>
#include <adicontroller/task.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
#include <qtwrapper/settings.hpp>
#include <app/app_version.h>
#include <coreplugin/documentmanager.h>
#include <extensionsystem/pluginmanager.h>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/at.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QMetaType>
#include <chrono>
#include <future>
#include <string>
#include <fstream>


Q_DECLARE_METATYPE( boost::uuids::uuid );

using namespace u5303a;

namespace u5303a {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "u5303a";
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

    namespace so = adicontroller::SignalObserver;

    struct ObserverData {
        const char * objtext;
        const char * dataInterpreterClsid;
        const so::Description desc;
    };

    static ObserverData observers [] = {
        { acqrscontrols::u5303a::timecount_observer_name  // objtext; must contains unitid
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
            // increment sample number
            if ( auto run = document::instance()->sampleRun() )
                ++( *run );
        }
    };

    struct exec_fsm_ready {
        void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
            // // make immediate inject 
            // adicontroller::task::instance()->fsmInject();
        }
    };

    struct exec_fsm_inject {
        void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
            task::instance()->sample_injected();
            for ( auto& iController : iControllers ) {
                if ( auto session = iController->getInstrumentSession() )
                    session->event_out( adicontroller::Instrument::instEventInjectOut ); // loopback to peripherals
            }
        }
    };

    struct exec_fsm_complete {
        void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
            adicontroller::task::instance()->fsmStart();
            adicontroller::task::instance()->fsmReady();
        }
    };

    // template<typename T> struct wrap {};
    
    //..........................................
    class document::impl {
    public:
        static document * instance_; // workaround
        static std::mutex mutex_;
        static const std::chrono::steady_clock::time_point uptime_;
        static const uint64_t tp0_;

        std::shared_ptr< acqrscontrols::u5303a::tdcdoc > tdcdoc_;
        std::shared_ptr< adcontrols::SampleRun > nextSampleRun_;
        std::shared_ptr< ::u5303a::iControllerImpl > iControllerImpl_;
        std::shared_ptr< adextension::iSequenceImpl > iSequenceImpl_;
        std::vector< std::shared_ptr< adextension::iController > > iControllers_;
        std::vector< std::shared_ptr< adextension::iController > > activeControllers_;
        std::map< boost::uuids::uuid, std::shared_ptr< adicontroller::SignalObserver::Observer > > observers_;
        bool isMethodDirty_;

        std::deque< std::shared_ptr< const acqrscontrols::u5303a::waveform > > que_;
        std::shared_ptr< adcontrols::ControlMethod::Method > cm_;
        std::shared_ptr< acqrscontrols::u5303a::method > method_;
        std::shared_ptr< adcontrols::TimeDigitalMethod > tdm_;
        std::shared_ptr< ResultWriter > resultWriter_;
        std::shared_ptr< const adcontrols::TofChromatogramsMethod > tofChromatogramsMethod_;
        std::shared_ptr< adcontrols::MassSpectrometer > massSpectrometer_;

        int32_t device_status_;
        // double triggers_per_second_;
        
        std::shared_ptr< QSettings > settings_;  // user scope settings
        QString ctrlmethod_filename_;

        std::map< QString, bool > moduleStates_;

        // display data
        std::vector< std::shared_ptr< adcontrols::Trace > > traces_;
        std::map< boost::uuids::uuid
                  , std::array< std::shared_ptr< adcontrols::MassSpectrum >
                                , acqrscontrols::u5303a::nchannels > > spectra_;

        impl() : tdcdoc_( std::make_shared< acqrscontrols::u5303a::tdcdoc >() )
               , nextSampleRun_( std::make_shared< adcontrols::SampleRun >() )
               , iControllerImpl_( std::make_shared< u5303a::iControllerImpl >() )
               , iSequenceImpl_( std::make_shared< adextension::iSequenceImpl >( "U5303A" ) )
               , isMethodDirty_( true )
               , device_status_( 0 )
               , cm_( std::make_shared< adcontrols::ControlMethod::Method >() )
               , method_( std::make_shared< acqrscontrols::u5303a::method >() )
               , tdm_( std::make_shared< adcontrols::TimeDigitalMethod>() )
               , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                           , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                           , QLatin1String( "u5303a" ) ) )
                 //, triggers_per_second_(0)
               , resultWriter_( std::make_shared< ResultWriter >() ) {

            adcontrols::TofChromatogramsMethod tofm;
            tofm.setNumberOfTriggers( 1000 );
            tdcdoc_->setTofChromatogramsMethod( tofm );
            massSpectrometer_ = adcontrols::MassSpectrometerBroker::make_massspectrometer( adcontrols::iids::adspectrometer_uuid );
        }

        void addInstController( std::shared_ptr< adextension::iController > p );
        void handleConnected( adextension::iController * controller );
        void handleLog( adextension::iController *, const QString& );
        void handleDataEvent( adicontroller::SignalObserver::Observer *, unsigned int events, unsigned int pos );
        void setControllerState( const QString&, bool enable );
        void loadControllerState();
        void takeSnapshot();
        std::shared_ptr< adcontrols::MassSpectrum > getHistogram( double resolution ) const;

        bool prepareStorage( const boost::uuids::uuid&, adicontroller::SampleProcessor& sp ) const;
        bool closingStorage( const boost::uuids::uuid&, adicontroller::SampleProcessor& sp ) const;

        void handle_fsm_state_changed( bool enter, int id_state, adicontroller::Instrument::eInstStatus st ) {
            if ( enter )
                emit document::instance()->instStateChanged( st );
        }
        
        void handle_fsm_action( adicontroller::Instrument::idFSMAction a ) {

            typedef boost::mpl::vector< exec_fsm_stop, exec_fsm_start, exec_fsm_ready, exec_fsm_inject, exec_fsm_complete > actions;
            
            switch( a ) {
            case adicontroller::Instrument::fsmStop:
                boost::mpl::at_c<actions, adicontroller::Instrument::fsmStop>::type()( iControllers_ );
                break;
            case adicontroller::Instrument::fsmStart:
                boost::mpl::at_c<actions, adicontroller::Instrument::fsmStart>::type()( iControllers_ );
                break;                
            case adicontroller::Instrument::fsmReady:
                boost::mpl::at_c<actions, adicontroller::Instrument::fsmReady>::type()( iControllers_ );
                break;                                
            case adicontroller::Instrument::fsmInject:
                boost::mpl::at_c<actions, adicontroller::Instrument::fsmInject>::type()( iControllers_ );
                break;                                                
            case adicontroller::Instrument::fsmComplete:
                boost::mpl::at_c<actions, adicontroller::Instrument::fsmComplete>::type()( iControllers_ );
                break;                                                                
            }
        }
    };

    std::mutex document::impl::mutex_;
    document * document::impl::instance_( 0 );
    const std::chrono::steady_clock::time_point document::impl::uptime_ = std::chrono::steady_clock::now();
    const uint64_t document::impl::tp0_ =
        std::chrono::duration_cast<std::chrono::nanoseconds>( document::impl::uptime_.time_since_epoch() ).count();
}
    
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

                ADDEBUG() << "u5303a actionConnect connecting to " << iController->module_name().toStdString();

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
        if ( auto masterObserver = adicontroller::task::instance()->masterObserver() ) {

            masterObserver->setPrepareStorage( [&]( adicontroller::SampleProcessor& sp ) {
                    return impl_->prepareStorage( boost::uuids::uuid{ 0 }, sp );
                } );
            
            masterObserver->setClosingStorage( [&]( adicontroller::SampleProcessor& sp ) {
                    return impl_->closingStorage( boost::uuids::uuid{ 0 }, sp );
                } );
            
            for ( auto& iController : impl_->iControllers_ ) {
                if ( auto session = iController->getInstrumentSession() ) {
                    if ( auto observer = session->getObserver() )
                        masterObserver->addSibling( observer );
                }
            }
        }

        // FSM Action
        adicontroller::task::instance()->connect_fsm_action( std::bind( &impl::handle_fsm_action, impl_, std::placeholders::_1 ) );

        // FSM State
        adicontroller::task::instance()->connect_fsm_state(
            std::bind( &impl::handle_fsm_state_changed, impl_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
        
        adicontroller::task::instance()->fsmStart();
        adicontroller::task::instance()->fsmReady();
    }
}

void
document::actionInject()
{
    ADDEBUG() << "\t#### Action INJECT IN ####";
    adicontroller::task::instance()->fsmInject();
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
    auto cm = MainWindow::instance()->getControlMethod();
    setControlMethod( cm, QString() );
    
    adicontroller::task::instance()->fsmStart();
}

void
document::actionStop()
{
    adicontroller::task::instance()->fsmStop();
}

void
document::addInstController( adextension::iController * p )
{
    try {

        if ( auto ptr = p->pThis() )
            impl_->addInstController( p->pThis() );

    } catch ( std::bad_weak_ptr& ) {
        
        QMessageBox::warning( MainWindow::instance(), "U5303A plugin"
                              , QString( tr( "Instrument controller %1 has no shared_ptr; ignored." ) ).arg( p->module_name() ) );

    }
}

void
document::impl::addInstController( std::shared_ptr< adextension::iController > p )
{

    using adextension::iController;
    using adicontroller::SignalObserver::Observer;

    if ( p->module_name() == "u5303a" ) { // handle only this device

        for ( auto it = iControllers_.begin(); it != iControllers_.end(); ++it )
            if ( ( *it )->module_name() == "u5303a" )
                return;

        iControllers_.push_back( p );

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
    while( auto sample = adicontroller::task::instance()->deque() )
        ;

    // push new sample
    adicontroller::task::instance()->prepare_next_sample( run, cm );

    // set INJECTION WAITING
    adicontroller::task::instance()->fsmReady();

    boost::filesystem::path dir( run->dataDirectory() );
    boost::filesystem::path stem( run->filePrefix() );
    impl_->resultWriter_->setRunName( dir.string(), stem.string() );

    emit sampleRunChanged();
}



void
document::prepare_for_run()
{
    using adcontrols::ControlMethod::MethodItem;

    auto cm = MainWindow::instance()->getControlMethod();
    setControlMethod( *cm, QString() );
    impl_->isMethodDirty_ = false;

    prepare_next_sample( impl_->nextSampleRun_, *impl_->cm_ );
    
    ADDEBUG() << "### prepare_for_run ###";
    
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

int32_t
document::device_status() const
{
    return impl_->device_status_;
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
            QMessageBox::information( 0, "u5303a::document"
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
                adicontroller::SampleProcessor::prepare_sample_run( *run, false );

                MainWindow::instance()->setSampleRun( *run );
                document::setSampleRun( run );

            } catch ( std::exception& ex ) {
                ADDEBUG() << ex.what();
            }
        }
    }
}

void
document::finalClose()
{
    for ( auto iController : impl_->iControllers_ )
        iController->disconnect( true );

    // make empty que
    while( auto sample = adicontroller::task::instance()->deque() )
        ;

    task::instance()->finalize();

    boost::filesystem::path dir = user_preference::path( impl_->settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "u5303a::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }

    auto cm = MainWindow::instance()->getControlMethod();
    if ( cm ) {
        boost::filesystem::path fname( dir / Constants::LAST_METHOD );
        save( QString::fromStdWString( fname.wstring() ), *cm );        
    }

    if ( auto run = sampleRun() ) {
        boost::filesystem::path fname( dir / "samplerun.xml" );
        std::wofstream outf( fname.string() );
        adcontrols::SampleRun::xml_archive( outf, *run );
    }

    // for debugging convension
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

    if ( auto settings = impl_->settings_ ) {
        settings->beginGroup( Constants::THIS_GROUP );
        
        settings->beginWriteArray( "ControlModule" );

        int i = 0;
        for ( auto& state : impl_->moduleStates_ ) {
            settings->setArrayIndex( i++ );
            settings->setValue( "module_name", state.first );
            settings->setValue( "enable", state.second );
        }

        settings->endArray();
        settings->endGroup();

        settings->sync();
    }
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
    adfs::file adfile = folder.addFile( filename.toStdWString(), filename.toStdWString() );
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
            // task::instance()->set_softaverage_count( uint32_t ( tdcm.numberOfTriggers() ) );
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
    if ( code == adicontroller::Receiver::CLIENT_ATTACHED ) {

        // do nothing
        
    } else if ( code == adicontroller::Receiver::STATE_CHANGED ) {

        if ( value & adicontroller::Instrument::eErrorFlag ) {
            QMessageBox::warning( MainWindow::instance(), "U5303A Error"
                                  , QString( "Module %1 error with code %2" ).arg( ic->module_name(), QString::number( value, 16 ) ) );
        }
    }
}

void
document::impl::handleLog( adextension::iController *, const QString& )
{
}

void
document::impl::handleDataEvent( adicontroller::SignalObserver::Observer *, unsigned int events, unsigned int pos )
{
}

acqrscontrols::u5303a::tdcdoc *
document::tdc()
{
    return impl_->tdcdoc_.get();
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


void
document::commitData()
{
    // save time data
    impl_->resultWriter_->commitData();
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
    resultWriter_->dump_waveform();
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

    uint32_t serialnumber(0);

    // get waveform(s)
    auto spectra = spectra_[ u5303a_observer ];
    
    int ch = 1;
    for ( auto ms: spectra ) {
        if ( ms ) {
            serialnumber = ms->getMSProperty().trigNumber();
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

u5303a::iControllerImpl *
document::iController()
{
    return impl_->iControllerImpl_.get();
}

adextension::iSequenceImpl *
document::iSequence()
{
    return impl_->iSequenceImpl_.get();
}

void
document::result_to_file( std::shared_ptr< acqrscontrols::u5303a::threshold_result > ch1 )
{
    *( impl_->resultWriter_ ) << ch1;
}

bool
document::isControllerEnabled( const QString& module ) const
{
    if ( impl_->iControllerImpl_->module_name() == module )
        return true;
    return false;
}

bool
document::impl::prepareStorage( const boost::uuids::uuid& uuid, adicontroller::SampleProcessor& sp ) const
{
#if 0
    // todo
    std::string objtext;

    auto it = observers_.find( uuid );
    if ( it != observers_.end() )
        objtext = it->second->objtext();
    else if ( uuid == boost::uuids::uuid{ 0 } )
        objtext = "master.observer";
    else
        return false;

    // "{E45D27E0-8478-414C-B33D-246F76CF62AD}"
    static boost::uuids::uuid uuid_massspectrometer = boost::uuids::string_generator()( adspectrometer::MassSpectrometer::clsid_text ); 

    // if ( auto scanLaw = document::instance()->scanLaw() ) {
    static std::string this_spectrometer = "d8472724-40dd-4859-a1de-064b4a5e8320"; // "malpix large multum chamber"
        
    adfs::stmt sql( sp.filesystem().db() );
    sql.prepare( "\
INSERT OR REPLACE INTO ScanLaw ( objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer) VALUES ( ?,?,?,?,?,? )" );
    sql.bind( 1 ) = uuid;
    sql.bind( 2 ) = objtext;
    sql.bind( 3 ) = 4000.0; // scanLaw->kAcceleratorVoltage();
    sql.bind( 4 ) = 0.0;    // scanLaw->tDelay();
    sql.bind( 5 ) = this_spectrometer;
    sql.bind( 6 ) = uuid_massspectrometer;
    
    if ( sql.step() != adfs::sqlite_done )
        ADDEBUG() << "sqlite error";

    sql.prepare( "INSERT OR REPLACE INTO Spectrometer ( id, scanType, description, fLength ) VALUES ( ?,?,?,? )" );
    sql.bind( 1 ) = this_spectrometer;
    sql.bind( 2 ) = 0;
    sql.bind( 3 ) = std::string( "MULTUM CHAMBER" );
    sql.bind( 4 ) = 0.5; // 0.5m fLength
    if ( sql.step() != adfs::sqlite_done )
        ADDEBUG() << "sqlite error";
#endif
    return true;
}

bool
document::impl::closingStorage( const boost::uuids::uuid&, adicontroller::SampleProcessor& ) const
{
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
    tdc()->setTofChromatogramsMethod( m );

    std::lock_guard< std::mutex > lock( impl_->mutex_ );    

    auto prev = impl_->tofChromatogramsMethod_;

    if ( ( impl_->tofChromatogramsMethod_ = tdc()->tofChromatogramsMethod() ) ) {

        task::instance()->setTofChromatogramsMethod( *impl_->tofChromatogramsMethod_ );

        if ( impl_->traces_.size() > impl_->tofChromatogramsMethod_->size() ) 
            impl_->traces_.resize( impl_->tofChromatogramsMethod_->size() );

        while ( impl_->traces_.size() < impl_->tofChromatogramsMethod_->size() )
            impl_->traces_.push_back( std::make_shared< adcontrols::Trace >( uint32_t( impl_->traces_.size() ), 8192 - 512, 8192 ) );

        size_t idx( 0 );
        std::for_each( impl_->traces_.begin(), impl_->traces_.end(), [&] ( std::shared_ptr< adcontrols::Trace >& trace ) {
            const auto method = impl_->tofChromatogramsMethod_->begin() + idx++;
            trace->setIsCountingTrace( method->intensityAlgorithm() == method->eCounting );
        } );
    }

}

void
document::addCountingChromatogramsPoint( uint64_t timeSinceEpoch
                                         , uint32_t serialnumber
                                         , const std::vector<uint32_t>& values )
{
    auto injectTime = task::instance()->injectTimeSinceEpoch();

    double seconds = ( timeSinceEpoch - task::instance()->upTimeSinceEpoch() ) * 1.0e-9;
    
    if ( auto method = impl_->tofChromatogramsMethod_ ) {
        
        auto size = std::min( std::min( values.size(), method->size() ), impl_->traces_.size() );
        if ( size ) {
            std::lock_guard< std::mutex > lock( impl_->mutex_ );            
        	for ( uint32_t fcn = 0; fcn < uint32_t( size ); ++fcn ) {
        		auto item = method->begin() + fcn;
                impl_->traces_ [ fcn ]->append( serialnumber, seconds, values [ fcn ] );
                impl_->traces_ [ fcn ]->setIsCountingTrace( true );
        	}
            if ( impl_->traces_[ 0 ]->size() >= 2 )
                emit dataChanged( trace_observer, 1 ); // on right axis
        }
    }
}

void
document::getTraces( std::vector< std::shared_ptr< adcontrols::Trace > >& traces )
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
	traces = impl_->traces_;
}

std::shared_ptr< const adcontrols::MassSpectrometer >
document::massSpectrometer() const
{
    return impl_->massSpectrometer_;
}

