/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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
#include "idgmodimpl.hpp"
#include <adacquire/constants.hpp>
#include <adcontrols/controlmethod.hpp>
// #include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/xchromatogramsmethod.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/samplerun.hpp>
#include <adextension/icontroller.hpp>
#include <adextension/ieditorfactory_t.hpp>
#include <adextension/ireceiver.hpp>
#include <adextension/isequenceimpl.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adlog/logger.hpp>
#include <adplugin/lifecycle.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/split_filename.hpp>
#include <adwidgets/adtracewidget.hpp>
#include <adwidgets/cherrypicker.hpp>
#include <adwidgets/countingwidget.hpp>
#include <adwidgets/dgwidget.hpp>
#include <adwidgets/outputwidget.hpp>
#include <adwidgets/samplerunwidget.hpp>
#include <adwidgets/xchromatogramswidget.hpp>
#include <qtwrapper/make_widget.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/plugin_manager.hpp>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/imode.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/exception/all.hpp>
#include <boost/uuid/uuid.hpp>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QTabBar>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <qdebug.h>
#include <boost/uuid/uuid_io.hpp>
#include <boost/json.hpp>
#include <csignal>
#include <filesystem>

using namespace acquire;

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
    QFile file( ":/acquire/stylesheet/tabbar.qss" );
    file.open( QFile::ReadOnly );
    QString tabStyle( file.readAll() );


    if ( auto widget = new adwidgets::SampleRunWidget ) {
        createDockWidget( widget, "Sample Run", "SampleRunWidget" );
        connect( widget, &adwidgets::SampleRunWidget::apply
                 , [this,widget](){
                       auto sampleRun = std::make_shared< adcontrols::SampleRun >();
                       widget->getSampleRun( *sampleRun );
                       ADDEBUG() << "************ SampleRun apply method time: " << sampleRun->methodTime();
                       if ( auto edit = findChild< QLineEdit *>( "dataSaveIn" ) )
                           edit->setText( QString::fromStdWString( sampleRun->dataDirectory() ) );
                       this->setSampleRun( *sampleRun );
                   });
    }

    if ( auto widget = qtwrapper::make_widget< adwidgets::ADTraceWidget >( "adTraces" ) ) {
        createDockWidget( widget, "ADC", "ADTraces" );
        connect( widget, &adwidgets::ADTraceWidget::dataChanged
                 , [](int row, int column) {
                       document::instance()->handleTraceMethodChanged();
                   });
        connect( document::instance(), &document::on_auto_zero_changed, widget, &adwidgets::ADTraceWidget::handleVOffsets );
    }


    if ( auto sse = qtwrapper::make_widget< adwidgets::dgWidget >( "delayPulseMonitor" ) ) {

        createDockWidget( sse, "ACQUIRE", "ACQUIRE" );

        connect( sse, &adwidgets::dgWidget::hostChanged, this
                 , [sse](const QString& host, const QString& port ){
                       document::instance()->set_http_addr( host, port );
                       sse->setUrl( host, port );
                   });

        connect( document::instance(), &document::onTick, this
                 , [sse]( const QByteArray& data ){
                       sse->handleTick( data );
                   });

        connect( document::instance(), &document::onDelayPulseData, this
                 , [sse]( const QByteArray& data ){
                       sse->handleDelayPulseData( data );
                   });
    }

    if ( auto widget = qtwrapper::make_widget< adwidgets::OutputWidget >( "Output", document::instance()->console() ) ) {
        createDockWidget( widget, "Output", "Output" );
        document::instance()->console() << "Hello World" << std::endl;
        connect( widget, &adwidgets::OutputWidget::onInputLine
                 , []( const QString& line ){
                       document::instance()->handleConsoleIn( line );
                   });
    }

    if ( auto widget = qtwrapper::make_widget< adwidgets::XChromatogramsWidget >( "tofChromatograms" ) ) {
        createDockWidget( widget, "Chromatograms", "Chromatograms" );
        connect( widget, &adwidgets::XChromatogramsWidget::valueChanged, this, &MainWindow::handleXChromatogramsMethod );
                 // , [widget](){
                 //     document::instance()->set_tof_chromatograms_method( widget->readJson(), true );
                 // });
    }

    if ( auto widget = qtwrapper::make_widget< adwidgets::CherryPicker >("ModulePicker") ) {
        createDockWidget( widget, "Modules", "CherryPicker" );
        connect( widget, &adwidgets::CherryPicker::stateChanged
                 , []( const QString& key, bool enable ){
                       document::instance()->setControllerSettings( key, enable );
                   });
    }

