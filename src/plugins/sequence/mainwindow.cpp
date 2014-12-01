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
#include "sequenceplugin.hpp"
#include "sequencewnd.hpp"
#include "sequenceeditor.hpp"
#include "constants.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adextension/isequence.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adportable/configuration.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adsequence/sequence.hpp>
#include <adsequence/schema.hpp>
#include <adwidgets/controlmethodwidget.hpp>
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
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>

#include <QDockWidget>
#include <QMenu>
#include <QResizeEvent>
#include <qstackedwidget.h>

# include <QtWidgets/QVBoxLayout>
# include <QtWidgets/QHBoxLayout>
# include <QtWidgets/QToolButton>
# include <QtWidgets/QTextEdit>
# include <QtWidgets/qlabel.h>
# include <QtWidgets/qlineedit.h>

#include <QtGui/qicon.h>
#include <qdebug.h>
#include <boost/any.hpp>

namespace sequence {

    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
	public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };

}

using namespace sequence;

MainWindow * MainWindow::instance_ = 0;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , actionConnect_ ( 0 )
                                        , ctrlMethodName_( 0 )
                                        , procMethodName_( 0 )
                                        , defaultControlMethod_( new adcontrols::ControlMethod() )
{
    instance_ = this;
	setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::South );
    setDocumentMode( true );
}

