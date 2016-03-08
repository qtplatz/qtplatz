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

#include "mainwindow.hpp"
#include "constants.hpp"
#include "document.hpp"
#include "masterobserver.hpp"
#include "waveformwnd.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adextension/icontroller.hpp>
#include <adextension/isequence.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adicontroller/instrument.hpp>
#include <adicontroller/receiver.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplugin/constants.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycleaccessor.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/string.hpp>
#include <adportable/utf.hpp>
#include <adlog/logger.hpp>
#include <adwidgets/cherrypicker.hpp>
#include <adwidgets/controlmethodwidget.hpp>
#include <adwidgets/insttreeview.hpp>
#include <adwidgets/samplerunwidget.hpp>
#include <qtwrapper/qstring.hpp>

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/variant.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
#include <QApplication>
#include <QAction>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMetaType>
#include <QToolBar>
#include <QToolButton>
#include <QTextEdit>
#include <QTabBar>
#include <QPluginLoader>
#include <QLibrary>
#include <QtCore>
#include <QUrl>

Q_DECLARE_METATYPE( boost::uuids::uuid );

namespace acquire {

    class MainWindow::impl {
    public:
        impl() : cmEditor_( new adwidgets::ControlMethodWidget )
               , runEditor_( new adwidgets::SampleRunWidget )
               , traceBox_( 0 ) {
        }

        ~impl() {
            runEditor_.reset();
            cmEditor_.reset();
        }

        static MainWindow * instance_; // dtor will be called from Untils::FancyMainWindow

        std::set< QString > config_names_;

        std::unique_ptr< adwidgets::ControlMethodWidget > cmEditor_;
        std::unique_ptr< adwidgets::SampleRunWidget > runEditor_;
        QComboBox * traceBox_;
    };

    MainWindow * MainWindow::impl::instance_( 0 );
}

using namespace acquire;

MainWindow * 
MainWindow::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [](){ impl::instance_ = new MainWindow(); } );
    return impl::instance_;
}


MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , impl_( new impl() )
{
    connect( impl_->cmEditor_.get(), &adwidgets::ControlMethodWidget::onImportInitialCondition, this, &MainWindow::handleControlMethod );
}

size_t 
MainWindow::findInstControllers( std::vector< std::shared_ptr< adextension::iController > >& vec ) const
{
    for ( auto v : ExtensionSystem::PluginManager::getObjects< adextension::iController >() ) {
        ADDEBUG() << v->module_name().toStdString();
        try {
            vec.push_back( v->shared_from_this() );
        } catch ( std::bad_weak_ptr& ) {
#if defined _DEBUG || defined DEBUG
            ADDEBUG() << "adextension::iController does not inherit from shared_ptr";
#endif
            // ignore old iController implementation
        }
    }
    return vec.size();
}

void
MainWindow::init( const adportable::Configuration& config )
{
}

void
MainWindow::OnInitialUpdate()
{
	setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    setDocumentMode( true );
    
    impl_->cmEditor_->OnInitialUpdate();
    impl_->runEditor_->OnInitialUpdate();
    
    // then, series of individual control method widgets
    auto sequences = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSequence >();
    for ( auto s : sequences )
        document::instance()->addConfiguration( s->configuration() );
    
    if ( auto combo = findChild< QComboBox * >( "Configuration" ) ) {
        auto list = document::instance()->configurations();
        if ( ! list.empty() ) {
            {
                QSignalBlocker block( combo ); // prevent call of currentChanged
                for ( const auto& name : list )
                    combo->addItem( name );
            }
            if ( document::instance()->currentConfiguration().isEmpty() )
                document::instance()->setCurrentConfiguration( *list.begin() );
            auto it = std::find( list.begin(), list.end(), document::instance()->currentConfiguration() );
            if ( it == list.end() ) {
                document::instance()->setCurrentConfiguration( *list.begin() );
                combo->setCurrentIndex( 0 );
            } else {
                document::instance()->setCurrentConfiguration( *it );
                combo->setCurrentIndex( int( std::distance( list.begin(), it ) ) );
            }
        }
    }


	for ( auto s: sequences ) {

        for ( size_t i = 0; i < s->size(); ++i ) {

            QString objname = QString("config/%1").arg( s->configuration() );
            
            const adextension::iEditorFactory& factory = ( *s )[ i ];
            if ( factory.method_type() == adextension::iEditorFactory::CONTROL_METHOD ) {

                if ( auto widget = factory.createEditor( 0 ) ) {
                    widget->setObjectName( factory.title() );
                    if ( auto p = qobject_cast<adplugin::LifeCycle *>( widget ) )
                        p->OnInitialUpdate();                    
                    createDockWidget( widget, factory.title(), objname );
                }
                
            }

        }
    }

    auto conf = document::instance()->currentConfiguration();
    if ( ! conf.isEmpty() )
        changeConfiguration( conf );

    // and this must be very last.
    createDockWidget( impl_->cmEditor_.get(), "Control Method", "ControlMethodWidget" );

	setSimpleDockWidgetArrangement();

    connect( impl_->cmEditor_.get(), &adwidgets::ControlMethodWidget::onCurrentChanged, this, [this] ( QWidget * w ){ w->parentWidget()->raise(); } );

    auto instTree = findChild< adwidgets::InstTreeView * >();

    for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
        connect( iController, &adextension::iController::onControlMethodChanged, this, &MainWindow::handleControlMethod );
        if ( instTree )
            instTree->addItem( iController->module_name(), iController->module_name(), false );
    }
}

