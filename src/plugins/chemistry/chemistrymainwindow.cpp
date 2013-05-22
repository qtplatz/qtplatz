/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "chemistrymainwindow.hpp"
#include "sdfileview.hpp"
#include "sdfilemodel.hpp"
#include "massdefectform.hpp"
#include <adchem/chopper.hpp>
#include <adchem/mol.hpp>

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
#if QT_VERSION >= 0x050000
# include <QtWidgets/QVBoxLayout>
# include <QtWidgets/QHBoxLayout>
# include <QtWidgets/QToolButton>
# include <QtWidgets/QTextEdit>
# include <QtWidgets/qlabel.h>
#else
# include <QtGui/QVBoxLayout>
# include <QtGui/QHBoxLayout>
# include <QtGui/QToolButton>
# include <QtGui/QTextEdit>
# include <QtGui/qlabel.h>
#endif

#include <QtGui/qicon.h>
#include <qdebug.h>

#if defined _MSC_VER
#  pragma warning(disable:4100)
#endif
#include <openbabel/mol.h>
#include <openbabel/fingerprint.h>
#if defined _MSC_VER
#  pragma warning(default:4100)
#endif

#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <algorithm>

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
	                                       , toolBarLayout_( 0 )
										   , toolBarDockWidget_( 0 )
	                                       , actionSearch_( 0 )
{
	instance_ = this;
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
  
    size_t npos = 0;
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
		addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
		dockWidget->show();
        if ( npos++ > 1 )
			tabifyDockWidget( dockWidgets[1], dockWidget );
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
ChemistryMainWindow::createDockWidget( QWidget * widget, const QString& title )
{
	QDockWidget * dockWidget = addDockForWidget( widget );
	dockWidget->setObjectName( widget->objectName() );
    if ( title.isEmpty() )
		dockWidget->setWindowTitle( widget->objectName() );
	else
		dockWidget->setWindowTitle( title );

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

    QWidget * fragments = new SDFileView;
	fragments->setObjectName( "fragments" );
    createDockWidget( fragments, "Fragments" );
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
		std::vector< adchem::Mol > details;
		const std::vector< adchem::Mol >& data = model->data();
        if ( raw >= 0 && data.size() > unsigned( raw ) ) {
			// const adchem::Mol& target = data[ raw ];
			double m = data[ raw ].getExactMass();
            BOOST_FOREACH( const adchem::Mol& mol, data ) {
				double mz = mol.getExactMass();
				if ( m - 0.01 < mz && mz < m + 0.01 )
					details.push_back( mol );
			}
		}
		view->setData( details );
	}
}

void
ChemistryMainWindow::handleViewFragments( int raw, const SDFileModel * model )
{
    SDFileView * view = 0;
	foreach( QDockWidget * dock, dockWidgets() ) {
		if ( dock->objectName() == "fragments" ) {
			view = dynamic_cast< SDFileView *>( dock->widget() );
			break;
		}
	}

	if ( view && ( raw >= 0 && model->data().size() > unsigned( raw ) ) ) {
		using OpenBabel::OBMol;
		using OpenBabel::OBBond;
		using adchem::Mol;

		Mol mol = model->data()[ raw ];
		std::vector< Mol > fragments;
    
        std::vector< int > indecies = adchem::Chopper::chop( mol );
		while ( ! indecies.empty() ) {
			BOOST_FOREACH( int index, indecies ) {
                OBMol dup( mol );
				std::pair< OBMol, OBMol > sub = adchem::Chopper::split( dup, index );
				std::ostringstream o;
				o << std::fixed << mol.getExactMass() - sub.first.GetExactMass();
				adchem::Mol::SetAttribute( sub.first, "loss(m/z)", o.str() );
				adchem::Mol::SetAttribute( sub.first, "loss", sub.second.GetFormula() );
				fragments.push_back( sub.first );
			}
			mol = adchem::Chopper::split( mol, indecies[ 0 ] ).first;
			indecies = adchem::Chopper::chop( mol );
		}

		using adchem::Mol;

		std::sort( fragments.begin(), fragments.end()
			, boost::bind( &Mol::getExactMass, _2, true ) < boost::bind( &Mol::getExactMass, _1, true ) );

		fragments.erase( 
			std::unique( fragments.begin(), fragments.end()
			, boost::bind( &Mol::GetFormula, _1) == boost::bind( &Mol::GetFormula, _2 ) )
			, fragments.end() );
		view->setData( fragments );
	}
}


ChemistryMainWindow *
ChemistryMainWindow::instance()
{
	return instance_;
}
