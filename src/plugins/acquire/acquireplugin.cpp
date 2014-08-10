// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "acquireplugin.hpp"
#include "acquiremode.hpp"
#include "brokerevent_i.hpp"
#include "constants.hpp"
#include "mainwindow.hpp"
#include "orbconnection.hpp"
#include "qbroker.hpp"
#include "receiver_i.hpp"

#include <acewrapper/constants.hpp>
#include <acewrapper/ifconfig.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adinterface/brokerC.h>
#include <adinterface/controlserverC.h>
#include <adinterface/receiverC.h>
#include <adinterface/signalobserverC.h>
#include <adinterface/observerevents_i.hpp>
#include <adinterface/eventlog_helper.hpp>
#include <adinterface/eventlog_helper.hpp>

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adcontrols/timeutil.hpp>
#include <adorbmgr/orbmgr.hpp>
#include <adextension/imonitorfactory.hpp>
#include <adextension/icontroller.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/date_string.hpp>
#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
#include <adplugin/loader.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/orbbroker.hpp>

#include <adportable/debug_core.hpp>
#include <adlog/logging_handler.hpp>
#include <adlog/logger.hpp>

#include <adportable/date_string.hpp>
#include <adportable/fft.hpp>

#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>

#include <qtwrapper/application.hpp>
#include <qtwrapper/qstring.hpp>
#include <servant/servantplugin.hpp>

#include <tao/Object.h>
#include <utils/fancymainwindow.h>

#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/modemanager.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>

#include <QAction>
#include <QComboBox>
#include <QtCore/qplugin.h>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QLabel>
#else
#include <QtGui/QHBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#endif

#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QMessageBox>
#include <qdebug.h>

#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <fstream>
#include <functional>

using namespace acquire;
using namespace acquire::internal;

namespace acquire {

    namespace internal {

        class AcquireImpl {
        public:
            ~AcquireImpl() {
            }
            AcquireImpl() : timePlot_(0), spectrumPlot_(0) {
            }

            Broker::Session_var brokerSession_;

            std::unique_ptr< brokerevent_i > brokerEvent_;
            std::map< int, adcontrols::Trace > traces_;
            adwplot::ChromatogramWidget * timePlot_;
            adwplot::SpectrumWidget * spectrumPlot_;
            QIcon icon_;
            void loadIcon() {
                icon_.addFile( constants::ICON_CONNECT );
                icon_.addFile( constants::ICON_CONNECT_SMALL );
            }

            void initialize_broker_session() {
                brokerEvent_.reset( new brokerevent_i );
                brokerEvent_->assign_message( [=]( const std::string& text ){
                        handle_message( text );
                    });
                brokerEvent_->assign_portfolio_created( [=]( const std::wstring& file ){
                        handle_portfolio_created( file );
                    });
                brokerEvent_->assign_folium_added(
                    [=]( const std::wstring& token
                         , const std::wstring& path
                         , const std::wstring& folderId ){
                        handle_folium_added( token, path, folderId );
                    });
                brokerSession_->connect( "user", "pass", "acquire", brokerEvent_->_this() );
            }

            void terminate_broker_session() {
                // disconnect broker session
                brokerSession_->disconnect( brokerEvent_->_this() );
                adorbmgr::orbmgr::deactivate( brokerEvent_->_this() );
            }

        protected:
            void handle_message( const std::string& msg ) {
                auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
                for ( auto handler: vec )
                    handler->message( QString( msg.c_str() ) );
            }

            void handle_portfolio_created( const std::wstring& token ) {
                auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
                for ( auto handler: vec )
                    handler->portfolio_created( qtwrapper::qstring( token ) );
            }

            void handle_folium_added( const std::wstring& token, const std::wstring& path, const std::wstring& id ) {
                auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
                for ( auto handler: vec )
                    handler->folium_added( qtwrapper::qstring( token )
                                           , qtwrapper::qstring( path ), qtwrapper::qstring( id ) );
            }
        };

    }
}

// static
QToolButton * 
AcquirePlugin::toolButton( QAction * action )
{
    QToolButton * button = new QToolButton;
    if ( button )
        button->setDefaultAction( action );
    return button;
}

AcquirePlugin::~AcquirePlugin()
{
    shutdown_broker();

    delete mainWindow_;
    delete pImpl_;
    ADTRACE() << "====== AcquirePlugin dtor complete ===============";
}

AcquirePlugin::AcquirePlugin() : mainWindow_(0)
                               , pImpl_( new AcquireImpl() )
                               , actionConnect_(0)
                               , actionRun_(0)
                               , actionInitRun_(0)
                               , actionStop_(0)
                               , actionSnapshot_(0)
                               , actionInject_(0)
                               , pConfig_( 0 )
                               , traceBox_( 0 ) 
                               , work_( io_service_ )
{
}

