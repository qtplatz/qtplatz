/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include "dataprocessor.hpp"
#include "dataprocplugin.hpp"
#include "dataprocessorfactory.hpp"

#include "msprocessingwnd.hpp"
#include "elementalcompwnd.hpp"
#include "mscalibrationwnd.hpp"
#include "mscalibspectrawnd.hpp"
#include "chromatogramwnd.hpp"
#include "sessionmanager.hpp"

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/processmethod.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/widget_factory.hpp>
#include <adportable/configuration.hpp>
#include <adportable/utf.hpp>
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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTabWidget>
#include <QTextEdit>
#include <QLabel>
#include <QtGui/QIcon>
#include <QLineEdit>
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
                                          , actionSelMSProcess_( 0 )
                                          , actionSelElementalComp_( 0 )
                                          , actionSelMSCalibration_( 0 )
                                          , actionSelMSCalibSpectra_( 0 )
                                          , actionSelChromatogram_( 0 )
                                          , stack_( 0 )
                                          , currentFeature_( CentroidProcess )
{
}

MainWindow *
MainWindow::instance()
{
    return DataprocPlugin::instance()->mainWindow();
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

Utils::StyledBar *
MainWindow::createStyledBarTop()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( am ) {
            QList<int> globalcontext;
            globalcontext << Core::Constants::C_GLOBAL_ID;

            actionSelMSProcess_ = new QAction( "MS Process", this );
            connect( actionSelMSProcess_, SIGNAL( triggered() ), this, SLOT( actionSelMSProcess() ) );
            am->registerAction( actionSelMSProcess_, "dataproc.selMSProcess", globalcontext );
            toolBarLayout->addWidget( toolButton( actionSelMSProcess_ ) );

            actionSelElementalComp_ = new QAction( "Elemental Comp", this );
            connect( actionSelElementalComp_, SIGNAL( triggered() ), this, SLOT( actionSelElementalComp() ) );
            am->registerAction( actionSelMSProcess_, "dataproc.selElementalComp", globalcontext );
            toolBarLayout->addWidget( toolButton( actionSelElementalComp_ ) );

            actionSelMSCalibration_ = new QAction( "MS Calibration", this );
            connect( actionSelMSCalibration_, SIGNAL( triggered() ), this, SLOT( actionSelMSCalibration() ) );
            am->registerAction( actionSelMSProcess_, "dataproc.selMSCalibration", globalcontext );
            toolBarLayout->addWidget( toolButton( actionSelMSCalibration_ ) );

            actionSelMSCalibSpectra_ = new QAction( "MS Calib. Spectra", this );
            connect( actionSelMSCalibSpectra_, SIGNAL( triggered() ), this, SLOT( actionSelMSCalibSpectra() ) );
            am->registerAction( actionSelMSProcess_, "dataproc.selMSCalibSpectra", globalcontext );
            toolBarLayout->addWidget( toolButton( actionSelMSCalibSpectra_ ) );

            actionSelChromatogram_ = new QAction( "Chromatogram", this );
            connect( actionSelChromatogram_, SIGNAL( triggered() ), this, SLOT( actionSelChromatogram() ) );
            am->registerAction( actionSelChromatogram_, "dataproc.selChromatogram", globalcontext );
            toolBarLayout->addWidget( toolButton( actionSelChromatogram_ ) );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        toolBarLayout->addWidget( new QLabel( tr("Sequence:") ) );
        toolBarLayout->addWidget( new QLineEdit );
    }
    return toolBar;
}

Utils::StyledBar *
MainWindow::createStyledBarMiddle()
{
    Utils::StyledBar * toolBar2 = new Utils::StyledBar;

    if ( toolBar2 ) {
        toolBar2->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar2 );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( am ) {
            QList<int> context;
            context << Core::Constants::C_GLOBAL_ID;
            
            actionApply_ = new QAction( QIcon( ":/dataproc/image/apply_small.png" ), tr("Apply" ), this );
            bool res = connect( actionApply_, SIGNAL( triggered() ), this, SLOT( actionApply() ) );
            assert( res );
            am->registerAction( actionApply_, "dataproc.apply", context );
            toolBarLayout->addWidget( toolButton( am->command( "dataproc.apply" )->action() ) );

            toolBarLayout->addWidget( new Utils::StyledSeparator );

            QComboBox * features = new QComboBox;
            features->addItem( "Centroid" );
            features->addItem( "Isotope" );
            features->addItem( "Calibration" );
            features->addItem( "Find peaks" );
            toolBarLayout->addWidget( features );

            bool r = connect( features, SIGNAL( currentIndexChanged(int) ), this, SLOT( handleFeatureSelected(int) ) );
            assert( r );
            r = connect( features, SIGNAL( activated(int) ), this, SLOT( handleFeatureActivated(int) ) );
            assert( r );
            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        }
    }
    return toolBar2;
}