void
MainWindow::OnFinalClose()
{
    QList< QDockWidget *> dockWidgets = this->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QObjectList list = dockWidget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycle * pLifeCycle = dynamic_cast<adplugin::LifeCycle *>( obj );
            if ( pLifeCycle )
                pLifeCycle->OnFinalClose();
        }
    }
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& objname )
{
    if ( widget->windowTitle().isEmpty() ) // avoid QTC_CHECK warning on console
        widget->setWindowTitle( title );

    if ( widget->objectName().isEmpty() )
        widget->setObjectName( objname );

    QDockWidget * dockWidget = addDockForWidget( widget );
    dockWidget->setObjectName( widget->objectName() );
    if ( title.isEmpty() )
        dockWidget->setWindowTitle( widget->objectName() );
    else
        dockWidget->setWindowTitle( title );

    if ( !objname.isEmpty() )
        dockWidget->setObjectName( objname );        

    addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

    return dockWidget;
}

class scopedSetTrackingEnabled {
    Utils::FancyMainWindow& w_;
public:
    scopedSetTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) {
        w_.setTrackingEnabled( false );
    }
    ~scopedSetTrackingEnabled() {
        w_.setTrackingEnabled( true );
    }
};

void
MainWindow::setSimpleDockWidgetArrangement()
{
    scopedSetTrackingEnabled lock( *this );
    
    auto widgets = dockWidgets();
    for ( auto widget: widgets ) {
		widget->setFloating( false );
		removeDockWidget( widget );
	}

    QList< QDockWidget * > left, right;
    QString cconf = QString("config/%1").arg( document::instance()->currentConfiguration() );


    auto it = std::find_if( widgets.begin(), widgets.end(), [] ( QDockWidget * dock ) { return dock->objectName() == "SampleRunWidget"; } );
    if ( it != widgets.end() ) {
        left.push_back( *it );
        widgets.erase( it );
    }

    it = std::find_if( widgets.begin(), widgets.end(), [] ( QDockWidget * dock ) { return dock->objectName() == "ControlMethodWidget"; } );
    if ( it != widgets.end() ) {
        right.push_back( *it );
        widgets.erase( it );
    }

    for ( auto dock : widgets ) {
        if (  dock->objectName() == cconf )
            left.push_back( dock );
    }
    for ( auto dock : widgets ) {
        if ( !dock->objectName().contains( QString( "config/" ) ) )
            right.push_back( dock );
    }

    for ( const auto& list : { left, right } ) {
        size_t pos = 0;
        for ( auto widget : list ) {
            addDockWidget( Qt::BottomDockWidgetArea, widget );
            widget->show();
            if ( pos++ >= 1 )
                tabifyDockWidget( list [ 0 ], widget );
        }
        if ( !list.isEmpty() )
            ( *list.begin() )->raise();
    }
}

void
MainWindow::handle_message( unsigned long msg, unsigned long value )
{
	// this is debugging purpose only, 
	// wired from AcquirePlugin::handle_message
}

void
MainWindow::eventLog( const QString& text )
{
	emit signal_eventLog( text );
}