void
AcquirePlugin::initialize_actions()
{
    pImpl_->loadIcon();

    actionConnect_ = new QAction( QIcon(":/acquire/images/Button Refresh.png"), tr("Connect to control server..."), this);
    connect( actionConnect_, &QAction::triggered, this, &AcquirePlugin::actionConnect );
  
    actionInitRun_ = new QAction(QIcon(":/acquire/images/Button Last.png"), tr("Preparing"), this);
    connect( actionInitRun_, &QAction::triggered, this, &AcquirePlugin::actionInitRun );
  
    actionRun_ = new QAction(QIcon(":/acquire/images/Button Play.png"), tr("Run"), this);
    connect( actionRun_, &QAction::triggered, this, &AcquirePlugin::actionRun );
  
    actionStop_ = new QAction(QIcon(":/acquire/images/Button Stop.png"), tr("Stop"), this);
    connect( actionStop_, &QAction::triggered, this, &AcquirePlugin::actionStop );
  
    actionInject_ = new QAction(QIcon(":/acquire/images/Button Add.png"), tr("Inject (recording data)"), this);
    connect( actionInject_, &QAction::triggered, this, &AcquirePlugin::actionInject );

    //------------ snapshot -------------
    actionSnapshot_ = new QAction(QIcon(":/acquire/images/snapshot_small.png"), tr("Take spectrum snapshot"), this);
    connect( actionSnapshot_, &QAction::triggered, this, &AcquirePlugin::actionSnapshot );

    actionConnect_->setEnabled( true );
    actionInitRun_->setEnabled( false );
    actionRun_->setEnabled( false );
	actionStop_->setEnabled( false );
	actionInject_->setEnabled( false );
    actionSnapshot_->setEnabled( true );
  
    //const AcquireManagerActions& actions = mainWindow_->acquireManagerActions();
    Core::Context globalcontext( ( Core::Id( Core::Constants::C_GLOBAL ) ) );

    if ( auto am = Core::ActionManager::instance() ) {
        Core::Command * cmd = 0;
        cmd = am->registerAction( actionConnect_, constants::CONNECT, globalcontext );
        do {
            //Core::ICore::instance()->modeManager()->addAction( cmd, 90 );
        } while(0);

        cmd = am->registerAction( actionInitRun_, constants::INITIALRUN, globalcontext );
        cmd = am->registerAction( actionRun_, constants::RUN, globalcontext );
        cmd = am->registerAction( actionStop_, constants::STOP, globalcontext );
        cmd = am->registerAction( actionInject_, constants::ACQUISITION, globalcontext );
        cmd = am->registerAction( actionSnapshot_, constants::SNAPSHOT, globalcontext );
    }
}

bool
AcquirePlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);
    Q_UNUSED(error_message);

    adportable::core::debug_core::instance()->hook( adlog::logging_handler::log );

    Core::Context context( (Core::Id( "Acquire.MainView" )), (Core::Id( Core::Constants::C_NAVIGATION_PANE )) );

    AcquireMode * mode = new AcquireMode(this);
    if ( mode )
        mode->setContext( context );
    else
        return false;

    std::wstring apppath = qtwrapper::application::path( L".." ); // := "~/qtplatz/bin/.."
    std::wstring configFile = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/acquire.config" );
    boost::filesystem::path plugindir = boost::filesystem::path( configFile ).branch_path();

    const wchar_t * query = L"/AcquireConfiguration/Configuration";

    pConfig_ = new adportable::Configuration();

    if ( ! adportable::ConfigLoader::loadConfigFile( *pConfig_, configFile, query ) ) {
        ADWARN() << "AcquirePlugin::initialize loadConfig '" << configFile << "' load failed";
        return false;
    }

    mainWindow_ = new MainWindow(0);
    if ( mainWindow_ )
        mainWindow_->init( *pConfig_ );

    try {
        initialize_actions();
    } catch ( ... ) {
        ADERROR() << "exception handled for initailize_actions: " << boost::current_exception_diagnostic_information();
    }

    auto qbroker = new QBroker();
    connect( qbroker, &QBroker::initialized, this, &AcquirePlugin::handle_broker_initialized );
    addObject( qbroker );

    // try {
    //     // initialize_broker();
    //     OrbConnection::instance()->initialize();
    //     Broker::Manager_var mgr = OrbConnection::instance()->brokerManager();
    // } catch ( ... ) {
    //     ADERROR() << "exception handled for initailize_broker: " << boost::current_exception_diagnostic_information();
    // }

    mode->setWidget( createContents( mode ) );

    mainWindow_->setSimpleDockWidgetArrangement();
    addAutoReleasedObject(mode);

  return true;
}

