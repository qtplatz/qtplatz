/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "quanconstants.hpp"
#include "doubletabwidget.hpp"
#include "panelswidget.hpp"
#include "paneldata.hpp"
#include <qtwrapper/trackingenabled.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/profile.hpp>
#include <adlog/logger.hpp>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <utils/styledbar.h>

#include <QFileDialog>
#include <QLineEdit>
#include <QStackedWidget>
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

using namespace quan;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QWidget *parent) : QWidget( parent ) // Utils::FancyMainWindow(parent)
{
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    //QWidget * centralWidget = new QWidget;
    //setCentralWidget( centralWidget );

    QVBoxLayout * viewLayout = new QVBoxLayout( this );
    viewLayout->setMargin(0);
    viewLayout->setSpacing(0);
    
    auto tabWidget = new DoubleTabWidget( this );
    QStringList tabs;
    tabs << "Sub1" << "Sub2";
    tabWidget->addTab( "Tab1", "Tab1 fullname", tabs );
    tabs << "Sub3" << "Sub4";
    tabWidget->addTab( "TabA", "TabA fullname", tabs );

    viewLayout->addWidget( tabWidget );

    auto stack = new QStackedWidget;
    viewLayout->addWidget( stack );
    // connect( tabWidget, DoubleTabWidget::currentIndexChanged, this, MainWindow::showProperties );

    if ( auto panels = new PanelsWidget( stack ) ) {
        auto panel = std::make_shared< PanelData >();
        panel->setDisplayName( "Panel" );
        panel->setWidget( new QTextEdit );
        panel->setIcon( QIcon( QLatin1String( ":/quan/images/BuildSettings.png" ) ) );
        panels->addPanel( panel.get() );

        stack->addWidget( panel->widget() );
        stack->setCurrentWidget( panel->widget() );
    }
    
    return this;
#if 0
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::East );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;

	editorHolderLayout->setMargin( 0 );
	editorHolderLayout->setSpacing( 0 );
	    
    if ( QWidget * editorWidget = new QWidget ) {

        editorWidget->setLayout( editorHolderLayout );
        editorHolderLayout->addWidget( new QTextEdit() );

        Utils::StyledBar * toolBar1 = createTopStyledBar();
        Utils::StyledBar * toolBar2 = createMidStyledBar();
        
        //---------- central widget ------------
        QWidget * centralWidget = new QWidget;
        if ( centralWidget ) {
            setCentralWidget( centralWidget );
            
            QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
            centralWidget->setLayout( centralLayout );
            centralLayout->setMargin( 0 );
            centralLayout->setSpacing( 0 );
            // ----------------- top tool bar -------------------
            centralLayout->addWidget( toolBar1 );              // [1]
            // ----------------------------------------------------
            
            centralLayout->addWidget( editorWidget ); // [0]
            centralLayout->setStretch( 0, 1 );
            centralLayout->setStretch( 1, 0 );
            
            // ----------------- mid tool bar -------------------
            centralLayout->addWidget( toolBar2 );              // [1]
        }
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
#endif

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
        Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( am ) {
            // [file open] button
            //toolBarLayout->addWidget( toolButton( am->command( Constants::FILE_OPEN )->action() ) );
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //---
            //toolBarLayout->addWidget( topLineEdit_.get() );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    }
    return toolBar;
}

Utils::StyledBar *
MainWindow::createMidStyledBar()
{
    if ( Utils::StyledBar * toolBar = new Utils::StyledBar ) {

        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( am ) {
            // print, method file open & save buttons
            //toolBarLayout->addWidget(toolButton(am->command(Constants::PRINT_CURRENT_VIEW)->action()));
            //toolBarLayout->addWidget(toolButton(am->command(Constants::METHOD_OPEN)->action()));
            //toolBarLayout->addWidget(toolButton(am->command(Constants::FILE_OPEN)->action()));
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //----------
            QList<int> context;
            context << Core::Constants::C_GLOBAL_ID;
            
            // QComboBox * features = new QComboBox;
            // features->addItem( "Centroid" );
            // features->addItem( "Isotope" );
            // features->addItem( "Calibration" );
            // features->addItem( "Find peaks" );
            // toolBarLayout->addWidget( features );

            //connect( features, SIGNAL( currentIndexChanged(int) ), this, SLOT( handleFeatureSelected(int) ) );
            //connect( features, SIGNAL( activated(int) ), this, SLOT( handleFeatureActivated(int) ) );

            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );

            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        }
		return toolBar;
    }
    return 0;
}

void
MainWindow::onInitialUpdate()
{
    setSimpleDockWidgetArrangement();
}

void
MainWindow::createDockWidgets()
{
    createDockWidget( new QTextEdit(), "Sequence" );
    // if ( QWidget * w = new DropTargetForm( this ) ) {
    //     connect( w, SIGNAL( dropped( const QList<QString>& ) ), this, SLOT( handleDropped( const QList<QString>& ) ) );
    //     createDockWidget( w, "Drop Target" );
    // }
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title )
{
#if 0
    QDockWidget * dockWidget = addDockForWidget( widget );

    dockWidget->setObjectName( widget->objectName() );
    if ( title.isEmpty() )
        dockWidget->setWindowTitle( widget->objectName() );
    else
        dockWidget->setWindowTitle( title );

    addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

    return dockWidget;
#endif
    return 0;
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
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
    Core::ActionManager * mgr = Core::ICore::instance()->actionManager();
    return toolButton( mgr->command(id)->action() );
}

QAction *
MainWindow::createAction( const QString& iconname, const QString& msg, QObject * parent )
{
    QIcon icon;
    icon.addFile( iconname );
    return new QAction( icon, msg, parent );
}

void
MainWindow::createActions()
{
    //actions_[ idActFileOpen ] = createAction( Constants::ICON_FILE_OPEN, tr("Open protain file..."), this );
    //connect( actions_[ idActFileOpen ], SIGNAL( triggered() ), this, SLOT( actFileOpen() ) );

    const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;

    if ( Core::ActionManager * am = Core::ICore::instance()->actionManager() ) {
        
        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( "Quan" );
        
        //Core::Command * cmd = 0;

        //cmd = am->registerAction( actions_[ idActFileOpen ], Constants::FILE_OPEN, gc );
        //menu->addAction( cmd );

        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}