void
MainWindow::setControlMethod( const adcontrols::ControlMethod::Method& m )
{
    impl_->cmEditor_->setControlMethod( m );
}

std::shared_ptr< adcontrols::ControlMethod::Method >
MainWindow::getControlMethod()
{
    auto mp = std::make_shared< adcontrols::ControlMethod::Method >();
    impl_->cmEditor_->getControlMethod( *mp );
    return mp;
}

void
MainWindow::getControlMethod( adcontrols::ControlMethod::Method& m )
{
    impl_->cmEditor_->getControlMethod( m );
}

void
MainWindow::setSampleRun( const adcontrols::SampleRun& m )
{
    impl_->runEditor_->setSampleRun( m );
}

bool
MainWindow::getSampleRun( adcontrols::SampleRun& m )
{
    impl_->runEditor_->getSampleRun( m );
    return true;
}


void
MainWindow::handle_shutdown()
{
}

void
MainWindow::handle_debug_print( unsigned long priority, unsigned long category, QString text )
{
    Q_UNUSED( priority );
    Q_UNUSED( category );
    emit signal_eventLog( text );
}

void
MainWindow::handleControlMethod()
{
    auto ptr = acquire::document::instance()->controlMethod();

    for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
        iController->preparing_for_run( *ptr );
    }
    if ( impl_->cmEditor_ )
        impl_->cmEditor_->setControlMethod( *ptr );
}

struct observer2ptree {

    void operator()( adicontroller::SignalObserver::Observer * observer, boost::property_tree::ptree& pt ) {

        const auto& desc = observer->description();

        pt.put( "observer.objid", observer->objid() );        
        pt.put( "observer.objtext", observer->objtext() );
        pt.put( "observer.desc.trace_id", desc.trace_id() );
        pt.put( "observer.desc.trace_display_name", adportable::utf::to_utf8( desc.trace_display_name() ) );
        pt.put( "observer.desc.trace_method", desc.trace_method() );

        int count( 0 );
        boost::property_tree::ptree vec;

        for ( auto sibling : observer->siblings() ) {
            boost::property_tree::ptree child;
            observer2ptree()( sibling.get(), child );
            vec.push_back( std::make_pair( "", child ) );
            ++count;
        }
        if ( count )
            pt.add_child( "siblings", vec );
    }
    
};

void
MainWindow::iControllerConnected( adextension::iController * inst )
{
    ADDEBUG() << "iControllerConnected( adextension::iController * inst )";

    if ( auto tree = findChild< adwidgets::InstTreeView * >() ) {

        tree->setChecked( inst->module_name(), true );

        boost::property_tree::ptree pt;
        if ( inst->module_name() == "Acquire" ) {
            observer2ptree()( document::instance()->masterObserver(), pt );
        } else {
            if ( auto session = inst->getInstrumentSession() ) {
                if ( auto observer = session->getObserver() ) 
                    observer2ptree()( observer, pt );
            }
        }
        std::ostringstream o;
        boost::property_tree::write_json( o, pt );
        tree->setObserverTree( inst->module_name(), QString::fromStdString( o.str() ) );
    }
}

