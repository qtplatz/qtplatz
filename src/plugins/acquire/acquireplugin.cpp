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
#include "document.hpp"
#include "mastercontroller.hpp"
#include "mainwindow.hpp"

#if HAVE_CORBA
#include "orb_i.hpp"
#include "orbconnection.hpp"
#include "qbroker.hpp"
#include "receiver_i.hpp"
#endif

#include <acewrapper/constants.hpp>
#include <acewrapper/ifconfig.hpp>
#include <adextension/isnapshothandler.hpp>

#if HAVE_CORBA
# include <adinterface/brokerC.h>
# include <adinterface/controlserverC.h>
# include <adinterface/receiverC.h>
# include <adinterface/signalobserverC.h>
# include <adinterface/observerevents_i.hpp>
# include <adinterface/eventlog_helper.hpp>
# include <adinterface/controlmethodhelper.hpp>
# include <adorbmgr/orbmgr.hpp>
# include <tao/Object.h>
#endif

#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adcontrols/timeutil.hpp>
#include <adextension/imonitorfactory.hpp>
#include <adextension/icontroller.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/date_string.hpp>
#include <adportable/profile.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin_manager/manager.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/orbbroker.hpp>

#include <adportable/debug_core.hpp>
#include <adlog/logging_handler.hpp>
#include <adlog/logger.hpp>

#include <adportable/date_string.hpp>
#include <adportable/fft.hpp>
#include <adportable/debug.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>

#include <qtwrapper/application.hpp>
#include <qtwrapper/qstring.hpp>
#include <servant/servantplugin.hpp>
#include <utils/fancymainwindow.h>

#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/rightpane.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>

#include <QAction>
//#include <QComboBox>
#include <QtCore/qplugin.h>
//#include <QFileDialog>
//#include <QHBoxLayout>
//#include <QBoxLayout>
//#include <QToolButton>
//#include <QLabel>
//#include <QLineEdit>
//#include <QTableWidget>
//#include <QTextEdit>
//#include <QToolButton>
#include <QMessageBox>
//#include <qdebug.h>

#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <future>
#include <map>

using namespace acquire;

namespace acquire {

#if HAVE_CORBA

    class AcquireImpl {
    public:
        
        ~AcquireImpl() {
        }
        
        AcquireImpl() : timePlot_(0), spectrumPlot_(0) {
        }

        // Broker::Session_var brokerSession_;
        // std::unique_ptr< brokerevent_i > brokerEvent_;

        std::map< int, adcontrols::Trace > traces_;
        adplot::ChromatogramWidget * timePlot_;
        adplot::SpectrumWidget * spectrumPlot_;
        QIcon icon_;
        void loadIcon() {
            // icon_.addFile( Constants::ICON_CONNECT );
            // icon_.addFile( Constants::ICON_CONNECT_SMALL );
        }

        // void initialize_broker_session() {
        //     brokerEvent_.reset( new brokerevent_i );
        //     brokerEvent_->assign_message( [=]( const std::string& text ){
        //             handle_message( text );
        //         });
        //     brokerEvent_->assign_portfolio_created( [=]( const std::wstring& file ){
        //             handle_portfolio_created( file );
        //         });
        //     brokerEvent_->assign_folium_added(
        //         [=]( const std::wstring& token
        //              , const std::wstring& path
        //              , const std::wstring& folderId ){
        //             handle_folium_added( token, path, folderId );
        //         });
        //     brokerSession_->connect( "user", "pass", "acquire", brokerEvent_->_this() );
        // }
        
        // void terminate_broker_session() {
        //     // disconnect broker session
        //     if ( !CORBA::is_nil( brokerSession_ ) && brokerEvent_ ) {
        //         brokerSession_->disconnect( brokerEvent_->_this() );
        //         adorbmgr::orbmgr::deactivate( brokerEvent_->_this() );
        //     }
        // }

    protected:
        // void handle_message( const std::string& msg ) {
        //     auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
        //     for ( auto handler: vec )
        //         handler->message( QString( msg.c_str() ) );
        // }

        // void handle_portfolio_created( const std::wstring& token ) {
        //     auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
        //     for ( auto handler: vec )
        //         handler->portfolio_created( qtwrapper::qstring( token ) );
        // }
            