void
AcquirePlugin::extensionsInitialized()
{
	QList< adextension::iMonitorFactory * > monitors = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iMonitorFactory >();
	std::for_each( monitors.begin(), monitors.end(), [&]( adextension::iMonitorFactory * factory ){
            mainWindow_->addMonitorWidget( factory->create(), factory->title() );
        });
	mainWindow_->OnInitialUpdate();
}

void
AcquirePlugin::handle_broker_initialized()
{
    Broker::Manager_var mgr = OrbConnection::instance()->brokerManager();
    if ( CORBA::is_nil( mgr ) )
        return;

    pImpl_->brokerSession_ = mgr->getSession( L"acquire" );
    if ( CORBA::is_nil( pImpl_->brokerSession_ ) )
        return;
    
    pImpl_->initialize_broker_session();
    threads_.push_back( std::thread( boost::bind( &boost::asio::io_service::run, &io_service_ ) ) );
}

ExtensionSystem::IPlugin::ShutdownFlag
AcquirePlugin::aboutToShutdown()
{
    ADTRACE() << "====== AcquirePlugin shutting down...  ===============";

    actionDisconnect();
    pImpl_->terminate_broker_session();
    mainWindow_->OnFinalClose();

    io_service_.stop();
    for ( std::thread& t: threads_ )
        t.join();

    ADTRACE() << "====== AcquirePlugin shutdown complete ===============";
    
	return SynchronousShutdown;
}

void
AcquirePlugin::actionConnect()
{
    // using adinterface::EventLog::LogMessageHelper;
    auto iControllers = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >();
    if ( !iControllers.isEmpty() ) {
        for ( auto& iController : iControllers ) {
            iController->wait_for_connection_ready();
        }
    }

    if ( CORBA::is_nil( session_.in() ) ) { //&& !orbServants_.empty() ) {

        Broker::Manager_var broker = OrbConnection::instance()->brokerManager(); // Broker::Manager::_narrow( orbServants_[ 0 ]->_this() );
        if ( ! CORBA::is_nil( broker ) ) {
            using namespace acewrapper::constants;
            CORBA::Object_var obj = broker->find_object( adcontroller::manager::_name() );
            ::ControlServer::Manager_var manager = ::ControlServer::Manager::_narrow( obj );
            if ( !CORBA::is_nil( manager ) ) {
                session_ = manager->getSession( L"acquire" );
                if ( !CORBA::is_nil( session_.in() ) ) {
                    
                    receiver_i_.reset( new receiver_i );

                    receiver_i_->assign_message( [=] ( ::Receiver::eINSTEVENT code, uint32_t value ){
                        this->handle_receiver_message( code, value ); } );

                    receiver_i_->assign_log( [=] ( const ::EventLog::LogMessage& log ){
                        this->handle_receiver_log( log ); } );

                    receiver_i_->assign_shutdown( [=](){
                        this->handle_receiver_shutdown(); } );

                    receiver_i_->assign_debug_print( [=]( int32_t pri, int32_t cat, std::string text ){ 
                        this->handle_receiver_debug_print( pri, cat, text ); } );

                    connect( this
                        , SIGNAL( onReceiverMessage( unsigned long, unsigned long ) ), this, SLOT( handle_message( unsigned long, unsigned long ) ) );
                        
                    if ( session_->connect( receiver_i_->_this(), "acquire" ) )
                        actionConnect_->setEnabled( false );
                        
                    if ( session_->status() <= ControlServer::eConfigured )
                        session_->initialize();

                    // Master signal observer
                    observer_ = session_->getObserver();
                    if ( !CORBA::is_nil( observer_.in() ) ) {
                        if ( !sink_ ) {
                            sink_.reset( new adinterface::ObserverEvents_i );

                            sink_->assignConfigChanged( [=] ( uint32_t oid, SignalObserver::eConfigStatus st ){
                                this->handle_observer_config_changed( oid, st );
                            } );

                            sink_->assignUpdateData( [=] ( uint32_t oid, int32_t pos ){
                                this->handle_observer_update_data( oid, pos );
                            } );

                            sink_->assignMethodChanged( [=] ( uint32_t oid, int32_t pos ){
                                this->handle_observer_method_changed( oid, pos );
                            } );
                            sink_->assignEvent( [=] ( uint32_t oid, int32_t pos, int32_t ev ){
                                this->handle_observer_event( oid, pos, ev );
                            } );

                            observer_->connect( sink_->_this(), SignalObserver::Frequent, "acquireplugin" );
                                
                            connect( this, SIGNAL( onObserverConfigChanged( unsigned long, long ) )
                                , this, SLOT( handle_config_changed( unsigned long, long ) ) );
                            connect( this, SIGNAL( onUpdateUIData( unsigned long, long ) )
                                , this, SLOT( handle_update_ui_data( unsigned long, long ) ) );
                            connect( this, SIGNAL( onObserverMethodChanged( unsigned long, long ) )
                                , this, SLOT( handle_method_changed( unsigned long, long ) ) );
                            connect( this, SIGNAL( onObserverEvent( unsigned long, long, long ) )
                                , this, SLOT( handle_event( unsigned long, long, long ) ) );
                        }
                        
                        // connect to 1st layer siblings ( := top shadow(cache) observer for each instrument )
                        SignalObserver::Observers_var siblings = observer_->getSiblings();
                        size_t nsize = siblings->length();

                        for ( CORBA::ULong i = 0; i < nsize; ++i ) {
                            SignalObserver::Observer_var var = SignalObserver::Observer::_duplicate( siblings[ i ] );
                            populate( var );
                        }
                    }
                }
            }
        }
    }

    if ( ! CORBA::is_nil( session_ ) ) {
        actionInitRun_->setEnabled( true );
        actionRun_->setEnabled( true );
        ADTRACE() << "adcontroller status: " << session_->status();
    }
}