void
MainWindow::iControllerMessage( adextension::iController * p, uint32_t msg, uint32_t value )
{
    ADDEBUG() << "iControllerMessage(" << p->module_name().toStdString() << ", msg=" << msg << ", value=" << value << ")";
    static const QString state_names[] = {
        "Nothing"
        , "Not Connected"             //= 0x00000001,  // no instrument := no driver software loaded
        , "Off"                      //= 0x00000002,  // software driver can be controled, but hardware is currently off
        , "Initializing"             //= 0x00000003,  // startup initializing (only at the begining after startup)
        , "StandBy"                  //= 0x00000004,  // instrument is stand by state
        , "PreparingForRun"          //= 0x00000005,  // preparing for next method (parameters being be set value)
        , "ReadyForRun"              //= 0x00000006,  // method is in initial state, ready to run (INIT RUN, MS HTV is ready)
        , "WaitingForContactClosure" //= 0x00000007,  //
        , "Running"                  //= 0x00000008,  // method is in progress
        , "Stop"                     //= 0x00000009,  // stop := detector is not monitoring, pump is off
    };

    if ( auto tree = findChild< adwidgets::InstTreeView * >() ) {
        if ( msg == adicontroller::Receiver::STATE_CHANGED && value < sizeof(state_names)/sizeof(state_names[0]) ) {
            tree->setInstState( p->module_name(), state_names[ value ] );
        }
    }
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::West );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;
	editorHolderLayout->setMargin( 0 );
	editorHolderLayout->setSpacing( 0 );
	    
    if ( QWidget * editorWidget = new QWidget ) {

        editorWidget->setLayout( editorHolderLayout );

        editorHolderLayout->addWidget( createTopStyledToolbar() );
        if ( auto wnd = new WaveformWnd() ) {
            editorHolderLayout->addWidget( wnd );
            // for compile check
            QVariant v;
            v.setValue( boost::uuids::uuid() );
        }
        
        //---------- central widget ------------
        if ( QWidget * centralWidget = new QWidget ) {

            setCentralWidget( centralWidget );
            
            QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
            centralWidget->setLayout( centralLayout );
            centralLayout->setMargin( 0 );
            centralLayout->setSpacing( 0 );
            // ----------------------------------------------------
            centralLayout->addWidget( editorWidget ); // [ToolBar + WaveformWnd]
            // ----------------------------------------------------

            centralLayout->setStretch( 0, 1 );
            centralLayout->setStretch( 1, 0 );
            // ----------------- mid tool bar -------------------
            centralLayout->addWidget( createMidStyledToolbar() );      // [Middle toolbar]
        }
    }

	if ( Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter ) {

        QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
        outputPane->setObjectName( QLatin1String( "SequenceOutputPanePlaceHolder" ) );

        mainWindowSplitter->addWidget( this );        // [Central Window]
        mainWindowSplitter->addWidget( outputPane );  // [Output (log) Window]

        mainWindowSplitter->setStretchFactor( 0, 9 );
        mainWindowSplitter->setStretchFactor( 1, 1 );
        mainWindowSplitter->setOrientation( Qt::Vertical );

        // Split Navigation and Application window
        Core::MiniSplitter * splitter = new Core::MiniSplitter;               // entier this view
        if ( splitter ) {
            splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) ); // navegate
            splitter->addWidget( mainWindowSplitter );                            // *this + ontput
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 1 );
            splitter->setOrientation( Qt::Horizontal );
            splitter->setObjectName( QLatin1String( "SequenceModeWidget" ) );
        }

        createDockWidgets();

        return splitter;
    }
    return 0;
}

// static
QToolButton * 
MainWindow::toolButton( QAction * action )
{
    QToolButton * button = new QToolButton;
    if ( button )
        button->setDefaultAction( action );
    return button;
}

// static
QToolButton * 
MainWindow::toolButton( const char * id )
{
    return toolButton( Core::ActionManager::instance()->command( id )->action() );
}

void
MainWindow::actMethodOpen()
{
    QString name
        = QFileDialog::getOpenFileName( this
                                        , tr("Open control method")
                                        , document::instance()->recentFile( Constants::GRP_METHOD_FILES, true )
                                        , tr( "Control method files(*.cmth)" ) );
    adcontrols::ControlMethod::Method m;
    if ( document::load( name, m ) ) {
        document::instance()->setControlMethod( m, name );
        setControlMethod( m );
    }
    
}

void
MainWindow::actMethodSave()
{
    QString name
        = QFileDialog::getSaveFileName( this
                                        , tr("Save control method")
                                        , document::instance()->recentFile( Constants::GRP_METHOD_FILES, true )
                                        , tr( "Control method files(*.cmth)" ) );
    
    adcontrols::ControlMethod::Method m;
    getControlMethod( m );

    document::instance()->setControlMethod( m );
    if ( document::save( name, m ) ) {
        document::instance()->setControlMethod( m, name );
    }

}


