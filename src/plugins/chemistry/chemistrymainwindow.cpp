/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "chemistrymainwindow.hpp"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <utils/styledbar.h>

#include <QDockWidget>
#include <qmenu.h>
#include <QResizeEvent>
#include <qstackedwidget.h>
#include <QVBoxLayout>

namespace Chemistry { namespace Internal {

    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
	public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };
} // Internal
}

using namespace Chemistry::Internal;

ChemistryMainWindow::~ChemistryMainWindow()
{
}

ChemistryMainWindow::ChemistryMainWindow() : toolBar_( new QWidget )
	                                       , toolBarLayout_( new QHBoxLayout( toolBar_ ) )
										   , toolBarDockWidget_( 0 )
{
	toolBarLayout_->setMargin( 0 );
    toolBarLayout_->setSpacing( 0 );
}


void
ChemistryMainWindow::OnInitialUpdate()
{
	setSimpleDockWidgetArrangement();
}

void
ChemistryMainWindow::activateLayout()
{
}

QWidget *
ChemistryMainWindow::createContents( Core::IMode * mode )
{
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;
	editorHolderLayout->setMargin( 0 );
	editorHolderLayout->setSpacing( 0 );
	    
    QWidget * editorAndFindWidget = new QWidget;
	editorAndFindWidget->setLayout( editorHolderLayout );
	editorHolderLayout->addWidget( new Core::EditorManagerPlaceHolder( mode ) );
	editorHolderLayout->addWidget( new Core::FindToolBarPlaceHolder( editorAndFindWidget ) );

	Core::MiniSplitter * documentAndRightPane = new Core::MiniSplitter;
    documentAndRightPane->addWidget( editorAndFindWidget );
    documentAndRightPane->addWidget( new Core::RightPanePlaceHolder( mode ) );
	documentAndRightPane->setStretchFactor( 0, 1 );
    documentAndRightPane->setStretchFactor( 1, 0 );

    Utils::StyledBar * chemToolBar = new Utils::StyledBar;
    chemToolBar->setProperty( "topBorder", true );
    QHBoxLayout * chemToolBarLayout = new QHBoxLayout( chemToolBar );
    chemToolBarLayout->setMargin( 0 );
    chemToolBarLayout->setSpacing( 0 );
	chemToolBarLayout->addWidget( toolBar_ );
	chemToolBarLayout->addWidget( new Utils::StyledSeparator );
	//
	QDockWidget * dock = new QDockWidget( "Chemistry Toolbar" );
	dock->setObjectName( QLatin1String( "Chemistry Toolbar" ) );
    dock->setWidget( chemToolBar );
	dock->setFeatures( QDockWidget::NoDockWidgetFeatures );
	dock->setAllowedAreas( Qt::BottomDockWidgetArea );
	dock->setTitleBarWidget( new QWidget( dock ) );
	dock->setProperty( "manaaged_dockwidget", QLatin1String( "true" ) );
	addDockWidget( Qt::BottomDockWidgetArea, dock );
	setToolBarDockWidget( dock );

	QWidget * centralWidget = new QWidget;
	setCentralWidget( centralWidget );

	QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
	centralWidget->setLayout( centralLayout );
	centralLayout->setMargin( 0 );
	centralLayout->setSpacing( 0 );
    centralLayout->addWidget( documentAndRightPane );
	centralLayout->setStretch( 0, 1 );
	centralLayout->setStretch( 1, 0 );

	// Right-side window with editor, output etc.
	Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter;
	mainWindowSplitter->addWidget( this );
    QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
    outputPane->setObjectName( QLatin1String( "ChemistryOutputPanePlaceHolder" ) );
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

	return splitter;
}

void
ChemistryMainWindow::setSimpleDockWidgetArrangement()
{
	Chemistry::Internal::setTrackingEnabled x( *this );

	QList< QDockWidget *> dockWidgets = this->dockWidgets();
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
		dockWidget->setFloating( false );
		removeDockWidget( dockWidget );
	}
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
		int area = Qt::BottomDockWidgetArea;
		QVariant p = dockWidget->property( "Chemistry.Default.Area" );
		if ( p.isValid() ) 
			area = Qt::DockWidgetArea( p.toInt() );
		addDockWidget( Qt::DockWidgetArea( area ), dockWidget );
		dockWidget->show();
	}

	QDockWidget * toolBarDock = toolBarDockWidget();
	toolBarDock->show();
	update();
}

void
ChemistryMainWindow::setToolBarDockWidget( QDockWidget * dock )
{
	toolBarDockWidget_ = dock;
}