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
#include "dataprocessor.hpp"
#include "dataprocplugin.hpp"
#include "dataprocessorfactory.hpp"

#include "msprocessingwnd.hpp"
#include "elementalcompwnd.hpp"
#include "mscalibrationwnd.hpp"
#include "chromatogramwnd.hpp"
#include "sessionmanager.hpp"

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/processmethod.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/manager.hpp>
#include <adportable/configuration.hpp>
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
#include <QMessageBox>
#include <QResizeEvent>
#include <qstackedwidget.h>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QTextEdit>
#include <QtGui/qlabel.h>
#include <QtGui/qicon.h>
#include <qdebug.h>

namespace dataproc {
    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
	public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };
}


using namespace dataproc;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow( QWidget *parent ) :  Utils::FancyMainWindow(parent)
                                          , toolBar_( 0 )
                                          , toolBarLayout_( 0 )
                                          , toolBarDockWidget_( 0 )
                                          , actionSearch_( 0 )
                                          , actionApply_( 0 )
                                          , currentFeature_( CentroidProcess )
{
}

void
MainWindow::activateLayout()
{
}

void
MainWindow::createActions()
{
    //actionConnect_ = new QAction( QIcon( ":/chemistry/images/search.png" ), tr( "Connect" ), this );
    //connect( actionConnect_, SIGNAL( triggered() ), this, SLOT( actionConnect_ ) );
}

QWidget *
MainWindow::createContents( Core::IMode * mode
                            , const adportable::Configuration& config, const std::wstring& apppath )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::South );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    Core::MiniSplitter * documentAndRightPane = new Core::MiniSplitter;

    // documentAndRightPane->addWidget( monitorView_ = HmqSignalMonitorView::Create() /* editorAndFindWidget */ );
    documentAndRightPane->addWidget( new QTextEdit );
    documentAndRightPane->addWidget( new Core::RightPanePlaceHolder( mode ) );
    documentAndRightPane->setStretchFactor( 0, 1 );
    documentAndRightPane->setStretchFactor( 1, 0 );

    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( am ) {
            // toolBarLayout->addWidget(toolButton(am->command(Constants::CONNECT)->action()));
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addWidget( new QLabel( tr("Sequence:") ) );
    }
    Utils::StyledBar * toolBar2 = new Utils::StyledBar;
    if ( toolBar2 ) {
        toolBar2->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar2 );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( am ) {
            QList<int> globalcontext;
            globalcontext << Core::Constants::C_GLOBAL_ID;
            
            actionApply_ = new QAction( QIcon( ":/dataproc/image/apply_small.png" ), tr("Apply" ), this );
            assert( connect( actionApply_, SIGNAL( triggered() ), this, SLOT( actionApply() ) ) );
            am->registerAction( actionApply_, "dataproc.connect", globalcontext );
            toolBarLayout->addWidget( toolButton( am->command( "dataproc.connect" )->action() ) );
            /**/
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            /**/
            QComboBox * features = new QComboBox;
            features->addItem( "Centroid" );
            features->addItem( "Isotope" );
            features->addItem( "Calibration" );
            features->addItem( "Find peaks" );
            toolBarLayout->addWidget( features );

            assert( connect( features, SIGNAL( currentIndexChanged(int) ), this, SLOT( handleFeatureSelected(int) ) ) );
            assert( connect( features, SIGNAL( activated(int) ), this, SLOT( handleFeatureActivated(int) ) ) );

            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        }
    }

    //---------- central widget ------------
    QWidget * centralWidget = new QWidget;
    setCentralWidget( centralWidget );

    std::vector< QWidget * > wnd;
    Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
    if ( splitter3 ) {
        QTabWidget * tab = new QTabWidget;
        splitter3->addWidget( tab );

        wnd.push_back( new MSProcessingWnd );
        tab->addTab( wnd.back(), QIcon(":/acquire/images/debugger_stepoverproc_small.png"), "MS Processing" );
        wnd.push_back( new ElementalCompWnd );
        tab->addTab( wnd.back(), QIcon(":/acquire/images/debugger_snapshot_small.png"), "Elemental Composition" );
        wnd.push_back( new MSCalibrationWnd( config, apppath ) );
        tab->addTab( wnd.back(), QIcon(":/acquire/images/debugger_continue_small.png"), "MS Calibration" );
        wnd.push_back( new ChromatogramWnd( apppath ) );
        tab->addTab( wnd.back(),  QIcon(":/acquire/images/watchpoint.png"), "Chromatogram" );
        if ( DataprocPlugin::instance()->dataprocessorFactory() )
            DataprocPlugin::instance()->dataprocessorFactory()->setEditor( tab );
    }

    for ( std::vector< QWidget *>::iterator it = wnd.begin(); it != wnd.end(); ++it ) {
        assert( connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) )
                         , *it, SLOT( handleSessionAdded( Dataprocessor* ) ) ) );
        assert( connect( SessionManager::instance(), SIGNAL( signalSelectionChanged( Dataprocessor*, portfolio::Folium& ) )
                         , *it, SLOT( handleSelectionChanged( Dataprocessor*, portfolio::Folium& ) ) ) );
        using adcontrols::ProcessMethod;
        assert( connect( DataprocPlugin::instance(), SIGNAL( onApplyMethod( const ProcessMethod& ) )
                         , *it, SLOT( onApplyMethod( const ProcessMethod& ) ) ) );
    }
    assert( connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) )
                     , this, SLOT( handleSessionAdded( Dataprocessor* ) ) ) );
    assert( connect( SessionManager::instance(), SIGNAL( signalSelectionChanged( Dataprocessor*, portfolio::Folium& ) )
                     , this, SLOT( handleSelectionChanged( Dataprocessor*, portfolio::Folium& ) ) ) );

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter3 );
    toolBarAddingLayout->addWidget( toolBar2 );
    
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
    outputPane->setObjectName( QLatin1String( "HmsqOutputPanePlaceHolder" ) );
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
    splitter->setObjectName( QLatin1String( "HmsqModeWidget" ) );

    createDockWidgets( config, apppath );

	return splitter;
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
    dataproc::setTrackingEnabled x( *this );

    QList< QDockWidget *> widgets = dockWidgets();

    foreach ( QDockWidget * widget, widgets ) {
        widget->setFloating( false );
        removeDockWidget( widget );
    }
  
    size_t npos = 0;
    foreach ( QDockWidget * widget, widgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
        if ( npos++ >= 2 )
            tabifyDockWidget( widgets[1], widget );
    }

    QDockWidget * toolBarDock = toolBarDockWidget();
    if ( toolBarDock )
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
MainWindow::createDockWidgets( const adportable::Configuration& config, const std::wstring& apppath )
{
    using adportable::Configuration;

    const Configuration * pTab = Configuration::find( config, L"ProcessMethodEditors" );
    if ( pTab ) {
            
        for ( Configuration::vector_type::const_iterator it = pTab->begin(); it != pTab->end(); ++it ) {
            
            const std::wstring name = it->name();
            
            if ( it->isPlugin() ) {
                QWidget * pWidget = adplugin::manager::widget_factory( *it, apppath.c_str(), 0 );
                if ( pWidget ) {
                    // query process method
                    connect( this, SIGNAL( signalGetProcessMethod( adcontrols::ProcessMethod& ) )
                             , pWidget, SLOT( getContents( adcontrols::ProcessMethod& ) ), Qt::DirectConnection );
                    createDockWidget( pWidget, qtwrapper::qstring( it->title() ) );
                } else {
                    QMessageBox::critical(0, QLatin1String("dataprocmanager"), qtwrapper::qstring::copy(it->name()) );
                }
            }
        }
    }       

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

/*
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
MainWindow::setRF( const HMQSignal::hmqDATA& data )
{
    if ( monitorView_ )
        monitorView_->setRF( data );
}

void
MainWindow::setMethod( const ControlMethod::Method& m )
{
}

void
MainWindow::onDataChanged( const dataMediator* mediator )
{
}
*/

void
MainWindow::handleSessionAdded( dataproc::Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    emit signalUpdateFile( &file );
}

void
MainWindow::handleSelectionChanged( dataproc::Dataprocessor *, portfolio::Folium& )
{
}


void
MainWindow::handleApplyMethod()
{
}

void
MainWindow::getProcessMethod( adcontrols::ProcessMethod& pm )
{
    emit signalGetProcessMethod( pm );
}

void
MainWindow::OnInitialUpdate()
{
    using adcontrols::ProcessMethod;

    QList< QDockWidget *> widgets = dockWidgets();
  
    foreach ( QDockWidget * widget, widgets ) {
        QWidget * obj = widget->widget();
		adplugin::LifeCycleAccessor accessor( obj );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle ) {
			pLifeCycle->OnInitialUpdate();
			connect( obj, SIGNAL( onMethodApply( ProcessMethod& ) ), this, SLOT( onMethodApply( ProcessMethod& ) ), Qt::DirectConnection );
		}
    }
    setSimpleDockWidgetArrangement();
}