void
MainWindow::createActions()
{
    Core::ActionContainer * menu = Core::ActionManager::instance()->createMenu( Constants::MENU_ID ); // Menu ID

    if ( !menu )
        return;

    const Core::Context context( (Core::Id( Core::Constants::C_GLOBAL ) ) );
    menu->menu()->setTitle( "Acquire" );

    //------------ snapshot -------------
    if ( auto action = createAction( Constants::ICON_SNAPSHOT, tr( "Snapshot" ), this ) ) {
        connect( action, &QAction::triggered, [](){ document::instance()->actionSnapshot(); } );
        action->setEnabled( false );
        Core::Command * cmd = Core::ActionManager::registerAction( action, Constants::ACTION_SNAPSHOT, context );
        menu->addAction( cmd );
    }

    if ( auto action = createAction( Constants::ICON_CONNECT, tr( "Connect" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionConnect(); } );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_CONNECT, context );
        menu->addAction( cmd );        
    }

    if ( auto action = createAction( Constants::ICON_INITRUN, tr( "Prepare" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionInitRun(); } );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_INITIALRUN, context );
        menu->addAction( cmd );
    }
    

    if ( auto action = createAction( Constants::ICON_RUN, tr( "Run" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionRun(); } );
        action->setEnabled( false );        
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_RUN, context );
        menu->addAction( cmd );        
    }

    if ( auto action = createAction( Constants::ICON_STOP, tr( "Stop" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionStop(); } );
        action->setEnabled( false );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_STOP, context );
        menu->addAction( cmd );        
    }

    if ( auto action = createAction( Constants::ICON_INJECT, tr( "Inject" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionInject(); } );
        action->setEnabled( false );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_INJECT, context );
        menu->addAction( cmd );                
    }

    //------------ method file open/save
    if ( auto action = createAction( Constants::ICON_FILE_OPEN, tr( "Method open..." ), this ) ) {
        connect( action, &QAction::triggered, MainWindow::instance(), &MainWindow::actMethodOpen );
        auto cmd = Core::ActionManager::registerAction( action, Constants::METHODOPEN, context );
        menu->addAction( cmd );
    }

    if ( auto action = createAction( Constants::ICON_FILE_SAVE, tr( "Method save..." ), this ) ) {
        connect( action, &QAction::triggered, MainWindow::instance(), &MainWindow::actMethodSave );
        auto cmd = Core::ActionManager::registerAction( action, Constants::METHODSAVE, context );
        menu->addAction( cmd );
    }
#if 0    
    do {
        QIcon icon;
        icon.addPixmap( QPixmap( Constants::ICON_REC_ON ), QIcon::Normal, QIcon::On );
        icon.addPixmap( QPixmap( Constants::ICON_REC_PAUSE ), QIcon::Normal, QIcon::Off );
        if ( auto action = new QAction( icon, tr( "REC" ), this ) ) {
            action->setCheckable( true );
            // action->setChecked( true );
            action->setEnabled( false );
            auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_REC, context );
            menu->addAction( cmd );        
            connect( action, &QAction::triggered, [this](bool rec){
                    document::instance()->actionRec(rec);
                    if ( auto action = Core::ActionManager::command(Constants::ACTION_REC)->action() )
                        if ( !action->isEnabled() )
                            action->setEnabled( true );
                } );
        }
    } while ( 0 );
    
    if ( auto action = createAction( Constants::ICON_SYNC, tr( "Sync trig." ), this ) ) {
        connect( action, &QAction::triggered, [](){ document::instance()->actionSyncTrig(); } );
        action->setEnabled( false );        
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_SYNC, context );
        menu->addAction( cmd );        
    }
#endif
    
    if ( auto action = createAction( Constants::ICON_PDF, tr( "PDF" ), this ) ) {
        connect( action, &QAction::triggered, this, &MainWindow::printCurrentView );
        auto cmd = Core::ActionManager::registerAction( action, Constants::PRINT_CURRENT_VIEW, context );
        menu->addAction( cmd );        
    }

    if ( auto action = createAction( Constants::ICON_IMAGE, tr( "Screenshot" ), this ) ) {
        connect( action, &QAction::triggered, this, &MainWindow::saveCurrentImage );
        auto cmd = Core::ActionManager::registerAction( action, Constants::SAVE_CURRENT_IMAGE, context );
        menu->addAction( cmd );        
    }

    do {
        QIcon icon;
        icon.addPixmap( QPixmap( Constants::ICON_DOCKHIDE ), QIcon::Normal, QIcon::Off );
        icon.addPixmap( QPixmap( Constants::ICON_DOCKSHOW ), QIcon::Normal, QIcon::On );
        auto * action = new QAction( icon, tr( "Hide dock" ), this );
        action->setCheckable( true );
        Core::ActionManager::registerAction( action, Constants::HIDE_DOCK, context );
        connect( action, &QAction::triggered, MainWindow::instance(), &MainWindow::hideDock );
    } while ( 0 );

    handleInstState( 0 );
    Core::ActionManager::instance()->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );

}

