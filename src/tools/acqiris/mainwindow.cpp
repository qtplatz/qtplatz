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
#if defined USING_CHARTS
# include "chartview.hpp"
#else
# include "waveformview.hpp"
#endif
#include "document.hpp"
#include "outputwidget.hpp"
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrswidgets/acqiriswidget.hpp>
#include <adportable/debug.hpp>
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
#if defined USING_CHARTS
    auto widget = new ChartView( this );
#else
    auto widget = new WaveformView( this );
#endif
    setCentralWidget( widget );    

    grabGesture( Qt::PanGesture );
    grabGesture( Qt::PinchGesture );

    connect( document::instance(), &document::updateData, this, &MainWindow::handleUpdateData );

    document::instance()->initialSetup();
    createDockWidgets();
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
    //connect(a, SIGNAL(triggered()), this, SLOT(close()));
    connect(a, &QAction::triggered, this, [&](){
            ADDEBUG() << "close...";
            close();
        });
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
    QMessageBox::about(this, tr("About"), tr("Digital oscilloscope app based on Acqiris/Agilent/Keysight digitizer") );
}

void
MainWindow::createDockWidgets()
{
    if ( auto widget = new acqrswidgets::AcqirisWidget() ) {
        auto dock = createDockWidget( widget, "Digitizer Config", "AcqirisWidget" );
        addDockWidget( Qt::RightDockWidgetArea, dock );
    }
    
    if ( auto widget = new OutputWidget( std::cout ) ) {
        auto dock = createDockWidget( widget, "Console", "ConsoleWidget" );
        addDockWidget( Qt::BottomDockWidgetArea, dock );
    }    
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

    return dockWidget;
}

void
MainWindow::onInitialUpdate()
{
    if ( auto widget = findChild< acqrswidgets::AcqirisWidget * >() ) {
        
        connect( widget, &acqrswidgets::AcqirisWidget::dataChanged, this, []( const acqrswidgets::AcqirisWidget * w, int subType ){
                auto m = std::make_shared< aqdrv4::acqiris_method >();
                w->getContents( m );
                document::instance()->handleValueChanged( m, aqdrv4::SubMethodType( subType ) );
            });

        // checkState for ch1, ch2
        connect( widget, &acqrswidgets::AcqirisWidget::stateChanged, this, [widget]( const QModelIndex& index , bool checked ){
                auto m = std::make_shared< aqdrv4::acqiris_method >();
                widget->getContents( m );
                document::instance()->handleValueChanged( m, aqdrv4::SubMethodType( 0 ) );
            });        

        connect( document::instance(), &document::on_acqiris_method_adapted, this, [this,widget](){
                if ( auto adapted = document::instance()->adapted_acqiris_method() )
                    widget->setContents( adapted );
            });

        auto m = document::instance()->acqiris_method();
        widget->setContents( m );
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
#if defined USING_CHARTS
        if ( auto view = findChild< ChartView * >() )
            view->setData( wform );
#else
        if ( auto view = findChild< WaveformView * >() )
            view->setData( wform );
#endif
    }
}
