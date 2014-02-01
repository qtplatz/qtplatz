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
#include "chromatogramwnd.hpp"
#include "dataprocessor.hpp"
#include "dataprocplugin.hpp"
#include "dataprocessorfactory.hpp"
#include "elementalcompwnd.hpp"
#include "filepropertywidget.hpp"
#include "msprocessingwnd.hpp"
#include "mscalibrationwnd.hpp"
#include "mscalibspectrawnd.hpp"
#include "mspeakswnd.hpp"
#include "spectrogramwnd.hpp"
#include "mspropertyform.hpp"
#include "sessionmanager.hpp"

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/processmethod.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/widget_factory.hpp>
#include <adportable/configuration.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <qtwrapper/qstring.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <boost/any.hpp>

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

using namespace dataproc;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow( QWidget *parent ) : Utils::FancyMainWindow(parent)
                                          , toolBar_( 0 )
                                          , toolBarLayout_( 0 )
                                          , toolBarDockWidget_( 0 )
                                          , axisChoice_( 0 )
                                          , actionSearch_( 0 )
                                          , actionApply_( 0 )
                                          , actionSelMSProcess_( 0 )
                                          , actionSelElementalComp_( 0 )
                                          , actionSelMSCalibration_( 0 )
                                          , actionSelMSCalibSpectra_( 0 )
                                          , actionSelChromatogram_( 0 )
                                          , actionSelMSPeaks_( 0 )
                                          , actionSelSpectrogram_( 0 )
                                          , stack_( 0 )
                                          , processMethodNameEdit_( new QLineEdit ) 
                                          , currentFeature_( CentroidProcess )
                                          , msPeaksWnd_( 0 )
    , wndMSProcessing_( 0 ) 
{
    std::fill( actions_.begin(), actions_.end(), static_cast<QAction *>(0) );
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
MainWindow::install_actions()
{
    const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;

    if ( QAction * action = new QAction( tr("Data processing"), this ) ) {

		actions_[ idActCreateSpectrogram ] = new QAction( tr("Create spectrogram"), this );
        connect( actions_[ idActCreateSpectrogram ], SIGNAL( triggered() ), this, SLOT( actCreateSpectrogram() ) );

        if ( Core::ActionManager *am = Core::ICore::instance()->actionManager() ) {

            Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID );
            menu->menu()->setTitle( "Dataproc" );

            if ( Core::Command * cmd = am->registerAction( actions_[ idActCreateSpectrogram ], Constants::CREATE_SPECTROGRAM, gc ) )
                menu->addAction( cmd );


			// add 'dataproc' menu item to Tools
			am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
		}
    }
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

            actionSelMSPeaks_ = new QAction( "TOF Plots", this );
            connect( actionSelMSPeaks_, SIGNAL( triggered() ), this, SLOT( actionSelMSPeaks() ) );
            am->registerAction( actionSelMSPeaks_, "dataproc.selTOFPlots", globalcontext );
            toolBarLayout->addWidget( toolButton( actionSelMSPeaks_ ) );

            actionSelSpectrogram_ = new QAction( "Spectrogram", this );
            connect( actionSelSpectrogram_, SIGNAL( triggered() ), this, SLOT( actionSelSpectrogram() ) );
            am->registerAction( actionSelSpectrogram_, "dataproc.selSpectrogram", globalcontext );
            toolBarLayout->addWidget( toolButton( actionSelSpectrogram_ ) );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        
        axisChoice_ = new QComboBox;
        axisChoice_->addItem( "m/z" );
        axisChoice_->addItem( "time" );
        toolBarLayout->addWidget( new QLabel( tr("Axis:") ) );
        toolBarLayout->addWidget( axisChoice_ );
        
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
            // print, method file open & save buttons
            toolBarLayout->addWidget(toolButton(am->command(Constants::PRINT_CURRENT_VIEW)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::METHOD_OPEN)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::METHOD_SAVE)->action()));
			toolBarLayout->addWidget(toolButton(am->command(Constants::CALIBFILE_APPLY)->action()));
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //----------
            QList<int> context;
            context << Core::Constants::C_GLOBAL_ID;
            
            actionApply_ = new QAction( QIcon( Constants::ICON_METHOD_APPLY ), tr("Apply" ), this );
            bool res = connect( actionApply_, SIGNAL( triggered() ), this, SLOT( actionApply() ) );
            assert( res );
            am->registerAction( actionApply_, Constants::METHOD_APPLY, context );
            toolBarLayout->addWidget( toolButton( am->command( Constants::METHOD_APPLY )->action() ) );

            QComboBox * features = new QComboBox;
            features->addItem( "Centroid" );
            features->addItem( "Isotope" );
            features->addItem( "Calibration" );
            features->addItem( "Find peaks" );
            toolBarLayout->addWidget( features );

            connect( features, SIGNAL( currentIndexChanged(int) ), this, SLOT( handleFeatureSelected(int) ) );
            connect( features, SIGNAL( activated(int) ), this, SLOT( handleFeatureActivated(int) ) );

            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("Process Method:" ) ) );
            toolBarLayout->addWidget( processMethodNameEdit_.get() );

            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        }
    }
    return toolBar2;
}

