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
#include <adwidgets/controlmethodwidget.hpp>
#include <adwidgets/insttreeview.hpp>
#include <adwidgets/samplerunwidget.hpp>
#include <qtwrapper/qstring.hpp>

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/imode.h>
#include <coreplugin/editormanager/editormanager.h>
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

#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include <QToolButton>
#include <QTextEdit>
#include <QPluginLoader>
#include <QLibrary>
#include <QtCore>
#include <QUrl>
#include <QMessageBox>
#include <QTabBar>

namespace acquire {

    class MainWindow::impl {
    public:
        impl() : cmEditor_( new adwidgets::ControlMethodWidget )
               , runEditor_( new adwidgets::SampleRunWidget )
               , timePlot_( new adplot::ChromatogramWidget )
               , spectrumPlot_( new adplot::SpectrumWidget ) {
        }
        
        static std::unique_ptr< MainWindow > instance_;

        adplot::ChromatogramWidget * timePlot_;
        adplot::SpectrumWidget * spectrumPlot_;
        
        std::unique_ptr< adwidgets::ControlMethodWidget > cmEditor_;
        std::unique_ptr< adwidgets::SampleRunWidget > runEditor_;
        QComboBox * traceBox_;
    };

    std::unique_ptr< MainWindow > MainWindow::impl::instance_; 
}

using namespace acquire;

MainWindow * 
MainWindow::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [](){ impl::instance_.reset( new MainWindow() ); } );
    return impl::instance_.get();
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

    createDockWidget( impl_->runEditor_.get(), "Sample Run", "SampleRunWidget" ); // this must be first

    // then, series of individual control method widgets
    auto visitables = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSequence >();

	for ( auto v: visitables ) {

        for ( size_t i = 0; i < v->size(); ++i ) {

            const adextension::iEditorFactory& factory = ( *v )[ i ];
            if ( factory.method_type() == adextension::iEditorFactory::CONTROL_METHOD ) {

                if ( auto widget = factory.createEditor( 0 ) ) {
                    widget->setObjectName( factory.title() );
                    createDockWidget( widget, factory.title(), "ControlMethod" );
                    impl_->cmEditor_->addEditor( widget ); // will call OnInitialUpdate
                }
                
            }

        }
    }

    if ( auto tree = new adwidgets::InstTreeView() )
        createDockWidget( tree, "Status", "InstStatus" );

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
    
    size_t nsize = widgets.size();
    size_t npos = 0;
    for ( auto widget: widgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
        if ( npos++ >= 1 && npos < nsize )
            tabifyDockWidget( widgets[0], widget );
    }

    if ( !widgets.isEmpty() )
        (*widgets.begin())->raise();
}