void
AcquirePlugin::populate( SignalObserver::Observer_var& observer )
{
    SignalObserver::Description_var topLevelDesc = observer->getDescription();

    std::wstring topLevelName = topLevelDesc->trace_display_name.in();
    traceBox_->addItem( qtwrapper::qstring( topLevelName ) );

    SignalObserver::Observers_var children = observer->getSiblings();
    for ( CORBA::ULong i = 0; i < children->length(); ++i ) {
        SignalObserver::Description_var secondLevelDesc = children[i]->getDescription();
        CORBA::WString_var secondLevelName = children[i]->getDescription()->trace_display_name.in();
        traceBox_->addItem( qtwrapper::qstring( L"   " + std::wstring( secondLevelName ) ) );
    }
}

void
AcquirePlugin::actionDisconnect()
{
    if ( ! CORBA::is_nil( session_ ) ) {

        // disconnect from observer
        observer_ = session_->getObserver();
        if ( ! CORBA::is_nil( observer_.in() ) ) {
            observer_->disconnect( sink_->_this() );
			adorbmgr::orbmgr::deactivate( sink_->_this() );
        }

        // disconnect instrument control session
        session_->disconnect( receiver_i_.get()->_this() );
        adorbmgr::orbmgr::deactivate( receiver_i_->_this() );
    }
}
    
void
AcquirePlugin::actionInitRun()
{
    if ( ! CORBA::is_nil( session_ ) ) {
        ControlMethod::Method m;
        session_->prepare_for_run( m );
        ADTRACE() << "adcontroller status: " << session_->status();
    }
}

void
AcquirePlugin::actionRun()
{
    if ( ! CORBA::is_nil( session_ ) ) {
        session_->start_run();
        ADTRACE() << "adcontroller status: " << session_->status();
    }
}

void
AcquirePlugin::actionStop()
{
    if ( ! CORBA::is_nil( session_ ) ) {
        session_->stop_run();
        ADTRACE() << "adcontroller status: " << session_->status();
    }
}

void
AcquirePlugin::actionInject()
{
    if ( ! CORBA::is_nil( session_ ) ) {
		session_->event_out( ControlServer::event_InjectOut ); // simulate inject out to modules
        ADTRACE() << "adcontroller status: " << session_->status();
    }
}

void
AcquirePlugin::actionSnapshot()
{
    if ( CORBA::is_nil( observer_ ) )
        return;
    SignalObserver::Observers_var siblings = observer_->getSiblings();
    for ( CORBA::ULong  i = 0; i < siblings->length(); ++i ) {
        SignalObserver::Description_var desc = siblings[i]->getDescription();
        if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA ) {
            CORBA::ULongLong first, second;
            siblings[i]->uptime_range( first, second );
            double m1 = double(second) / 60.0e6;
            double m0 = double(second - 1000) / 60.0e6;  // 1s before
			selectRange( m0, m1, 0, 0 );
        }
    }
}

bool
AcquirePlugin::readMassSpectra( const SignalObserver::DataReadBuffer& rb
                                , const adcontrols::MassSpectrometer& spectrometer
                                , const adcontrols::DataInterpreter& dataInterpreter
                                , unsigned long objid )
{
    if ( ! rdmap_[ objid ] )
        rdmap_[ objid ].reset( new adcontrols::MassSpectrum );

    adcontrols::MassSpectrum& ms = *rdmap_[ objid ];

    size_t idData = 0;
    adcontrols::translate_state state = 
        dataInterpreter.translate( ms
                                   , reinterpret_cast< const char *>(rb.xdata.get_buffer()), rb.xdata.length()
                                   , reinterpret_cast< const char *>(rb.xmeta.get_buffer()), rb.xmeta.length()
                                   , spectrometer, idData++, 0 );
    if ( state == adcontrols::translate_complete ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        fifo_ms_.push_back( rdmap_[ objid ] );
        rdmap_[ objid ].reset( new adcontrols::MassSpectrum );
    } 
    return state != adcontrols::translate_error;
}