        // void handle_folium_added( const std::wstring& token, const std::wstring& path, const std::wstring& id ) {
        //     auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
        //     for ( auto handler: vec )
        //         handler->folium_added( qtwrapper::qstring( token )
        //                                , qtwrapper::qstring( path ), qtwrapper::qstring( id ) );
        // }
    };
#endif
}

AcquirePlugin::~AcquirePlugin()
{
    orb_i_->shutdown();

    delete orb_i_;
    delete pImpl_;
    ADTRACE() << "====== AcquirePlugin dtor complete ===============";
}

AcquirePlugin::AcquirePlugin() : pImpl_( new AcquireImpl() )
                               , orb_i_( new orb_i() )
                               // , actionConnect_(0)
                               // , actionRun_(0)
                               // , actionInitRun_(0)
                               // , actionStop_(0)
                               // , actionSnapshot_(0)
                               // , actionInject_(0)
                               // , actMethodOpen_(0)
                               // , actMethodSave_(0)
                               // , pConfig_( 0 )
                               //, traceBox_( 0 ) 
                               //, work_( io_service_ )
                               //, strand_( io_service_ )
{
}

// void
// AcquirePlugin::initialize_actions()
// {
//     pImpl_->loadIcon();

//     actionConnect_ = new QAction( QIcon(":/acquire/images/Button Refresh.png"), tr("Connect to control server..."), this);
//     connect( actionConnect_, &QAction::triggered, this, &AcquirePlugin::actionConnect );
  
//     actionInitRun_ = new QAction(QIcon(":/acquire/images/Button Last.png"), tr("Preparing"), this);
//     connect( actionInitRun_, &QAction::triggered, this, &AcquirePlugin::actionInitRun );
  
//     actionRun_ = new QAction(QIcon(":/acquire/images/Button Play.png"), tr("Run"), this);
//     connect( actionRun_, &QAction::triggered, this, &AcquirePlugin::actionRun );
  
//     actionStop_ = new QAction(QIcon(":/acquire/images/Button Stop.png"), tr("Stop"), this);
//     connect( actionStop_, &QAction::triggered, this, &AcquirePlugin::actionStop );
  
//     actionInject_ = new QAction(QIcon(":/acquire/images/Button Add.png"), tr("Inject (recording data)"), this);
//     connect( actionInject_, &QAction::triggered, this, &AcquirePlugin::actionInject );

//     //------------ snapshot -------------
//     actionSnapshot_ = new QAction(QIcon(":/acquire/images/snapshot_small.png"), tr("Take spectrum snapshot"), this);
//     connect( actionSnapshot_, &QAction::triggered, this, &AcquirePlugin::actionSnapshot );
    
//     //------------ method file open/save
//     actMethodOpen_ = new QAction( QIcon( ":/acquire/images/fileopen.png" ), tr( "Method open..." ), this );
//     connect( actMethodOpen_, &QAction::triggered, MainWindow::instance(), &MainWindow::actMethodOpen );

//     actMethodSave_ = new QAction( QIcon( ":/acquire/images/filesave.png" ), tr( "Method save..." ), this );
//     connect( actMethodSave_, &QAction::triggered, MainWindow::instance(), &MainWindow::actMethodSave );

//     actionConnect_->setEnabled( true );
//     actionInitRun_->setEnabled( false );
//     actionRun_->setEnabled( false );
// 	actionStop_->setEnabled( false );
// 	actionInject_->setEnabled( false );
//     actionSnapshot_->setEnabled( true );
  
//     Core::Context context( ( Core::Id( "Acquire.MainView" ), Core::Id( Core::Constants::C_GLOBAL ) ) );

//     if ( auto am = Core::ActionManager::instance() ) {
//         Core::Command * cmd = 0;
//         cmd = am->registerAction( actionConnect_, Constants::CONNECT, context );

//         cmd = am->registerAction( actionInitRun_, Constants::INITIALRUN, context );
//         cmd = am->registerAction( actionRun_, Constants::RUN, context );
//         cmd = am->registerAction( actionStop_, Constants::STOP, context );
//         cmd = am->registerAction( actionInject_, Constants::ACQUISITION, context );
//         cmd = am->registerAction( actionSnapshot_, Constants::SNAPSHOT, context );
//         cmd = am->registerAction( actMethodOpen_, Constants::METHODOPEN, context );
//         cmd = am->registerAction( actMethodSave_, Constants::METHODSAVE, context );
//         (void)cmd;
//     }
// }

