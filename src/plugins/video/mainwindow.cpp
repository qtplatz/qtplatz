/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcv/imagewidget.hpp>
#include "videoprocwnd.hpp"
#include "imageview.hpp"
#include "player.hpp"
#include "videocapturewnd.hpp"
#include <qtwrapper/font.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <adcontrols/adcv/contoursmethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adextension/ieditorfactory_t.hpp>
#include <adwidgets/adcv/contoursform.hpp>
#include <adwidgets/playercontrols.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adwidgets/mspeaktable.hpp>
#include <adportable/profile.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/make_widget.hpp>
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

#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QGuiApplication>
#include <QtGlobal>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStandardPaths>
#include <QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QTextEdit>
#include <QTableView>
#include <QDockWidget>
#include <QStandardItemModel>
#include <QMenu>
#include <QMessageBox>

#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <numeric>

using namespace video;

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


        if ( auto wnd = new VideoProcWnd() ) {
            stack_->addWidget( wnd );
            wnd->setStyleSheet( "background-color: black; color: green;");
        }

        if ( auto wnd = new VideoCaptureWnd() ) {
            stack_->addWidget( wnd );
            wnd->setStyleSheet( "background-color: black; color: green;");
        }

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
            // splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) ); // navegate
            splitter->addWidget( mainWindowSplitter );                            // *this + ontput
            // splitter->setStretchFactor( 0, 0 );
            // splitter->setStretchFactor( 1, 1 );
            splitter->setOrientation( Qt::Horizontal );
            splitter->setObjectName( QLatin1String( "VIDEOWidget" ) );
        }

        createDockWidgets();

        return splitter;
    }
    return this;
}