bool
AcquirePlugin::readTrace( const SignalObserver::Description& desc
                         , const SignalObserver::DataReadBuffer& rb
                         , const adcontrols::DataInterpreter& dataInterpreter
                          , unsigned long objid )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( trace_accessors_.find( objid ) == trace_accessors_.end() )
        trace_accessors_[ objid ] = std::shared_ptr< adcontrols::TraceAccessor >( new adcontrols::TraceAccessor );

    adcontrols::TraceAccessor& accessor = *trace_accessors_[ objid ];
    if ( dataInterpreter.translate( accessor
									, reinterpret_cast< const char *>( rb.xdata.get_buffer() ), rb.xdata.length()
									, reinterpret_cast< const char * >( rb.xmeta.get_buffer() ), rb.xmeta.length()
                                    , rb.events ) == adcontrols::translate_complete ) {
        return true;
    }
    return false;
}

void
AcquirePlugin::handle_update_ui_data( unsigned long objId, long pos )
{
    std::shared_ptr< adcontrols::MassSpectrum > ms;
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( ! fifo_ms_.empty() )
            ms = fifo_ms_.back();
        fifo_ms_.clear();
    } while(0);
    if ( ms )
        pImpl_->spectrumPlot_->setData( ms, 0 );
        
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( trace_accessors_.find( objId ) == trace_accessors_.end() )
            return;
        adcontrols::TraceAccessor& accessor = *trace_accessors_[ objId ];
        for ( int fcn = 0; fcn < static_cast<int>(accessor.nfcn()); ++fcn ) {
            if ( pImpl_->traces_.find( fcn ) == pImpl_->traces_.end() )
                pImpl_->traces_[ fcn ] = adcontrols::Trace( fcn );
            adcontrols::Trace& trace = pImpl_->traces_[ fcn ];
            if ( accessor >> trace && trace.size() >= 2 )
                pImpl_->timePlot_->setData( trace, fcn );
        }
        accessor.clear();
    } while ( 0 );

}

bool
AcquirePlugin::readCalibrations( observer_type& obs )
{
    CORBA::WString_var dataClass;
    SignalObserver::Observer_ptr tgt = std::get<0>( obs ).in();
    auto spectrometer = std::get<4>( obs );

    SignalObserver::octet_array_var data;

    bool success = false;
    CORBA::ULong idx = 0;
    while ( tgt->readCalibration( idx++, data, dataClass ) ) {
        if ( std::wcscmp( dataClass, adcontrols::MSCalibrateResult::dataClass() ) == 0 ) {
            adcontrols::MSCalibrateResult result;
            if ( adportable::serializer< adcontrols::MSCalibrateResult >::deserialize(
                     result, reinterpret_cast< const char *>(data->get_buffer()), data->length() ) )
                success = true;
            if ( spectrometer )
                spectrometer->setCalibration( idx - 1, result );
        }
    }
    return success;
}

void
AcquirePlugin::handle_update_data( unsigned long objId, long pos )
{
    ACE_UNUSED_ARG( pos );

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        
        if ( observerMap_.find( objId ) == observerMap_.end() ) {
            SignalObserver::Observer_var tgt = observer_->findObserver( objId, true );
            if ( CORBA::is_nil( tgt.in() ) )
                return;

            CORBA::WString_var name = tgt->dataInterpreterClsid();
            if ( auto spectrometer = adcontrols::MassSpectrometer::create( name.in() ))  {

                SignalObserver::Description_var desc = tgt->getDescription();
                observerMap_[ objId ] = std::make_tuple( tgt, desc, name.in(), false, spectrometer );
                npos_map_[ objId ] = pos;
            } else {
                ADTRACE() << "receive data from unavilable spectrometer: " << name.in();
                return;
            }
        }
    } while (0);

    long& npos = npos_map_[ objId ];

    if ( pos < npos )
        return;

    auto it = observerMap_.find( objId );
    SignalObserver::Observer_ptr tgt = std::get<0>( it->second ).in();
    SignalObserver::Description_var& desc = std::get<1>( it->second );
    auto spectrometer = std::get<4>( it->second );

    const adcontrols::DataInterpreter& dataInterpreter = spectrometer->getDataInterpreter();

    if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA ) {
        if ( ! std::get<3>( it->second ) )
            std::get<3>( it->second ) = readCalibrations( it->second );
        
        try {
            SignalObserver::DataReadBuffer_var rb;
            while ( tgt->readData( npos, rb ) ) {
                ++npos;
                readMassSpectra( rb, *spectrometer, dataInterpreter, objId );
            }
            emit onUpdateUIData( objId, pos );
        } catch ( CORBA::Exception& ex ) {
            ADTRACE() << "handle_update_data got an corba exception: " << ex._info().c_str();
        }

    } else if ( desc->trace_method == SignalObserver::eTRACE_TRACE ) {
        try {
            SignalObserver::DataReadBuffer_var rb;
            while ( tgt->readData( npos, rb ) ) {
                npos = rb->pos + rb->ndata;
                readTrace( desc, rb, dataInterpreter, objId );
                emit onUpdateUIData( objId, pos );
                return;
            }
        } catch ( CORBA::Exception& ex ) {
            ADTRACE() << "handle_update_data got an corba exception: " << ex._info().c_str();
        }
    }
}