bool
AcquirePlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);
    Q_UNUSED(error_message);

    adportable::core::debug_core::instance()->hook( adlog::logging_handler::log );

    do {
        std::wstring apppath = qtwrapper::application::path( L".." ); // := "~/qtplatz/bin/.."
        std::wstring configFile = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/acquire.config" );
        boost::filesystem::path plugindir = boost::filesystem::path( configFile ).branch_path();

        const wchar_t * query = L"/AcquireConfiguration/Configuration";

        //pConfig_ = new adportable::Configuration();

        // if ( ! adportable::ConfigLoader::loadConfigFile( *pConfig_, configFile, query ) ) {
        //     ADWARN() << "AcquirePlugin::initialize loadConfig '" << configFile << "' load failed";
        //     return false;
        // }
    } while(0);
    
    Core::Context context( (Core::Id( "Acquire.MainView" )), (Core::Id( Core::Constants::C_NAVIGATION_PANE )) );

    if ( AcquireMode * mode = new AcquireMode(this) ) {
        mode->setContext( context );
        
        if ( auto mainWindow = MainWindow::instance() ) {

            mainWindow->activateWindow();
            mainWindow->createActions();
            
            //mainWindow->init( *pConfig_ );
            
            mode->setWidget( mainWindow->createContents( mode ) );
            
            addAutoReleasedObject(mode);
            mainWindow->setSimpleDockWidgetArrangement();
        }
    }

    // try {
    //     initialize_actions();
    // } catch ( ... ) {
    //     ADERROR() << "exception handled for initailize_actions: " << boost::current_exception_diagnostic_information();
    // }
    
    // CORBA DEPENDENT
    auto qbroker = new QBroker();
    connect( qbroker, &QBroker::initialized, this, &AcquirePlugin::handle_broker_initialized );
    addObject( qbroker );
    // <--

    if ( auto iExtension = document::instance()->masterController() ) {
        addObject( iExtension );
        connect( iExtension, &adextension::iController::connected, MainWindow::instance(), &MainWindow::iControllerConnected );
    }

    return true;
}

void
AcquirePlugin::extensionsInitialized()
{
    if ( auto mainWindow = MainWindow::instance() ) {

        mainWindow->OnInitialUpdate();
        document::instance()->initialSetup();
        mainWindow->setControlMethod( *document::instance()->controlMethod() );
        mainWindow->setSampleRun( *document::instance()->sampleRun() );

        // gather and initialize control method,time events
        mainWindow->handleControlMethod();
    }
}

void
AcquirePlugin::handle_broker_initialized()
{
    if ( orb_i_ ) {
        orb_i_->initialize();
    }
}

ExtensionSystem::IPlugin::ShutdownFlag
AcquirePlugin::aboutToShutdown()
{
    ADTRACE() << "====== AcquirePlugin shutting down...  ===============";

    document::instance()->actionDisconnect();

    if ( orb_i_ )
        orb_i_->shutdown();

    auto iBroker = ExtensionSystem::PluginManager::instance()->getObject< adextension::iBroker >();
    removeObject( iBroker );

    if ( auto mainWindow = MainWindow::instance() ) {

        document::instance()->finalClose( mainWindow );
        mainWindow->OnFinalClose();
    }

//    io_service_.stop();
//    for ( auto& t: threads_ )
//        t.join();

    ADTRACE() << "====== AcquirePlugin shutdown complete ===============";
    
	return SynchronousShutdown;
}

// void
// AcquirePlugin::actionConnect()
// {
//     // Pressed 'CONNECT' button on Acquire View
//     document::instance()->actionConnect( true ); // fetch method from MainWindow
//     orb_i_->actionConnect();
// }

// void
// AcquirePlugin::populate( SignalObserver::Observer_var& observer )
// {
//     SignalObserver::Description_var topLevelDesc = observer->getDescription();

// #if 0
//     std::string topLevelName = topLevelDesc->trace_display_name.in();
//     traceBox_->addItem( QString::fromStdString( topLevelName ) );

