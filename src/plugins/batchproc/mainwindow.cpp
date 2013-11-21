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
#include "batchprocconstants.hpp"
#include "droptargetform.hpp"
#include "batchprocdelegate.hpp"
#include "import.hpp"
#include "task.hpp"
#include "process.hpp"
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
#include <QMenu>

#include <boost/filesystem.hpp>

using namespace batchproc;

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , tableView_( new QTableView( this ) )
                                        , model_( new QStandardItemModel )
                                        , delegate_( new BatchprocDelegate )
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
		tableView_->setItemDelegate( delegate_.get() );
        connect( tableView_.get(), SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu( const QPoint& ) ) );
        connect( delegate_.get(), SIGNAL( stateChanged( const QModelIndex& ) ), this, SLOT( handleStateChanged( const QModelIndex& ) ) );
		tableView_->setContextMenuPolicy( Qt::CustomContextMenu );
	}
    connect( this, SIGNAL( emitProgress(int, int, int) ), this, SLOT( handleProgress( int, int, int ) ) );
/*
    Core::MiniSplitter * documentAndRightPane = new Core::MiniSplitter;
    if ( documentAndRightPane ) {
        documentAndRightPane->addWidget( editorAndFindWidget );
        documentAndRightPane->addWidget( new Core::RightPanePlaceHolder( mode ) );
        documentAndRightPane->setStretchFactor( 0, 1 );
        documentAndRightPane->setStretchFactor( 1, 0 );
    }
*/
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        toolBarLayout->addItem( new QSpacerItem( 40, 20 ) );
        toolBarLayout->addWidget( new QLabel( tr("Control Method: ") ) );

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
        // centralLayout->addWidget( documentAndRightPane ); // [0]
        centralLayout->addWidget( editorAndFindWidget ); // [0]
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
	using namespace batchproc::Constants;

    QStandardItemModel& model = *model_;
    model.setColumnCount( Constants::c_batchproc_num_columns );
    model.setHeaderData( 0, Qt::Vertical, QObject::tr( "#" ) );
    model.setHeaderData( c_batchproc_filename, Qt::Horizontal, QObject::tr( "File name" ) );
    model.setHeaderData( c_batchproc_process, Qt::Horizontal, QObject::tr( "Process" ) );
	model.setHeaderData( c_batchproc_progress, Qt::Horizontal, QObject::tr( "Progress" ) );
    model.setRowCount( 1 );

	tableView_->resizeRowsToContents();

    setSimpleDockWidgetArrangement();
}

void
MainWindow::createDockWidgets()
{
    if ( QWidget * w = new DropTargetForm( this ) ) {
        connect( w, SIGNAL( dropped( const QList<QString>& ) ), this, SLOT( handleDropped( const QList<QString>& ) ) );
        createDockWidget( w, "Drop Target" );
    }
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

void
MainWindow::handleDropped( const QList<QString>& files )
{
    QStandardItemModel& model = *model_;

    for ( auto file: files ) {

        boost::filesystem::path path( file.toStdWString() );
        
        if ( std::find( files_.begin(), files_.end(), path ) == files_.end() ) {

            const int row = int( files_.size()  );
            files_.push_back( file.toStdWString() );

            if ( model.rowCount() < int( files_.size() ) )
                model.setRowCount( files_.size() );

            model.setData( model.index( row, Constants::c_batchproc_filename ), file );
			model.setData( model.index( row, Constants::c_batchproc_process ), QVariant::fromValue( process( PROCESS_IMPORT ) ) );
			model.setData( model.index( row, Constants::c_batchproc_state ), "Waiting" );
            if ( QStandardItem * cbx = model.itemFromIndex( model.index( row, Constants::c_batchproc_state ) ) ) {
                cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
                cbx->setEditable( true );
                model.setData( model.index( row, Constants::c_batchproc_state ), Qt::Unchecked, Qt::CheckStateRole );
            }
            model.item( row, Constants::c_batchproc_filename )->setEditable( false );
        }
    }
    tableView_->resizeColumnsToContents();
	tableView_->resizeRowsToContents();
}

void
MainWindow::showContextMenu( const QPoint& pt )
{
	QModelIndex index = tableView_->currentIndex();

    if ( index.isValid() ) {

        std::vector< QAction * > actions;
        QMenu menu;
        
        QString file = model_->index( index.row(), Constants::c_batchproc_filename ).data( Qt::EditRole ).toString();
        actions.push_back( menu.addAction( "Cancel " + file ) );
        if ( QAction * selected = menu.exec( this->mapToGlobal( pt ) ) ) {
            if ( selected == actions[ 0 ] ) {
                auto proc = model_->index( index.row(), Constants::c_batchproc_process ).data().value< process >();
                proc.state( PROCESS_CANCELING );
                model_->setData( model_->index( index.row(), Constants::c_batchproc_process ), QVariant::fromValue( proc ) );
            }
        }
    }
}

void
MainWindow::handleStateChanged( const QModelIndex& index )
{
    if ( index.column() == Constants::c_batchproc_state ) {

        if ( bool state = index.data( Qt::CheckStateRole ) == Qt::Checked ) {

            model_->item( index.row(), Constants::c_batchproc_process )->setEditable( false );
            // model_->item( index.row(), index.column() )->setEditable( false ); // cbx

            process proc = model_->index( index.row(), Constants::c_batchproc_process ).data().value< process >();

            if ( proc.state() == PROCESS_IDLE ) {

                proc.state( PROCESS_RUNNING );
				model_->setData( model_->index( index.row(), Constants::c_batchproc_process ), QVariant::fromValue( proc ) );

                QString file = model_->index( index.row(), Constants::c_batchproc_filename ).data( Qt::EditRole ).toString();

                auto proc
                    = std::make_shared< import >( index.row(), file.toStdWString(), L"", [=](int row, int curr, int total)->bool{
                            emit emitProgress( row, curr, total );
                            return model_->index( row, Constants::c_batchproc_process ).data().value< process >().state() == PROCESS_CANCELING;
                        });
                if ( proc && *proc )
                    task::instance()->post( *proc );
            }

        } else {
            process proc = model_->index( index.row(), Constants::c_batchproc_process ).data().value< process >();
            proc.state( PROCESS_CANCELING );
            model_->setData( model_->index( index.row(), Constants::c_batchproc_process ), QVariant::fromValue( proc ) );
        }
    }
}

void
MainWindow::handleProgress( int rowId, int curr, int total )
{
    QStandardItemModel& model = *model_;

    QModelIndex index = model.index( rowId, Constants::c_batchproc_progress );
    model.setData( index, double( curr * 100 ) / total );
}
