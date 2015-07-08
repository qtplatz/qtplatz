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
#include "waveformwnd.hpp"
#include "document.hpp"
#include "isequenceimpl.hpp"
#include "ap240_constants.hpp"
#include <ap240w/ap240form.hpp>
#include <ap240/digitizer.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <adlog/logger.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adinterface/controlserver.hpp>
#include <adportable/date_string.hpp>
#include <adportable/profile.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycleaccessor.hpp>
#include <adwidgets/controlmethodwidget.hpp>
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

namespace ap240 {

    template<class T> class iEditorFactoryT : public adextension::iEditorFactory {

        MainWindow& mainWindow_;
        QString title_;
        QString itemname_;

	public:
        iEditorFactoryT( MainWindow& w
                         , const QString& title ) : mainWindow_( w )
                                                  , title_( title ) {
        }

        QWidget * createEditor( QWidget * parent ) {
			return new T( parent );
		}

        QString title() const { return title_; }
		
        adextension::iEditorFactory::METHOD_TYPE method_type() const {
			return adextension::iEditorFactory::CONTROL_METHOD;
		}
    };
}

using namespace ap240;

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
    auto widget = new ap240form();
    createDockWidget( widget, "AP240", "AP240" );
    connect( widget, &ap240form::valueChanged, [this]( ap240form::idCategory cat, int, int ch, const QVariant& v ){
            if ( cat == ap240form::idThreshold ) {
                document::instance()->setThreshold( ch, v.toDouble() );
            } else if ( auto form = findChild< ap240form * >() ) {
                ap240::method m;
                form->get( m );
                document::instance()->setControlMethod( m, QString() );
            }
        });
}

void
MainWindow::OnInitialUpdate()
{
    if ( auto form = findChild< ap240form *>() ) {
        form->OnInitialUpdate();
        boost::any ptr( document::instance()->controlMethod() );
        form->getContents( ptr );
    }

    setSimpleDockWidgetArrangement();

    connect( document::instance(), SIGNAL( on_reply(const QString&, const QString&) )
             , this, SLOT( handle_reply( const QString&, const QString& ) ) );
    
    connect( document::instance(), SIGNAL( on_status(int) ), this, SLOT( handle_status(int) ) );
    for ( auto action: actions_ )
        action->setEnabled( false );
    actions_[ idActConnect ]->setEnabled( true );

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
        editorHolderLayout->addWidget( new WaveformWnd() );
        
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

void
MainWindow::setData( const adcontrols::MassSpectrum& )
{
    // if ( monitorView_ )
    //     monitorView_->setData( ms );
}

void
MainWindow::setData( const adcontrols::Trace&, const std::wstring& )
{
    // if ( monitorView_ )
    //     monitorView_->setData( trace, traceId );
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
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_CONNECT)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_INITRUN)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_RUN)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_STOP)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_INJECT)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACT_SNAPSHOT)->action()));
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
            //toolBarLayout->addWidget(toolButton(am->command(Constants::PRINT_CURRENT_VIEW)->action()));
            //toolBarLayout->addWidget(toolButton(am->command(Constants::METHOD_OPEN)->action()));
            // [file open] button
            toolBarLayout->addWidget(toolButton(am->command(Constants::FILE_OPEN)->action()));
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //----------
            Core::Context context( ( Core::Id( Core::Constants::C_GLOBAL ) ) );
            
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );

            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        }
		return toolBar;
    }
    return 0;
}

