/**************************************************************************
** Copyright (C) 2016-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "moltablewnd.hpp"
#include "outputwidget.hpp"
#include <adui/manhattanstyle.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/settings.hpp>
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QComboBox>
#include <QDirIterator>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QtDebug>
#include <functional>
#include <iostream>

#ifdef Q_WS_MAC
const QString rsrcPath = ":/adui/images/mac";
#else
const QString rsrcPath = ":/adui/images/win";
#endif

using namespace sdfview;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
                                        , timer_(new QTimer(this))
{
#ifndef WIN32
    Q_INIT_RESOURCE( adui );
#endif
    auto baseName = QApplication::style()->objectName();
    qApp->setStyle( new adui::ManhattanStyle( baseName ) );

    setDockNestingEnabled( true );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );

    // setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::East );
    setDocumentMode( true );
    // setDockNestingEnabled( true );

    statusBar()->setProperty( "p_styled", true );
    statusBar()->addWidget( new QLabel );

    if ( auto p = statusBar()->findChild<QLabel *>() ) {
        p->setText( "STATUS:" );
    }

    if ( ( progressBar_ = new QProgressBar( this ) ) ) {
        progressBar_->setTextVisible(false);
        statusBar()->addPermanentWidget( progressBar_ );
        // progressBar_->hide();
    }

    setupFileActions();
    setupEditActions();
    {
        QMenu *helpMenu = new QMenu(tr("Help"), this);
        menuBar()->addMenu(helpMenu);
        helpMenu->addAction(tr("About"), this, SLOT(about()));
        helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
    }
    setCentralWidget( new QWidget );
    auto layout = new QVBoxLayout( centralWidget() );
    layout->addWidget( new sdfview::MolTableWnd() );
    // layout->setContentsMargins( { 0 } );
    layout->setSpacing( 0 );

    grabGesture( Qt::PanGesture );
    grabGesture( Qt::PinchGesture );

    createDockWidgets();
}

MainWindow::~MainWindow()
{
}

void
MainWindow::closeEvent( QCloseEvent *e )
{
}

void
MainWindow::setupFileActions()
{
    if ( ! QFile::exists( rsrcPath ) ) {
        ADDEBUG() << "############# Resource path does not exists ###############";
        QDirIterator it(":", QDirIterator::Subdirectories);
        while (it.hasNext()) {
            qDebug() << it.next();
        }
    }

    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("File Actions"));
    addToolBar(tb);

    QMenu *menu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(menu);

    QAction *a;

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
    document::instance()->load( f );
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
    QString fn = QFileDialog::getOpenFileName(this
                                              , tr("Open File...")
                                              , qtwrapper::settings( *document::instance()->settings() ).recentFile( "SDF", "Files" )
                                              , tr("SDF Files (*.sdf);;All Files (*)"));
    if ( !fn.isEmpty() ) {
        if ( load(fn) ) {
            qtwrapper::settings( *document::instance()->settings() ).addRecentFiles( "SDF", "Files", fn );
        }
    }
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
    connect( document::instance(), &document::onConnectionChanged, [this]{ handleConnectionChanged(); } );
    document::instance()->initialSetup();
    if ( auto table = findChild< sdfview::MolTableWnd * >() ) {
        connect( document::instance(), &document::onSDFileChanged, table, &sdfview::MolTableWnd::handleSDFileChanged );
        connect( table, &sdfview::MolTableWnd::onProgressInitiated, this, &MainWindow::handleProgressInitated );
        connect( table, &sdfview::MolTableWnd::onProgressFinished, this, &MainWindow::handleProgressFinished );
        connect( table, &sdfview::MolTableWnd::onProgress, this, &MainWindow::handleProgress );
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
}

void
MainWindow::handleConnectionChanged()
{
    if ( auto table = findChild< sdfview::MolTableWnd * >() ) {
        ADDEBUG() << "##### " << __FUNCTION__ << " #####";
        table->setQuery(
            "SELECT t1.id,svg,synonym,formula,mass,csid,smiles,InChI,InChiKey,SystematicName FROM mols t1 LEFT OUTER JOIN synonyms t2 on t1.id = t2.id" );
    } else {
        ADDEBUG() << "##### " << __FUNCTION__ << " ##### " << "MolTableWnd not found";
    }
}

void
MainWindow::handleProgressInitated( int total )
{
    progressBar_->show();
    progressBar_->reset();
    progressBar_->setMinimum( 0 );
    progressBar_->setMaximum( total );
}

void
MainWindow::handleProgressFinished()
{
    progressBar_->hide();
}

void
MainWindow::handleProgress( int current )
{
    progressBar_->setValue( current );
}