void
AcquirePlugin::handle_config_changed( unsigned long objid, long pos )
{
    (void)objid;
    (void)pos;
}

void
AcquirePlugin::handle_method_changed( unsigned long objid, long pos )
{
    (void)objid;
    (void)pos;
}

void
AcquirePlugin::handle_event( unsigned long objid, long pos, long flags )
{
    (void)objid;
    (void)pos;
}

void
AcquirePlugin::handle_message( unsigned long /* Receiver::eINSTEVENT */ msg, unsigned long value )
{

    using namespace ControlServer;

    if ( msg == Receiver::STATE_CHANGED ) {

		ADTRACE() << "handle_message( STATE_CHANGED, " << value << ")";

        eStatus status = eStatus( value );
        if ( status == eWaitingForContactClosure ) {
            actionInject_->setEnabled( true );
            actionStop_->setEnabled( true );
            actionRun_->setEnabled( false );
        } else if ( status == ePreparingForRun ) {
            actionStop_->setEnabled( false );
        } else if ( status == eReadyForRun ) {
            actionStop_->setEnabled( false );
			actionRun_->setEnabled( true );
        } else if ( status == eRunning ) {
            actionStop_->setEnabled( true );
			actionInject_->setEnabled( false );
			actionRun_->setEnabled( false );
        }
    }
}

void
AcquirePlugin::handle_shutdown()
{
    mainWindow_->handle_shutdown();
}

void
AcquirePlugin::handle_debug_print( unsigned long priority, unsigned long category, QString text )
{
    mainWindow_->handle_debug_print( priority, category, text );
}

void
AcquirePlugin::handle_monitor_selected(int)
{
}

void
AcquirePlugin::handle_monitor_activated(int)
{
}

void
AcquirePlugin::handleSelected( const QPointF& pt )
{
	selectRange( pt.x(), pt.x(), pt.y(), pt.y() );
}

void
AcquirePlugin::handleSelected( const QRectF& rc )
{
	selectRange( rc.x(), rc.x() + rc.width(), rc.y(), rc.y() + rc.height() );
}

void
AcquirePlugin::selectRange( double x1, double x2, double y1, double y2 )
{
    (void)y1; (void)y2;

    SignalObserver::Observers_var siblings = observer_->getSiblings();
    CORBA::ULong nsize = siblings->length();

    for ( CORBA::ULong i = 0; i < nsize; ++i ) {
        SignalObserver::Description_var desc = siblings[i]->getDescription();
        
        if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA 
             && desc->spectrometer == SignalObserver::eMassSpectrometer ) {

            SignalObserver::Observer_var tgt = SignalObserver::Observer::_duplicate( siblings[i] );

            if ( pImpl_ && ! CORBA::is_nil( pImpl_->brokerSession_ ) ) {
				boost::filesystem::path path( adportable::profile::user_data_dir<char>() );
				path /= "data";
				path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
				if ( ! boost::filesystem::exists( path ) ) {
					boost::system::error_code ec;
					boost::filesystem::create_directories( path, ec );
				}
				path /= "acquire.adfs";
				
                try {
					pImpl_->brokerSession_->coaddSpectrum( path.wstring().c_str() /* L"acquire" */, tgt, x1, x2 );
                } catch ( std::exception& ex ) {
                    QMessageBox::critical( 0, "acquireplugin::handleRButtonRange", ex.what() );
                }
            }
        }
    }
}

void
AcquirePlugin::handle_observer_config_changed( uint32_t objid, SignalObserver::eConfigStatus st )
{
    emit onObserverConfigChanged( objid, long( st ) );
}

void
AcquirePlugin::handle_observer_update_data( uint32_t objid, int32_t pos )
{
    io_service_.post( std::bind(&AcquirePlugin::handle_update_data, this, objid, pos ) );    
}

void
AcquirePlugin::handle_observer_method_changed( uint32_t objid, int32_t pos )
{
    emit onObserverMethodChanged( objid, pos );
}

void
AcquirePlugin::handle_observer_event( uint32_t objid, int32_t pos, int32_t events )
{
    emit onObserverEvent( objid, pos, events );
}

void
AcquirePlugin::handle_receiver_message( Receiver::eINSTEVENT code, uint32_t value )
{
    emit onReceiverMessage( static_cast<unsigned long>(code), value );
    // handle_message
}