void
MainWindow::createActions()
{
    // enum idActions { idActConnect, idActInitRun, idActRun, idActStop, idActSnapshot, idActInject, idActFileOpen, numActions };

    actions_[ idActConnect ] = createAction( Constants::ICON_CONNECT,  tr("Connect"), this );    
    actions_[ idActInitRun ] = createAction( Constants::ICON_INITRUN,  tr("Initial run"), this );    
    actions_[ idActRun ]     = createAction( Constants::ICON_RUN,      tr("Run"), this );    
    actions_[ idActStop ]    = createAction( Constants::ICON_STOP,     tr("Stop"), this );    
    actions_[ idActSnapshot ]= createAction( Constants::ICON_SNAPSHOT, tr("Snapshot"), this );    
    actions_[ idActInject ]  = createAction( Constants::ICON_INJECT,   tr("INJECT"), this );
    actions_[ idActFileOpen ]= createAction( Constants::ICON_FILE_OPEN,tr("Open protain file..."), this );
    connect( actions_[ idActConnect ], SIGNAL( triggered() ), this, SLOT( actConnect() ) );
    connect( actions_[ idActInitRun ], SIGNAL( triggered() ), this, SLOT( actInitRun() ) );
    connect( actions_[ idActRun ], SIGNAL( triggered() ), this, SLOT( actRun() ) );
    connect( actions_[ idActStop ], SIGNAL( triggered() ), this, SLOT( actStop() ) );
    connect( actions_[ idActInject ], SIGNAL( triggered() ), this, SLOT( actInject() ) );
    connect( actions_[ idActSnapshot ], SIGNAL( triggered() ), this, SLOT( actSnapshot() ) );
    connect( actions_[ idActFileOpen ], SIGNAL( triggered() ), this, SLOT( actFileOpen() ) );

    const Core::Context gc( (Core::Id( Core::Constants::C_GLOBAL )) );

    if ( Core::ActionManager * am = Core::ActionManager::instance() ) {

        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( "AP240" );

        Core::Command * cmd = 0;
        cmd = am->registerAction( actions_[ idActConnect ], Constants::ACT_CONNECT, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActInitRun ], Constants::ACT_INITRUN, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActRun ], Constants::ACT_RUN, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActStop ], Constants::ACT_STOP, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActInject ], Constants::ACT_INJECT, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActSnapshot ], Constants::ACT_SNAPSHOT, gc );
        menu->addAction( cmd );
        cmd = am->registerAction( actions_[ idActFileOpen ], Constants::FILE_OPEN, gc );
        menu->addAction( cmd );

        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}

QAction *
MainWindow::createAction( const QString& iconname, const QString& msg, QObject * parent )
{
    QIcon icon;
    icon.addFile( iconname );
    return new QAction( icon, msg, parent );
}

void
MainWindow::actConnect()
{
    document::instance()->ap240_connect();
}

void
MainWindow::actInitRun()
{
    document::instance()->prepare_for_run();
}

void
MainWindow::actRun()
{
    document::instance()->ap240_start_run();
}

void
MainWindow::actStop()
{
    document::instance()->ap240_stop();
}

void
MainWindow::actSnapshot()
{
    auto waveforms = document::instance()->findWaveform();
    adcontrols::MassSpectrum ms;
    for ( auto waveform: { waveforms.first, waveforms.second } ) {
        
        if ( waveform ) {
            
            if ( document::toMassSpectrum( ms, *waveform ) ) {
                
                boost::filesystem::path path( adportable::profile::user_data_dir<char>() );
                path /= "data";
                path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
                if ( ! boost::filesystem::exists( path ) ) {
                    boost::system::error_code ec;
                    boost::filesystem::create_directories( path, ec );
                }
                path /= "ap240.adfs";
                std::wstring title = ( boost::wformat( L"Spectrum %1%" ) % waveform->serialnumber_ ).str();
                std::wstring folderId;
                if ( document::appendOnFile( path.wstring(), title, ms, folderId ) ) {
                    auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSnapshotHandler >();
                    for ( auto handler: vec )
                        handler->folium_added( path.string().c_str(), "/Processed/Spectra", QString::fromStdWString( folderId ) );
                }
            }
        }
    }
}

void
MainWindow::actInject()
{
    document::instance()->ap240_trigger_inject();
}

void
MainWindow::actFileOpen()
{
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
MainWindow::handle_status( int status )
{
    if ( status == controlserver::eStandBy ) {
        for ( auto action: actions_ )
            action->setEnabled( true );
        actions_[ idActConnect ]->setEnabled( false );
        actInitRun();
        //mw->onStatus( status );
    }
}

bool
MainWindow::editor_factories( iSequenceImpl& impl )
{
    return true;        
}


void
MainWindow::setControlMethod( const ap240::method& m )
{
    if ( auto form = findChild< ap240form * >() ) {
        form->set( m );
    }
}

void
MainWindow::getControlMethod( ap240::method& m )
{
    if ( auto form = findChild< ap240form * >() ) {
        form->get( m );
    }
}

void
MainWindow::editor_commit()
{
    
    // editor_->commit();
    // todo...
}