#if defined Q_OS_LINUX
    for ( auto dock: dockWidgets() ) {
        dock->widget()->setStyleSheet( "* { font-size: 9pt; }"
                                       "QHeaderView::section { font-size: 9pt; }" );
    }

    for ( auto tabbar: findChildren< QTabBar * >() )
        tabbar->setStyleSheet( "QTabBar { font-size: 9pt; }" );
#endif
}

size_t
MainWindow::findInstControllers( std::vector< std::shared_ptr< adextension::iController > >& vec ) const
{
#if QTC_VERSION >= 0x08'00'00
    for ( auto v: qtwrapper::plugin_manager::getObjects< adextension::iController >() ) {
        try {
            vec.emplace_back( v->shared_from_this() );
        } catch ( std::bad_weak_ptr& ) {
            ADWARN() << "adextension::iController does not have weak_ptr -- maybe deprecated plugin instance";
        }
    }
#else
    for ( auto v: ExtensionSystem::PluginManager::getObjects< adextension::iController >() ) {
        try {
            vec.push_back( v->shared_from_this() );
        } catch ( std::bad_weak_ptr& ) {
            ADWARN() << "adextension::iController does not have weak_ptr -- maybe deprecated plugin instance";
        }
    }
#endif
    return vec.size();
}

void
MainWindow::OnInitialUpdate()
{
    connect( document::instance(), &document::instStateChanged, this, &MainWindow::handleInstState );

    connect( document::instance(), &document::onModulesFailed, this, &MainWindow::handleModulesFailed );

    connect( document::instance(), &document::sampleRunChanged, this, [this] {
            setSampleRun( *document::instance()->sampleRun() );
        });

    // if ( auto w = findChild< OperationForm * >() )
    //     w->setEnabled( false );

    for ( auto dock: dockWidgets() ) {
        if ( auto widget = qobject_cast<adplugin::LifeCycle *>( dock->widget() ) ) {
            widget->setContents( boost::any( document::instance()->controlMethod() ) ); // document -> GUI
            widget->OnInitialUpdate();
        }
    }

    // Set control method name on MidToolBar
    if ( auto edit = findChild< QLineEdit * >( "methodName" ) ) {
        QString methodName = document::instance()->recentFile( Constants::GRP_METHOD_FILES, false );
        if ( ! methodName.isEmpty() )
            edit->setText( methodName );
    }

    setSimpleDockWidgetArrangement();
    QVariant state = document::instance()->settings()->value( "DOCK_LOCATIONS" );
    restoreState( state.toByteArray() );

    connect( document::instance(), &document::on_reply, this, &MainWindow::handle_reply );

    for ( auto id : { Constants::ACTION_RUN, Constants::ACTION_STOP, Constants::ACTION_REC, Constants::ACTION_SNAPSHOT } ) {
        if ( auto action = Core::ActionManager::command( id )->action() )
            action->setEnabled( false );
    }

#if QTC_VERSION >= 0x08'00'00
    for ( auto iController: qtwrapper::plugin_manager::getObjects< adextension::iController >() ) {
        document::instance()->addInstController( iController );
    }
#else
    for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
        document::instance()->addInstController( iController );
    }
#endif
    // initialize module picker
    if ( auto picker = findChild< adwidgets::CherryPicker * >( "ModulePicker" ) ) {

#if QTC_VERSION >= 0x08'00'00
        for ( auto iController: qtwrapper::plugin_manager::getObjects< adextension::iController >() ) {
            bool checked = document::instance()->isControllerEnabled( iController->module_name() );
            bool enabled = !document::instance()->isControllerBlocked( iController->module_name() );
            picker->addItem( iController->module_name(), iController->module_name(), checked, enabled );
        }
#else
        for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
            bool checked = document::instance()->isControllerEnabled( iController->module_name() );
            bool enabled = !document::instance()->isControllerBlocked( iController->module_name() );
            picker->addItem( iController->module_name(), iController->module_name(), checked, enabled );
        }