void
MainWindow::OnInitialUpdate()
{
	using adextension::iSequence;

    if ( auto editor = findChild< SequenceWnd * >() )
        editor->OnInitialUpdate( adsequence::schema() );

    QList< iSequence * > visitables = ExtensionSystem::PluginManager::instance()->getObjects< iSequence >();

    QWidget * timeEvent = new adwidgets::ControlMethodWidget;
    createDockWidget( timeEvent, "Control Method", "ControlMethodWidget" );

	for ( iSequence * v: visitables ) {

        for ( size_t i = 0; i < v->size(); ++i ) {
            adextension::iEditorFactory& factory = (*v)[ i ];
            
            QString objname = 
                ( factory.method_type()
                  == adextension::iEditorFactory::PROCESS_METHOD ) ? "ProcessMethodEditor" : "ControlMethodEditor";

            QWidget * widget = factory.createEditor( 0 );
			if ( widget ) {
				createDockWidget( widget, factory.title(), objname );
                if ( factory.method_type() == adextension::iEditorFactory::CONTROL_METHOD )
                    connect( widget, SIGNAL( onTriggerAdd( const adcontrols::controlmethod::MethodItem& ) )
                             , timeEvent, SLOT( handleAdd( const adcontrols::controlmethod::MethodItem& ) ) );
                
                adplugin::LifeCycleAccessor accessor( widget );
                adplugin::LifeCycle * pLifeCycle = accessor.get();
                if ( pLifeCycle ) {
                    pLifeCycle->OnInitialUpdate();
                    editors_.push_back( pLifeCycle );
                }
            }
        }
    }


    // load GUI defined default values for get configuration
    getControlMethod( *defaultControlMethod_ );

    setSimpleDockWidgetArrangement();
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
    if ( Core::ActionManager * am = Core::ActionManager::instance() ) {
        
        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( tr("Sample Sequence") );

        if ( auto p = new QAction( QIcon( ":/quan/images/fileopen.png" ), tr( "Open Sequence..." ), this ) ) {
            am->registerAction( p, Constants::FILE_OPEN, Core::Context( Core::Constants::C_GLOBAL ) );   // Tools->Sequence->Open
            connect( p, &QAction::triggered, this, &MainWindow::handleOpenSequence );
            menu->addAction( am->command( Constants::FILE_OPEN ) );
            am->registerAction( p, Core::Constants::OPEN, Core::Context( Constants::C_SEQUENCE_MODE ) );  // File->Open
       }

        //------------ method --------------
        // if ( auto p = new QAction( QIcon( ":/quan/images/fileopen.png" ), tr( "Open Method..." ), this ) ) {
        //     am->registerAction( p, Constants::QUAN_METHOD_OPEN, Core::Context( Constants::C_SEQUENCE_MODE ) );
        //     connect( p, &QAction::triggered, this, &MainWindow::handleOpenQuanMethod );
        //     menu->addAction( am->command( Constants::QUAN_METHOD_OPEN ) );
        // }

        // if ( auto p = new QAction( QIcon( ":/quan/images/filesave.png" ), tr( "Save Quan Method..." ), this ) ) {
        //     am->registerAction( p, Constants::QUAN_METHOD_SAVE, Core::Context( Constants::C_QUAN_MODE ) );
        //     connect( p, &QAction::triggered, this, &MainWindow::handleSaveQuanMethod );
        //     menu->addAction( am->command( Constants::QUAN_METHOD_SAVE ) );
        // }
        //------------ sequence --------------
        // if ( auto p = new QAction( QIcon( ":/quan/images/fileopen.png" ), tr( "Open Quan Sequence..." ), this ) ) {
        //     am->registerAction( p, Constants::QUAN_SEQUENCE_OPEN, Core::Context( Constants::C_QUAN_MODE ) );
        //     connect( p, &QAction::triggered, this, &MainWindow::handleOpenQuanSequence );
        //     menu->addAction( am->command( Constants::QUAN_SEQUENCE_OPEN ) );
        // }

        // if ( auto p = new QAction( QIcon( ":/quan/images/filesave.png" ), tr( "Save Quan Sequence..." ), this ) ) {
        //     am->registerAction( p, Constants::QUAN_SEQUENCE_SAVE, Core::Context( Constants::C_QUAN_MODE ) );
        //     connect( p, &QAction::triggered, this, &MainWindow::handleSaveQuanSequence );
        //     menu->addAction( am->command( Constants::QUAN_SEQUENCE_SAVE ) );
        // }

        // if ( auto p = new QAction( QIcon( ":/quan/images/run.png" ), tr("Run"), this ) ) {
        //     am->registerAction( p, Constants::QUAN_SEQUENCE_RUN, Core::Context( Constants::C_QUAN_MODE ) );
        //     connect( p, &QAction::triggered, this, &MainWindow::run );
        //     menu->addAction( am->command( Constants::QUAN_SEQUENCE_RUN ) );
        // }

        // if ( auto p = new QAction( QIcon(":/quan/images/stop.png"), tr("Stop"), this ) ) {
        //     am->registerAction( p, Constants::QUAN_SEQUENCE_STOP, Core::Context( Constants::C_QUAN_MODE ) );
        //     connect( p, &QAction::triggered, this, &MainWindow::stop );
        //     menu->addAction( am->command( Constants::QUAN_SEQUENCE_STOP ) );
        //     p->setEnabled( false );
        // }
        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    // setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::East );
    setDocumentMode( true );
    setDockNestingEnabled( true );
    
    if ( QBoxLayout * editorHolderLayout = new QVBoxLayout ) {

        editorHolderLayout->setMargin( 0 );
        editorHolderLayout->setSpacing( 0 );
	    
        if ( QWidget * editorWidget = new QWidget ) {

            editorWidget->setLayout( editorHolderLayout );
            
            editorHolderLayout->addWidget( new SequenceWnd( adsequence::schema() ) ); // toolbar [top]

            if ( Core::MiniSplitter * splitter1 = new Core::MiniSplitter ) {

                splitter1->addWidget( editorWidget );        // [Editor]
                splitter1->setStretchFactor( 0, 1 );
                splitter1->setStretchFactor( 1, 0 );

                //---------- central widget ------------
                if ( QWidget * centralWidget = new QWidget ) {
                    setCentralWidget( centralWidget );

                    QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
                    centralWidget->setLayout( centralLayout );
                    centralLayout->setMargin( 1 );
                    centralLayout->setSpacing( 1 );
                    centralLayout->addWidget( splitter1 ); // editor

                    // ------------------ tool bar
                    centralLayout->addWidget( createMidStyledBar() ); // toolbar [mid]

                    centralLayout->setStretch( 0, 1 );
                    centralLayout->setStretch( 1, 0 );

                }
            }
        }

        // Right-side window with editor, output etc.
        if ( Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter ) {

            QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
            outputPane->setObjectName( QLatin1String( "SequenceOutputPanePlaceHolder" ) );
            mainWindowSplitter->addWidget( this );
            mainWindowSplitter->addWidget( outputPane );
            mainWindowSplitter->setStretchFactor( 0, 10 );
            mainWindowSplitter->setStretchFactor( 1, 0 );
            mainWindowSplitter->setOrientation( Qt::Vertical );
        
            // Navigation and right-side window
            if ( Core::MiniSplitter * splitter = new Core::MiniSplitter ) { 

                splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) ); // navegate
                splitter->addWidget( mainWindowSplitter );                            // *this + ontput
                splitter->setStretchFactor( 0, 0 );
                splitter->setStretchFactor( 1, 1 );
                splitter->setObjectName( QLatin1String( "SequenceModeWidget" ) );

                return splitter;
            }
        }
    }
    return 0;
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
    sequence::setTrackingEnabled x( *this );

    for ( auto widget: dockWidgets() ) {
        widget->setFloating( false );
        removeDockWidget( widget );
    }

    QDockWidget * master = 0;
    for ( auto widget: findChildren<QDockWidget *>( "ControlMethodEditor" ) ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
	 	if ( master == 0 )
	 		master = widget;
	 	else
            tabifyDockWidget( master, widget );
    }
    // if ( auto widget = findChild< QDockWidget *>( "ControlMethodWidget" ) ) {
    //     addDockWidget( Qt::BottomDockWidgetArea, widget );
    //     widget->show();
    // }        
    
    //master = 0;
    for ( auto widget: findChildren<QDockWidget *>( "ProcessMethodEditor" ) ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
		if ( master == 0 )
			master = widget;
		else
            tabifyDockWidget( master, widget );
    }

    update();
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& objname )
{
    QDockWidget * dockWidget = addDockForWidget( widget );
    dockWidget->setObjectName( widget->objectName() );
    if ( title.isEmpty() )
        dockWidget->setWindowTitle( widget->objectName() );
    else
        dockWidget->setWindowTitle( title );

    if ( !objname.isEmpty() )
        dockWidget->setObjectName( objname );        

    addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

    return dockWidget;
}