QAction *
MainWindow::createAction( const QString& iconname, const QString& msg, QObject * parent )
{
    QIcon icon;
    icon.addFile( iconname );
    return new QAction( icon, msg, parent );
}

void
MainWindow::saveCurrentImage()
{
    ADDEBUG() << "saveCurrentImage()";
    qApp->beep();
    if ( auto screen = QGuiApplication::primaryScreen() ) {
        
        auto pixmap = QPixmap::grabWidget( this );
#if 0
        if ( auto sample = document::instance()->sampleRun() ) {
            boost::filesystem::path path( sample->dataDirectory() );
            if ( ! boost::filesystem::exists( path ) ) {
                boost::system::error_code ec;
                boost::filesystem::create_directories( path, ec );
            }
            int runno(0);
            if ( boost::filesystem::exists( path ) && boost::filesystem::is_directory( path ) ) {
                using boost::filesystem::directory_iterator;
                for ( directory_iterator it( path ); it != directory_iterator(); ++it ) {
                    boost::filesystem::path fname = (*it);
                    if ( fname.extension().string() == ".png" ) {
                        runno = std::max( runno, adportable::split_filename::trailer_number_int( fname.stem().wstring() ) );
                    }
                }
            }
            
            std::wostringstream o;
            o << L"acquire_" << std::setw( 4 ) << std::setfill( L'0' ) << runno + 1;
            path /= o.str();
            path.replace_extension( ".png" );

            ADDEBUG() << "saveCurrentImage(" << path.string() << ")";

            pixmap.save( QString::fromStdWString( path.wstring() ), "png" );
        }
#endif
    }
}

void
MainWindow::printCurrentView()
{
    saveCurrentImage();
}

void
MainWindow::hideDock( bool hide )
{
    for ( auto& w :  dockWidgets() ) {
        if ( hide )
            w->hide();
        else
            w->show();
    }
}

void
MainWindow::handleInstState( int status )
{
    if ( status <= adicontroller::Instrument::eNotConnected ) {

        if ( auto action = Core::ActionManager::instance()->command( Constants::ACTION_CONNECT )->action() )
            action->setEnabled( true  );

        for ( auto id : { Constants::ACTION_RUN, Constants::ACTION_STOP, Constants::ACTION_SNAPSHOT } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( false );
        }

    } else if ( status == adicontroller::Instrument::eStandBy ) {

        if ( auto action = Core::ActionManager::command( Constants::ACTION_CONNECT )->action() )
            action->setEnabled( false );
        
        for ( auto id :
            { Constants::ACTION_INITIALRUN, Constants::ACTION_RUN /*, Constants::ACTION_STOP, */, Constants::ACTION_SNAPSHOT } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( true );
        }

    } else if ( status == adicontroller::Instrument::eWaitingForContactClosure ) {

        for ( auto id : { Constants::ACTION_INJECT, Constants::ACTION_STOP, Constants::ACTION_SNAPSHOT } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( true );
        }
        if ( auto action = Core::ActionManager::command( Constants::ACTION_RUN )->action() )
            action->setEnabled( false );

    } else if ( status == adicontroller::Instrument::ePreparingForRun ) {

        if ( auto action = Core::ActionManager::command( Constants::ACTION_STOP )->action() )
            action->setEnabled( false );        

    } else if ( status == adicontroller::Instrument::eReadyForRun ) {

        if ( auto action = Core::ActionManager::command( Constants::ACTION_STOP )->action() )
            action->setEnabled( false );
        if ( auto action = Core::ActionManager::command( Constants::ACTION_RUN )->action() )
            action->setEnabled( true );                
        
    } else if ( status == adicontroller::Instrument::eRunning ) {
        if ( auto action = Core::ActionManager::command( Constants::ACTION_STOP )->action() )
            action->setEnabled( true );
        if ( auto action = Core::ActionManager::command( Constants::ACTION_INJECT )->action() )
            action->setEnabled( false );
        if ( auto action = Core::ActionManager::command( Constants::ACTION_RUN )->action() )
            action->setEnabled( false );
        
    } else if ( status == adicontroller::Instrument::eStop ) {
        // Disable
        // for ( auto id : { Constants::ACTION_STOP } ) {
        //     if ( auto action = Core::ActionManager::command( id )->action() )
        //         action->setEnabled( false );
        // }
        // for ( auto id : { Constants::ACTION_RUN, Constants::ACTION_SNAPSHOT } ) {
        //     if ( auto action = Core::ActionManager::command(Constants::ACTION_RUN)->action() )
        //         action->setEnabled( true );
        // }
    }

    //ADDEBUG() << "handleInstState(" << status << ")";

}