void
MainWindow::handle_message( unsigned long msg, unsigned long value )
{
   ACE_UNUSED_ARG(msg);
   ACE_UNUSED_ARG(value);
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
    //              [mainWindow]
    // splitter> ---------------------
    //              [OutputPane]
  
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        splitter->addWidget( this );
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
            cmdLayout->addWidget( toolButton( am->command( constants::ACTION_CONNECT )->action() ) );
            cmdLayout->addWidget( toolButton( am->command( constants::ACTION_INITIALRUN )->action() ) );
            cmdLayout->addWidget( toolButton( am->command( constants::ACTION_RUN )->action() ) );
            cmdLayout->addWidget( toolButton( am->command( constants::ACTION_STOP )->action() ) );
            cmdLayout->addWidget( toolButton( am->command( constants::ACTION_INJECT )->action() ) );
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
            toolBarLayout->addWidget( toolButton( am->command( constants::ACTION_SNAPSHOT )->action() ) ); //actionSnapshot_ ) );
            toolBarLayout->addWidget( toolButton( am->command( constants::METHODOPEN )->action() ) ); //actMethodOpen_ ) );
            toolBarLayout->addWidget( toolButton( am->command( constants::METHODSAVE )->action() ) ); //actMethodSave_ ) );
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("Traces:") ) );
            impl_->traceBox_ = new QComboBox;
            impl_->traceBox_->addItem( "-----------------------------" );
            connect( impl_->traceBox_, SIGNAL( currentIndexChanged(int) ), this, SLOT( handle_monitor_selected(int) ) );
            connect( impl_->traceBox_, SIGNAL( activated(int) ), this, SLOT( handle_monitor_activated(int) ) );
            toolBarLayout->addWidget( impl_->traceBox_ );
            toolBarLayout->addWidget( new QLabel( tr("  ") ), 10 );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addWidget( new QLabel( tr("Threads:") ) );
    }

    if ( QWidget* centralWidget = new QWidget ) {

        setCentralWidget( centralWidget );

        Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
        if ( splitter3 ) {
            splitter3->addWidget( impl_->timePlot_ );
            splitter3->addWidget( impl_->spectrumPlot_ );
            splitter3->setOrientation( Qt::Vertical );

            using adplot::ChromatogramWidget;
                
            //connect( impl_->timePlot_, static_cast< void( ChromatogramWidget::*)( const QPointF& ) >( &ChromatogramWidget::onSelected ), this, &MainWindow::handleSelected );
            //connect( impl_->timePlot_, static_cast< void( ChromatogramWidget::*)( const QRectF& ) >( &ChromatogramWidget::onSelected ), this, &MainWindow::handleSelected );
        }

        QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
        toolBarAddingLayout->setMargin(0);
        toolBarAddingLayout->setSpacing(0);
        //toolBarAddingLayout->addWidget( rightPaneSplitter );
        toolBarAddingLayout->addWidget( toolBar );
        toolBarAddingLayout->addWidget( splitter3 );
        toolBarAddingLayout->addWidget( toolBar2 );
    }
    
    return splitter2;
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
                                        , document::instance()->recentFile( constants::GRP_METHOD_FILES, true )
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
                                        , document::instance()->recentFile( constants::GRP_METHOD_FILES, true )
                                        , tr( "Control method files(*.cmth)" ) );
    
    adcontrols::ControlMethod::Method m;
    getControlMethod( m );

    document::instance()->setControlMethod( m );
    if ( document::save( name, m ) ) {
        document::instance()->setControlMethod( m, name );
    }

}

/*
void
AcquirePlugin::initialize_actions()
{
    pImpl_->loadIcon();

    actionConnect_->setEnabled( true );
    actionInitRun_->setEnabled( false );
    actionRun_->setEnabled( false );
	actionStop_->setEnabled( false );
	actionInject_->setEnabled( false );
    actionSnapshot_->setEnabled( true );
  
    Core::Context context( ( Core::Id( "Acquire.MainView" ), Core::Id( Core::Constants::C_GLOBAL ) ) );

    if ( auto am = Core::ActionManager::instance() ) {
        Core::Command * cmd = 0;
        cmd = am->registerAction( actionConnect_, constants::CONNECT, context );

        cmd = am->registerAction( actionInitRun_, constants::INITIALRUN, context );
        cmd = am->registerAction( actionRun_, constants::RUN, context );
        cmd = am->registerAction( actionStop_, constants::STOP, context );
        cmd = am->registerAction( actionInject_, constants::ACQUISITION, context );
        cmd = am->registerAction( actionSnapshot_, constants::SNAPSHOT, context );
        cmd = am->registerAction( actMethodOpen_, constants::METHODOPEN, context );
        cmd = am->registerAction( actMethodSave_, constants::METHODSAVE, context );
        (void)cmd;
    }
}
*/