Utils::StyledBar *
MainWindow::createTopStyledToolbar()
{
    if ( auto toolBar = qtwrapper::make_widget< Utils::StyledBar >( "topToolbar" ) ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        Core::ActionManager * am = Core::ActionManager::instance(); // ->actionManager();
        if ( am ) {
            Core::Context globalcontext( (Core::Id( Core::Constants::C_GLOBAL )) );

            if ( auto p = new QAction( tr("Manipulation"), this ) ) {
                connect( p, &QAction::triggered, [=,this](){ stack_->setCurrentIndex( 0 ); } );
                am->registerAction( p, "VIDEO.manipulation", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }

            if ( auto p = new QAction( tr("Capture"), this ) ) {
                connect( p, &QAction::triggered, [=,this](){ stack_->setCurrentIndex( 1 ); } );
                am->registerAction( p, "VIDEO.capture", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }

            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
            //---
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbMoments", "Moments" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbContours", "Contours" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbGray", "Gray" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbBlur", "Blur" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbDFT", "DFT" ) );
            //--
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addItem( new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
            //--
            toolBarLayout->addWidget( qtwrapper::make_widget< QCheckBox >( "cbZAuto", "Z-Auto" ) );
            toolBarLayout->addWidget( qtwrapper::make_widget< QDoubleSpinBox >( "zScale" ) );
            //--

            toolBarLayout->addWidget( new Utils::StyledSeparator );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        return toolBar;
    }
    return nullptr;
}

Utils::StyledBar *
MainWindow::createMidStyledToolbar()
{
    if ( auto toolBar = qtwrapper::make_widget< Utils::StyledBar >( "midToolbar" ) ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );

        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

        if ( auto spin = qtwrapper::make_widget< QDoubleSpinBox >( "zoomSpin" ) ) {
            spin->setRange( 0.5, 999.0 );
            spin->setValue( 1.0 );
            spin->setSingleStep( 0.020 );
            // spin->setSuffix( "\u03bcs" ); // == &mu;s
            toolBarLayout->addWidget( spin );
        }

        toolBarLayout->addWidget( toolButton( Core::ActionManager::command( Constants::HIDE_DOCK )->action() ) );

        return toolBar;
    }
    return nullptr;
}

void
MainWindow::onInitialUpdate()
{
    for ( auto widget: dockWidgets() ) {
        if ( auto lifecycle = qobject_cast< adplugin::LifeCycle * >( widget->widget() ) )
            lifecycle->OnInitialUpdate();
    }

    if ( auto form = findChild< adwidgets::adcv::ContoursForm * >() ) {
        // ADDEBUG() << "onInitialUpdate: " << document::instance()->contoursMethod().to_json();
        form->setValues( document::instance()->contoursMethod() );
        // ADDEBUG() << "onInitialUpdate: ----- done.";
    }

    if ( auto wnd = findChild< VideoProcWnd * >() ) {
        if ( auto spin = findChild< QDoubleSpinBox * >( "zoomSpin" ) ) {
            connect( wnd, &VideoProcWnd::onZoom, spin, &QDoubleSpinBox::setValue );
            connect( spin, qOverload<double>(&QDoubleSpinBox::valueChanged), wnd, &VideoProcWnd::handleZoomScale );
        }
    }

    if ( auto topBar = findChild< Utils::StyledBar * >( "topToolbar" ) ) {
        if ( auto cbx = topBar->findChild< QCheckBox * >( "cbZAuto" ) ) {
            cbx->setChecked( document::instance()->zScaleAutoEnabled() );
            connect( cbx, &QCheckBox::toggled, document::instance(), &document::setZScaleAutoEnabled );
            if ( auto wnd = findChild< VideoProcWnd * >() )
                connect( cbx, &QCheckBox::toggled, wnd, &VideoProcWnd::handleZAutoScaleEnabled );
        }
        if ( auto spin = topBar->findChild< QDoubleSpinBox * >( "zScale" ) ) {
            spin->setRange( 0.01, 99999.99 );
            spin->setSingleStep( 1.0 );
            spin->setValue( document::instance()->zScale() );
            connect( spin, qOverload<double>(&QDoubleSpinBox::valueChanged), document::instance(), &document::setZScale );
            if ( auto wnd = findChild< VideoProcWnd * >() )
                connect( spin, qOverload<double>(&QDoubleSpinBox::valueChanged), wnd, &VideoProcWnd::handleZScale );
        } else {
            assert(0);
        }
    }

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
    if ( Core::ActionManager * am = Core::ActionManager::instance() ) {

        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( tr("VIDEO") );

        if ( auto p = new QAction( QIcon( ":/video/images/if_webcam_add_64984.png" ), tr( "Camera" ), this ) ) {
            am->registerAction( p, Constants::VIDEO_CAPTURE, Core::Context( Core::Constants::C_GLOBAL ) );
            connect( p, &QAction::triggered, this, [](){ document::instance()->captureCamera(); } );
            menu->addAction( am->command( Constants::VIDEO_CAPTURE ) );
        }

        if ( auto p = new QAction( QIcon( ":/video/images/filesave.png" ), tr( "Save video to..." ), this ) ) {
            am->registerAction( p, Constants::VIDEO_FILE_SAVE, Core::Context( Core::Constants::C_GLOBAL ) );   // Tools|Malpix|Open SQLite file...
            //connect( p, &QAction::triggered, this, &MainWindow::capturedVideoSaveTo );
            menu->addAction( am->command( Constants::VIDEO_FILE_SAVE ) );
        }

        menu->addAction( am->command( Constants::HIDE_DOCK ) );

        // am->registerAction( p, Constants::VIDEO_PRINT_PDF, Core::Context( Core::Constants::C_GLOBAL ) );   // Tools|Malpix|Open SQLite file...
        // connect( p, &QAction::triggered, this, &MainWindow::filePrintPdf );
        // menu->addAction( am->command( Constants::VIDEO_PRINT_PDF ) );
        // am->registerAction( p, Core::Constants::PRINT, Core::Context( Constants::C_VIDEO_MODE ) );    // File|Print

        do {
            QIcon icon;
            icon.addPixmap( QPixmap( Constants::ICON_DOCKHIDE ), QIcon::Normal, QIcon::Off );
            icon.addPixmap( QPixmap( Constants::ICON_DOCKSHOW ), QIcon::Normal, QIcon::On );
            auto * action = new QAction( icon, tr( "Hide dock" ), this );
            action->setCheckable( true );
            am->registerAction( action, Constants::HIDE_DOCK, Core::Context( Core::Constants::C_GLOBAL ) );
            connect( action, &QAction::triggered, this, &MainWindow::hideDock );
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
    if ( auto form = new adwidgets::adcv::ContoursForm( this ) ) {

        auto dock = createDockWidget( form, tr( "Contours" ), "Contours" );
        dock->setMinimumWidth( 200 );
        dock->setMaximumWidth( 400 );
        dock->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );

        connect( form, &adwidgets::adcv::ContoursForm::valueChanged, [=]( auto id, int value ){
            document::instance()->setContoursMethod( form->toJson() );
        });

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
    if ( !pdfname.isEmpty() )
        printer.setOutputFileName( pdfname );

    QPrintDialog dialog(&printer, this);

    if (dialog.exec() == QDialog::Accepted) {

        settings.setValue( "Printer/FileName", printer.outputFileName() );

        QPainter painter(&printer);
        if ( auto view = findChild< VideoProcWnd * >() )
            view->print( painter, printer );
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