void
MainWindow::OnFinalClose()
{
    using adcontrols::ProcessMethod;

    QList< QDockWidget *> widgets = dockWidgets();
  
    foreach ( QDockWidget * widget, widgets ) {
        QObjectList list = widget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycleAccessor accessor( obj );
            adplugin::LifeCycle * pLifeCycle = accessor.get();
            if ( pLifeCycle ) {
				disconnect( obj, SIGNAL( onMethodApply( ProcessMethod& ) ), this, SLOT( onMethodApply( ProcessMethod& ) ) );
                pLifeCycle->OnFinalClose();
            }
        }
    }
}

void
MainWindow::onMethodApply( adcontrols::ProcessMethod& pm )
{
    DataprocPlugin::instance()->applyMethod( pm );
}

void
MainWindow::actionApply()
{
    adcontrols::ProcessMethod m;
    getProcessMethod( m );
    size_t n = m.size();
    if ( n > 0 ) {
        Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
        if ( processor ) {
            if ( currentFeature_ == CalibrationProcess )
                processor->applyCalibration( m );
			else
                processor->applyProcess( m, currentFeature_ );
        }
    }
}

void
MainWindow::handleFeatureSelected( int value )
{
    currentFeature_ = static_cast< ProcessType >( value );
}

void
MainWindow::handleFeatureActivated( int value )
{
    currentFeature_ = static_cast< ProcessType >( value );
}