QWidget *
MainWindow::createContents( Core::IMode * mode
                            , const adportable::Configuration& config, const std::wstring& apppath )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::South );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    Utils::StyledBar * toolBar1 = createStyledBarTop();
    Utils::StyledBar * toolBar2 = createStyledBarMiddle();

    //---------- central widget ------------
    QWidget * centralWidget = new QWidget;
    setCentralWidget( centralWidget );

    std::vector< QWidget * > wnd;
    Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
    if ( splitter3 ) {
        stack_ = new QStackedWidget;
        splitter3->addWidget( stack_ );

        wnd.push_back( new MSProcessingWnd );
        stack_->addWidget( wnd.back() );

        wnd.push_back( new ElementalCompWnd );
        stack_->addWidget( wnd.back() );

        wnd.push_back( new MSCalibrationWnd( config, apppath ) );
        stack_->addWidget( wnd.back() );

        wnd.push_back( new MSCalibSpectraWnd( config, apppath ) );
        stack_->addWidget( wnd.back() );

        wnd.push_back( new ChromatogramWnd( apppath ) );
        stack_->addWidget( wnd.back() );
    }

    for ( std::vector< QWidget *>::iterator it = wnd.begin(); it != wnd.end(); ++it ) {
        bool r;
        r = connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) )
                     , *it, SLOT( handleSessionAdded( Dataprocessor* ) ) );
        assert( r );
        r = connect( SessionManager::instance(), SIGNAL( signalSelectionChanged( Dataprocessor*, portfolio::Folium& ) )
                     , *it, SLOT( handleSelectionChanged( Dataprocessor*, portfolio::Folium& ) ) );
        assert( r );
        r = connect( DataprocPlugin::instance(), SIGNAL( onApplyMethod( const adcontrols::ProcessMethod& ) )
                     , *it, SLOT( handleApplyMethod( const adcontrols::ProcessMethod& ) ) );
        assert( r );
    }
    bool res = connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) )
                        , this, SLOT( handleSessionAdded( Dataprocessor* ) ) );
    assert( res );
    res = connect( SessionManager::instance(), SIGNAL( signalSelectionChanged( Dataprocessor*, portfolio::Folium& ) )
                   , this, SLOT( handleSelectionChanged( Dataprocessor*, portfolio::Folium& ) ) );
    assert( res );

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( toolBar1 );  // top most toolbar
    toolBarAddingLayout->addWidget( splitter3 ); // Spectra|chrmatogram pane
    toolBarAddingLayout->addWidget( toolBar2 );  // middle toolbar

    // Right-side window with editor, output etc.
    Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter;
    QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
    outputPane->setObjectName( QLatin1String( "OutputPanePlaceHolder" ) );
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
    splitter->setObjectName( QLatin1String( "ModeWidget" ) );

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
                //QWidget * pWidget = adplugin::manager::widget_factory( *it, apppath.c_str(), 0 );
                std::string wiid = adportable::utf::to_utf8( it->_interface() );
                QWidget * pWidget = adplugin::widget_factory::create( wiid.c_str(), 0, 0 );
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
			bool res = connect( obj, SIGNAL( apply( adcontrols::ProcessMethod& ) ), this, SLOT( onMethodApply( adcontrols::ProcessMethod& ) ), Qt::DirectConnection );
			assert( res );
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
MainWindow::applyCalibration( const adcontrols::MSAssignedMasses& assigned )
{
    adcontrols::ProcessMethod m;
    getProcessMethod( m );
    if ( m.size() > 0 ) {
        Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
        if ( processor )
            processor->applyCalibration( m, assigned );
    }
}

void
MainWindow::applyCalibration( const adcontrols::MSAssignedMasses& assigned, portfolio::Folium& folium )
{
    adcontrols::ProcessMethod m;
    getProcessMethod( m );
    if ( m.size() > 0 ) {
        Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
        if ( processor )
            processor->applyCalibration( m, assigned, folium );
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

void
MainWindow::actionSelMSProcess()
{
    stack_->setCurrentIndex( 0 );
}

void
MainWindow::actionSelElementalComp()
{
    stack_->setCurrentIndex( 1 );
}

void
MainWindow::actionSelMSCalibration()
{
    stack_->setCurrentIndex( 2 );
}

void
MainWindow::actionSelMSCalibSpectra()
{
    stack_->setCurrentIndex( 3 );
}

void
MainWindow::actionSelChromatogram()
{
    stack_->setCurrentIndex( 4 );
}