bool
MainWindow::getControlMethod( adcontrols::ControlMethod& m ) const
{
    boost::any any( &m );  
    for ( adplugin::LifeCycle * editor: editors_ )
        editor->getContents( any ); // read values from UI
    return true;
}

bool
MainWindow::setControlMethod( const adcontrols::ControlMethod& m )
{
    boost::any any( m );  
    for ( adplugin::LifeCycle * editor: editors_ )
        editor->setContents( any );
    return true;
}

bool
MainWindow::getProcessMethod( adcontrols::ProcessMethod& m ) const
{
    boost::any any( &m );  
    for ( adplugin::LifeCycle * editor: editors_ )
        editor->getContents( any ); // read values from UI
    return true;
}

bool
MainWindow::setProcessMethod( const adcontrols::ProcessMethod& m )
{
    boost::any any( m );  
    for ( adplugin::LifeCycle * editor: editors_ )
        editor->setContents( any );
    return true;
}

void
MainWindow::setControlMethodName( const QString& name )
{
    ctrlMethodName_->setText( name );
}

void
MainWindow::setProcessMethodName( const QString& name )
{
    procMethodName_->setText( name );
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

MainWindow *
MainWindow::instance()
{
    return instance_;
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

Utils::StyledBar *
MainWindow::createTopStyledBar()
{
	Utils::StyledBar * toolBar = new Utils::StyledBar;
    return toolBar;
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
        
        toolBarLayout->addItem( new QSpacerItem( 40, 20 ) );
        toolBarLayout->addWidget( new QLabel( tr("Control Method: ") ) );
        toolBarLayout->addWidget( ctrlMethodName_ = new QLineEdit );
        ctrlMethodName_->setReadOnly( true );
        
        toolBarLayout->addItem( new QSpacerItem( 40, 20 ) );
        toolBarLayout->addWidget( new QLabel( tr("Process Method: ") ) );
        toolBarLayout->addWidget( procMethodName_ = new QLineEdit );
        procMethodName_->setReadOnly( true );
        
        toolBarLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ) );
    }
    return toolBar;
}

void
MainWindow::handleOpenSequence()
{
}