#endif

        connect( document::instance(), &document::moduleConfigChanged, this
                 , [picker](){
                       for ( auto& module: document::instance()->controllerSettings() ) {
                           picker->setChecked( module.first, module.second );
                       }
                   });
    }

    // if ( auto widget = findChild< acquirewidgets::ThresholdWidget * >() ) {
    //     widget->setJson( document::instance()->threshold_method().toJson( QJsonDocument::Compact ) );
    //     widget->setJson( document::instance()->threshold_action().toJson( QJsonDocument::Compact ) );
    // }

    if ( WaveformWnd * wnd = centralWidget()->findChild<WaveformWnd *>() ) {
        wnd->onInitialUpdate();
        connect( document::instance(), &document::on_threshold_method_changed, wnd, &WaveformWnd::handle_threshold_method );
        connect( document::instance(), &document::on_threshold_action_changed, wnd, &WaveformWnd::handle_threshold_action );
        connect( document::instance(), &document::on_threshold_level_changed, wnd, &WaveformWnd::handle_threshold_level );
    }

    if ( auto w = findChild< adwidgets::dgWidget * >( "delayPulseMonitor" ) ) {
        QString host, port;
        std::tie( host, port ) = document::instance()->http_addr();
        w->setUrl( host, port ); // QString("http://%1:%2").arg( host, port ) );
    }

    if ( auto widget = findChild< adwidgets::XChromatogramsWidget * >( "tofChromatograms" ) ) {
        widget->setValue( *document::instance()->xChromatogramsMethod() );
        // QByteArray json = document::instance()->tof_chromatograms_method();
        // if ( !json.isEmpty() )
        //     widget->setJson( json );
    }

    if ( auto btn = findChild< QPushButton * >( "btnZERO" ) ) {
        connect( btn, &QPushButton::clicked, []{ document::instance()->setAutoZero(); } );
    }

}

void
MainWindow::OnFinalClose()
{
    document::instance()->settings()->setValue( "DOCK_LOCATIONS", saveState() );

    for ( auto dock: dockWidgets() ) {
        //adplugin::LifeCycleAccessor accessor( dock->widget() );
        if ( auto editor = qobject_cast< adplugin::LifeCycle * >( dock->widget() ) ) // accessor.get() ) {
            editor->OnFinalClose();
    }
}