void
AcquirePlugin::handle_receiver_log( const ::EventLog::LogMessage& log )
{
    std::wstring text = adinterface::EventLog::LogMessageHelper::toString( log );
    std::string date = adportable::date_string::utc_to_localtime_string( log.tv.sec, log.tv.usec ) + "\t";
    QString qtext = date.c_str();
    qtext += qtwrapper::qstring::copy( text );
    mainWindow_->eventLog( qtext );
}

void
AcquirePlugin::handle_receiver_shutdown()
{
    // handle_shutdown
}

void
AcquirePlugin::handle_receiver_debug_print( int32_t, int32_t, std::string text )
{
    QString qtext( text.c_str() );
    mainWindow_->eventLog( qtext );
}

#if 0
void
AcquirePlugin::initialize_broker()
{
    adplugin::orbServant * adBroker = 0;
    adplugin::orbBroker * orbBroker = 0;

    adplugin::plugin_ptr adbroker_plugin = adplugin::loader::select_iid( ".*\\.orbfactory\\.adbroker" );
    if ( adbroker_plugin ) {

        if ( ( orbBroker = adbroker_plugin->query_interface< adplugin::orbBroker >() ) ) {

            try {
                orbBroker->orbmgr_init( 0, 0 );
            }
            catch ( ... ) {
                ADERROR() << "orbBroker raize an exception: " << boost::current_exception_diagnostic_information();
                QMessageBox::warning( 0, "AcquirePlugin", QString::fromStdString( boost::current_exception_diagnostic_information() ) );
                return;
            }

            try { 

                if ( ( adBroker = orbBroker->create_instance() ) ) {
                    orbServants_.push_back( adBroker );
                    Broker::Manager_var mgr = Broker::Manager::_narrow( adBroker->_this() );
                    addObject ( new QBroker( mgr._retn() ) );
                }

            } catch ( ... ) {
                ADERROR() << "orbBroker raize an exception: " << boost::current_exception_diagnostic_information();
				QMessageBox::warning( 0, "AcquirePlugin", QString::fromStdString( boost::current_exception_diagnostic_information() ) );
                return;
            }
        }
    }
    if ( adBroker == 0 || orbBroker == 0 ) {
        ADTRACE() << "adBroker does not initialized (it might be not configured)";
        return;
    }


    // ----------------------- initialize corba servants ------------------------------
    std::vector< adplugin::plugin_ptr > factories;
    adplugin::loader::select_iids( ".*\\.adplugins\\.orbfactory\\..*", factories );
    for ( const adplugin::plugin_ptr& plugin: factories ) {

        ADTRACE() << "initializing " << plugin->clsid() << "{iid: " << plugin->iid() << "}";
        
        if ( plugin->iid() == adbroker_plugin->iid() ) // skip "adBroker"
            continue;
        
        try {

            if ( adplugin::orbServant * servant = (*orbBroker)( plugin.get() ) )
                orbServants_.push_back( servant );

        } catch ( ... ) {

            ADERROR() << "exception while initializing " << plugin->clsid() << "\t" << boost::current_exception_diagnostic_information();
            QMessageBox::warning( 0, "Exception AcquirePlugin"
                                  , "If you are on Windows 7 sp1, some of important Windows update is getting involved. \
Please make sure you have up-to-date for Windows");
        }
    }
}
#endif

void
AcquirePlugin::shutdown_broker()
{
    ADTRACE() << "AcquirePlugin::shutdown_broker() ...";

    auto iBroker = ExtensionSystem::PluginManager::instance()->getObject< adextension::iBroker >();
	removeObject( iBroker );

    OrbConnection::instance()->shutdown();
#if 0
    // destriction must be reverse order
    for ( orbservant_vector_type::reverse_iterator it = orbServants_.rbegin(); it != orbServants_.rend(); ++it )
        (*it)->deactivate();

	if ( adplugin::plugin_ptr adbroker_plugin = adplugin::loader::select_iid( ".*\\.orbfactory\\.adbroker" ) ) {

        if ( adplugin::orbBroker * orbBroker = adbroker_plugin->query_interface< adplugin::orbBroker >() ) {
            
            try {

                orbBroker->orbmgr_shutdown();
                orbBroker->orbmgr_fini();
                orbBroker->orbmgr_wait();

            } catch ( boost::exception& ex ) {
                ADERROR() << boost::diagnostic_information( ex );
            }
            
        }
    }
#endif
    ADTRACE() << "AcquirePlugin::shutdown_broker() completed.";
}