QWidget *
MainWindow::createContents( Core::IMode * mode
                            , const adportable::Configuration& config, const std::wstring& apppath )
{
	(void)config;

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

        wnd.push_back( wndMSProcessing_ = new MSProcessingWnd );
        wnd.back()->setWindowTitle( "MS Process" );
        stack_->addWidget( wnd.back() );

        wnd.push_back( new ElementalCompWnd );
        wnd.back()->setWindowTitle( "Elemental Comp." );
        stack_->addWidget( wnd.back() );

        wnd.push_back( new MSCalibrationWnd );
        wnd.back()->setWindowTitle( "MS Calibration" );
        stack_->addWidget( wnd.back() );

        wnd.push_back( new MSCalibSpectraWnd );
        wnd.back()->setWindowTitle( "MS Calibration(2)" );
        stack_->addWidget( wnd.back() );

        wnd.push_back( new ChromatogramWnd( apppath ) );
        wnd.back()->setWindowTitle( "Chromatogram" );
        stack_->addWidget( wnd.back() );

        msPeaksWnd_ = new MSPeaksWnd;
        wnd.push_back( msPeaksWnd_ );
        wnd.back()->setWindowTitle( "TOF Peaks" );
        stack_->addWidget( wnd.back() );

        wnd.push_back( new SpectrogramWnd );
        wnd.back()->setWindowTitle( "Spectrogram" );
        stack_->addWidget( wnd.back() );
    }
	
	bool res = connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) )
                        , this, SLOT( handleSessionAdded( Dataprocessor* ) ) );
    assert( res );

    // The handleSelectionChanged on MainWindow should be called in advance 
    // for all stacked child widgets.  This is significantly important for child widget has right device size
    // especially for QwtPlot widget calculates QRectF intersection for annotation interference check using
    // QScaleMap.
    res = connect( SessionManager::instance(), SIGNAL( signalSelectionChanged( Dataprocessor*, portfolio::Folium& ) )
                   , this, SLOT( handleSelectionChanged( Dataprocessor*, portfolio::Folium& ) ) );  assert( res );

    for ( auto it: wnd ) { // std::vector< QWidget *>::iterator it = wnd.begin(); it != wnd.end(); ++it ) {
        // with respect to above comment, this connection should be called after MainWindow::handleSelectionChanged
        // connection has been established.
        res = connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) )
                       , it, SLOT( handleSessionAdded( Dataprocessor* ) ) );  assert( res );

        res = connect( SessionManager::instance(), SIGNAL( signalSelectionChanged( Dataprocessor*, portfolio::Folium& ) )
                       , it, SLOT( handleSelectionChanged( Dataprocessor*, portfolio::Folium& ) ) );  assert( res );

        res = connect( DataprocPlugin::instance(), SIGNAL( onApplyMethod( const adcontrols::ProcessMethod& ) )
                       , it, SLOT( handleApplyMethod( const adcontrols::ProcessMethod& ) ) );   assert( res );
        
        connect( SessionManager::instance(), SIGNAL( signalCheckStateChanged( Dataprocessor*, portfolio::Folium&, bool ) )
                 , it, SLOT( handleCheckStateChanged( Dataprocessor*, portfolio::Folium&, bool ) ) );

        connect( axisChoice_, SIGNAL( currentIndexChanged( int ) ), it, SLOT( handleAxisChanged( int ) ) );
    }


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

    createDockWidgets();

	return splitter;
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
    qtwrapper::TrackingEnabled<Utils::FancyMainWindow> x( *this );

    QList< QDockWidget *> widgets = dockWidgets();

    for ( auto widget: widgets ) {
        widget->setFloating( false );
        removeDockWidget( widget );
    }
  
    size_t npos = 0;
    for ( auto widget: widgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
        if ( npos++ >= 2 )
            tabifyDockWidget( widgets[1], widget );
    }
	widgets[1]->raise();

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
MainWindow::createDockWidgets()
{
    using adportable::Configuration;

    static const struct { 
        const char * title;
        const char * wiid;
    } widgets [] = { 
        {  "Centroid" ,        "qtwidgets::CentroidForm" }
        , { "MS Calibration",  "qtwidgets2::MSCalibrationForm" }
        , { "MS Chromatogr.",  "qtwidgets2::MSChromatogramWidget" }
        , { "Targeting",       "qtwidgets::TargetForm" }
        //, { "Isotope",       "qtwidgets::IsotopeForm" }
        , { "Elemental Comp.", "qtwidgets::ElementalCompositionForm" }
        , { "Peak Find",       "qtwidgets::PeakMethodForm" }
        , { "Data property",   "dataproc::MSPropertyForm" }      // local
        , { "MS Peaks",        "qtwidgets2::MSPeakTable" }
        , { "TOF Peaks",       "qtwidgets2::MSPeakView" }
    };
    
    for ( auto widget: widgets ) {

        QWidget * pWidget = adplugin::widget_factory::create( widget.wiid, 0, 0 );
        if ( pWidget && std::strcmp(widget.wiid, "qtwidgets2::MSPeakView") == 0 ) {
            connect( this, SIGNAL(onAddMSPeaks( const adcontrols::MSPeaks& )), pWidget, SLOT(handle_add_mspeaks( const adcontrols::MSPeaks& )) );
            adplugin::LifeCycleAccessor accessor( pWidget );
            if ( adplugin::LifeCycle * p = accessor.get() ) {
                boost::any a( msPeaksWnd_ );
                p->setContents( a );
            }
        }

        if ( pWidget && std::strcmp( widget.wiid, "qtwidgets2::MSPeakTable" ) == 0 ) {
            connect( pWidget, SIGNAL( currentChanged( int, int ) ), wndMSProcessing_, SLOT( handleCurrentChanged( int, int ) ) );
            connect( pWidget, SIGNAL( formulaChanged( int, int ) ), wndMSProcessing_, SLOT( handleFormulaChanged( int, int ) ) );
            connect( pWidget, SIGNAL( triggerLockMass() ), wndMSProcessing_, SLOT( handleLockMass() ) );
        }

        if ( !pWidget ) {
            if ( std::strcmp( widget.wiid, "dataproc::FilePropertyWidget" ) == 0 ) 
                pWidget = new FilePropertyWidget;
            if ( std::strcmp( widget.wiid, "dataproc::MSPropertyForm" ) == 0 )
                pWidget = new MSPropertyForm;
        }
        
        if ( pWidget ) {
            createDockWidget( pWidget, widget.title );
        } else {
            QMessageBox::critical(0, QLatin1String("dataprocmanager"), widget.wiid );
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
MainWindow::handleSessionAdded( dataproc::Dataprocessor * )
{
}

void
MainWindow::handleSelectionChanged( dataproc::Dataprocessor *, portfolio::Folium& folium )
{
	if ( portfolio::Folder folder = folium.getParentFolder() ) {
		if ( folder.name() == L"MSCalibration" ) {
            if ( stack_->currentIndex() != 2 && stack_->currentIndex() != 3 )
                actionSelMSCalibration();
        } else if ( folder.name() == L"Spectra" ) {
			if ( stack_->currentIndex() == 2 || stack_->currentIndex() == 3 )
				actionSelMSProcess();
        } else if ( folder.name() == L"Spectrograms" ) {
            if ( stack_->currentIndex() != 6 )
                actionSelSpectrogram();
		}

        adcontrols::MassSpectrumPtr centroid;

        if ( folder.name() == L"Spectra" ) {
            
            if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium.data() ) ) {

                if ( auto f = portfolio::find_first_of( folium.attachments(), []( portfolio::Folium& a ){
                            return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
                    centroid = portfolio::get< adcontrols::MassSpectrumPtr >( f );
                    // if ( auto fpkinfo = portfolio::find_first_of( fCentroid.attachments(), [] ( portfolio::Folium& a ){
                    //             return portfolio::is_type< adcontrols::MSPeakInfoPtr >( a ); } ) ) {
                    //     pkinfo = portfolio::get< adcontrols::MSPeakInfoPtr >( fpkinfo );
                    // }
                } else {
                    centroid = std::make_shared< adcontrols::MassSpectrum >();  // empty data for clear table
                }
            }
        }
        
        // set data property to MSPropertyForm
        boost::any any( folium );
        for ( auto widget: dockWidgets() ) {
            adplugin::LifeCycleAccessor accessor( widget->widget() );
            if ( adplugin::LifeCycle * pLifeCycle = accessor.get() ) {
                pLifeCycle->setContents( any );
                pLifeCycle->setContents( boost::any( centroid ) );
            }
        }
    }
}

int
MainWindow::currentProcessView( std::string& title ) const
{
    int id = stack_->currentIndex();
	title = stack_->currentWidget()->windowTitle().toStdString();
    return id;
}

void
MainWindow::handleApplyMethod()
{
}

void
MainWindow::handle_add_mspeaks( const adcontrols::MSPeaks& peaks )
{
    emit onAddMSPeaks( peaks );
}

void
MainWindow::getProcessMethod( adcontrols::ProcessMethod& pm )
{
    using adcontrols::ProcessMethod;

	boost::any any( static_cast< adcontrols::ProcessMethod *>( &pm ) );
    for ( auto widget: dockWidgets() ) {
		adplugin::LifeCycleAccessor accessor( widget->widget() );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
        if ( pLifeCycle )
			pLifeCycle->getContents( any );
    }
}

void
MainWindow::setProcessMethod( const adcontrols::ProcessMethod& pm )
{
	boost::any any( pm );
	for ( auto widget: dockWidgets() ) {
		adplugin::LifeCycleAccessor accessor( widget->widget() );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle )
			pLifeCycle->setContents( any );
	}
}

void
MainWindow::OnInitialUpdate()
{
    using adcontrols::ProcessMethod;

    QList< QDockWidget *> widgets = dockWidgets();
  
    for ( auto widget: widgets ) {
        QWidget * obj = widget->widget();
		adplugin::LifeCycleAccessor accessor( obj );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle ) {
			pLifeCycle->OnInitialUpdate();
			connect( obj, SIGNAL( apply( adcontrols::ProcessMethod& ) ), this
                     , SLOT( onMethodApply( adcontrols::ProcessMethod& ) ), Qt::DirectConnection );
		}
    }
    setSimpleDockWidgetArrangement();
}

void
MainWindow::OnFinalClose()
{
    using adcontrols::ProcessMethod;

    QList< QDockWidget *> widgets = dockWidgets();
  
    for ( auto widget: widgets ) {
        QObjectList list = widget->children();
        for ( auto obj: list ) {
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
    adportable::debug(__FILE__, __LINE__) << "dataproc::MainWindow::actionApply(" << currentFeature_ << ")";
    qtwrapper::waitCursor wait;

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

void
MainWindow::actionSelMSPeaks()
{
    stack_->setCurrentIndex( 5 );
}

void
MainWindow::actionSelSpectrogram()
{
    stack_->setCurrentIndex( 6 );
}

void
MainWindow::processMethodSaved( const QString& name )
{
    processMethodNameEdit_->setText( name );
}

void
MainWindow::processMethodLoaded( const QString& name, const adcontrols::ProcessMethod& m )
{
    processMethodNameEdit_->setText( name );

	QList< QDockWidget * > docs = this->dockWidgets();

	std::for_each( docs.begin(), docs.end(), [&]( QDockWidget * dock ){
		QWidget * obj = dock->widget();
		adplugin::LifeCycleAccessor accessor( obj );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle ) {
            boost::any any( m );
			pLifeCycle->setContents( any );
        }
    });
}

void
MainWindow::printCurrentView( const QString& pdfname ) const
{
    QWidget * w = stack_->currentWidget();
    if ( connect( this, SIGNAL( onPrintCurrentView( const QString& ) ), w, SLOT( handlePrintCurrentView( const QString& ) ) ) ) {
        emit onPrintCurrentView( pdfname );
        disconnect( this, SIGNAL( onPrintCurrentView( const QString& ) ), w, SLOT( handlePrintCurrentView( const QString& ) ) );
    }
}

void
MainWindow::saveDefaultMSCalibrateResult( portfolio::Folium& )
{
    
}


///
void
MainWindow::actCreateSpectrogram()
{
    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
        processor->createSpectrogram();
    }
}

void
MainWindow::actClusterSpectrogram()
{
    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() )
        processor->clusterSpectrogram();
}