void
MainWindow::createActions()
{
    Core::ActionContainer * menu = Core::ActionManager::instance()->createMenu( constants::MENU_ID ); // Menu ID

    if ( !menu )
        return;

    const Core::Context context( (Core::Id( Core::Constants::C_GLOBAL ) ) );
    menu->menu()->setTitle( "Acquire" );

    //------------ snapshot -------------
    // actionSnapshot_ = new QAction(QIcon(":/acquire/images/snapshot_small.png"), tr("Take spectrum snapshot"), this);
    // connect( actionSnapshot_, &QAction::triggered, this, &AcquirePlugin::actionSnapshot );
    if ( auto action = createAction( constants::ICON_SNAPSHOT, tr( "Snapshot" ), this ) ) {
        connect( action, &QAction::triggered, [](){ document::instance()->actionSnapshot(); } );
        action->setEnabled( false );
        Core::Command * cmd = Core::ActionManager::registerAction( action, constants::ACTION_SNAPSHOT, context );
        menu->addAction( cmd );
    }

    // actionConnect_ = new QAction( QIcon(":/acquire/images/Button Refresh.png"), tr("Connect to control server..."), this);
    // connect( actionConnect_, &QAction::triggered, this, &AcquirePlugin::actionConnect );
    if ( auto action = createAction( constants::ICON_CONNECT, tr( "Connect" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionConnect(); } );
        auto cmd = Core::ActionManager::registerAction( action, constants::ACTION_CONNECT, context );
        menu->addAction( cmd );        
    }

    // actionInitRun_ = new QAction(QIcon(":/acquire/images/Button Last.png"), tr("Preparing"), this);
    // connect( actionInitRun_, &QAction::triggered, this, &AcquirePlugin::actionInitRun );
    if ( auto action = createAction( constants::ICON_INITRUN, tr( "Prepare" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionInitRun(); } );
        auto cmd = Core::ActionManager::registerAction( action, constants::ACTION_INITIALRUN, context );
        menu->addAction( cmd );
    }
    

    // actionRun_ = new QAction(QIcon(":/acquire/images/Button Play.png"), tr("Run"), this);
    // connect( actionRun_, &QAction::triggered, this, &AcquirePlugin::actionRun );
    if ( auto action = createAction( constants::ICON_RUN, tr( "Run" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionRun(); } );
        action->setEnabled( false );        
        auto cmd = Core::ActionManager::registerAction( action, constants::ACTION_RUN, context );
        menu->addAction( cmd );        
    }

    // actionStop_ = new QAction(QIcon(":/acquire/images/Button Stop.png"), tr("Stop"), this);
    // connect( actionStop_, &QAction::triggered, this, &AcquirePlugin::actionStop );
    if ( auto action = createAction( constants::ICON_STOP, tr( "Stop" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionStop(); } );
        action->setEnabled( false );
        auto cmd = Core::ActionManager::registerAction( action, constants::ACTION_STOP, context );
        menu->addAction( cmd );        
    }

    // actionInject_ = new QAction(QIcon(":/acquire/images/Button Add.png"), tr("Inject (recording data)"), this);
    // connect( actionInject_, &QAction::triggered, this, &AcquirePlugin::actionInject );
    if ( auto action = createAction( constants::ICON_INJECT, tr( "Inject" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionInject(); } );
        action->setEnabled( false );
        auto cmd = Core::ActionManager::registerAction( action, constants::ACTION_INJECT, context );
        menu->addAction( cmd );                
    }

    //------------ method file open/save
    if ( auto action = createAction( constants::ICON_FILE_OPEN, tr( "Method open..." ), this ) ) {
        connect( action, &QAction::triggered, MainWindow::instance(), &MainWindow::actMethodOpen );
        auto cmd = Core::ActionManager::registerAction( action, constants::METHODOPEN, context );
        menu->addAction( cmd );
    }

    if ( auto action = createAction( constants::ICON_FILE_SAVE, tr( "Method save..." ), this ) ) {
        connect( action, &QAction::triggered, MainWindow::instance(), &MainWindow::actMethodSave );
        auto cmd = Core::ActionManager::registerAction( action, constants::METHODSAVE, context );
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
    
    if ( auto action = createAction( constants::ICON_PDF, tr( "PDF" ), this ) ) {
        connect( action, &QAction::triggered, this, &MainWindow::printCurrentView );
        auto cmd = Core::ActionManager::registerAction( action, constants::PRINT_CURRENT_VIEW, context );
        menu->addAction( cmd );        
    }

    if ( auto action = createAction( constants::ICON_IMAGE, tr( "Screenshot" ), this ) ) {
        connect( action, &QAction::triggered, this, &MainWindow::saveCurrentImage );
        auto cmd = Core::ActionManager::registerAction( action, constants::SAVE_CURRENT_IMAGE, context );
        menu->addAction( cmd );        
    }

    do {
        QIcon icon;
        icon.addPixmap( QPixmap( constants::ICON_DOCKHIDE ), QIcon::Normal, QIcon::Off );
        icon.addPixmap( QPixmap( constants::ICON_DOCKSHOW ), QIcon::Normal, QIcon::On );
        auto * action = new QAction( icon, tr( "Hide dock" ), this );
        action->setCheckable( true );
        Core::ActionManager::registerAction( action, constants::HIDE_DOCK, context );
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

        if ( auto action = Core::ActionManager::instance()->command( constants::ACTION_CONNECT )->action() )
            action->setEnabled( true  );

        for ( auto id : { constants::ACTION_RUN, constants::ACTION_STOP, constants::ACTION_SNAPSHOT } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( false );
        }

    } else if ( status == adicontroller::Instrument::eStandBy ) {

        if ( auto action = Core::ActionManager::command( constants::ACTION_CONNECT )->action() )
            action->setEnabled( false );
        
        for ( auto id :
            { constants::ACTION_INITIALRUN, constants::ACTION_RUN /*, constants::ACTION_STOP, */, constants::ACTION_SNAPSHOT } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( true );
        }

        // if ( auto action = Core::ActionManager::command(constants::ACTION_REC)->action() )
        //     action->setChecked( true );
    } else if ( status == adicontroller::Instrument::eWaitingForContactClosure ) {

        for ( auto id : { constants::ACTION_INJECT, constants::ACTION_STOP, constants::ACTION_SNAPSHOT } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( true );
        }
        if ( auto action = Core::ActionManager::command( constants::ACTION_RUN )->action() )
            action->setEnabled( false );

    } else if ( status == adicontroller::Instrument::ePreparingForRun ) {

        if ( auto action = Core::ActionManager::command( constants::ACTION_STOP )->action() )
            action->setEnabled( false );        

    } else if ( status == adicontroller::Instrument::eReadyForRun ) {

        if ( auto action = Core::ActionManager::command( constants::ACTION_STOP )->action() )
            action->setEnabled( false );
        if ( auto action = Core::ActionManager::command( constants::ACTION_RUN )->action() )
            action->setEnabled( true );                
        
    } else if ( status == adicontroller::Instrument::eRunning ) {
        if ( auto action = Core::ActionManager::command( constants::ACTION_STOP )->action() )
            action->setEnabled( true );
        if ( auto action = Core::ActionManager::command( constants::ACTION_INJECT )->action() )
            action->setEnabled( false );
        if ( auto action = Core::ActionManager::command( constants::ACTION_RUN )->action() )
            action->setEnabled( false );
        
    } else if ( status == adicontroller::Instrument::eStop ) {
        // Disable
        // for ( auto id : { constants::ACTION_STOP } ) {
        //     if ( auto action = Core::ActionManager::command( id )->action() )
        //         action->setEnabled( false );
        // }
        // for ( auto id : { constants::ACTION_RUN, constants::ACTION_SNAPSHOT } ) {
        //     if ( auto action = Core::ActionManager::command(constants::ACTION_RUN)->action() )
        //         action->setEnabled( true );
        // }
    }

    ADDEBUG() << "handleInstState(" << status << ")";

}