QWidget *
AcquirePlugin::createContents( Core::IMode * mode )
{
    
        //              [mainWindow]
        // splitter> ---------------------
        //              [OutputPane]
  
        Core::MiniSplitter * splitter = new Core::MiniSplitter;
        if ( splitter ) {
            splitter->addWidget( mainWindow_ );
            splitter->addWidget( new Core::OutputPanePlaceHolder( mode ) );

            splitter->setStretchFactor( 0, 10 );
            splitter->setStretchFactor( 1, 0 );
            splitter->setOrientation( Qt::Vertical ); // horizontal splitter bar
        }

        //
        //         <splitter2>         [mainWindow]
        // [Navigation] | [splitter ------------------- ]
        //                             [OutputPane]

        Core::MiniSplitter * splitter2 = new Core::MiniSplitter;
        if ( splitter2 ) {
            splitter2->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
            splitter2->addWidget( splitter );
            splitter2->setStretchFactor( 0, 0 );
            splitter2->setStretchFactor( 1, 1 );
        }
      
        Utils::StyledBar * toolBar = new Utils::StyledBar;
        if ( toolBar ) {
            toolBar->setProperty( "topBorder", true );
            QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
            toolBarLayout->setMargin(0);
            toolBarLayout->setSpacing(0);
            // Core::ActionManager *am = Core::ICore::instance()->actionManager();
            if ( auto am = Core::ActionManager::instance() ) {
                toolBarLayout->addWidget(toolButton(am->command(constants::CONNECT)->action()));
                toolBarLayout->addWidget(toolButton(am->command(constants::INITIALRUN)->action()));
                toolBarLayout->addWidget(toolButton(am->command(constants::RUN)->action()));
                toolBarLayout->addWidget(toolButton(am->command(constants::STOP)->action()));
                toolBarLayout->addWidget(toolButton(am->command(constants::ACQUISITION)->action()));
            }
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("Sequence:") ) );
        }
        Utils::StyledBar * toolBar2 = new Utils::StyledBar;
        if ( toolBar2 ) {
            toolBar2->setProperty( "topBorder", true );
            QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar2 );
            toolBarLayout->setMargin(0);
            toolBarLayout->setSpacing(0);
            //Core::ActionManager *am = Core::ICore::instance()->actionManager();
            if ( auto am = Core::ActionManager::instance() ) {
                toolBarLayout->addWidget( toolButton( actionSnapshot_ ) );
                toolBarLayout->addWidget( new Utils::StyledSeparator );
                toolBarLayout->addWidget( new QLabel( tr("Traces:") ) );
                traceBox_ = new QComboBox;
                traceBox_->addItem( "-----------------------------" );
                connect( traceBox_, SIGNAL( currentIndexChanged(int) ), this, SLOT( handle_monitor_selected(int) ) );
                connect( traceBox_, SIGNAL( activated(int) ), this, SLOT( handle_monitor_activated(int) ) );
                toolBarLayout->addWidget( traceBox_ );
                toolBarLayout->addWidget( new QLabel( tr("  ") ), 10 );
            }
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("Threads:") ) );
        }

        /*
        //  [TraceWidget] | [RightPanePlaceHolder]
        Core::MiniSplitter * rightPaneSplitter = new Core::MiniSplitter;
        if ( rightPaneSplitter ) {
        rightPaneSplitter->addWidget( new adwidgets::TraceWidget );
        //rightPaneHSplitter->addWidget( new Core::RightPanePlaceHolder( mode ) );
        rightPaneSplitter->addWidget( new QTextEdit( "RightPanePlaceHolder" ) );
        rightPaneSplitter->setStretchFactor( 0, 1 );
        rightPaneSplitter->setStretchFactor( 1, 0 );
        }
        */

        QWidget* centralWidget = new QWidget;
        mainWindow_->setCentralWidget( centralWidget );

        Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
        if ( splitter3 ) {
            pImpl_->timePlot_ = new adwplot::ChromatogramWidget;
            pImpl_->spectrumPlot_ = new adwplot::SpectrumWidget;

            splitter3->addWidget( pImpl_->timePlot_ );
            splitter3->addWidget( pImpl_->spectrumPlot_ );
            splitter3->setOrientation( Qt::Vertical );

			bool res;
			res = connect( pImpl_->timePlot_, SIGNAL( onSelected( const QPointF& ) ), this, SLOT( handleSelected( const QPointF& ) ) );
			assert( res );
			res = connect( pImpl_->timePlot_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( handleSelected( const QRectF& ) ) );
			assert( res );
        }

        QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
        toolBarAddingLayout->setMargin(0);
        toolBarAddingLayout->setSpacing(0);
        //toolBarAddingLayout->addWidget( rightPaneSplitter );
        toolBarAddingLayout->addWidget( toolBar );
        toolBarAddingLayout->addWidget( splitter3 );
        toolBarAddingLayout->addWidget( toolBar2 );

        return splitter2;
}


Q_EXPORT_PLUGIN( AcquirePlugin )


///////////////////

