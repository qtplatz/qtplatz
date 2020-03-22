/**************************************************************************
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
** Author: Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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
# if HAVE_OPENCV
# include "opencvwnd.hpp"
# include "imageview.hpp"
# endif
#include "player.hpp"
#include "playerwnd.hpp"
#if PROCESSINGWND
# include "processingwnd.hpp"
#endif
//#include "population_protocol.hpp"
#include "uimediator.hpp"
#include "contoursform.hpp"
//#include <mpxcontrols/histogrammethod.hpp>
//#include <mpxprocessor/processor.hpp>
//#include <mpxwidgets/histogrammethodform.hpp>
// #include <mpxwidgets/playercontrols.hpp>
// #include <mpxwidgets/spectrogramplot.hpp>
#include <qtwrapper/font.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/make_widget.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/contoursmethod.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adextension/ieditorfactory_t.hpp>
#include <adwidgets/centroidform.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adwidgets/mspeaktable.hpp>
#include <adportable/profile.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adcv/imagewidget.hpp>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/documentmanager.h>
#include <utils/styledbar.h>

#include <QBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QGuiApplication>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStandardPaths>
#include <QToolBar>
#include <QToolButton>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QTableView>
#include <QDockWidget>
#include <QStandardItemModel>
#include <QPainter>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <numeric>

using namespace cluster;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow( parent )
                                        , stack_( new QStackedWidget )
{
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::West );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * topRightLayout = new QVBoxLayout;
	topRightLayout->setMargin( 0 );
	topRightLayout->setSpacing( 0 );

    if ( QWidget * editorWidget = new QWidget ) {

        editorWidget->setLayout( topRightLayout );

        topRightLayout->addWidget( createTopStyledToolbar() );

        if ( auto wnd = new PlayerWnd() ) {
            stack_->addWidget( wnd );
            // connect( wnd, &PlayerWnd::endOfFile, this, [&](){
            //         if ( auto ctrl = findChild< mpxwidgets::PlayerControls * >( "playerControls" ) )
            //             ctrl->setState( QMediaPlayer::StoppedState );
            //     });
        }

#if HAVE_OPENCV
        if ( auto wnd = new OpenCVWnd() ) {
            stack_->addWidget( wnd );
        }
#endif

        topRightLayout->addWidget( stack_ );

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
            splitter->setObjectName( QLatin1String( "CLUSTERProcWidget" ) );
        }

        createDockWidgets();

        return splitter;
    }
    return this;
}

Utils::StyledBar *
MainWindow::createTopStyledToolbar()
{
    Utils::StyledBar * toolBar = qtwrapper::make_widget< Utils::StyledBar >( "topToolbar" );
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        Core::ActionManager * am = Core::ActionManager::instance(); // ->actionManager();
        if ( am ) {
            Core::Context globalcontext( (Core::Id( Core::Constants::C_GLOBAL )) );

            enum { _1 = 0, _2 = 1 };

            if ( auto p = new QAction( tr("Player"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( _1 ); } );
                am->registerAction( p, "CLUSTER.player", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }
#if HAVE_OPENCV
            if ( auto p = new QAction( tr("Manipulation"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( _2 ); } );
                am->registerAction( p, "CLUSTER.manipulation", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }
#endif
            // [file open] button
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
            //---
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbMoments", "Moments" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbContours", "Contours" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbGray", "Gray" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbBlur", "Blur" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbDFT", "DFT" ) );

            //---------------------------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addItem( new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbZAuto-Manip", "Z-Auto" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QSpinBox >( "ZScale-Manip" ) );

            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbZAuto-Raw", "Z-Auto" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QSpinBox >( "ZScale-Raw" ) );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    }

    for ( auto& label: toolBar->findChildren< QLabel * >() )
        label->setTextFormat( Qt::RichText );

    for ( auto& spin: toolBar->findChildren< QSpinBox * >() )
        spin->setRange( 0, std::numeric_limits< int >::max() );

    return toolBar;
}

Utils::StyledBar *
MainWindow::createMidStyledToolbar()
{
    Utils::StyledBar * toolBar = qtwrapper::make_widget< Utils::StyledBar >( "midToolbar" );
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );

        if ( auto am = Core::ActionManager::instance() ) {
            toolBarLayout->addWidget( toolButton( am->command( Constants::CLUSTER_PRINT_PDF )->action() ) );
            toolBarLayout->addWidget( toolButton( am->command( Constants::CLUSTER_FILE_OPEN )->action() ) );
        }

        toolBarLayout->addWidget( new Utils::StyledSeparator );
#if 0
        if ( auto widget = qtwrapper::make_widget< mpxwidgets::PlayerControls >( "playerContnrols" ) ) {
            using mpxwidgets::PlayerControls;
            connect( widget, &PlayerControls::play, this, [widget](){
                    if ( document::instance()->currentProcessor() ) {
                        document::instance()->player()->Play();
                        widget->setState( QMediaPlayer::PlayingState );
                    }
                });
            connect( widget, &PlayerControls::pause, this, [widget](){
                    if ( document::instance()->currentProcessor() ) {
                        document::instance()->player()->Stop();
                        widget->setState( QMediaPlayer::PausedState );
                    }
                });
            connect( widget, &PlayerControls::stop, this, [widget](){
                    if ( document::instance()->currentProcessor() ) {
                        document::instance()->player()->Stop();
                        widget->setState( QMediaPlayer::StoppedState );
                    }
                });
            connect( widget, &PlayerControls::next, this, [widget](){
                    if ( document::instance()->currentProcessor() )
                        document::instance()->player()->Next();
                });
            connect( widget, &PlayerControls::previous, this, [widget](){
                    if ( document::instance()->currentProcessor() )
                        document::instance()->player()->Prev();
                });

            connect( widget, &PlayerControls::changeRate, this, []( double rate ){ document::instance()->player()->setRate( rate ); } );

            toolBarLayout->addWidget( widget );
        }
#endif
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

        toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbCellRange", "Cells" ) );

        toolBarLayout->addWidget( qtwrapper::make_widget< QLabel >( "lbCellRange" ) );

        toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbTofRange", "TOF" ) );

        if ( auto spin = qtwrapper::make_widget< QDoubleSpinBox >( "tTOFCentre" ) ) {
            spin->setRange( 0.0, 500.0 );
            spin->setSingleStep( 0.020 );
            spin->setSuffix( "\u03bcs" ); // == &mu;s
            toolBarLayout->addWidget( spin );
        }

        if ( auto spin = qtwrapper::make_widget< QDoubleSpinBox >( "tTOFWidth" ) ) {
            spin->setRange( 0.0, 256 * 0.020 );
            spin->setSingleStep( 0.020 );
            spin->setSuffix( "\u03bcs" ); // == &mu;s
            toolBarLayout->addWidget( spin );
        }

        toolBarLayout->addWidget( toolButton( Core::ActionManager::command( Constants::HIDE_DOCK )->action() ) );
    }

    for ( auto& label: toolBar->findChildren< QLabel * >() )
        label->setTextFormat( Qt::RichText );

    for ( auto& spin: toolBar->findChildren< QSpinBox * >() )
        spin->setRange( 0, std::numeric_limits< int >::max() );

    return toolBar;
}

void
MainWindow::onInitialUpdate()
{
    for ( auto widget: dockWidgets() ) {
        if ( auto lifecycle = qobject_cast< adplugin::LifeCycle * >( widget->widget() ) )
            lifecycle->OnInitialUpdate();
    }

    if ( auto form = findChild< ContoursForm * >() ) {
        auto& m = document::instance()->contoursMethod();
        form->setBlurSize( m.blurSize() );
        form->setResize( m.sizeFactor() );
        form->setCannyThreshold( m.cannyThreshold() );
        form->setMinSizeThreshold( m.minSizeThreshold() );
        form->setMaxSizeThreshold( m.maxSizeThreshold() );

        connect( form, &ContoursForm::valueChanged, []( ContoursForm::valueId id, int value ){
                if ( id == ContoursForm::idResize )
                    document::instance()->contoursMethod().setSizeFactor( value );
                else if ( id == ContoursForm::idBlurSize )
                    document::instance()->contoursMethod().setBlurSize( value );
                else if ( id == ContoursForm::idCannyThreshold )
                    document::instance()->contoursMethod().setCannyThreshold( value );
                else if ( id == ContoursForm::idMinSizeThreshold )
                    document::instance()->contoursMethod().setMinSizeThreshold( value );
                else if ( id == ContoursForm::idMaxSizeThreshold )
                    document::instance()->contoursMethod().setMaxSizeThreshold( value );
                document::instance()->updateContoursMethod();
            });
    }


    connect( document::instance(), &document::mappedImageChanged, this, [&](){
            if ( auto img = document::instance()->mappedImage() ) {
                if ( document::instance()->zAutoScaleEnable( zMapRaw ) ) {
                    if ( auto spin = findChild< QSpinBox * >( "ZScale-Raw" ) ) {
                        QSignalBlocker block( spin );
                        spin->setValue( document::instance()->zAutoScale( zMapRaw ) );
                    }
                }
            }
        });

    connect( document::instance(), &document::tofWindowChanged, this, [&](double delay, double width){
            if ( auto spin = findChild< QDoubleSpinBox * >( "tTOFCentre" ) )
                spin->setValue( delay * std::micro::den );
            if ( auto spin = findChild< QDoubleSpinBox * >( "tTOFWidth" ) )
                spin->setValue( width * std::micro::den );
        });

    if ( auto wnd = findChild< PlayerWnd *>() ) {
        connect( document::instance(), &document::currentProcessorChanged, wnd,  &PlayerWnd::handleProcessorChanged ); // file sel changed
        connect( document::instance(), &document::currentProcessorChanged, this, &MainWindow::handleProcessorChanged ); // file sel changed
        connect( document::instance(), &document::dataChanged,             wnd,  &PlayerWnd::handleDataChanged );
        connect( document::instance(), &document::checkStateChanged,       wnd,  &PlayerWnd::handleCheckStateChanged );
        connect( document::instance(), &document::axisChanged,             wnd,  &PlayerWnd::handleAxisChanged );

        connect( wnd, &PlayerWnd::frameChanged, this, &MainWindow::handleFrameChanged );

        connect( document::instance()->player(), &Player::onSignaled, wnd, &PlayerWnd::handlePlayerSignal );
        connect( document::instance()->player(), &Player::next, wnd, &PlayerWnd::handleNextMappedSpectra );

        // redraw when TOF checkbox checked or tof-range spin control changed
        connect( document::instance(), &document::tofWindowChanged, [wnd](double,double){ wnd->handleTofWindowChanged(); });
        connect( document::instance(), &document::tofWindowEnabled, [wnd](bool){ wnd->handleTofWindowChanged(); });
        connect( document::instance(), &document::contoursMethodChanged, wnd, &PlayerWnd::handleContoursMethodChanged );

        wnd->setStyleSheet( "background-color: rgb(24,0,0); color: green;" );
    }

#if HAVE_OPENCV
    if ( auto wnd = findChild< OpenCVWnd * >() ) {
        connect( document::instance(), &document::currentProcessorChanged, wnd, &OpenCVWnd::handleProcessorChanged ); // file sel changed
        connect( document::instance(), &document::mappedImageChanged, wnd, &OpenCVWnd::handleMappedImage );
        connect( document::instance(), &document::dataChanged,        wnd, &OpenCVWnd::handleDataChanged );
        connect( document::instance(), &document::contoursMethodChanged, wnd, &OpenCVWnd::handleContoursMethodChanged );
    }
#endif

    // connect( uiMediator::instance(), &uiMediator::onTofDelayChanged, [](double delay){
    //         double width = document::instance()->histogramMethod().timeWindow();
    //         document::instance()->setHistogramWindow( delay / std::micro::den, width );
    //     });

    connect( uiMediator::instance(), &uiMediator::onTofWidthChanged, [](double window){
            // double delay = document::instance()->histogramMethod().timeDelay();
            // document::instance()->setHistogramWindow( delay, window / std::micro::den );
        });

    connect( uiMediator::instance(), &uiMediator::onCellRangeToggled, [](bool enable){
            document::instance()->setCellSelectionEnabled( enable );
        });

    // __initializeHistogramMethodForm();

    // [[1]]
    connect( uiMediator::instance(), &uiMediator::onTofRangeSelected, this, &MainWindow::handleTofTimeRange );

    if ( auto midBar = findChild< Utils::StyledBar * >( "midToolbar" ) ) {
        // [[2]]
        if ( auto cbx = midBar->findChild< QCheckBox * >( "cbCellRange" ) ) { // id = 0
            connect( cbx, &QCheckBox::toggled, this, [](bool checked){
                    document::instance()->setCellSelectionEnabled( checked );
                    uiMediator::instance()->cellRangeToggled( checked );
                });
            connect( uiMediator::instance(), &uiMediator::onCellRangeToggled, cbx, &QCheckBox::setChecked );
        }

        if ( auto cbx = midBar->findChild< QCheckBox * >( "cbTofRange" ) ) {
            connect( cbx, &QCheckBox::toggled, this, [](bool checked){
                    document::instance()->setHistogramWindowEnabled( checked );
                    uiMediator::instance()->tofRangeToggled( checked );
                });
            connect( uiMediator::instance(), &uiMediator::onTofRangeToggled, cbx, &QCheckBox::setChecked );
        }

        if ( auto spin = midBar->findChild< QDoubleSpinBox * >( "tTOFCentre" ) ) {
            connect( spin, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
                     , uiMediator::instance(), &uiMediator::tofDelayChanged );
        }
        if ( auto spin = midBar->findChild< QDoubleSpinBox * >( "tTOFWidth" ) ) {
            connect( spin, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
                     , uiMediator::instance(), &uiMediator::tofWidthChanged );
        }
    }

    if ( auto topBar = findChild< Utils::StyledBar * >( "topToolbar" ) ) {

        std::pair< uint, QString > filters [] { { 1, "cbGray" }, { 2, "cbDFT" }, { 4, "cbBlur" }, { 8, "cbContours" } };

        if ( auto wnd = findChild< PlayerWnd * >() ) {
            for ( auto& obj: filters ) {
                if ( auto cbx = topBar->findChild< QCheckBox * >( obj.second ) )
                    connect( cbx, &QCheckBox::toggled, this, [wnd,obj](bool checked){ wnd->setFilter( obj.first, checked ); });
            }
        }

        if ( auto cbx = topBar->findChild< QCheckBox * >( "cbMoments" ) ) {
            connect( cbx, &QCheckBox::toggled, [](bool checked){
                    document::instance()->setMomentsEnabled( checked );
                });
        }

        if ( auto cbx = topBar->findChild< QCheckBox * >( "cbZAuto-Manip" ) ) {
            connect( cbx, &QCheckBox::toggled, [](bool checked){
                    document::instance()->setZAutoScaleEnable( zMapManip, checked );
                    uiMediator::instance()->zAutoScaleToggled( zMapManip, checked );
                });
        }

        if ( auto spin = topBar->findChild< QSpinBox * >( "ZScale-Manip" ) ) {
            connect( spin, static_cast< void(QSpinBox::*)(int)>( &QSpinBox::valueChanged ),
                     []( int value ){
                         document::instance()->setZScale( zMapManip, value );
                         uiMediator::instance()->zScaleChanged( zMapManip, value);
                     });
        }

        if ( auto cbx = topBar->findChild< QCheckBox * >( "cbZAuto-Raw" ) ) {
            connect( cbx, &QCheckBox::toggled, []( bool checked ){
                    document::instance()->setZAutoScaleEnable( zMapRaw, checked );
                    uiMediator::instance()->zAutoScaleToggled( zMapRaw, checked );
                });
        }

        if ( auto spin = topBar->findChild< QSpinBox * >( "ZScale-Raw" ) ) {
            connect( spin, static_cast< void(QSpinBox::*)(int)>( &QSpinBox::valueChanged )
                     , []( int value ){
                         document::instance()->setZScale( zMapRaw, value );
                         uiMediator::instance()->zScaleChanged( zMapRaw, value);
                     });
        }
    }

    if ( auto wnd = findChild< PlayerWnd * >() ) {
        //
        connect( wnd, &PlayerWnd::timeRangeSelectedOnSpectrum, uiMediator::instance(), &uiMediator::tofRangeSelected );

        connect( uiMediator::instance(), &uiMediator::onZAutoScaleToggled, wnd, &PlayerWnd::setZAutoScale );
        connect( uiMediator::instance(), &uiMediator::onZScaleChanged, wnd, &PlayerWnd::setZScale );

        //////////////
        connect( uiMediator::instance(), &uiMediator::onHistogramClearCycleChanged, wnd, &PlayerWnd::handleHistogramClearCycleChanged );
    }

    if ( auto cbx = findChild< QCheckBox * >( "cbZAuto-Manip" ) )
        cbx->setChecked( document::instance()->zAutoScaleEnable( zMapManip ) );

    if ( auto cbx = findChild< QCheckBox * >( "cbZAuto-Raw" ) )
        cbx->setChecked( document::instance()->zAutoScaleEnable( zMapRaw ) );

#if defined Q_OS_LINUX
    auto fsize = qtwrapper::font_size()( 9 );

    for ( auto dock: dockWidgets() )
        dock->widget()->setStyleSheet( QString( "* { font-size: %1pt; }" ).arg( fsize ) );

    for ( auto tabbar: findChildren< QTabBar * >() )
        tabbar->setStyleSheet( QString( "QTabBar { font-size: %1pt; }" ).arg( fsize) );
#endif
}

void
MainWindow::onFinalClose()
{
    document::instance()->player()->Stop();

    for ( auto widget: dockWidgets() ) {
        if ( auto lifecycle = qobject_cast< adplugin::LifeCycle * >( widget->widget() ) )
            lifecycle->OnFinalClose();
    }

    commit();
    // delete iSequenceImpl factories
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
    return toolButton( Core::ActionManager::instance()->command(id)->action() );
}

void
MainWindow::createActions()
{
    const Core::Context gc( (Core::Id( Core::Constants::C_GLOBAL )) );

    if ( Core::ActionManager * am = Core::ActionManager::instance() ) {

        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( tr("CLUSTER Review") );

        if ( auto p = new QAction( QIcon( ":/cluster/images/fileopen.png" ), tr( "Open PNG file..." ), this ) ) {

            am->registerAction( p, Constants::CLUSTER_FILE_OPEN, Core::Context( Constants::C_CLUSTER_MODE ) );
            connect( p, &QAction::triggered, this, &MainWindow::handleOpen );

            menu->addAction( am->command( Constants::CLUSTER_FILE_OPEN ) );
       }

        if ( auto p = new QAction( QIcon( ":/cluster/images/filesave.png" ), tr( "Print to PDF..." ), this ) ) {
            am->registerAction( p, Constants::CLUSTER_PRINT_PDF, Core::Context( Core::Constants::C_GLOBAL ) );   // Tools|Cluster|Open SQLite file...
            connect( p, &QAction::triggered, this, &MainWindow::filePrintPdf );
            menu->addAction( am->command( Constants::CLUSTER_PRINT_PDF ) );
            am->registerAction( p, Core::Constants::PRINT, Core::Context( Constants::C_CLUSTER_MODE ) );    // File|Print
        }

        do {
            QIcon icon;
            icon.addPixmap( QPixmap( Constants::ICON_DOCKHIDE ), QIcon::Normal, QIcon::Off );
            icon.addPixmap( QPixmap( Constants::ICON_DOCKSHOW ), QIcon::Normal, QIcon::On );

            auto * action = new QAction( icon, tr( "Hide dock" ), this );

            action->setCheckable( true );
            auto cmd = Core::ActionManager::registerAction( action, Constants::HIDE_DOCK, gc );
            connect( action, &QAction::triggered, this, &MainWindow::hideDock );
            menu->addAction( cmd );
        } while ( 0 );

        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}

void
MainWindow::handleIndexChanged( int index, int subIndex )
{
    (void)index;
    if ( stack_ ) {
        stack_->setCurrentIndex( subIndex );
    }
}

void
MainWindow::commit()
{
}

void
MainWindow::createDockWidgets()
{
    //if ( auto form = new mpxwidgets::HistogramMethodForm( this ) ) {
    if ( auto form = new ContoursForm( this ) ) {

        auto dock = createDockWidget( form, tr( "Contours" ), "Contours" );
        dock->setMinimumWidth( 200 );
        dock->setMaximumWidth( 400 );
        dock->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );

    }

    if ( auto w = new adwidgets::MSPeakTable( this ) ) {
        auto dock = createDockWidget( w, tr( "MS Peaks" ), "MSPeakTable" );
        dock->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    }
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& pageName )
{
    if ( widget->windowTitle().isEmpty() || widget->windowTitle() == "Form" )
        widget->setWindowTitle( title );

    if ( widget->objectName().isEmpty() ) // avoid QTC warning at fancymainwindow.cpp L327,L328
        widget->setObjectName( pageName );

    QDockWidget * dockWidget = addDockForWidget( widget );
    dockWidget->setObjectName( pageName.isEmpty() ? widget->objectName() : pageName );

    if ( title.isEmpty() )
        dockWidget->setWindowTitle( widget->objectName() );
    else
        dockWidget->setWindowTitle( title );

    addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

    return dockWidget;
}

void
MainWindow::filePrintPdf()
{
	QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );
    printer.setOrientation( QPrinter::Landscape );

    auto& settings = document::instance()->settings();
    auto pdfname = settings.value( "Printer/FileName" ).toString();
    if ( !pdfname.isEmpty() ) {
        boost::filesystem::path path( pdfname.toStdString() );
        if ( auto processor = document::instance()->currentProcessor() ) {
            // if ( auto dp = processor->dataprocessor() ) {
            //     boost::filesystem::path filename( dp->filename() );
            //     auto name = path.parent_path() / filename.filename();
            //     name.replace_extension( ".pdf" );
            //     pdfname = QString::fromStdString( name.string() );
            // }
        }
        printer.setOutputFileName( pdfname );
    }

    ADDEBUG() << "filePrint: " << pdfname.toStdString();

    QPrintDialog dialog(&printer, this);
    dialog.setOption( QAbstractPrintDialog::PrintToFile );

    if (dialog.exec() == QDialog::Accepted) {

        settings.setValue( "Printer/FileName", printer.outputFileName() );

        QPainter painter(&printer);
        //if ( auto view = findChild< OpenCVWnd * >() )
        if ( auto view = findChild< PlayerWnd * >() )
            view->print( painter, printer );
    }
}

void
MainWindow::handleProcessorChanged()
{
    if ( auto p = document::instance()->currentProcessor() ) {
        //if ( auto matrix = p->mappedSpectra( 0, 0 ) ) {

            // if ( auto ctrl = findChild< mpxwidgets::PlayerControls * >() ) {

            //     ctrl->setNumberOfFrames( p->frameCounts() );
            //     ctrl->setCurrentFrame( p->trigNumber() );
            //     ctrl->setDuration( p->duration() );
            //     ctrl->setTime( p->elapsedTime() );

            //     boost::filesystem::path path( p->dataprocessor()->filename() );
            //     ctrl->setName( QString::fromStdString( path.stem().string() ) );
            // }
        //}
    }
}

bool
MainWindow::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);

    reader.setAutoTransform(true);

    const QImage newImage = reader.read();

    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2").arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    document::instance()->settings().setValue( "RecentFile", fileName );
#if HAVE_OPENCV
    cv::Mat image = cv::imread( fileName.toStdString().c_str(), cv::IMREAD_GRAYSCALE );
    if ( ! image.empty() ) {
        for ( auto view: findChildren< OpenCVWnd *>() )
            view->setImage( image );
    }
#endif
    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)
             , QString::number( newImage.width() )
             , QString::number( newImage.height() )
             , QString::number( newImage.depth()  ) );

    statusBar()->showMessage(message);

    return true;
}

static void
initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    auto& settings = document::instance()->settings();
    auto recentFile = settings.value( "RecentFile", "" ).toString();

    if ( recentFile.isEmpty() ) {
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    } else {
        QDir dir( recentFile );
        dialog.setDirectory( dir.dirName() );
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes =
        acceptMode == QFileDialog::AcceptOpen ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();

    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();

    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");

    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void
MainWindow::handleOpen()
{
    QFileDialog dialog( this, tr("Open Image File") );
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {
    }

}

void
MainWindow::handleFrameChanged( double timeSinceInject, uint64_t trigNumber, uint64_t trigCounts )
{
    // if ( auto form = findChild< mpxwidgets::HistogramMethodForm * >() ) {
    //     form->setHistogramClearCycle( int( trigCounts == 0 ? 1 : trigCounts ) );
    // }

    // if ( auto widget = findChild< mpxwidgets::PlayerControls * >() ) {
    //     widget->setTime( timeSinceInject );
    // }
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
MainWindow::handleTofTimeRange( int splotId, const QRectF& rc )
{
    double delay = rc.left() / std::micro::den;
    double width = ( rc.right() - rc.left() ) / std::micro::den;

    if ( auto toolBar = findChild< Utils::StyledBar * >( "midToolbar" ) ) {
        if ( auto spin = toolBar->findChild< QDoubleSpinBox * >( "tTOFCentre" ) )
            spin->setValue( delay * std::micro::den );
        if ( auto spin = toolBar->findChild< QDoubleSpinBox * >( "tTOFWidth" ) )
            spin->setValue( width * std::micro::den );
        if ( auto cbx = toolBar->findChild< QCheckBox * >( "cbTofRange" ) )
            cbx->setChecked( true );
    }
}