void
MainWindow::activateLayout()
{
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;
	editorHolderLayout->setContentsMargins( {} );
	editorHolderLayout->setSpacing( 0 );

    // handle ControlMethod (load/save)
    connect( document::instance(), &document::onControlMethodChanged, this, [this]( const QString& name ) {
            this->setControlMethod( document::instance()->controlMethod() );
            if ( !name.isEmpty() ) {
                if ( auto edit = findChild< QLineEdit * >( "methodName" ) )
                    edit->setText( name );
            }
        } );

    if ( QWidget * editorWidget = new QWidget ) {

        editorWidget->setLayout( editorHolderLayout );

        editorHolderLayout->addWidget( createTopStyledToolbar() );
        if ( auto wnd = new WaveformWnd() ) {
            editorHolderLayout->addWidget( wnd );
            //connect( document::instance(), &document::on_threshold_method_changed, wnd, &WaveformWnd::handle_threshold_method );
            //connect( document::instance(), &document::onControlMethodChanged, wnd, &WaveformWnd::handle_method );
            // validation for uuid class registoration -- will be compile error if not registered
            QVariant v;
            v.setValue( boost::uuids::uuid() );
        }

        //---------- central widget ------------
        if ( QWidget * centralWidget = new QWidget ) {

            setCentralWidget( centralWidget );

            QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
            centralWidget->setLayout( centralLayout );
            centralLayout->setContentsMargins( {} );
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
#if QTC_VERSION <= 0x03'02'81
        QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
#else
        QWidget * outputPane = new Core::OutputPanePlaceHolder( mode->id(), mainWindowSplitter );
#endif
        outputPane->setObjectName( QLatin1String( "SequenceOutputPanePlaceHolder" ) );

        mainWindowSplitter->addWidget( this );        // [Central Window]
        mainWindowSplitter->addWidget( outputPane );  // [Output (log) Window]

        mainWindowSplitter->setStretchFactor( 0, 9 );
        mainWindowSplitter->setStretchFactor( 1, 1 );
        mainWindowSplitter->setOrientation( Qt::Vertical );

        // Split Navigation and Application window
        Core::MiniSplitter * splitter = new Core::MiniSplitter;               // entier this view
        if ( splitter ) {
            //splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) ); // navegate
            splitter->addWidget( mainWindowSplitter );                            // *this + ontput
            //splitter->setStretchFactor( 0, 0 );
            //splitter->setStretchFactor( 1, 1 );
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

    //const std::string left = "ACQUIRE;ACQUIREThreshold"; // ;SampleRunWidget";
    const std::string left = "SampleRunWidget";

    for ( auto widget : dockWidgets() ) {
        widget->setFloating( false );
        removeDockWidget( widget );
    }

    QList< QDockWidget * > left_widgets, right_widgets;

    for ( auto widget : dockWidgets() ) {
        auto objname = widget->objectName().toStdString();
        if ( left.find( objname ) != std::string::npos )
            left_widgets.push_back( widget );
        else
            right_widgets.push_back( widget );
    }

    for ( auto& list : { left_widgets, right_widgets } ) {
        int idx( 0 );
        for ( auto& widget : list ) {
            addDockWidget( Qt::BottomDockWidgetArea, widget );
            if ( idx++ )
                tabifyDockWidget( list.at( 0 ), widget );
            widget->show();
        }
    }
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
        toolBarLayout->setContentsMargins( {} );
        toolBarLayout->setSpacing( 0 );
        if ( auto am = Core::ActionManager::instance() ) {
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_CONNECT)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_RUN)->action()));
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //---
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_INJECT)->action()));
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );

            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_STOP)->action()));

            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_REC)->action()));

            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_SYNC)->action()));

            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_SNAPSHOT)->action()));
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //---
            //toolBarLayout->addWidget( topLineEdit_.get() );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

        toolBarLayout->addWidget( qtwrapper::make_widget< QPushButton >( "btnZERO", "ZERO" ) );

    }
    return toolBar;
}

Utils::StyledBar *
MainWindow::createMidStyledToolbar()
{
    if ( Utils::StyledBar * toolBar = new Utils::StyledBar ) {

        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setContentsMargins( {} );
        toolBarLayout->setSpacing(0);
        Core::ActionManager * am = Core::ActionManager::instance();
        if ( am ) {
            // print, method file open & save buttons
            toolBarLayout->addWidget(toolButton(am->command(Constants::PRINT_CURRENT_VIEW)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::SAVE_CURRENT_IMAGE)->action()));
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //----------
            auto sampleRun = document::instance()->sampleRun();
            if ( auto label = new QLabel ) {
                label->setText( tr("Data save in:" ) );
                toolBarLayout->addWidget( label );
            }
            if ( auto edit = new QLineEdit ) {
                edit->setObjectName( "dataSaveIn" );
                edit->setReadOnly( true );
                toolBarLayout->addWidget( edit );
                edit->setText( QString::fromStdWString( sampleRun->dataDirectory() ) );
                //edit->setClearButtonEnabled( true );
                auto icon = QIcon( Constants::ICON_FOLDER_OPEN );
                if ( auto action = edit->addAction( icon, QLineEdit::ActionPosition::TrailingPosition ) )
                    connect( action, &QAction::triggered, this, &MainWindow::handleDataSaveIn );
            }

            if ( auto edit = new QLineEdit ) {
                edit->setObjectName( "runName" );
                edit->setReadOnly( true );
                edit->setText( QString::fromStdWString( sampleRun->filePrefix() ) );
                edit->setFixedWidth( 120 );
                toolBarLayout->addWidget( edit );
                auto icon = QIcon( Constants::ICON_FILE_OPEN );
                if ( auto action = edit->addAction( icon, QLineEdit::ActionPosition::TrailingPosition ) )
                    connect( action, &QAction::triggered, this, &MainWindow::handleRunName );
            }

            toolBarLayout->addItem( new QSpacerItem(32, 20, QSizePolicy::Minimum, QSizePolicy::Minimum) );
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            if ( auto label = new QLabel ) {
                label->setText( tr("Control Method:" ) );
                toolBarLayout->addWidget( label );
            }
            if ( auto edit = new QLineEdit ) {
                edit->setObjectName( "methodName" );
                edit->setText( "" );
                auto openIcon = QIcon( Constants::ICON_FILE_OPEN );
                if ( auto action = edit->addAction( openIcon, QLineEdit::ActionPosition::TrailingPosition ) )
                    connect( action, &QAction::triggered, this, &MainWindow::handleControlMethodOpen );
                auto saveIcon = QIcon( Constants::ICON_FILE_SAVE );
                if ( auto action = edit->addAction( saveIcon, QLineEdit::ActionPosition::TrailingPosition ) )
                    connect( action, &QAction::triggered, this, &MainWindow::handleControlMethodSaveAs );

                toolBarLayout->addWidget( edit );
            }

            toolBarLayout->addItem( new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

            toolBarLayout->addWidget( toolButton( am->command( Constants::HIDE_DOCK )->action() ) );
        }
		return toolBar;
    }
    return 0;
}

