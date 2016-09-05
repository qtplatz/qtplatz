/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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
#include "document.hpp"
// #include "waveformview.hpp"
#include "chartview.hpp"
#include <boost/any.hpp>
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QComboBox>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QtDebug>
#include <functional>
#include <iostream>

#ifdef Q_WS_MAC
const QString rsrcPath = ":/resources/images/mac";
#else
const QString rsrcPath = ":/resources/images/win";
#endif

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
                                        , stacked_( 0 )
                                        , timer_(new QTimer(this))
{
    setDockNestingEnabled( true );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );

    statusBar()->setProperty( "p_styled", true );
    statusBar()->addWidget( new QLabel );
    
    if ( auto p = statusBar()->findChild<QLabel *>() ) {
        p->setText( "STATUS:" );
    }

    setupFileActions();
    setupEditActions();
    {
        QMenu *helpMenu = new QMenu(tr("Help"), this);
        menuBar()->addMenu(helpMenu);
        helpMenu->addAction(tr("About"), this, SLOT(about()));
        helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
    }
#if 0
    auto widget = new WaveformView( this );
#else
    auto widget = new ChartView( this );
#endif
    setCentralWidget( widget );    

    grabGesture( Qt::PanGesture );
    grabGesture( Qt::PinchGesture );

    connect( document::instance(), &document::updateData, this, &MainWindow::handleUpdateData );

    document::instance()->initialSetup();
}

MainWindow::~MainWindow()
{
}

void
MainWindow::closeEvent( QCloseEvent *e )
{
    if ( document::instance()->finalClose() )
        e->accept();
    else
        e->ignore();
}

void
MainWindow::setupFileActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("File Actions"));
    addToolBar(tb);

    QMenu *menu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(menu);

    QAction *a;

    QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/filenew.png"));
    a = new QAction( newIcon, tr("&New"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);
    connect(a, SIGNAL(triggered()), this, SLOT(fileNew()));
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction(QIcon::fromTheme("document-open", QIcon(rsrcPath + "/fileopen.png")), tr("&Open..."), this);
    a->setShortcut(QKeySequence::Open);
    connect(a, SIGNAL(triggered()), this, SLOT(fileOpen()));
    tb->addAction(a);
    menu->addAction(a);

    menu->addSeparator();

    actionSave = a = new QAction(QIcon::fromTheme("document-save", QIcon(rsrcPath + "/filesave.png")), tr("&Save"), this);
    a->setShortcut(QKeySequence::Save);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSave()));
    a->setEnabled(false);
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction(tr("Save &As..."), this);
    a->setPriority(QAction::LowPriority);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
    menu->addAction(a);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    a = new QAction(QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png")), tr("&Print..."), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrint()));
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction(QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png")),
                    tr("Print Preview..."), this);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPreview()));
    menu->addAction(a);

    a = new QAction(QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png")),
                    tr("&Export PDF..."), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPdf()));
    tb->addAction(a);
    menu->addAction(a);

    menu->addSeparator();
#endif

    a = new QAction(tr("&Quit"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(a, SIGNAL(triggered()), this, SLOT(close()));
    menu->addAction(a);
}

void
MainWindow::setupEditActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Edit Actions"));
    addToolBar(tb);
    QMenu *menu = new QMenu(tr("&Edit"), this);
    menuBar()->addMenu(menu);
}

void
MainWindow::setupTextActions()
{
}

bool
MainWindow::load(const QString &f)
{
    setCurrentFileName( f );
    return true;
}

void
MainWindow::setCurrentFileName(const QString &fileName)
{
}

void
MainWindow::fileNew()
{
}

void MainWindow::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Open File..."),
                                              QString(), tr("HTML-Files (*.htm *.html);;All Files (*)"));
    if ( !fn.isEmpty() )
        load(fn);
}

bool
MainWindow::fileSave()
{
    return true;
}

bool
MainWindow::fileSaveAs()
{
    return true;
}

void
MainWindow::filePrint()
{
}

void
MainWindow::filePrintPreview()
{
}

void
MainWindow::printPreview(QPrinter *printer)
{
}

void
MainWindow::filePrintPdf()
{
}

void
MainWindow::clipboardDataChanged()
{
}

void
MainWindow::about()
{
    QMessageBox::about(this, tr("About"), tr("This is the GUI version of MALPIX4 examination platrom.") );
}

void
MainWindow::createDockWidgets()
{
#if 0
    static const struct { 
        const QString title;
        const char * wiid;
        const char * pageName;
        std::function<QWidget *()> factory;
    } widgets [] = { 
        // { tr( "Console" ), "eventtool::ConsoleWidget", "ConsoleWidget", [this] () { return new adwidgets::ConsoleWidget( std::cout, this ); } }
    };
    
    // for ( auto& widget: widgets ) {
    //     if ( auto pWidget = widget.factory() ) {
    //         createDockWidget( pWidget, widget.title, widget.pageName );

    //         if ( auto lifecycle = dynamic_cast< adplugin::LifeCycle * >( pWidget ) ) {
    //             lifecycle->OnInitialUpdate();
    //         }

    //     }
    // }
#endif
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& pageName )
{
    if ( widget->windowTitle().isEmpty() ) // avoid QTC_CHECK warning on console
        widget->setWindowTitle( title );
    if ( widget->objectName().isEmpty() )
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
MainWindow::handleInstState( int stat )
{
    // if ( auto form = findChild< malpix::mpxwidgets::Mpx4Form * >() ) {
    //     form->setInstStatus( static_cast<adinterface::instrument::eInstStatus>(stat) );
    // }
}

void
MainWindow::handleFeatureSelected( int )
{
}

void
MainWindow::handleFeatureActivated( int )
{
}

void
MainWindow::onInitialUpdate()
{
    auto docks = findChildren< QDockWidget *>();
    for ( auto& widget : docks ) {
        widget->setFloating( false );
        removeDockWidget( widget );
    }
    size_t npos = 0;

    for ( auto widget: docks ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
        if ( npos++ >= 2 )
            tabifyDockWidget( docks[ 1 ], widget );
    }

}


QDockWidget *
MainWindow::addDockForWidget( QWidget * widget )
{
    auto dock = new QDockWidget();
    dock->setWidget( widget );
    dock->setMouseTracking( true );

    return dock;
}

void
MainWindow::handleUpdateData()
{
    if ( auto wform = document::instance()->recentWaveform() ) {
        // if ( auto view = findChild< WaveformView * >() )
        //     view->setData( wform );
        // else
        if ( auto view = findChild< ChartView * >() )
            view->setData( wform );
    }
}
