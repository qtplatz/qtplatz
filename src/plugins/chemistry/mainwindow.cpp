/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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
#include "chemconnection.hpp"
#include "document.hpp"
#include "chemistryconstants.hpp"
#include "chemquery.hpp"
#include "massdefectform.hpp"
#include "moltablewnd.hpp"
#include <adportable/profile.hpp>
#include <adchem/sdfile.hpp>
#include <adwidgets/molview.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>

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
#include <utils/styledbar.h>

#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>
#include <QResizeEvent>
#include <qstackedwidget.h>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#include <QtGui/QIcon>
#include <QStandardItemModel>
#include <QProgressBar>

#include <boost/filesystem/path.hpp>
#include <boost/bind.hpp>
#include <algorithm>

using namespace chemistry;

MainWindow * MainWindow::instance_ = 0;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow( QWidget * parent ) : Utils::FancyMainWindow( parent )
                                           , toolBar_( 0 )
                                           , toolBarLayout_( 0 )
                                           , actionSearch_( 0 )
                                           , progressBar_( 0 )
{
    
	instance_ = this;
}


void
MainWindow::OnInitialUpdate()
{
    connect( document::instance(), &document::onConnectionChanged, [this]{ handleConnectionChanged(); } );
    
	setSimpleDockWidgetArrangement();
    document::instance()->initialSetup();
}

void
MainWindow::OnClose()
{
    document::instance()->finalClose();
}

void
MainWindow::activateLayout()
{
}

void
MainWindow::createActions()
{
	actionSearch_ = new QAction( QIcon( ":/chemistry/images/search.png" ), tr("Search"), this );
    connect( actionSearch_, SIGNAL( triggered() ), this, SLOT( actionSearch() ) );
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

    if ( auto wnd = findChild< MolTableWnd * >() ) {
		wnd->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( wnd, SIGNAL( dropped( const QList<QUrl>& ) ), this, SLOT( handleDropped( const QList<QUrl>& ) ) );
    }

    QWidget * editorAndFindWidget = new QWidget;
    if ( editorAndFindWidget ) {
        
        editorAndFindWidget->setLayout( editorHolderLayout );
        editorAndFindWidget->setLayout( editorHolderLayout );

        editorHolderLayout->addWidget( new MolTableWnd() );
    }

    Core::MiniSplitter * documentAndRightPane = new Core::MiniSplitter;
    if ( documentAndRightPane ) {
        documentAndRightPane->addWidget( editorAndFindWidget );
        documentAndRightPane->addWidget( new Core::RightPanePlaceHolder( mode ) );
        documentAndRightPane->setStretchFactor( 0, 1 );
        documentAndRightPane->setStretchFactor( 1, 0 );
    }

    Utils::StyledBar * toolBar1 = createTopStyledBar();
    Utils::StyledBar * toolBar2 = createMidStyledBar();

	//---------- centraol widget ------------
	QWidget * centralWidget = new QWidget;
	setCentralWidget( centralWidget );

	QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
	centralLayout->setMargin( 0 );
	centralLayout->setSpacing( 0 );
    centralLayout->addWidget( toolBar1 );
    centralLayout->addWidget( documentAndRightPane );
	centralLayout->addWidget( toolBar2 );

	// Right-side window with editor, output etc.
	Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter;
    QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
    outputPane->setObjectName( QLatin1String( "ChemistryOutputPanePlaceHolder" ) );
	mainWindowSplitter->addWidget( this );
    mainWindowSplitter->addWidget( outputPane );
	mainWindowSplitter->setStretchFactor( 0, 10 );
	mainWindowSplitter->setStretchFactor( 1, 0 );
	mainWindowSplitter->setOrientation( Qt::Vertical );

	// Navigation and right-side window
	Core::MiniSplitter * splitter = new Core::MiniSplitter;
	splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
    splitter->addWidget( mainWindowSplitter );
    splitter->setStretchFactor( 0, 0 );
    splitter->setStretchFactor( 1, 1 );
    splitter->setObjectName( QLatin1String( "ChemistryModeWidget" ) );

	createDockWidgets();

    if ( auto view = findChild< adwidgets::MolView * >() ) {
        if ( auto wnd = findChild< MolTableWnd * >() ) {
            connect( wnd, &MolTableWnd::activated, [=]( const QModelIndex& index ){
                });
        }
    }

	return splitter;
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
	qtwrapper::TrackingEnabled<MainWindow> x( *this );

	QList< QDockWidget *> dockWidgets = this->dockWidgets();
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
		dockWidget->setFloating( false );
		removeDockWidget( dockWidget );
	}
  
    size_t npos = 0;
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
		addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
		dockWidget->show();
        if ( npos++ > 1 )
			tabifyDockWidget( dockWidgets[1], dockWidget );
	}

	update();
}


QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& objname )
{
    if ( widget->windowTitle().isEmpty() ) // avoid QTC_CHECK warning on console
        widget->setWindowTitle( title );

    if ( widget->objectName().isEmpty() )
        widget->setObjectName( objname );

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
MainWindow::createDockWidgets()
{
    if ( auto w = new adwidgets::MolView( this ) ) {
        createDockWidget( w, "MOL", "MolView" );
    }

    if ( auto w = new QTextEdit( this ) ) {
        createDockWidget( w, "Edit", "Text" );
    }

    // MassDefectForm * form = new MassDefectForm;
	// form->setObjectName( "massdefect" );
    // form->OnInitialUpdate();
	// createDockWidget( form );
}

void
MainWindow::createToolbar()
{
	QWidget * toolbarContainer = new QWidget;
	QHBoxLayout * hbox = new QHBoxLayout( toolbarContainer );
    hbox->setMargin( 0 );
    hbox->setSpacing( 0 );
    hbox->addWidget( toolButton( "STOP" ) ); // should create action in 'plugin' with icon
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

// slot
void
MainWindow::actionSearch()
{
}

MainWindow *
MainWindow::instance()
{
	return instance_;
}

void
MainWindow::handleDropped( const QList< QUrl >& urls )
{
    if ( auto wnd = findChild< MolTableWnd * >() ) {
        for ( auto& url: urls ) {
            boost::filesystem::path path( url.toLocalFile().toStdWString() );
            //topLineEdit_->setText( QString::fromStdWString( path.wstring() ) );
            adchem::SDFile file( path.string() );
            // wnd->setMol( file, *progressBar_ );
        }
    }
}

Utils::StyledBar *
MainWindow::createMidStyledBar()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );

        if ( auto am = Core::ActionManager::instance() ) {
            toolBarLayout->addWidget(toolButton(am->command(Constants::SDFILE_OPEN)->action()));
        }
        
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        progressBar_ = new QProgressBar;
		progressBar_->setVisible( false );
        toolBarLayout->addWidget( progressBar_ );
        progressBar_->setStyleSheet( QString("QProgressBar { color: lightgreen }") );
    }
    return toolBar;
}

Utils::StyledBar *
MainWindow::createTopStyledBar()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
    }
    return toolBar;
}

void
MainWindow::actSDFileOpen()
{
    boost::filesystem::path datapath( adportable::profile::user_data_dir<char>() );
    datapath /= "data";

    QString name
        = QFileDialog::getOpenFileName( this
                                        , tr("Open Chemistry database" )
										, datapath.string().c_str()
                                        , tr("Structure Data files(*.sdf);;MDL MOL files(*.mol);;QtPlatz Chem DB(*.adfs)") );
    if ( ! name.isEmpty() ) {

        if ( auto wnd = findChild< MolTableWnd * >() ) {
            qtwrapper::waitCursor wait;
            QFileInfo finfo( name );
            
            if ( finfo.suffix() == "sdf" || finfo.suffix() == "mol" ) {
                
                //topLineEdit_->setText( name );
                adchem::SDFile file( name.toStdString() );
                //wnd->setMol( file, *progressBar_ );
                
            } else if ( finfo.suffix() == "adfs" ) {
                
                if ( auto connection = std::make_shared< ChemConnection >() ) {
                    
                    if ( connection->connect( name.toStdWString() ) )
                        document::instance()->setConnection( connection.get() );
                    
                }
            }
        }
	}
}

void
MainWindow::handleConnectionChanged()
{
    // auto self( document::instance()->connection()->shared_from_this() );
    // auto query = std::make_shared< ChemQuery >( self->db() );

    // query->prepare( "SELECT * FROM mols" );
    // document::instance()->setQuery( query.get() );

    if ( auto table = findChild< MolTableWnd * >() ) {
        table->setQuery( "SELECT * FROM mols" );
    }
}