void
MainWindow::createActions()
{
    Core::ActionContainer * menu = Core::ActionManager::instance()->createMenu( Constants::MENU_ID ); // Menu ID

    if ( !menu )
        return;

    const Core::Context context( (Utils::Id( Core::Constants::C_GLOBAL ) ) );

    menu->menu()->setTitle( "ACQUIRE" );


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
        connect( action, &QAction::triggered, [] () { document::instance()->actionRun(); } );
        action->setEnabled( false );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_RUN, context );
        menu->addAction( cmd );
    }

    if ( auto action = createAction( Constants::ICON_INJECT, tr( "Inject" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionInject(); } );
        action->setEnabled( false );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_INJECT, context );
        menu->addAction( cmd );
    }

    if ( auto action = createAction( Constants::ICON_STOP, tr( "Stop" ), this ) ) {
        connect( action, &QAction::triggered, [] () { document::instance()->actionStop(); } );
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
            // action->setChecked( true );
            action->setEnabled( false );
            auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_REC, context );
            menu->addAction( cmd );
            connect( action, &QAction::triggered, [](bool rec){
                document::instance()->actionRec(rec);
                if ( auto action = Core::ActionManager::command(Constants::ACTION_REC)->action() )
                    if ( !action->isEnabled() )
                        action->setEnabled( true );
            });
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
MainWindow::handleModulesFailed( const QStringList& list )
{
    if ( list.isEmpty() )
        return;

    QString msg( "Instrument connection failed with: " );
    for ( auto& x: list )
        msg += x;

    QMessageBox::warning( this, tr("QtPlatz/acquireplugin"), msg );
}


void
MainWindow::handleInstState( int status )
{
    // if ( auto w = findChild< OperationForm * >() )
    //     w->setEnabled( status > adacquire::Instrument::eNotConnected );

    if ( status > adacquire::Instrument::eNotConnected ) {
        if ( auto action = Core::ActionManager::instance()->command( Constants::ACTION_CONNECT )->action() )
            action->setEnabled( false );
    } else {
        if ( auto action = Core::ActionManager::instance()->command( Constants::ACTION_CONNECT )->action() )
            action->setEnabled( true  );
        for ( auto id : { Constants::ACTION_RUN, Constants::ACTION_STOP, Constants::ACTION_REC, Constants::ACTION_SNAPSHOT } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( false );
        }
    }

    if ( status == adacquire::Instrument::eStandBy ) {
        ADDEBUG() << "STATE CHANGE TO: " << "Stand By";
        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, true )
                    , std::make_pair( Constants::ACTION_STOP, true )
                    , std::make_pair( Constants::ACTION_REC, true )
                    , std::make_pair( Constants::ACTION_INJECT, false ) // <== false
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true ) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() ) {
                action->setEnabled( pair.second );
            }
        }

    } else if ( status == adacquire::Instrument::ePreparingForRun || status == adacquire::Instrument::eReadyForRun ) {
        ADDEBUG() << "STATE CHANGE TO: " << ( status == adacquire::Instrument::eReadyForRun ? "Ready for Run" : "Preparing for Run" );
        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, false )
                    , std::make_pair( Constants::ACTION_STOP, true )
                    , std::make_pair( Constants::ACTION_REC, true )
                    , std::make_pair( Constants::ACTION_INJECT, false ) // <== false
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() ) {
                action->setEnabled( pair.second );
            }
        }

    } else if ( status == adacquire::Instrument::eWaitingForContactClosure ) {
        ADDEBUG() << "STATE CHANGE TO: " << "Waiting for Contact Closure";
        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, false )
                    , std::make_pair( Constants::ACTION_STOP, true )
                    , std::make_pair( Constants::ACTION_REC, true )
                    , std::make_pair( Constants::ACTION_INJECT, true )  // <== true
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() ) {
                action->setEnabled( pair.second );
            }
        }

    } else if ( status == adacquire::Instrument::eRunning ) {
        ADDEBUG() << "STATE CHANGE TO: " << "Running";
        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, true )
                    , std::make_pair( Constants::ACTION_STOP, true )
                    , std::make_pair( Constants::ACTION_REC, true )
                    , std::make_pair( Constants::ACTION_INJECT, false ) // <== false
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true ) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() ) {
                action->setEnabled( pair.second );
            }
        }

    } else if ( status == adacquire::Instrument::eStop ) {
        ADDEBUG() << "STATE CHANGE TO: " << "Stop";
        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, true )
                    , std::make_pair( Constants::ACTION_STOP, false )
                    , std::make_pair( Constants::ACTION_REC, false )
                    , std::make_pair( Constants::ACTION_INJECT, false ) // <== false
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true ) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() ) {
                action->setEnabled( pair.second );
            }
        }
    }
}

