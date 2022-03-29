/**************************************************************************
** Copyright (C) 2013-2022 MS-Cheminformatics LLC
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
#include "iu5303afacade.hpp"
#include "moleculeswidget.hpp"
#include <u5303a/digitizer.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrswidgets/thresholdwidget.hpp>
#include <acqrswidgets/u5303awidget.hpp>
#include <adacquire/constants.hpp>
#include <adcontrols/controlmethod.hpp>
#if TOFCHROMATOGRAMSMETHOD
# include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
# include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#endif
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
#include <adwidgets/cherrypicker.hpp>
#include <adwidgets/countingwidget.hpp>
#include <adwidgets/dgwidget.hpp>
#include <adwidgets/findslopeform.hpp>
#include <adwidgets/moltableview.hpp>
#include <adwidgets/progressinterface.hpp>
#include <adwidgets/samplerunwidget.hpp>
#if TOFCHROMATOGRAMSMETHOD
# include <adwidgets/tofchromatogramswidget.hpp>
#endif
#include <adwidgets/xchromatogramswidget.hpp>
#include <qtwrapper/make_widget.hpp>
#include <qtwrapper/settings.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/progressmanager/progressmanager.h>
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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStatusBar>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabBar>
#include <QToolButton>
#include <QTextEdit>
#include <QLabel>
#include <QIcon>
#include <qdebug.h>
#include <csignal>

using namespace accutof::acquire;

Q_DECLARE_METATYPE( boost::uuids::uuid );

MainWindow * MainWindow::instance_ = 0;

namespace {
    enum { numAxes = 2 };

    struct setCalibFileName {
        void operator()( QWidget * parent, const QString& file, QString&& ss ) const {
            if ( auto edit = parent->findChild< QLineEdit * >( "calibfile" ) ) {
                auto path = boost::filesystem::path( file.toStdString() );
                edit->setText( QString::fromStdString( path.stem().string() ) );
                edit->setToolTip( file );
                edit->setStyleSheet( ss );
            }
        }
    };
}

namespace accutof {
    namespace acquire {
        class MainWindow::impl {
        public:
            std::shared_ptr< adwidgets::ProgressInterface > progress_;
        };
    }
}


MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , impl_( std::make_unique< impl >() )
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
    QFile file( ":/accutof/stylesheet/tabbar.qss" );
    file.open( QFile::ReadOnly );
    QString tabStyle( file.readAll() );

    ///////// U5303A /////////
    if ( auto widget = qtwrapper::make_widget< acqrswidgets::u5303AWidget>( "U5303A" ) ) {

        createDockWidget( widget, tr( "U5303A" ), "ControlMethod" );

        connect( this, &MainWindow::onSampleRun, widget, &acqrswidgets::u5303AWidget::handleRunning );

        connect( widget, &acqrswidgets::u5303AWidget::applyTriggered, []() {
            document::instance()->applyTriggered();
        });

        connect( widget, &acqrswidgets::u5303AWidget::dataChanged, [this,widget]() {
                acqrscontrols::u5303a::method u;
                if ( widget->get( u ) ) {
                    using acqrscontrols::u5303a::method;
                    document::instance()->set_method( u );
                    if ( auto sform = findChild< adwidgets::findSlopeForm * >() ) {
                        // disable counting if pkd enabled && number of averages >= 2
                        bool disable = u._device_method().pkd_enabled ||
                            ( u.mode() == method::DigiMode::Averager && u._device_method().nbr_of_averages >= 2 );
                        sform->setDisabled( disable );
                    }
#if TOFCHROMATOGRAMSMETHOD
                    if ( auto widget = findChild< adwidgets::TofChromatogramsWidget * >() ) {
                        widget->setDigitizerMode( u.mode() == method::DigiMode::Digitizer );
                    }
#endif
                    if ( auto widget = findChild< adwidgets::XChromatogramsWidget * >( "XICs" ) ) {
                        widget->setDigitizerMode( u.mode() == method::DigiMode::Digitizer );
                    }
                }
            });
    }

    if ( auto widget = qtwrapper::make_widget< adwidgets::SampleRunWidget >( "SampleRunwidget" ) ) {
        createDockWidget( widget, tr( "Sample Run" ), "SampleRunWidget" );
        connect( this, &MainWindow::onSampleRun, widget, &adwidgets::SampleRunWidget::handleRunning );
        connect( widget, &adwidgets::SampleRunWidget::apply, document::instance(), &document::handleSampleRun );
    }

    if ( auto widget = qtwrapper::make_widget< MoleculesWidget >( "Molecules" ) ) {
        createDockWidget( widget, "Molecules", "MolList" );

        connect( widget, &MoleculesWidget::valueChanged, this, []( auto& json ){
            document::instance()->settings()->setValue( "molecules", QByteArray( json.toUtf8() ) );
        });
        if ( auto wnd = findChild< WaveformWnd * >() ) {
            connect( widget, &MoleculesWidget::valueChanged, wnd, &WaveformWnd::handleMolecules );
        }
    }
#if TOFCHROMATOGRAMSMETHOD
    if ( auto widget = qtwrapper::make_widget< adwidgets::TofChromatogramsWidget >( "Chromatograms" ) ) {
        createDockWidget( widget, tr( "Chromatograms" ), "Chromatograms" );
        if ( auto wnd = centralWidget()->findChild<WaveformWnd *>() ) {
            connect( widget, &adwidgets::TofChromatogramsWidget::editorValueChanged, [wnd] ( const QModelIndex& index, double value ) {
                if ( index.column() == 4 || index.column() == 5 )  // time | time-window
                    wnd->setSpanMarker( index.row(), index.column() - 4, value );
            });

            connect( widget, &adwidgets::TofChromatogramsWidget::valueChanged, [wnd,widget] () {
                auto m = widget->method();
                wnd->setMethod( m ); // draw markers
                document::instance()->setMethod( m );
            });
        }
    }
#endif
//----------
    if ( auto widget = qtwrapper::make_widget< adwidgets::XChromatogramsWidget >( "XICs" ) ) {
        createDockWidget( widget, tr( "XICs" ), "XICs" );
        if ( auto wnd = centralWidget()->findChild<WaveformWnd *>() ) {
            connect( widget, &adwidgets::XChromatogramsWidget::editorValueChanged, [wnd] ( const QModelIndex& index, double value ) {
                ADDEBUG() << "############## handle editorValueChanged ###################### " << index.column() << ", " << value;
                if ( index.column() == 3 || index.column() == 4 )  // mass, mass_window
                    wnd->setSpanMarker( index.row(), index.column() - 3, value );
            });
            connect( widget, &adwidgets::XChromatogramsWidget::valueChanged, this, &MainWindow::handleXChromatogramsMethod );
        }
    }
//<---------
    if ( auto widget = qtwrapper::make_widget< acqrswidgets::ThresholdWidget >( "ThresholdWidget", "", 1 ) ) {
        createDockWidget( widget, "Threshold", "ThresholdMethod" );
        connect( widget, &acqrswidgets::ThresholdWidget::valueChanged, [this] ( acqrswidgets::idCategory cat, int ch ) {
                if ( auto form = findChild< acqrswidgets::ThresholdWidget * >() ) {
                    if ( cat == acqrswidgets::idSlopeTimeConverter ) {
                        adcontrols::threshold_method tm;
                        form->get( ch, tm );
                        document::instance()->set_threshold_method( ch, tm ); // <--
                    } else if ( cat == acqrswidgets::idThresholdAction ) {
                        adcontrols::threshold_action am;
                        form->get( am );
                        document::instance()->set_threshold_action( am ); // <--
                    }
                }

            });
    }

    if ( auto widget = qtwrapper::make_widget< adwidgets::CherryPicker >("ModulePicker") ) {
        createDockWidget( widget, "Modules", "CherryPicker" );
        connect( widget, &adwidgets::CherryPicker::stateChanged
                 , []( const QString& key, bool enable ){
                       document::instance()->setControllerSettings( key, enable );
                   });
    }
}

size_t
MainWindow::findInstControllers( std::vector< std::shared_ptr< adextension::iController > >& vec ) const
{
    for ( auto v: ExtensionSystem::PluginManager::getObjects< adextension::iController >() ) {
        try {
            vec.push_back( v->shared_from_this() );
        } catch ( std::bad_weak_ptr& ) {
            ADWARN() << "adextension::iController does not have weak_ptr -- maybe deprecated plugin instance";
        }
    }
    return vec.size();
}

void
MainWindow::OnInitialUpdate()
{
    connect( document::instance(), &document::instStateChanged, this, &MainWindow::handleInstState );
    connect( document::instance(), &document::onModulesFailed,  this, &MainWindow::handleModulesFailed );
    connect( document::instance(), &document::sampleRunChanged, this, [&]{ setSampleRun( *document::instance()->sampleRun() ); });
    connect( document::instance(), &document::msCalibrationLoaded, this, &MainWindow::handleMSCalibrationLoaded );
    connect( document::instance(), &document::onDefferedWrite, this, &MainWindow::handleDefferedWrite );

    // initialize ui and method data
    for ( auto dock: dockWidgets() ) {
        if ( auto widget = qobject_cast<adplugin::LifeCycle *>( dock->widget() ) ) {
            widget->OnInitialUpdate();
            // setup control method on ui
            widget->setContents( boost::any( document::instance()->controlMethod() ) );
        }
    }

    // this must call after onInitialUpdate
    setSampleRun( *document::instance()->sampleRun() );
#if XCHROMATOGRAMSMETHOD
    if ( auto w = findChild< adwidgets::XChromatogramsWidget * >( "XICs" ) ) {
        w->setMassSpectrometer( document::instance()->massSpectrometer() );
    }
#endif
#if TOFCHROMATOGRAMSMETHOD
    if ( auto w = findChild< adwidgets::TofChromatogramsWidget * >( "Chromatograms" ) ) {
        w->setMassSpectrometer( document::instance()->massSpectrometer() );
    }
#endif

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

    // enumerate all controllers
    for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
        document::instance()->addInstController( iController );
    }

    // initialize module picker
    if ( auto picker = findChild< adwidgets::CherryPicker * >( "ModulePicker" ) ) {
        for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
            bool checked = document::instance()->isControllerEnabled( iController->module_name() );
            bool enabled = !document::instance()->isControllerBlocked( iController->module_name() );
            picker->addItem( iController->module_name(), iController->module_name(), checked, enabled );
        }

        connect( document::instance(), &document::moduleConfigChanged, this, [picker](){
                for ( auto& module: document::instance()->controllerSettings() ) {
                    picker->setChecked( module.first, module.second );
                }
            });
    }

	if ( WaveformWnd * wnd = centralWidget()->findChild<WaveformWnd *>() ) {
		wnd->onInitialUpdate();

        for ( auto cbx: findChildren< QCheckBox * >() ) {
            if ( cbx->objectName() == "cbxHistogram" ) {
                connect( cbx, &QCheckBox::toggled, document::instance(), &document::setLongTermHistogramEnabled );
                cbx->setChecked( document::instance()->longTermHistogramEnabled() );
            } else if ( cbx->objectName() == "cbxPKDResult" ) {
                connect( cbx, &QCheckBox::toggled, document::instance(), &document::setPKDSpectrumEnabled );
                cbx->setChecked( document::instance()->pkdSpectrumEnabled() );
            } else if ( cbx->objectName() == "cbxDark" ) {
                connect( cbx, &QCheckBox::toggled, document::instance(), &document::clearDark );
                cbx->setChecked( document::instance()->hasDark() );
                connect( document::instance(), &document::darkStateChanged, this, [=](int flag){
                                                                                      QSignalBlocker block( cbx );
                                                                                      cbx->setChecked( flag );
                                                                                  } );
            }
        }
        for ( int i = 0; i < numAxes; ++i ) {
            if ( auto cb = findChild< QCheckBox * >( QString( "cbY%1" ).arg( i ) ) ) {
                connect( cb, &QCheckBox::toggled, this, [wnd,i,this](bool checked){
                    auto top = findChild< QDoubleSpinBox * >( QString( "spT%1" ).arg( i ) );
                    auto bottom = findChild< QDoubleSpinBox * >( QString( "spB%1" ).arg( i ) );
                    wnd->handleScaleY( i, checked, top->value(), bottom->value() );
                } );
            }
            if ( auto sp = findChild< QDoubleSpinBox * >( QString( "spT%1" ).arg( i ) ) ) {
                connect( sp, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [wnd,i,this](double value){
                    auto cb = findChild< QCheckBox * >( QString( "cbY%1" ).arg( i ) );
                    auto bottom = findChild< QDoubleSpinBox * >( QString( "spB%1" ).arg( i ) );
                    wnd->handleScaleY( i, cb->isChecked(), value, bottom->value() );
                } );
            }
            if ( auto sp = findChild< QDoubleSpinBox * >( QString( "spB%1" ).arg( i ) ) ) {
                connect( sp, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [wnd,i,this](double value){
                    auto cb = findChild< QCheckBox * >( QString( "cbY%1" ).arg( i ) );
                    auto top = findChild< QDoubleSpinBox * >( QString( "spT%1" ).arg( i ) );
                    wnd->handleScaleY( i, cb->isChecked(), top->value(), value );
                } );
            }
        }
    }

    if ( auto settings = document::instance()->settings() ) {
        ///////// Restore AXIS /////////
        for ( size_t i = 0; i < numAxes; ++i ) {
            auto axis = settings->value( QString( "mainwindow/axis_%1" ).arg( QString::number( i ) ) ).toInt();
            if ( auto choice = findChild< QComboBox * >( QString( "axis_%1" ).arg( QString::number( i ) ) ) ) {
                choice->setCurrentIndex( axis );
                if ( auto wnd = findChild< WaveformWnd * >() )
                    wnd->setAxis( i, axis );
            }
        }

        // Restore molecules
        if ( auto mw = findChild< MoleculesWidget * >() ) {
            mw->setContents( settings->value( "molecules" ).toByteArray().toStdString() );
        }

        // Set molecules
        if ( auto wnd = findChild< WaveformWnd * >() ) {
            wnd->handleMolecules( QString::fromStdString( settings->value( "molecules" ).toByteArray().toStdString() ) );
        }
    }

#if XCHROMATOGRAMSMETHOD
    // update axis, closeup views
    if ( auto widget = findChild< adwidgets::XChromatogramsWidget * >( "XICs" ) ) {
        //int mode = int( document::instance()->method()->mode() );
        //widget->setDigitizerMode( mode == 0 );
        auto m = widget->getValue();
        if ( auto wnd = centralWidget()->findChild<WaveformWnd *>() ) {
            wnd->setMethod( m );
        }
    }
#endif

#if TOFCHROMATOGRAMSMETHOD && 0
    // update axis, closeup views
    if ( auto widget = findChild< adwidgets::TofChromatogramsWidget * >() ) {
        int mode = int( document::instance()->method()->mode() );
        widget->setDigitizerMode( mode == 0 );
        auto m = widget->method();
        if ( auto wnd = centralWidget()->findChild<WaveformWnd *>() ) {
            wnd->setMethod( m );
            document::instance()->setMethod( m );
        }
    }
#endif

    setCalibFileName()( this, document::instance()->msCalibFile(), "background-color: cyan;" );

#if ! defined Q_OS_MAC
    for ( auto dock: dockWidgets() )
        dock->widget()->setStyleSheet( "* { font-size: 9pt; }" );

    // tab-bar on tabify docks
    for ( auto tabbar: findChildren< QTabBar * >() )
        tabbar->setStyleSheet( "QTabBar { font-size: 9pt }" );
#endif
}

void
MainWindow::OnFinalClose()
{
    document::instance()->settings()->setValue( "DOCK_LOCATIONS", saveState() );

    for ( auto dock: dockWidgets() ) {
        // adplugin::LifeCycleAccessor accessor( dock->widget() );
        if ( auto editor = qobject_cast< adplugin::LifeCycle * >( dock->widget() ) ) { // accessor.get() ) {
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
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;
	editorHolderLayout->setMargin( 0 );
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

        editorHolderLayout->addWidget( createTopStyledToolbar() );      // [Top toolbar]
        if ( auto wnd = new WaveformWnd() ) {
            editorHolderLayout->addWidget( wnd );
            connect( document::instance(), &document::on_threshold_method_changed, wnd, &WaveformWnd::handle_threshold_method );
            connect( document::instance(), &document::on_threshold_action_changed, wnd, &WaveformWnd::handle_threshold_action );
            connect( document::instance(), &document::onControlMethodChanged, wnd, &WaveformWnd::handle_method );
            // validation for uuid class registoration -- will be compile error if not registered
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

    const std::string left = ""; //"ThresholdMethod;DelayPulseSSE"; // ;SampleRunWidget";

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
    dockWidget->setWindowTitle( title.isEmpty() ? widget->objectName() : title );

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
        toolBar->setObjectName( "topToolBar" );
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
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
            toolBarLayout->addWidget( toolButton( am->command( Constants::ACTION_DARK )->action() ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbxDark", "Dark subtraction" ) );
            //toolBarLayout->addWidget( topLineEdit_.get() );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

        for ( size_t i = 0; i < numAxes; ++i ) {
            // ======== spacer =========
            toolBarLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ) );

            // axis (time|m/z)
            toolBarLayout->addWidget( new QLabel( QString( tr( "Axis %1:" ) ).arg( i + 1 ) ) );
            {
                auto choice = new QComboBox;
                choice->setObjectName( QString( "axis_%1" ).arg( QString::number(i) ) );
                choice->addItems( QStringList() << "m/z" << "time" );
                toolBarLayout->addWidget( choice );
                choice->setProperty( "id", QVariant( int(i) ) ); // <------------ combo id
                connect( choice, qOverload<int>( &QComboBox::currentIndexChanged )
                         , [=,this] ( int index ) { axisChanged( choice, index ); } );
            }

            if ( auto cb = qtwrapper::make_widget< QCheckBox >( ( boost::format( "cbY%1%" ) % i ).str().c_str(), "Y-Auto" ) ) {
                cb->setCheckState( Qt::Checked ); // defalut start with auto
                toolBarLayout->addWidget( cb );
            }
            if ( auto sp = qtwrapper::make_widget< QDoubleSpinBox >( (boost::format( "spB%1%" ) % i ).str().c_str() ) ) {
                sp->setRange( -2000.0, 2000.0 );
                sp->setDecimals( 0 );
                sp->setSingleStep( 1 );
                toolBarLayout->addWidget( sp );
            }
            if ( auto sp = qtwrapper::make_widget< QDoubleSpinBox >( (boost::format( "spT%1%" ) % i ).str().c_str() ) ) {
                sp->setRange( -2000.0, 2000.0 );
                sp->setDecimals( 0 );
                sp->setSingleStep( 1 );
                toolBarLayout->addWidget( sp );
            }
        }
        //----
        // check boxes
        toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbxHistogram", "Histogram" ) );
        toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbxPKDResult", "PKD Result" ) );
    }
    return toolBar;
}

Utils::StyledBar *
MainWindow::createMidStyledToolbar()
{
    if ( Utils::StyledBar * toolBar = new Utils::StyledBar ) {
        toolBar->setObjectName( "midToolBar" );
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
#if 0
            if ( auto edit = new QLineEdit ) {
                edit->setObjectName( "runName" );
                edit->setReadOnly( true );
                edit->setText( QString::fromStdWString( sampleRun->filePrefix() ) );
                edit->setFixedWidth( 80 );
                toolBarLayout->addWidget( edit );
                auto icon = QIcon( Constants::ICON_FILE_OPEN );
                if ( auto action = edit->addAction( icon, QLineEdit::ActionPosition::TrailingPosition ) )
                    connect( action, &QAction::triggered, this, &MainWindow::handleRunName );
            }
#endif
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

            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_SEL_CALIBFILE)->action()));
            if ( auto edit = new QLineEdit ) {
                edit->setObjectName( "calibfile" );
                edit->setReadOnly( true );
                toolBarLayout->addWidget( edit );
            }

            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_XIC_CLEAR)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::ACTION_XIC_ZERO)->action()));

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

    const Core::Context context( (Core::Id( Core::Constants::C_GLOBAL ) ) );

    menu->menu()->setTitle( "U5303A" );

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

    if ( auto action = createAction( Constants::ICON_OWL, tr( "DARK" ), this ) ) {
        connect( action, &QAction::triggered, this, &MainWindow::actDark );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_DARK, context );
        menu->addAction( cmd );
    }

    if ( auto action = createAction( Constants::ICON_BALANCE, tr( "Auto-zero" ), this ) ) {
        connect( action, &QAction::triggered, document::instance(), &document::handleAutoZeroXICs );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_XIC_ZERO, context );
        menu->addAction( cmd );
    }
    if ( auto action = createAction( Constants::ICON_RECYCLE, tr( "Clear traces" ), this ) ) {
        connect( action, &QAction::triggered, document::instance(), &document::handleClearXICs );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_XIC_CLEAR, context );
        menu->addAction( cmd );
    }

    if ( auto action = createAction( Constants::ICON_CALIBRATE, tr( "Choose calibration data" ), this ) ) {
        connect( action, &QAction::triggered, this, &MainWindow::handleSelCalibFile );
        auto cmd = Core::ActionManager::registerAction( action, Constants::ACTION_SEL_CALIBFILE, context );
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
MainWindow::actDark()
{
    document::instance()->acquireDark();
}

void
MainWindow::axisChanged( QComboBox * combo, int currentIndex )
{
    auto id = combo->property( "id" ).toInt();
    // ADDEBUG() << "####################### axisChanged " << id << ", " << currentIndex;
    if ( auto wnd = findChild< WaveformWnd * >() ) {
        wnd->setAxis( id, currentIndex );
        //instance_->updateSetpoints();
    }

    if ( auto settings = document::instance()->settings() ) {
        settings->setValue( QString( "mainwindow/axis_%1" ).arg( id ), currentIndex );
    }
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

    QMessageBox::warning( this, tr("QtPlatz/u5303aplugin"), msg );
}


void
MainWindow::handleInstState( int status )
{
    if ( status <= adacquire::Instrument::eNotConnected ) {

        if ( auto action = Core::ActionManager::instance()->command( Constants::ACTION_CONNECT )->action() )
            action->setEnabled( true  );

        for ( auto id : { Constants::ACTION_RUN, Constants::ACTION_STOP, Constants::ACTION_REC, Constants::ACTION_SNAPSHOT } ) {
            if ( auto action = Core::ActionManager::command( id )->action() )
                action->setEnabled( false );
        }

    } else if ( status == adacquire::Instrument::eStandBy ) {

        if ( auto action = Core::ActionManager::command( Constants::ACTION_CONNECT )->action() )
            action->setEnabled( false );

        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, true )
                    , std::make_pair( Constants::ACTION_STOP, true )
                    , std::make_pair( Constants::ACTION_REC, true )
                    , std::make_pair( Constants::ACTION_INJECT, false ) // <== false
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true ) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() )
                action->setEnabled( pair.second );
        }

    } else if ( status == adacquire::Instrument::ePreparingForRun ||
                status == adacquire::Instrument::eReadyForRun ) {

        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, false )
                    , std::make_pair( Constants::ACTION_STOP, true )
                    , std::make_pair( Constants::ACTION_REC, true )
                    , std::make_pair( Constants::ACTION_INJECT, false ) // <== false
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() )
                action->setEnabled( pair.second );
        }

    } else if ( status == adacquire::Instrument::eWaitingForContactClosure ) {

        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, false )
                    , std::make_pair( Constants::ACTION_STOP, true )
                    , std::make_pair( Constants::ACTION_REC, true )
                    , std::make_pair( Constants::ACTION_INJECT, true )  // <== true
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() )
                action->setEnabled( pair.second );
        }

    } else if ( status == adacquire::Instrument::eRunning ) {

        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, true )
                    , std::make_pair( Constants::ACTION_STOP, true )
                    , std::make_pair( Constants::ACTION_REC, true )
                    , std::make_pair( Constants::ACTION_INJECT, false ) // <== false
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true ) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() )
                action->setEnabled( pair.second );
        }

    } else if ( status == adacquire::Instrument::eStop ) {
        for ( auto pair: { std::make_pair(Constants::ACTION_RUN, true )
                    , std::make_pair( Constants::ACTION_STOP, false )
                    , std::make_pair( Constants::ACTION_REC, false )
                    , std::make_pair( Constants::ACTION_INJECT, false ) // <== false
                    , std::make_pair( Constants::ACTION_SNAPSHOT, true)
                    , std::make_pair( Constants::ACTION_SYNC, true ) } ) {
            if ( auto action = Core::ActionManager::command( pair.first )->action() )
                action->setEnabled( pair.second );
        }
    }
    emit onSampleRun( status == adacquire::Instrument::eRunning );
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
    // for ( const auto& item: *ptr ) {
    //     ADDEBUG() << item.clsid() << "\t" << item.modelname();
    // }
    return ptr;
}

void
MainWindow::getEditorFactories( adextension::iSequenceImpl& impl )
{
    if ( std::shared_ptr< const adextension::iEditorFactory > p
         = std::make_shared< adextension::iEditorFactoryT< acqrswidgets::ThresholdWidget, QString, int > >(
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
    ADDEBUG() << "saveCurrentImage()";
    qApp->beep();
    if ( auto screen = QGuiApplication::primaryScreen() ) {

        auto pixmap = this->grab(); // QPixmap::grabWidget( this );

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
            QMessageBox::information( this, "accutofacquire MainWindow", "Hit QTBUG-33119 - no workaround. Please be patient and try it again." );
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

        boost::filesystem::path path( dstfile.toStdWString() );

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
                QMessageBox::information( this, "accutofacquire MainWindow", "Hit QTBUG-33119 - no workaround. Please be patient and try it again." );
            }

            if ( !file.isEmpty() ) {

                boost::filesystem::path fname( file.toStdString() );

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
            if ( document::load( name, *cm ) ) {
                auto path = boost::filesystem::path( name.toStdString() );
                edit->setText( QString::fromStdString( path.stem().string() ) );
                edit->setToolTip( name );
                this->setControlMethod( cm );
                document::instance()->setControlMethod( cm, name );
            }
        } catch ( ... ) {
            ADTRACE() << "Hit QTBUG-33119 that has no workaround right now.  Please be patient and try it again.";
            QMessageBox::information( this, "accutofacquire MainWindow", "Hit QTBUG-33119 - no workaround. Please be patient and try it again." );
        }
    }
}

void
MainWindow::handleControlMethodSaveAs()
{
    QString dstfile;

    if ( auto edit = findChild< QLineEdit * >( "methodName" ) ) {
        dstfile = edit->toolTip();
        if ( dstfile.isEmpty() )
            dstfile = QString::fromStdWString( document::instance()->sampleRun()->dataDirectory() );
        try {
            QString name = QFileDialog::getSaveFileName( this, tr( "Save Control Method" )
                                                         , dstfile
                                                         , tr( "Control Method Files(*.ctrl)" ) );
            if ( !name.isEmpty() ) {
                // save method on UI
                if ( auto cm = getControlMethod() ) {
                    if ( document::save( name, *cm ) )
                        edit->setText( name );
                }
            }
        } catch ( ... ) {
            ADTRACE() << "Hit QTBUG-33119 that has no workaround right now.  Please be patient and try it again.";
            QMessageBox::information( this, "accutofacquire MainWindow", "Hit QTBUG-33119 - no workaround. Please be patient and try it again." );
        }
    }
}

void
MainWindow::setSampleRun( const adcontrols::SampleRun& m )
{
    if ( auto edit = findChild< QLineEdit * >( "dataSaveIn" ) ) {
        edit->setText( QString::fromStdWString( std::wstring( m.dataDirectory() ) ) );
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
MainWindow::handleSelCalibFile()
{
    auto file = qtwrapper::settings( *document::instance()->settings() ).recentFile( Constants::GRP_MSCALIB_FILES, Constants::KEY_FILES );

	QFileDialog dlg( 0, "Open MS Calibration file", file );
	dlg.setNameFilter( tr("MSCalibrations(*.msclb)" ) );
	dlg.setFileMode( QFileDialog::ExistingFile );

    if ( dlg.exec() == QDialog::Accepted ) {
		auto result = dlg.selectedFiles();
        if ( auto sp = document::instance()->setMSCalibFile( result[ 0 ] ) ) {
            qtwrapper::settings( *document::instance()->settings() ).addRecentFiles( Constants::GRP_MSCALIB_FILES, Constants::KEY_FILES, result[0] );
            setCalibFileName()( this, file, "background-color:yellow;" );
#if XCHROMATOGRAMSMETHOD
            if ( auto w = findChild< adwidgets::XChromatogramsWidget * >( "XICs" ) ) {
                w->setMassSpectrometer( sp );
            }
#else
            if ( auto w = findChild< adwidgets::TofChromatogramsWidget * >( "Chromatograms" ) ) {
                w->setMassSpectrometer( sp );
            }
#endif
        } else {
            QMessageBox::warning( 0, tr( "select calibration file" ), tr( "Calibration file load failed" ) );
        }
	}
}

void
MainWindow::handleMSCalibrationLoaded( const QString& file )
{
    setCalibFileName()( this, file, "background-color:white;" );
#if TOFCHROMATOGRAMSMETHOD
    if ( auto w = findChild< adwidgets::TofChromatogramsWidget * >( "Chromatograms" ) ) {
        w->setMassSpectrometer( document::instance()->massSpectrometer() );
    }
#endif
#if XCHROMATOGRAMSMETHOD
    if ( auto w = findChild< adwidgets::XChromatogramsWidget * >( "XICs" ) ) {
        w->setMassSpectrometer( document::instance()->massSpectrometer() );
    }
#endif
}

#if TOFCHROMATOGRAMSMETHOD
void
MainWindow::handleTofChromatogramsMethod( const QString& json )
{
    boost::system::error_code ec;
    auto jv = boost::json::parse( json.toStdString(), ec );
    if ( !ec ) {
        auto xm = boost::json::value_to< adcontrols::TofChromatogramsMethod >( jv );
        if ( auto w = findChild< adwidgets::TofChromatogramsWidget * >( "Chromatograms" ) ) {
            w->setContents( xm );
        }
    }
}
#endif

void
MainWindow::handleXChromatogramsMethod( const QString& json )
{
    boost::system::error_code ec;
    auto jv = boost::json::parse( json.toStdString(), ec );
    if ( !ec ) {
        auto xm = boost::json::value_to< adcontrols::XChromatogramsMethod >( jv );
        if ( auto wnd = findChild< WaveformWnd * >() ) {
            wnd->setMethod( xm );
        }
    }
}

void
MainWindow::handleDefferedWrite( const QString& stem, int remain, int progress )
{
    if ( !impl_->progress_ ) {
        impl_->progress_ = std::make_shared< adwidgets::ProgressInterface >( 0, remain + progress );
        Core::ProgressManager::addTask( impl_->progress_->progress.future(), QString("Closing %1...").arg(stem), "accutof.task.deffered" );
    }
    (*impl_->progress_)( progress );
    if ( remain == 0 ) {
        impl_->progress_.reset();
    }
}
