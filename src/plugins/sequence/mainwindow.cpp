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

#include "mainwindow.hpp"
#include "sequenceplugin.hpp"
#include <adportable/configuration.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <qtwrapper/qstring.hpp>
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
#include <QMenu>
#include <QResizeEvent>
#include <qstackedwidget.h>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QTextEdit>
#include <QtGui/qlabel.h>
#include <QtGui/qicon.h>
#include <qdebug.h>


namespace sequence {

    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
	public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };

}

using namespace sequence;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , toolBar_( 0 )
                                        , toolBarLayout_( 0 )
                                        , toolBarDockWidget_( 0 )
                                        , actionConnect_ ( 0 )
{
}

void
MainWindow::createDockWidgets( const std::wstring& path
                               , adportable::Configuration& acquire
                               , adportable::Configuration& dataproc )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::South );
    setDocumentMode( true );

    const adportable::Configuration * pTab
        = adportable::Configuration::find( dataproc, L"ProcessMethodEditors" );
    if ( pTab ) {
        using namespace adportable;
        using namespace adplugin;
        
        // std::wstring loadpath = qtwrapper::wstring( dir.path() );
        // tab pages
        for ( Configuration::vector_type::const_iterator it = pTab->begin(); it != pTab->end(); ++it ) {
            
			const std::wstring name = it->name();
            // const std::wstring& component = it->attribute( L"component" );
            
            if ( it->isPlugin() ) {
                QWidget * pWidget = manager::widget_factory( *it, path.c_str(), 0 );
                if ( pWidget )
                    createDockWidget( pWidget, qtwrapper::qstring( it->title() ) );
            }
        }
    }            
}


void
MainWindow::OnInitialUpdate()
{
    setSimpleDockWidgetArrangement();

	QList< QDockWidget *> widgets = dockWidgets();
	foreach ( QDockWidget * widget, widgets ) {
        QWidget * obj = widget->widget();
		adplugin::LifeCycleAccessor accessor( obj );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle )
			pLifeCycle->OnInitialUpdate();
    }
}

void
MainWindow::OnFinalClose()
{
    QList< QDockWidget *> widgets = dockWidgets();
    foreach ( QDockWidget * widget, widgets ) {
		QWidget * obj = widget->widget();
		adplugin::LifeCycleAccessor accessor( obj );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle )
			pLifeCycle->OnFinalClose();
    }
}

void
MainWindow::activateLayout()
{
}

void
MainWindow::createActions()
{
    actionConnect_ = new QAction( QIcon( ":/chemistry/images/search.png" ), tr( "Connect" ), this );
    connect( actionConnect_, SIGNAL( triggered() ), this, SLOT( actionConnect_ ) );
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;
	editorHolderLayout->setMargin( 0 );
	editorHolderLayout->setSpacing( 0 );
	    
    QWidget * editorAndFindWidget = new QWidget;
	editorAndFindWidget->setLayout( editorHolderLayout );
	// editorHolderLayout->addWidget( new Core::EditorManagerPlaceHolder( mode ) );
	editorHolderLayout->addWidget( new QTextEdit );
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
	// toolBarLayout->addWidget( toolButton( actionSearch_ ) );
	toolBarLayout->addWidget( new QLabel( tr("Alchemy") ) );
	toolBarLayout->addWidget( new QLabel( tr("Chemistry") ) );
	toolBarLayout->addWidget( new QLabel( tr("Physics") ) );
	//
	QDockWidget * dock = new QDockWidget( "Sequence Toolbar" );
	dock->setObjectName( QLatin1String( "Sequence Toolbar" ) );
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
    outputPane->setObjectName( QLatin1String( "SequenceOutputPanePlaceHolder" ) );
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
    splitter->setObjectName( QLatin1String( "SequenceModeWidget" ) );

	return splitter;
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
    sequence::setTrackingEnabled x( *this );

    QList< QDockWidget *> dockWidgets = this->dockWidgets();
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        dockWidget->setFloating( false );
        removeDockWidget( dockWidget );
    }
  
    size_t npos = 0;
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->show();
        if ( npos++ >= 1 )
            tabifyDockWidget( dockWidgets[0], dockWidget );
    }

    QDockWidget * toolBarDock = toolBarDockWidget();

    toolBarDock->show();
    update();
}

void
MainWindow::setToolBarDockWidget( QDockWidget * dock )
{
	toolBarDockWidget_ = dock;
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
    Core::ActionManager * mgr = Core::ICore::instance()->actionManager();
    return toolButton( mgr->command(id)->action() );
}