//     SignalObserver::Observers_var children = observer->getSiblings();
//     for ( CORBA::ULong i = 0; i < children->length(); ++i ) {
//         SignalObserver::Description_var secondLevelDesc = children[i]->getDescription();
//         CORBA::String_var secondLevelName = children[i]->getDescription()->trace_display_name.in();
//         traceBox_->addItem( QString( "   %1" ).arg( secondLevelName.in() ) );
//     }
// #endif
// }

// void
// AcquirePlugin::actionDisconnect()
// {
//     orb_i_->actionDisconnect();
// }

// void
// AcquirePlugin::actionInitRun()
// {
//     handleCommitMethods();
//     orb_i_->actionInitRun();
// }

// void
// AcquirePlugin::actionRun()
// {
//     orb_i_->actionRun();
// }

// void
// AcquirePlugin::actionStop()
// {
//     orb_i_->actionStop();
// }

// void
// AcquirePlugin::actionInject()
// {
//     orb_i_->actionInject();
// }

// void
// AcquirePlugin::actionSnapshot()
// {
//     orb_i_->actionSnapshot();
// }

// void
// AcquirePlugin::handle_update_ui_data( unsigned long objId, long pos )
// {
//     std::shared_ptr< adcontrols::MassSpectrum > ms;
//     do {
//         std::lock_guard< std::mutex > lock( mutex_ );
//         if ( ! fifo_ms_.empty() )
//             ms = fifo_ms_.back();
//         fifo_ms_.clear();
//     } while(0);
    
//     if ( ms ) {
//         std::wostringstream o;
//         pImpl_->spectrumPlot_->setData( ms, 0 );
//         double elapsed_time = ms->getMSProperty().timeSinceInjection();

//         o << boost::wformat( L"Elapsed time: %.3f min; " ) % (elapsed_time / 60.0);

//         auto& descs = ms->getDescriptions();
//         for ( auto& d: descs )
//             o << d.text() << L"; ";
//         pImpl_->spectrumPlot_->setTitle( o.str() );
//     }
        
//     do {
//         std::lock_guard< std::mutex > lock( mutex_ );
//         if ( trace_accessors_.find( objId ) == trace_accessors_.end() )
//             return;

//         adcontrols::TraceAccessor& accessor = *trace_accessors_[ objId ];
//         for ( int fcn = 0; fcn < static_cast<int>(accessor.nfcn()); ++fcn ) {
//             if ( pImpl_->traces_.find( fcn ) == pImpl_->traces_.end() )
//                 pImpl_->traces_[ fcn ] = adcontrols::Trace( fcn );
//             adcontrols::Trace& trace = pImpl_->traces_[ fcn ];
//             if ( accessor >> trace && trace.size() >= 2 )
//                 pImpl_->timePlot_->setData( trace, fcn );
//         }
//         accessor.clear();
//     } while ( 0 );

// }

// void
// AcquirePlugin::handle_config_changed( unsigned long objid, long pos )
// {
//     (void)objid;
//     (void)pos;
// }

// void
// AcquirePlugin::handle_method_changed( unsigned long objid, long pos )
// {
//     (void)objid;
//     (void)pos;
// }

// void
// AcquirePlugin::handle_event( unsigned long objid, long pos, long flags )
// {
//     (void)objid;
//     (void)pos;
// }

void
AcquirePlugin::handle_shutdown()
{
    try { 
        MainWindow::instance()->handle_shutdown();
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
        assert( 0 );        
    }
}

void
AcquirePlugin::handle_debug_print( unsigned long priority, unsigned long category, QString text )
{
    try {
        MainWindow::instance()->handle_debug_print( priority, category, text );
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
        assert( 0 );        
    }
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
#if 0
    SignalObserver::Observers_var siblings = orb_i_->observer_->getSiblings();
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
					pImpl_->brokerSession_->coaddSpectrum( path.string().c_str() /* L"acquire" */, tgt, x1, x2 );
                } catch ( std::exception& ex ) {
                    QMessageBox::critical( 0, "acquireplugin::handleRButtonRange", ex.what() );
                }
            }
        }
    }
#endif
}

