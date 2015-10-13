/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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
#include "waveformwnd.hpp"
#include "document.hpp"
#include "isequenceimpl.hpp"
#include "u5303a_constants.hpp"
#include <u5303a/digitizer.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrswidgets/thresholdwidget.hpp>
#include <acqrswidgets/u5303awidget.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <adlog/logger.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/samplerun.hpp>
#include <adextension/icontroller.hpp>
#include <adextension/ieditorfactory_t.hpp>
#include <adextension/ireceiver.hpp>
#include <adextension/isequenceimpl.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adicontroller/constants.hpp>
#include <adportable/date_string.hpp>
#include <adportable/profile.hpp>
#include <adportable/split_filename.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycleaccessor.hpp>
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
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <boost/uuid/uuid.hpp>
#include <QApplication>
#include <QDockWidget>
#include <QMenu>
#include <QResizeEvent>
#include <qstackedwidget.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTextEdit>
#include <QLabel>
#include <QIcon>
#include <qdebug.h>

using namespace u5303a;

Q_DECLARE_METATYPE( boost::uuids::uuid );

MainWindow * MainWindow::instance_ = 0;

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
{
    instance_ = this;
}

MainWindow::~MainWindow()
{
}

MainWindow *
MainWindow::instance()
{
    return instance_;
}

void
MainWindow::createDockWidgets()
{
    if ( auto widget = new acqrswidgets::ThresholdWidget( acqrscontrols::u5303a::method::modelClass(), 1 ) ) {

        widget->setObjectName( "ThresholdWidget" );
        createDockWidget( widget, "U5303A", "ThresholdMethod" );

        connect( widget, &acqrswidgets::ThresholdWidget::valueChanged, [this] ( acqrswidgets::idCategory cat, int ch ) {
                if ( auto form = findChild< acqrswidgets::ThresholdWidget * >() ) {
                    if ( cat == acqrswidgets::idSlopeTimeConverter ) {
                        adcontrols::threshold_method tm;
                        form->get( ch, tm );
                        document::instance()->set_threshold_method( ch, tm );
                    }
                }
                
            });

    }

    if ( auto widget = new acqrswidgets::u5303AWidget ) {

        widget->setObjectName( "U5303A" );
        createDockWidget( widget, "U5303A", "ControlMethod" );

        connect( widget, &acqrswidgets::u5303AWidget::valueChanged, [this,widget]( acqrswidgets::idCategory cat, int ch ) {
                acqrscontrols::u5303a::method m;
                if ( widget->get( m ) )
                    document::instance()->set_method( m );
            });

    }
}

void
MainWindow::OnInitialUpdate()
{
    connect( document::instance(), &document::instStateChanged, this, &MainWindow::handleInstState );

    boost::any a( document::instance()->controlMethod() );
    for ( auto dock: dockWidgets() ) {
        if ( auto widget = qobject_cast<adplugin::LifeCycle *>( dock->widget() ) ) {
            widget->OnInitialUpdate();
            widget->setContents( a );
        }
    }

    setSimpleDockWidgetArrangement();

    connect( document::instance(), &document::on_reply, this, &MainWindow::handle_reply );

    for ( auto id : { Constants::ACTION_RUN, Constants::ACTION_STOP, Constants::ACTION_REC, Constants::ACTION_SNAPSHOT } ) {
        if ( auto action = Core::ActionManager::command( id )->action() )
            action->setEnabled( false );
    }

    for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
        document::instance()->addiController( iController );
    }
    
	if ( WaveformWnd * wnd = centralWidget()->findChild<WaveformWnd *>() ) {
		wnd->onInitialUpdate();
        connect( document::instance(), SIGNAL( on_waveform_received() ), wnd, SLOT( handle_waveform() ) );
    }
    
}

void
MainWindow::OnFinalClose()
{
    for ( auto dock: dockWidgets() ) {
        adplugin::LifeCycleAccessor accessor( dock->widget() );
        if ( auto editor = accessor.get() ) {
            editor->OnFinalClose();
        }
    }
}