void
MainWindow::setControlMethod( std::shared_ptr< const adcontrols::ControlMethod::Method> m )
{
    for ( auto dock: dockWidgets() ) {
        if ( auto widget = qobject_cast<adplugin::LifeCycle *>( dock->widget() ) ) {
            widget->setContents( boost::any(m) );
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
    ADDEBUG() << "saveCurrentImage()";
    qApp->beep();
    if ( auto screen = QGuiApplication::primaryScreen() ) {

        auto pixmap = this->grab(); // QPixmap::grabWidget( this );

        if ( auto sample = document::instance()->sampleRun() ) {
            std::filesystem::path path( sample->dataDirectory() );
            if ( ! std::filesystem::exists( path ) ) {
                std::error_code ec;
                std::filesystem::create_directories( path, ec );
            }
            int runno(0);
            if ( std::filesystem::exists( path ) && std::filesystem::is_directory( path ) ) {
                using std::filesystem::directory_iterator;
                for ( directory_iterator it( path ); it != directory_iterator(); ++it ) {
                    std::filesystem::path fname = (*it);
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

void
MainWindow::handleDataSaveIn()
{
    QString dstfile;

    if ( auto edit = findChild< QLineEdit * >( "dataSaveIn" ) ) {
        dstfile = edit->text();
        if ( dstfile.isEmpty() )
            dstfile = QString::fromStdWString( document::instance()->sampleRun()->dataDirectory() );
        try {
            QString dir = QFileDialog::getExistingDirectory( this, tr( "Data save in" )
                                                             , dstfile, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
            if ( !dir.isEmpty() ) {
                edit->setText( dir );
                auto next = std::make_shared< adcontrols::SampleRun >( *document::instance()->sampleRun() );
                next->setDataDirectory( dir.toStdWString() );
                document::instance()->setSampleRun( next );
                this->setSampleRun( *next );
            }

        } catch ( ... ) {
            ADTRACE() << "Hit QTBUG-33119 that has no workaround right now.  Please be patient and try it again.";
            QMessageBox::information( this, "InfiTOF2 MainWindow", "Hit QTBUG-33119 - no workaround. Please be patient and try it again." );
        }
    }
}

void
MainWindow::handleRunName()
{
    QString dstfile;

    if ( auto folder = findChild< QLineEdit * >( "dataSaveIn" ) ) {
        dstfile = folder->text();

        if ( dstfile.isEmpty() )
            dstfile = QString::fromStdWString( document::instance()->sampleRun()->dataDirectory() );

        std::filesystem::path path( dstfile.toStdWString() );

        if ( auto rname = findChild< QLineEdit * >( "runName" ) ) {

            QString name = rname->text();
            QString file;
            try {
                file = QFileDialog::getSaveFileName( this
                                                     , tr( "Run Name" )
                                                     , dstfile + "/" + name
                                                     , tr( "DATA(*.txt *.adfs)" ) );
            } catch ( ... ) {
                ADTRACE() << "Hit QTBUG-33119 that has no workaround right now.  Please be patient and try it again.";
                QMessageBox::information( this, "InfiTOF2 MainWindow", "Hit QTBUG-33119 - no workaround. Please be patient and try it again." );
            }

            if ( !file.isEmpty() ) {

                std::filesystem::path fname( file.toStdString() );

                auto dir = fname.parent_path();
                auto stem = fname.stem();

                folder->setText( QString::fromStdWString( dir.wstring() ) );
                rname->setText( QString::fromStdWString( stem.wstring() ) );

                auto sampleRun = std::make_shared< adcontrols::SampleRun >( *document::instance()->sampleRun() );
                sampleRun->setDataDirectory( dir.wstring() );
                sampleRun->setFilePrefix( stem.wstring() );
                document::instance()->setSampleRun( sampleRun );
            }
        }
    }
}

void
MainWindow::handleControlMethodOpen()
{
    QString dstfile;

    if ( auto edit = findChild< QLineEdit * >( "methodName" ) ) {
        dstfile = edit->text();
        if ( dstfile.isEmpty() )
            dstfile = QString::fromStdWString( document::instance()->sampleRun()->dataDirectory() );
        try {
            QString name = QFileDialog::getOpenFileName( this, tr( "Open Control Method" )
                                                        , dstfile
                                                        , tr( "Control Method Files(*.ctrl)" ) );
            auto cm = std::make_shared< adcontrols::ControlMethod::Method >();
        } catch ( ... ) {
            ADTRACE() << "Hit QTBUG-33119 that has no workaround right now.  Please be patient and try it again.";
            QMessageBox::information( this, "InfiTOF2 MainWindow", "Hit QTBUG-33119 - no workaround. Please be patient and try it again." );
        }
    }
}

void
MainWindow::handleControlMethodSaveAs()
{
    QString dstfile;

    if ( auto edit = findChild< QLineEdit * >( "methodName" ) ) {
        dstfile = edit->text();
        if ( dstfile.isEmpty() )
            dstfile = QString::fromStdWString( document::instance()->sampleRun()->dataDirectory() );
        try {
            QString name = QFileDialog::getSaveFileName( this, tr( "Save Control Method" )
                                                        , dstfile
                                                        , tr( "Control Method Files(*.ctrl)" ) );
            if ( !name.isEmpty() ) {
                // save method on UI
            }
        } catch ( ... ) {
            ADTRACE() << "Hit QTBUG-33119 that has no workaround right now.  Please be patient and try it again.";
            QMessageBox::information( this, "InfiTOF2 MainWindow", "Hit QTBUG-33119 - no workaround. Please be patient and try it again." );
        }
    }
}

void
MainWindow::setSampleRun( const adcontrols::SampleRun& m )
{
    if ( auto edit = findChild< QLineEdit * >( "dataSaveIn" ) ) {
        edit->setText( QString::fromStdWString( std::wstring( m.dataDirectory() ) ) );
    }

    if ( auto edit = findChild< QLineEdit * >( "runName" ) ) {
        edit->setText( QString::fromStdWString( std::wstring( m.filePrefix() ) ) );
    }

    if ( auto widget = findChild< adwidgets::SampleRunWidget * >() ) {
        widget->setSampleRun( m );
    }
}

std::shared_ptr< adcontrols::SampleRun >
MainWindow::getSampleRun() const
{
    auto sr = std::make_shared< adcontrols::SampleRun >();
    if ( auto widget = findChild< adwidgets::SampleRunWidget * >() ) {
        widget->getSampleRun( *sr );
    }
    return sr;
}

void
MainWindow::handleXChromatogramsMethod( const QString& json )
{
    // , [widget](){
    //     document::instance()->set_tof_chromatograms_method( widget->readJson(), true );
    // });
    boost::system::error_code ec;
    auto jv = boost::json::parse( json.toStdString(), ec );
    if ( !ec ) {
        auto xm = boost::json::value_to< adcontrols::XChromatogramsMethod >( jv );
        document::instance()->setXChromatogramsMethod( std::make_shared< adcontrols::XChromatogramsMethod >( xm ) );
    }
}