Utils::StyledBar *
MainWindow::createTopStyledToolbar()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 4 );
        if ( auto cmdLayout = new QHBoxLayout() ) {
            if ( auto am = Core::ActionManager::instance() ) {
                cmdLayout->addWidget( toolButton( am->command(Constants::ACTION_CONNECT)->action() ) );
                cmdLayout->addWidget( toolButton( am->command( Constants::ACTION_INITIALRUN )->action() ) );
                cmdLayout->addWidget( toolButton( am->command(Constants::ACTION_RUN)->action() ) );
                cmdLayout->addWidget( toolButton( am->command(Constants::ACTION_STOP)->action() ) );
                cmdLayout->addWidget( toolButton( am->command(Constants::ACTION_SNAPSHOT)->action() ) );
                cmdLayout->addWidget( toolButton( am->command(Constants::ACTION_INJECT)->action() ) );
            }
            toolBarLayout->addLayout( cmdLayout );
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );            
        }
        
        if ( auto infoLayout = new QHBoxLayout() ) {
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

            //-- separator --
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
            toolBarLayout->addItem( new QSpacerItem( 32, 0, QSizePolicy::Expanding ) );
        }
    }
    return toolBar;
}

Utils::StyledBar *
MainWindow::createMidStyledToolbar()
{
    if ( Utils::StyledBar * toolBar = new Utils::StyledBar ) {

        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);

        if ( auto am = Core::ActionManager::instance() ) {

            toolBarLayout->addWidget( toolButton( am->command( Constants::ACTION_SNAPSHOT )->action() ) );
            toolBarLayout->addWidget( toolButton( am->command( Constants::METHODOPEN )->action() ) );
            toolBarLayout->addWidget( toolButton( am->command( Constants::METHODSAVE )->action() ) );
            toolBarLayout->addWidget( new Utils::StyledSeparator );

            if ( auto combo = new QComboBox ) {
                combo->setObjectName( "Configuration" );
                toolBarLayout->addWidget( new QLabel( tr("Configuration:") ) );
                toolBarLayout->addWidget( combo );
                toolBarLayout->addSpacerItem( new QSpacerItem( 40, 0 ) );
                connect( combo, static_cast< void(QComboBox::*)(const QString&) >(&QComboBox::currentIndexChanged), [this](const QString& c){ changeConfiguration( c ); });
            }

            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("Traces:") ) );
            impl_->traceBox_ = new QComboBox;
            impl_->traceBox_->addItem( "-----------------------------" );
            //connect( impl_->traceBox_, SIGNAL( currentIndexChanged(int) ), this, SLOT( handle_monitor_selected(int) ) );
            //connect( impl_->traceBox_, SIGNAL( activated(int) ), this, SLOT( handle_monitor_activated(int) ) );
            toolBarLayout->addWidget( impl_->traceBox_ );
            toolBarLayout->addWidget( new QLabel( tr("  ") ), 10 );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addWidget( new QLabel( tr("Threads:") ) );
        
		return toolBar;
    }
    return 0;
}

void
MainWindow::createDockWidgets()
{
    createDockWidget( impl_->runEditor_.get(), "Sample Run", "SampleRunWidget" ); // this must be first
}

void
MainWindow::changeConfiguration( const QString& config )
{
    std::string stdconfig = config.toStdString(); // wordaround -- QString doesnt show on debugger
    document::instance()->setCurrentConfiguration( config );

    impl_->cmEditor_->clearAllEditors();

    QString objname = QString( "config/%1" ).arg( config );

    for ( auto dock : dockWidgets() ) {
        if ( dock->objectName() == objname ) {
            impl_->cmEditor_->addEditor( dock->widget() );
            ADDEBUG() << "add editor: " << dock->objectName().toStdString() << " -> " << dock->widget()->objectName().toStdString();
        }
    }
    
    if ( auto cm = document::instance()->controlMethod() )
        impl_->cmEditor_->setControlMethod( *cm );

    setSimpleDockWidgetArrangement();

    document::instance()->onConfigurationChanged( config );
}