void
MainWindow::activateLayout()
{
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
            connect( document::instance(), &document::on_threshold_method_changed, wnd, &WaveformWnd::handle_threshold_method );
            connect( document::instance(), &document::onControlMethodChanged, wnd, &WaveformWnd::handle_method );
            bool res = connect( document::instance(), &document::dataChanged, wnd, &WaveformWnd::dataChanged );
//#if defined _DEBUG
            QVariant v;
            v.setValue( boost::uuids::uuid() );
//#endif
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

void
MainWindow::setSimpleDockWidgetArrangement()
{
    qtwrapper::TrackingEnabled< Utils::FancyMainWindow > x( *this );

    QList< QDockWidget *> widgets = dockWidgets();

    for ( auto widget: widgets ) {
        widget->setFloating( false );
        removeDockWidget( widget );
    }
  
    size_t npos = 0;
    for ( auto widget: widgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
        if ( npos && npos++ < widgets.size() - 1 ) // last item is not on the tab
            tabifyDockWidget( widgets[0], widget );
    }
    // update();
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& page )
{
    if ( widget->windowTitle().isEmpty() ) // avoid QTC_CHECK warning on console
        widget->setWindowTitle( title );
    if ( widget->objectName().isEmpty() )
        widget->setObjectName( page );

    QDockWidget * dockWidget = addDockForWidget( widget );
    dockWidget->setObjectName( page.isEmpty() ? widget->objectName() : page );
    if ( title.isEmpty() )
        dockWidget->setWindowTitle( widget->objectName() );
    else
        dockWidget->setWindowTitle( title );

    addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

    return dockWidget;
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

Utils::StyledBar *
MainWindow::createTopStyledToolbar()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        if ( auto am = Core::ActionManager::instance() ) {
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_CONNECT)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_RUN)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_STOP)->action()));

            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_REC)->action()));            

            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_SNAPSHOT)->action()));
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //---
            //toolBarLayout->addWidget( topLineEdit_.get() );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
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
        Core::ActionManager * am = Core::ActionManager::instance();
        if ( am ) {
            // print, method file open & save buttons
            toolBarLayout->addWidget(toolButton(am->command(Constants::PRINT_CURRENT_VIEW)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::SAVE_CURRENT_IMAGE)->action()));
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //----------
            Core::Context context( ( Core::Id( Core::Constants::C_GLOBAL ) ) );
            
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );

            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

            toolBarLayout->addWidget( toolButton( am->command( Constants::HIDE_DOCK )->action() ) );
            
        }
		return toolBar;
    }
    return 0;
}