#if 0
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
            toolBarLayout->setSpacing( 4 );

            auto cmdLayout = new QHBoxLayout();
            // Core::ActionManager *am = Core::ICore::instance()->actionManager();
            if ( auto am = Core::ActionManager::instance() ) {
                cmdLayout->addWidget( toolButton( am->command( Constants::CONNECT )->action() ) );
                cmdLayout->addWidget( toolButton( am->command( Constants::INITIALRUN )->action() ) );
                cmdLayout->addWidget( toolButton( am->command( Constants::RUN )->action() ) );
                cmdLayout->addWidget( toolButton( am->command( Constants::STOP )->action() ) );
                cmdLayout->addWidget( toolButton( am->command( Constants::ACQUISITION )->action() ) );
            }
            toolBarLayout->addLayout( cmdLayout );
            toolBarLayout->addWidget( new Utils::StyledSeparator );

            auto infoLayout = new QHBoxLayout();

            if ( auto edit = new QLineEdit() ) {
                infoLayout->addWidget( new QLabel( tr( "Run name:" ) ) );
                edit->setReadOnly( true );
                infoLayout->addWidget( edit );
                infoLayout->setStretchFactor( edit, 2 );
                connect( acquire::document::instance(), &acquire::document::onSampleRunChanged,
                         [edit] ( const QString& name, const QString& dir ) { edit->setText( name ); } );
            }
            
            if ( auto label = new QLabel( "" ) ) {
                infoLayout->addWidget( label );
                infoLayout->setStretchFactor( label, 1 );
                connect( acquire::document::instance(), &acquire::document::onSampleRunLength,
                         [label] ( const QString& text ) { label->setText( text ); } );
            }
            
            infoLayout->addWidget( new Utils::StyledSeparator );
            if ( auto edit = new QLineEdit() ) {
                infoLayout->addWidget( new QLabel( tr( "Data save in:" ) ) );
                edit->setReadOnly( true );
                infoLayout->addWidget( edit );
                infoLayout->setStretchFactor( edit, 3 );
                connect( acquire::document::instance(), &acquire::document::onSampleRunChanged,
                         [edit] ( const QString& name, const QString& dir ) { edit->setText( dir ); } );
            }
            toolBarLayout->addLayout( infoLayout );
            toolBarLayout->addSpacerItem( new QSpacerItem( 32, 0, QSizePolicy::Expanding ) );
        }

        Utils::StyledBar * toolBar2 = new Utils::StyledBar;
        if ( toolBar2 ) {
            toolBar2->setProperty( "topBorder", true );
            QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar2 );
            toolBarLayout->setMargin(0);
            toolBarLayout->setSpacing(0);

            if ( auto am = Core::ActionManager::instance() ) {
                toolBarLayout->addWidget( toolButton( actionSnapshot_ ) );
                toolBarLayout->addWidget( toolButton( actMethodOpen_ ) );
                toolBarLayout->addWidget( toolButton( actMethodSave_ ) );
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

        QWidget* centralWidget = new QWidget;
        mainWindow_->setCentralWidget( centralWidget );

        Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
        if ( splitter3 ) {
            pImpl_->timePlot_ = new adplot::ChromatogramWidget;
            pImpl_->spectrumPlot_ = new adplot::SpectrumWidget;

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
#endif

// void
// AcquirePlugin::handleCommitMethods()
// {
//     // Update ControlMethod by UI data with individual initial conditions
//     adcontrols::ControlMethod::Method cm;
//     MainWindow::instance()->getControlMethod( cm );

//     auto iControllers = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >();
//     if ( !iControllers.isEmpty() ) {
//         for ( auto& iController : iControllers )
//             iController->preparing_for_run( cm ); // *ptr, os.str().c_str() );
//     }
//     acquire::document::instance()->setControlMethod( cm ); // commit

//     // Update document by UI data
//     adcontrols::SampleRun run;
//     MainWindow::instance()->getSampleRun( run );
//     acquire::document::instance()->setSampleRun( run ); // commit
// }

// void
// AcquirePlugin::handleReceiverMessage( unsigned long msg, unsigned long value )
// {
//     orb_i_->handle_controller_message( msg, value );
// }

Q_EXPORT_PLUGIN( AcquirePlugin )


///////////////////

