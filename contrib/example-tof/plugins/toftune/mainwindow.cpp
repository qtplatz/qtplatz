/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include "analyzerwidget.hpp"
#include "acquisitionwidget.hpp"
#include "bezelwidget.hpp"
#include "datamediator.hpp"
#include "toftuneplugin.hpp"
#include "tofsignalmonitorview.hpp"
#include "ieditorfactory.hpp"
#include "ionsourcewidget.hpp"
#include "isequenceimpl.hpp"
#include "sideframe.hpp"

#include <adinterface/brokerC.h>
#include <tofinterface/methodC.h>

#include <adcontrols/massspectrum.hpp>

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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTextEdit>
#include <QLabel>
#include <QIcon>
#include <qdebug.h>

using namespace toftune;

namespace toftune {

    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
	public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };
}


MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , toolBar_( 0 )
                                        , toolBarLayout_( 0 )
                                        , toolBarDockWidget_( 0 )
                                        , actionConnect_ ( 0 )
                                        , monitorView_( 0 )
                                        , method_( new TOF::ControlMethod )
{
}

MainWindow::~MainWindow()
{
}

void
MainWindow::OnInitialUpdate()
{
    setSimpleDockWidgetArrangement();
}

void
MainWindow::activateLayout()
{
}

void
MainWindow::createActions()
{
    actionConnect_ = new QAction( QIcon( ":/chemistry/images/search.png" ), tr( "Connect" ), this );
    connect( actionConnect_, SIGNAL( triggered() ), this, SLOT( actionConnect ) );
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setDocumentMode( true );
    setDockNestingEnabled( true );

#if 0
    do {
        QBoxLayout * editorHolderLayout = new QVBoxLayout;
        editorHolderLayout->setMargin( 0 );
        editorHolderLayout->setSpacing( 0 );
	    
        QWidget * editorAndFindWidget = new QWidget;
        editorAndFindWidget->setLayout( editorHolderLayout );
        editorHolderLayout->addWidget( new tofSignalMonitorView /* new Core::EditorManagerPlaceHolder( mode ) */ );
        // editorHolderLayout->addWidget( new QTextEdit /* Core::FindToolBarPlaceHolder( editorAndFindWidget ) */ );
    } while( 0 );
#endif

    Core::MiniSplitter * documentAndRightPane = new Core::MiniSplitter;

    documentAndRightPane->addWidget( monitorView_ = tofSignalMonitorView::Create() /* editorAndFindWidget */ );
    documentAndRightPane->addWidget( new Core::RightPanePlaceHolder( mode ) );
    documentAndRightPane->setStretchFactor( 0, 1 );
    documentAndRightPane->setStretchFactor( 1, 0 );

    Utils::StyledBar * toolBar = new Utils::StyledBar;
    toolBar->setProperty( "topBorder", true );
    QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
    toolBarLayout->setMargin( 0 );
    toolBarLayout->setSpacing( 0 );
    // toolBarLayout->addWidget( toolBar_ );
    toolBarLayout->addWidget( toolButton( actionConnect_ ) );
    toolBarLayout->addWidget( new QLabel( tr("Start acquisition") ) );
    //toolBarLayout->addWidget( new QLabel( tr("Chemistry") ) );
    //toolBarLayout->addWidget( new QLabel( tr("Physics") ) );
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
    outputPane->setObjectName( QLatin1String( "TofOutputPanePlaceHolder" ) );
    mainWindowSplitter->addWidget( this );
    mainWindowSplitter->addWidget( outputPane );
    mainWindowSplitter->setStretchFactor( 0, 10 );
    mainWindowSplitter->setStretchFactor( 1, 0 );
    mainWindowSplitter->setOrientation( Qt::Vertical );

    // Navigation and right-side window
    createDockWidgets();
    
    //return splitter;
    return mainWindowSplitter;
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
    toftune::setTrackingEnabled x( *this );

    QList< QDockWidget *> dockWidgets = this->dockWidgets();
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        dockWidget->setFloating( false );
        removeDockWidget( dockWidget );
    }
  
    size_t nsize = dockWidgets.size();
    size_t npos = 0;
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->show();
        if ( npos++ >= 1 && npos < nsize )
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
MainWindow::createDockWidgets()
{
    QWidget * ionSource = new IonSourceWidget;
    createDockWidget( ionSource, "Ion Source" );
    connect( ionSource, SIGNAL( dataChanged( const dataMediator* ) ), this, SLOT( onDataChanged( const dataMediator* ) ) );

    QWidget * analyzer = new AnalyzerWidget;
    createDockWidget( analyzer, "Analyzer" );
    connect( analyzer, SIGNAL( dataChanged( const dataMediator* ) ), this, SLOT( onDataChanged( const dataMediator* ) ) );

    QWidget * acquisition = new AcquisitionWidget;
    createDockWidget( acquisition, "Acquisition Method" );
    connect( acquisition, SIGNAL( dataChanged( const dataMediator* ) ), this, SLOT( onDataChanged( const dataMediator* ) ) );

    QWidget * bezel = new BezelWidget;
    bezel->setObjectName( "Bezel" );
    createDockWidget( bezel );
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

bool
MainWindow::editor_factories( iSequenceImpl& impl )
{
    impl << iEditorFactoryPtr( new iEditorFactoryT< IonSourceWidget >( *this, "Ion Source" ) )
         << iEditorFactoryPtr( new iEditorFactoryT< AnalyzerWidget >( *this, "Analyzer" ) )
         << iEditorFactoryPtr( new iEditorFactoryT< AcquisitionWidget >( *this, "Acquisition Method" ) )
        ;
    return true;
}

void
MainWindow::setData( const adcontrols::MassSpectrum& ms )
{
    if ( monitorView_ )
        monitorView_->setData( ms );
}

void
MainWindow::setData( const adcontrols::Trace& trace, const std::wstring& traceId )
{
    if ( monitorView_ )
        monitorView_->setData( trace, traceId );
}

void
MainWindow::setMethod( const ControlMethod::Method& m )
{
    using toftune::Internal::tofTunePlugin;

    for ( size_t i = 0; i < m.lines.length(); ++i ) {
        std::wstring modelname = m.lines[ i ].modelname.in();
        if ( modelname == L"tof" ) {
            const TOF::ControlMethod * method;
            m.lines[ i ].data >>= method;
            if ( ! method_ )
                method_.reset( new TOF::ControlMethod );
            *method_ = *method;

            QList< QDockWidget *> widgets = dockWidgets();
            foreach ( QDockWidget * widget, widgets ) {
                QObjectList list = widget->children();
                foreach( QObject * obj, list ) {
                    dataMediator * mediator = dynamic_cast< dataMediator * >( obj );
                    if ( mediator ) {
                        mediator->setMethod( *method_ );
                    }
                }
            }
        }
    }
}

void
MainWindow::onDataChanged( const dataMediator* mediator )
{
    mediator->getMethod( *method_ );
    std::string hint = mediator->hint();
    Internal::tofTunePlugin::instance()->setMethod( *method_, hint );
}

void
MainWindow::actionConnect()
{
}