void
MainWindow::createActions( const Core::Context& context )
{
    Core::ActionContainer * menu = Core::ActionManager::instance()->createMenu( Constants::MENU_ID ); // Menu ID

    if ( !menu )
        return;
    
    menu->menu()->setTitle( "U5303A" );

    // const Core::Context context( (Core::Id( Core::Constants::C_GLOBAL )) );
    
    if ( auto action = createAction( Constants::ICON_SNAPSHOT, tr( "Snapshot" ), this ) ) {
        connect( action, &QAction::triggered, [this](){ actSnapshot(); } );
        action->setEnabled( false );
        Core::Command * cmd = Core::ActionManager::registerAction( action, Constants::ACTION_SNAPSHOT, context );
        menu->addAction( cmd );
    }
    if ( auto action = createAction( Constants::ICON_CONNECT, tr( "Connect" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionConnect(); } );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_CONNECT, context );
        menu->addAction( cmd );        
    }

    if ( auto action = createAction( Constants::ICON_RUN, tr( "Run" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->start_run(); } );
        action->setEnabled( false );        
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_RUN, context );
        menu->addAction( cmd );        
    }

    if ( auto action = createAction( Constants::ICON_STOP, tr( "Stop" ), this ) ) {
        //connect( action, &QAction::triggered, [] () { document::instance()->actionRun( !document::instance()->isRecording() ); } );
        connect( action, &QAction::triggered, [] () { document::instance()->stop(); } );
        action->setEnabled( false );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_STOP, context );
        menu->addAction( cmd );        
    }

    do {
        QIcon icon;
        icon.addPixmap( QPixmap( Constants::ICON_REC_ON ), QIcon::Normal, QIcon::On );
        icon.addPixmap( QPixmap( Constants::ICON_REC_PAUSE ), QIcon::Normal, QIcon::Off );
        if ( auto action = new QAction( icon, tr( "REC" ), this ) ) {
            action->setCheckable( true );
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
MainWindow::actSnapshot()
{
    document::instance()->takeSnapshot();
}

void
MainWindow::handle_reply( const QString& method, const QString& reply )
{
	auto docks = dockWidgets();
    auto it = std::find_if( docks.begin(), docks.end(), []( const QDockWidget * w ){ return w->objectName() == "Log"; });
    if ( it != docks.end() ) {
		if ( auto edit = dynamic_cast< QTextEdit * >( (*it)->widget() ) )
			edit->append( QString("%1: %2").arg( method, reply ) );
	}
}

void
MainWindow::handleInstState( int status )
{
    if ( status <= adicontroller::Instrument::eNotConnected ) {

        if ( auto action = Core::ActionManager::instance()->command( Constants::ACTION_CONNECT )->action() )
            action->setEnabled( true  );

        for ( auto id : { Constants::ACTION_RUN, Constants::ACTION_STOP, Constants::ACTION_REC, Constants::ACTION_SNAPSHOT } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( false );
        }

    } else if ( status >= adicontroller::Instrument::eStandBy ) {

        if ( auto action = Core::ActionManager::command( Constants::ACTION_CONNECT )->action() )
            action->setEnabled( false );
        
        for ( auto id :
            { Constants::ACTION_RUN, Constants::ACTION_STOP, Constants::ACTION_REC, Constants::ACTION_SNAPSHOT, Constants::ACTION_SYNC } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( true );
        }
    }

}

void
MainWindow::setControlMethod( std::shared_ptr< const adcontrols::ControlMethod::Method> m )
{
    boost::any a( m );
    for ( auto dock: dockWidgets() ) {
        if ( auto widget = qobject_cast<adplugin::LifeCycle *>( dock->widget() ) ) {
            widget->setContents( a );
        }
    }
}

std::shared_ptr< adcontrols::ControlMethod::Method >
MainWindow::getControlMethod() const
{
    auto ptr = std::make_shared< adcontrols::ControlMethod::Method >();
    boost::any a( ptr );
    for ( auto dock: dockWidgets() ) {
        if ( auto widget = qobject_cast<adplugin::LifeCycle *>( dock->widget() ) ) {
            widget->getContents( a );
        }
    }
    return ptr;
}

void
MainWindow::getEditorFactories( adextension::iSequenceImpl& impl )
{
    if ( std::shared_ptr< const adextension::iEditorFactory > p
         = std::make_shared< adextension::iEditorFactoryV< acqrswidgets::ThresholdWidget, QString, int > >(
         QString( "U5303A Threshold" ), adextension::iEditorFactory::CONTROL_METHOD, QString("u5303a"), 1 ) ) {
        impl << p;
    }

    if ( std::shared_ptr< const adextension::iEditorFactory > p
         = std::make_shared< adextension::iEditorFactoryT< acqrswidgets::u5303AWidget > >("U5303A", adextension::iEditorFactory::CONTROL_METHOD ) ) {
        impl << p;
    }
}

void
MainWindow::editor_commit()
{
    // editor_->commit();
    // todo...
}

void
MainWindow::saveCurrentImage()
{
    qApp->beep();
    if ( auto screen = QGuiApplication::primaryScreen() ) {
        
        auto pixmap = QPixmap::grabWidget( this );

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
            o << L"u5303a_" << std::setw( 4 ) << std::setfill( L'0' ) << runno + 1;
            path /= o.str();
            path.replace_extension( ".png" );

            pixmap.save( QString::fromStdWString( path.wstring() ), "png" );
        }
    }
}

void
MainWindow::printCurrentView()
{
    saveCurrentImage();
}

void
MainWindow::iControllerConnected( adextension::iController * inst )
{
    if ( inst ) {
        QString model = inst->module_name();
        for ( auto dock : dockWidgets() ) {
            if ( auto receiver = qobject_cast<adextension::iReceiver *>( dock->widget() ) ) {
                receiver->onConnected( inst );
            }
        }
    }
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
