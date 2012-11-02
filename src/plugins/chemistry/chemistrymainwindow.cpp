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
#include "sdfileview.hpp"
#include "sdfilemodel.hpp"
#include "massdefectform.hpp"
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

#include <QDockWidget>
#include <qmenu.h>
#include <QResizeEvent>
#include <qstackedwidget.h>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxlayout>
#include <QtGui/QToolButton>
#include <QtGui/QTextEdit>
#include <QtGui/qlabel.h>
#include <QtGui/qicon.h>

#ifdef _MSC_VER
# pragma warning( disable: 4100 )
#endif
//#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#ifdef _MSC_VER
# pragma warning( default: 4100 )
#endif

#include <boost/foreach.hpp>

namespace chemistry {

    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
	public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };

}

using namespace chemistry;

ChemistryMainWindow * ChemistryMainWindow::instance_ = 0;

ChemistryMainWindow::~ChemistryMainWindow()
{
}

ChemistryMainWindow::ChemistryMainWindow() : toolBar_( 0 )
	                                       , actionSearch_( 0 )
	                                       , toolBarLayout_( 0 )
										   , toolBarDockWidget_( 0 )
{
	instance_ = this;
}


void
ChemistryMainWindow::OnInitialUpdate()
{
	//setSimpleDockWidgetArrangement();
}

void
ChemistryMainWindow::activateLayout()
{
}

void
ChemistryMainWindow::createActions()
{
	actionSearch_ = new QAction( QIcon( ":/chemistry/images/search.png" ), tr("Search"), this );
    connect( actionSearch_, SIGNAL( triggered() ), this, SLOT( actionSearch_ ) );
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

    Utils::StyledBar * toolBar = new Utils::StyledBar;
	toolBar->setProperty( "topBorder", true );
	QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
	toolBarLayout->setMargin( 0 );
	toolBarLayout->setSpacing( 0 );
	// toolBarLayout->addWidget( toolBar_ );
	toolBarLayout->addWidget( toolButton( actionSearch_ ) );
	toolBarLayout->addWidget( new QLabel( tr("Alchemy") ) );
	toolBarLayout->addWidget( new QLabel( tr("Chemistry") ) );
	toolBarLayout->addWidget( new QLabel( tr("Physics") ) );
	//
	QDockWidget * dock = new QDockWidget( "Chemistry Toolbar" );
	dock->setObjectName( QLatin1String( "Chemistry Toolbar" ) );
	// dock->setWidget( toolBar );
	dock->setFeatures( QDockWidget::NoDockWidgetFeatures );
	dock->setAllowedAreas( Qt::BottomDockWidgetArea );
	dock->setTitleBarWidget( new QWidget( dock ) );
	dock->setProperty( "manaaged_dockwidget", QLatin1String( "true" ) );
	addDockWidget( Qt::BottomDockWidgetArea, dock );
	setToolBarDockWidget( dock );

	//---------- centraol widget ------------
	QWidget * centralWidget = new QWidget;
	setCentralWidget( centralWidget );

	QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
	centralWidget->setLayout( centralLayout );
	centralLayout->setMargin( 0 );
	centralLayout->setSpacing( 0 );
    centralLayout->addWidget( documentAndRightPane );
	centralLayout->setStretch( 0, 1 );
	centralLayout->setStretch( 1, 0 );

	centralLayout->addWidget( toolBar );

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

	return splitter;
}

void
ChemistryMainWindow::setSimpleDockWidgetArrangement()
{
	chemistry::setTrackingEnabled x( *this );

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

QDockWidget *
ChemistryMainWindow::createDockWidget( QWidget * widget )
{
	QDockWidget * dockWidget = addDockForWidget( widget );
	dockWidget->setObjectName( widget->objectName() );
	addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
#if 0
	QAction * toggleViewAction = dockWidget->toggleViewAction();
	QList<int> globalContext;
	globalContext << Core::Constants::C_GLOBAL_ID;
	// Core::Command * cmd = Core::ActionManager::registerAction( toggleViewAction, "Chemistry." + widget->objectName(), globalContext );
#endif

	return dockWidget;
}

void
ChemistryMainWindow::createDockWidgets()
{
	QWidget * widget = new SDFileView;
	widget->setObjectName( "details" );
	createDockWidget( widget );

    MassDefectForm * form = new MassDefectForm;
	form->setObjectName( "massdefect" );
    form->OnInitialUpdate();
	createDockWidget( form );
}

void
ChemistryMainWindow::createToolbar()
{
	QWidget * toolbarContainer = new QWidget;
	QHBoxLayout * hbox = new QHBoxLayout( toolbarContainer );
    hbox->setMargin( 0 );
    hbox->setSpacing( 0 );
    hbox->addWidget( toolButton( "STOP" ) ); // should create action in 'plugin' with icon
}

// static
QToolButton * 
ChemistryMainWindow::toolButton( QAction * action )
{
	QToolButton * button = new QToolButton;
	if ( button )
		button->setDefaultAction( action );
	return button;
}

// static
QToolButton * 
ChemistryMainWindow::toolButton( const char * id )
{
	Core::ActionManager * mgr = Core::ICore::instance()->actionManager();
	return toolButton( mgr->command(id)->action() );
}

// slot
void
ChemistryMainWindow::actionSearch()
{
}

void
ChemistryMainWindow::handleViewDetails( int raw, const SDFileModel * model )
{
    SDFileView * view = 0;
	foreach( QDockWidget * dock, dockWidgets() ) {
		if ( dock->objectName() == "details" ) {
			view = dynamic_cast< SDFileView *>( dock->widget() );
			break;
		}
	}
    if ( view ) {
		std::vector< OpenBabel::OBMol > details;
		const std::vector< OpenBabel::OBMol >& data = model->data();
        if ( raw >= 0 && data.size() > raw ) {
			const OpenBabel::OBMol& target = data[ raw ];
			double m = const_cast< OpenBabel::OBMol& >( target ).GetExactMass();
            BOOST_FOREACH( const OpenBabel::OBMol& mol, data ) {
				double mz = const_cast< OpenBabel::OBMol& >(mol).GetExactMass();
				if ( m - 0.1 < mz && mz < m + 0.1 )
					details.push_back( mol );
			}
		}
		view->setData( details );
	}
}

ChemistryMainWindow *
ChemistryMainWindow::instance()
{
	return instance_;
}