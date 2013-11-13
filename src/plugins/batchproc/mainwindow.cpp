/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "batchmode.hpp"
#include "droptargetform.hpp"
#include <qtwrapper/trackingenabled.hpp>

#include <coreplugin/minisplitter.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <utils/styledbar.h>

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

using namespace batchproc;

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , tableView_( new QTableView( this ) )
                                        , model_( new QStandardItemModel )
{
}

MainWindow::~MainWindow()
{
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::East );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;
	editorHolderLayout->setMargin( 0 );
	editorHolderLayout->setSpacing( 0 );
	    
    QWidget * editorAndFindWidget = new QWidget;
    if ( editorAndFindWidget ) {
        editorAndFindWidget->setLayout( editorHolderLayout );
        editorHolderLayout->addWidget( tableView_.get() );
        tableView_->setModel( model_.get() );
        editorHolderLayout->addWidget( new QTextEdit );
    }

    Core::MiniSplitter * documentAndRightPane = new Core::MiniSplitter;
    if ( documentAndRightPane ) {
        documentAndRightPane->addWidget( editorAndFindWidget );
        documentAndRightPane->addWidget( new Core::RightPanePlaceHolder( mode ) );
        documentAndRightPane->setStretchFactor( 0, 1 );
        documentAndRightPane->setStretchFactor( 1, 0 );
    }

    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        toolBarLayout->addItem( new QSpacerItem( 40, 20 ) );
        toolBarLayout->addWidget( new QLabel( tr("Control Method: ") ) );
        // toolBarLayout->addWidget( ctrlMethodName_ = new QLineEdit );
        // ctrlMethodName_->setReadOnly( true );

        //toolBarLayout->addItem( new QSpacerItem( 40, 20 ) );
        //toolBarLayout->addWidget( new QLabel( tr("Process Method: ") ) );
        // toolBarLayout->addWidget( procMethodName_ = new QLineEdit );
        // procMethodName_->setReadOnly( true );

        toolBarLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ) );
        //
        QDockWidget * dock = new QDockWidget( "Batch Toolbar" );
        dock->setObjectName( QLatin1String( "Batch Toolbar" ) );
        // dock->setWidget( toolBar );
        dock->setFeatures( QDockWidget::NoDockWidgetFeatures );
        dock->setAllowedAreas( Qt::BottomDockWidgetArea );
        dock->setTitleBarWidget( new QWidget( dock ) );
        dock->setProperty( "manaaged_dockwidget", QLatin1String( "true" ) );
        addDockWidget( Qt::BottomDockWidgetArea, dock );
        // setToolBarDockWidget( dock );
    }

	//---------- central widget ------------
	QWidget * centralWidget = new QWidget;
    if ( centralWidget ) {
        setCentralWidget( centralWidget );

        QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
        centralWidget->setLayout( centralLayout );
        centralLayout->setMargin( 0 );
        centralLayout->setSpacing( 0 );
        centralLayout->addWidget( documentAndRightPane ); // [0]
        centralLayout->setStretch( 0, 1 );
        centralLayout->setStretch( 1, 0 );

        centralLayout->addWidget( toolBar );              // [1]
    }

	// Right-side window with editor, output etc.
	Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter;
    if ( mainWindowSplitter ) {
        QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
        outputPane->setObjectName( QLatin1String( "SequenceOutputPanePlaceHolder" ) );
        mainWindowSplitter->addWidget( this );
        mainWindowSplitter->addWidget( outputPane );
        mainWindowSplitter->setStretchFactor( 0, 10 );
        mainWindowSplitter->setStretchFactor( 1, 0 );
        mainWindowSplitter->setOrientation( Qt::Vertical );
    }

	// Navigation and right-side window
	Core::MiniSplitter * splitter = new Core::MiniSplitter;               // entier this view
    if ( splitter ) {
        splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) ); // navegate
        splitter->addWidget( mainWindowSplitter );                            // *this + ontput
        splitter->setStretchFactor( 0, 0 );
        splitter->setStretchFactor( 1, 1 );
        splitter->setObjectName( QLatin1String( "SequenceModeWidget" ) );
    }

    createDockWidgets();

	return splitter;
}

void
MainWindow::createActions()
{
}

void
MainWindow::onInitialUpdate()
{
    model_->setColumnCount( 10 );
    model_->setRowCount( 10 );
    setSimpleDockWidgetArrangement();
}

void
MainWindow::createDockWidgets()
{
    QWidget * w = new DropTargetForm( this );
    createDockWidget( w, "Drop Target" );
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title )
{
    QDockWidget * dockWidget = addDockForWidget( widget );

    dockWidget->setObjectName( widget->objectName() );
    if ( title.isEmpty() )
        dockWidget->setWindowTitle( widget->objectName() );
    else
        dockWidget->setWindowTitle( title );

    addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

    return dockWidget;
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
        if ( npos++ >= 2 )
            tabifyDockWidget( widgets[1], widget );
    }
	// widgets[1]->raise();

    // QDockWidget * toolBarDock = toolBarDockWidget();
    // if ( toolBarDock )
    //     toolBarDock->show();
    update();
}
