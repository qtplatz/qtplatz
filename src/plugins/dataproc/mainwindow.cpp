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
#include "chromatogramwnd.hpp"
#include "dataprocessor.hpp"
#include "dataprocplugin.hpp"
#include "dataprocessorfactory.hpp"
#include "elementalcompwnd.hpp"
#include "filepropertywidget.hpp"
#include "isequenceimpl.hpp"
#include "msprocessingwnd.hpp"
#include "mscalibrationwnd.hpp"
#include "mscalibspectrawnd.hpp"
#include "mspeakswnd.hpp"
#include "msspectrawnd.hpp"
#include "mspropertyform.hpp"
#include "sessionmanager.hpp"
#include "spectrogramwnd.hpp"

#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targeting.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/widget_factory.hpp>
#include <adportable/configuration.hpp>
#include <adportable/profile.hpp>
#include <adportable/utf.hpp>
#include <adlog/logger.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptides.hpp>
#include <adprot/peptide.hpp>
#include <adutils/adfile.hpp>
#include <adwidgets/centroidform.hpp>
#include <adwidgets/peptidewidget.hpp>
#include <adwidgets/targetingwidget.hpp>
#include <adwidgets/mspeaktable.hpp>
#include <adwidgets/mscalibratewidget.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/portfolio.hpp>
#include <qtwrapper/qstring.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/imode.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <utils/styledbar.h>

//#include <QCheckBox>
#include <QPushButton>
#include <QDir>
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
#include <QFileDialog>

#include <functional>
#include <fstream>

namespace dataproc {

    typedef boost::variant< MSProcessingWnd*, ElementalCompWnd*, MSCalibrationWnd*, MSCalibSpectraWnd*
                            , ChromatogramWnd*, MSPeaksWnd*, SpectrogramWnd*, MSSpectraWnd* > wnd_ptr_t;

    struct wnd_set_title : public boost::static_visitor < QWidget * > {
        const QString& text_;
        wnd_set_title( const QString& t ) : text_( t ) {}
        template<class T> QWidget * operator () ( T* wnd ) const { wnd->setWindowTitle( text_ ); return wnd; }
    };

    struct session_added_connector : public boost::static_visitor < bool > {
        QObject * this_;
        session_added_connector( QObject * p ) : this_(p) {}
        template<class T> bool operator () ( T* wnd ) const {
            return
                this_->connect( SessionManager::instance(), &SessionManager::signalSessionAdded, wnd
                                , [=]( Dataprocessor *dp ){ wnd->handleSessionAdded(dp);});
        }
    };

    struct selection_changed_connector : public boost::static_visitor< bool > {
        QObject * this_;
        selection_changed_connector( QObject * p ) : this_(p) {}
        template<class T> bool operator () ( T* wnd ) const {
            return
                this_->connect( SessionManager::instance(), &SessionManager::signalSelectionChanged, wnd
                                , [=]( Dataprocessor* dp, portfolio::Folium& f ){ wnd->handleSelectionChanged( dp, f ); });
        }
    };

    struct processed_connector : public boost::static_visitor< bool > {
        QObject * this_;
        processed_connector( QObject * p ) : this_(p) {}
        template<class T> bool operator () ( T* wnd ) const {
            return
                this_->connect( SessionManager::instance(), &SessionManager::onProcessed, wnd
                                , [=]( Dataprocessor* dp, portfolio::Folium& f ){ wnd->handleProcessed( dp, f ); });
        }
    };

    struct apply_method_connector : public boost::static_visitor< bool > {
        QObject * this_;
        apply_method_connector( QObject * p ) : this_(p) {}
        template<class T> bool operator () ( T* wnd ) const {
            return 
                this_->connect( DataprocPlugin::instance(), &DataprocPlugin::onApplyMethod, wnd
                                , [=]( const adcontrols::ProcessMethod& pm ){ wnd->handleApplyMethod( pm ); });
        }
    };

    struct check_state_changed_connector : public boost::static_visitor< bool > {
        QObject * this_;
        check_state_changed_connector( QObject * p ) : this_(p) {}
        template< class T > bool operator () ( T* wnd ) const {
            return 
                this_->connect( SessionManager::instance(), &SessionManager::signalCheckStateChanged, wnd //( Dataprocessor*, portfolio::Folium&, bool ) )
                                , [=]( Dataprocessor* dp, portfolio::Folium& f, bool st ){ wnd->handleCheckStateChanged( dp, f, st ); });
        }
    };
    template<> bool check_state_changed_connector::operator()( ElementalCompWnd * ) const { return false; }
    template<> bool check_state_changed_connector::operator()( MSCalibrationWnd * ) const { return false; }
    template<> bool check_state_changed_connector::operator()( ChromatogramWnd * ) const { return false; }

    struct axis_changed_connector : public boost::static_visitor< bool > {
        QObject * this_;
        QComboBox * sender_;
        axis_changed_connector( QObject * p, QComboBox * choice ) : this_(p), sender_(choice) {}
        template< class T > bool operator () ( T* receiver ) const {
            return
                this_->connect( sender_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
                                , receiver, [=] ( int idx ){ receiver->handleAxisChanged( idx ); } );
        }
    };
    // following 4 classes has no axis change handler
    template<> bool axis_changed_connector::operator()( ElementalCompWnd * ) const { return false; }
    template<> bool axis_changed_connector::operator()( ChromatogramWnd * ) const { return false; }
    template<> bool axis_changed_connector::operator()( MSPeaksWnd * ) const { return false; }
    template<> bool axis_changed_connector::operator()( SpectrogramWnd * ) const { return false; }
}

using namespace dataproc;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow( QWidget *parent ) : Utils::FancyMainWindow(parent)
                                          , toolBar_( 0 )
                                          , toolBarLayout_( 0 )
                                          , axisChoice_( 0 )
                                          , actionSearch_( 0 )
                                          , actionApply_( 0 )
                                          , stack_( 0 )
                                          , processMethodNameEdit_( new QLineEdit ) 
                                          , currentFeature_( CentroidProcess )
{
    std::fill( selPages_.begin(), selPages_.end(), static_cast<QAction *>(0) );
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

            if ( auto p = selPages_[ idSelMSProcess ] = new QAction( "MS Process", this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelMSProcess ); } );
                am->registerAction( p, "dataproc.selMSProcess", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }
            if ( auto p = selPages_[ idSelElementalComp] = new QAction( "Elemental Comp", this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelElementalComp ); } );
                am->registerAction( p, "dataproc.selElementalComp", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }
            if ( auto p = selPages_[ idSelMSCalibration ] = new QAction( "MS Calibration", this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelMSCalibration ); } );
                am->registerAction( p, "dataproc.selMSCalibration", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }
            if ( auto p = selPages_[ idSelMSCalibSpectra ] = new QAction( "MS Calib. Spectra", this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelMSCalibSpectra ); } );
                am->registerAction( p, "dataproc.selMSCalibSpectra", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }
            if ( auto p = selPages_[ idSelChromatogram ] = new QAction( "Chromatogram", this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelChromatogram ); } );
                am->registerAction( p, "dataproc.selChromatogram", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }
            if ( auto p = selPages_[ idSelMSPeaks ] = new QAction( "TOF Plots", this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelMSPeaks ); } );
                am->registerAction( p, "dataproc.selTOFPlots", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }

            if ( auto p = selPages_[ idSelSpectrogram ] = new QAction( "Spectrogram", this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelSpectrogram ); } );
                am->registerAction( p, "dataproc.selSpectrogram", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }

            if ( auto p = selPages_[ idSelSpectra ] = new QAction( "Spectra", this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelSpectra ); } );
                am->registerAction( p, "dataproc.selSpectra", globalcontext );
                toolBarLayout->addWidget( toolButton( p ) );
            }
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

void
MainWindow::selPage( idPage id )
{
    stack_->setCurrentIndex( id );
}

MainWindow::idPage
MainWindow::curPage() const
{
    return idPage( stack_->currentIndex() );
}

void
MainWindow::currentPageChanged( int idx )
{
    if ( idx == idSelSpectra ) {
        if ( auto p = dynamic_cast<MSSpectraWnd *>(stack_->widget( idx )) ) {
            p->onPageSelected();
        }
    }
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
            features->addItem( "Targeting" ); // Centroid + find targets
            features->addItem( "Calibration" );
            features->addItem( "Find peaks" );
            toolBarLayout->addWidget( features );

            connect( features, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::handleFeatureSelected ); // (int) ) );
            connect( features, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MainWindow::handleFeatureActivated );
            features->setContextMenuPolicy( Qt::CustomContextMenu );

            connect( features, &QComboBox::customContextMenuRequested, [=] ( QPoint pt ){
                    QMenu menu;
                    menu.addAction( am->command( Constants::PROCESS_ALL_CHECKED )->action() );
                    menu.exec( features->mapToGlobal( pt ) );
                } );

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

    // std::vector< QWidget * > wnd;
    std::vector< wnd_ptr_t > wnd;
    Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
    if ( splitter3 ) {

        stack_ = new QStackedWidget;
        splitter3->addWidget( stack_ );

        connect( stack_, &QStackedWidget::currentChanged, this, &MainWindow::currentPageChanged );

        wnd.push_back( new MSProcessingWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( "MS Process" ), wnd.back() ) );

        wnd.push_back( new ElementalCompWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( "Elemental Comp." ), wnd.back() ) );

        wnd.push_back( new MSCalibrationWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( "MS Calibration" ), wnd.back() ) );

        wnd.push_back( new MSCalibSpectraWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( "MS Calibration(2)" ), wnd.back() ) );

        wnd.push_back( new ChromatogramWnd( apppath ) );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( "Chromatogram" ), wnd.back() ) );

        wnd.push_back( new MSPeaksWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( "TOF Peaks" ), wnd.back() ) );

        wnd.push_back( new SpectrogramWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( "Spectrogram" ), wnd.back() ) );

        wnd.push_back( new MSSpectraWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( "Spectra" ), wnd.back() ) );
    }
    if ( auto pSrc = stack_->widget( idSelMSProcess ) ) {
        if ( auto pDst = stack_->widget( idSelSpectra ) )
            connect( dynamic_cast<MSProcessingWnd *>(pSrc), &MSProcessingWnd::dataChanged, dynamic_cast<MSSpectraWnd *>(pDst), &MSSpectraWnd::onDataChanged );
    }

    connect( SessionManager::instance(), &SessionManager::signalSessionAdded, this, &MainWindow::handleSessionAdded );
    connect( SessionManager::instance(), &SessionManager::onProcessed, this, &MainWindow::handleProcessed );

    // The handleSelectionChanged on MainWindow should be called in advance for all stacked child widgets.
    // This is significantly important for child widget has right QRectF for each QwtPlot.
    connect( SessionManager::instance(), &SessionManager::signalSelectionChanged, this, &MainWindow::handleSelectionChanged );


    for ( auto it: wnd ) { // std::vector< QWidget *>::iterator it = wnd.begin(); it != wnd.end(); ++it ) {
        boost::apply_visitor( session_added_connector(this), it );
        boost::apply_visitor( selection_changed_connector(this), it );
        boost::apply_visitor( processed_connector(this), it );
        boost::apply_visitor( apply_method_connector(this), it );

        boost::apply_visitor( check_state_changed_connector(this), it );
        boost::apply_visitor( axis_changed_connector(this, axisChoice_), it );
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

    update();
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& pageName )
{
    QDockWidget * dockWidget = addDockForWidget( widget );
    dockWidget->setObjectName( pageName.isEmpty() ? widget->objectName() : pageName );

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
        const char * pageName;
        std::function<QWidget *()> factory;
    } widgets [] = { 
        { "Centroid",         "adwidgets::CentroidForm",          "CentroidMethod", [] (){ return new adwidgets::CentroidForm; } } // should be first
        , { "MS Peaks",       "adwidgets::MSPeakTable",           "MSPeakTable", [] () { return new adwidgets::MSPeakTable; } }
        // , { "MS Calibration", "qtwidgets2::MSCalibrationForm",    "MSCalibrationMethod",  [](){ return adplugin::widget_factory::create("qtwidgets2::MSCalibrationForm", 0, 0 ); } }
        , { "MS Calibration", "adwidgets::MSCalibrateWidget",     "MSCalibrateWidget",   [] () { return new adwidgets::MSCalibrateWidget; } }
        , { "MS Chromatogr.", "qtwidgets2::MSChromatogramWidget", "MSChromatogrMethod",  [](){ return adplugin::widget_factory::create("qtwidgets2::MSChromatogramWidget", 0, 0 ); } }
        , { "Targeting",      "adwidgets::TargetingWidget",       "TargetingMethod", [] (){ return new adwidgets::TargetingWidget; } }
        , { "Peptide",        "adwidgets::PeptideWidget",         "PeptideMethod", [] (){ return new adwidgets::PeptideWidget; } }
        //, { "Elemental Comp.","qtwidgets::ElementalCompositionForm", "EleCompMethod",  }
        , { "Peak Find",      "qtwidgets::PeakMethodForm",        "PeakFindMethod",  [](){ return adplugin::widget_factory::create("qtwidgets::PeakMethodForm", 0, 0 ); } }
        , { "Data property",  "dataproc::MSPropertyForm",         "DataProperty", [] (){ return new dataproc::MSPropertyForm; } }
        , { "TOF Peaks",      "qtwidgets2::MSPeakView",           "TOFPeaks",  [](){ return adplugin::widget_factory::create("qtwidgets2::MSPeakView", 0, 0 ); } }
    };
    
    for ( auto& widget: widgets ) {

        QWidget * pWidget = widget.factory ? widget.factory() : adplugin::widget_factory::create( widget.wiid, 0, 0 );

        if ( pWidget && std::strcmp(widget.wiid, "qtwidgets2::MSPeakView") == 0 ) {
            // TOFPeaks
            connect( this, SIGNAL( onAddMSPeaks( const adcontrols::MSPeaks& ) ), pWidget, SLOT( handle_add_mspeaks( const adcontrols::MSPeaks& ) ) );
            adplugin::LifeCycleAccessor accessor( pWidget );
            if ( adplugin::LifeCycle * p = accessor.get() ) {
                if ( auto wnd = findChild< MSPeaksWnd *>() ) {
                    boost::any a( static_cast<QWidget *>(wnd) );
                    p->setContents( a );
                }
            }
        }

        if ( auto p = dynamic_cast< adwidgets::MSPeakTable *>( pWidget ) ) {
            // MS Peak list
            using adwidgets::MSPeakTable;
            if ( auto wnd = findChild< MSProcessingWnd *>() ) {
                connect( p, static_cast<void (MSPeakTable::*)(int, int)>(&MSPeakTable::currentChanged), wnd
                         , [=] ( int idx, int fcn ){ wnd->handleCurrentChanged( idx, fcn ); }
                    );
                connect( p, static_cast<void (MSPeakTable::*)(int, int)>(&MSPeakTable::formulaChanged), wnd
                         , [=] ( int idx, int fcn ){ wnd->handleFormulaChanged( idx, fcn ); }
                    );

                connect( p, &MSPeakTable::triggerLockMass, wnd, &MSProcessingWnd::handleLockMass );
                connect( this, &MainWindow::onDataMayCanged, wnd, &MSProcessingWnd::handleDataMayChanged );
            }
            connect( this, &MainWindow::onZoomedOnSpectrum, p, &MSPeakTable::handle_zoomed );
        }

        if ( auto p = dynamic_cast< adwidgets::PeptideWidget *>( pWidget ) ) {
            connect( p, &adwidgets::PeptideWidget::triggerFind, this, &MainWindow::handlePeptideTarget );
            // (const QVector< QPair<QString, QString> >&) ) );
        }

        if ( pWidget ) {
            createDockWidget( pWidget, widget.title, widget.pageName );
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
MainWindow::zoomedOnSpectrum( const QRectF& rc )
{
    emit onZoomedOnSpectrum( rc );
}

void
MainWindow::handleSessionAdded( dataproc::Dataprocessor * )
{
}

void
MainWindow::handleProcessed( dataproc::Dataprocessor * processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MainWindow::handleSelectionChanged( dataproc::Dataprocessor *, portfolio::Folium& folium )
{
	if ( portfolio::Folder folder = folium.getParentFolder() ) {
		if ( folder.name() == L"MSCalibration" ) {
            if ( stack_->currentIndex() != idSelMSCalibration && stack_->currentIndex() != idSelMSCalibSpectra )
                selPage( idSelMSCalibration );
        } else if ( folder.name() == L"Spectra" ) {
            if ( stack_->currentIndex() != idSelMSProcess &&
                 stack_->currentIndex() != idSelElementalComp &&  stack_->currentIndex() != idSelSpectra )
                selPage( idSelMSProcess );
        } else if ( folder.name() == L"Spectrograms" ) {
            if ( stack_->currentIndex() != idSelSpectrogram )
                selPage( idSelSpectrogram );
		}

        adcontrols::MassSpectrumPtr centroid;
        adcontrols::TargetingPtr targeting;

        if ( folder.name() == L"Spectra" ) {
            
            if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium.data() ) ) {

                if ( auto f = portfolio::find_first_of( folium.attachments(), []( portfolio::Folium& a ){
                            return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
					try {
						centroid = portfolio::get< adcontrols::MassSpectrumPtr >( f );
					} catch ( boost::bad_any_cast& ex ) {
						ADERROR() << boost::diagnostic_information( ex );
					}
                    
                    if ( auto t = portfolio::find_first_of( f.attachments(), []( portfolio::Folium& a) {
                                return a.name() == Constants::F_TARGETING;}) ) {
                        try {
                            targeting = portfolio::get< adcontrols::TargetingPtr >( t );
                        } catch ( boost::bad_any_cast& ex ) {
                            ADERROR() << boost::diagnostic_information( ex );
                        }
                    }
                } else {
                    centroid = std::make_shared< adcontrols::MassSpectrum >();  // empty data for clear table
                }
            }
			auto docks = dockWidgets();
			auto it = std::find_if( docks.begin(), docks.end(), []( QDockWidget * d ){	return d->objectName() == "MSPeakTable"; });
			if ( it != docks.end() )
				(*it)->raise();
        }
        
        // set data property to MSPropertyForm
        boost::any afolium( folium );
        boost::any acentroid( centroid );
        boost::any atargeting( targeting );
        for ( auto widget: dockWidgets() ) {
            adplugin::LifeCycleAccessor accessor( widget->widget() );
            if ( adplugin::LifeCycle * pLifeCycle = accessor.get() ) {
                pLifeCycle->setContents( afolium );
                pLifeCycle->setContents( acentroid );
                if ( targeting )
                    pLifeCycle->setContents( atargeting );
            }
        }
    }
}

void
MainWindow::lockMassHandled( const std::shared_ptr< adcontrols::MassSpectrum >& ptr )
{
    auto docks = dockWidgets();
    auto it = std::find_if( docks.begin(), docks.end(), []( QDockWidget * d ){	return d->objectName() == "MSPeakTable"; });
    if ( it != docks.end() ) {
        adplugin::LifeCycleAccessor accessor( (*it)->widget() );
        if ( adplugin::LifeCycle * pLifeCycle = accessor.get() ) {
            boost::any any( ptr );
            pLifeCycle->onUpdate( any );
        }
    }
}

void
MainWindow::dataMayChanged()
{
    auto docks = dockWidgets();
    auto it = std::find_if( docks.begin(), docks.end(), []( QDockWidget * d ){	return d->objectName() == "MSPeakTable"; });
    if ( it != docks.end() ) {
        adplugin::LifeCycleAccessor accessor( (*it)->widget() );
        if ( adplugin::LifeCycle * pLifeCycle = accessor.get() ) {
            boost::any any(int(0));
            pLifeCycle->onUpdate( any );
        }
    }
    emit onDataMayCanged();
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
MainWindow::handlePeptideTarget( const QVector<QPair<QString, QString> >& peptides )
{
    adprot::digestedPeptides digested;
    for ( auto& p: peptides )
        digested << adprot::peptide( p.first.toStdString(), p.second.toStdString(), 0 );
    
    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
        processor->findPeptide( digested );
    } else {
		QMessageBox::information( 0, "Dataproc", "No data exist for peptide targeting" );
    }
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
			// connect( obj, SIGNAL( apply( adcontrols::ProcessMethod& ) ), this
            //          , SLOT( onMethodApply( adcontrols::ProcessMethod& ) ), Qt::DirectConnection );
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
        QWidget * obj = widget->widget();
        adplugin::LifeCycleAccessor accessor( obj );
        adplugin::LifeCycle * pLifeCycle = accessor.get();
        if ( pLifeCycle ) {
            //disconnect( obj, SIGNAL( onMethodApply( ProcessMethod& ) ), this, SLOT( onMethodApply( ProcessMethod& ) ) );
            pLifeCycle->OnFinalClose();
        }
    }
}

// void
// MainWindow::onMethodApply( adcontrols::ProcessMethod& pm )
// {
//     DataprocPlugin::instance()->applyMethod( pm );
// }

void
MainWindow::handleProcessChecked()
{
    qtwrapper::waitCursor wait;
    adcontrols::ProcessMethod m;
    getProcessMethod( m );
    if ( m.size() > 0 ) {
        for ( auto& session : *SessionManager::instance() ) {
            if ( auto processor = session.processor() ) {
                if ( currentFeature_ == CalibrationProcess )
                    processor->applyCalibration( m );
                else {
                    for ( auto& folder : processor->portfolio().folders() ) {
                        for ( auto& folium : folder.folio() ) {
                            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                                if ( folium.empty() )
                                    processor->fetch( folium );
                                processor->applyProcess( folium, m, currentFeature_ );
                            }
                        }
                    }
                }
            }
        }
    }
}

void
MainWindow::handleExportPeakList()
{
    QString filename = QFileDialog::getSaveFileName( 0
                                                     , tr( "Save peak list for all checked spectra")
                                                     , currentDir()
                                                     , tr( "Text files(*.txt)" ) );
    if ( filename.isEmpty() )
        return;
    std::ofstream outf( filename.toStdString() );
    
    for ( auto& session : *SessionManager::instance() ) {
        if ( auto processor = session.processor() ) {
            auto spectra = processor->getPortfolio().findFolder( L"Spectra" );

            for ( auto& folium: spectra.folio() ) {
                if ( folium.attribute( L"isChecked" ) == L"true" ) {
                    if ( folium.empty() )
                        processor->fetch( folium );

                    // output filename
                    outf << adportable::utf::to_utf8( processor->filename() ) << std::endl;

                    portfolio::Folio atts = folium.attachments();
                    auto itCentroid = std::find_if( atts.begin(), atts.end(), []( portfolio::Folium& f ) {
                            return f.name() == Constants::F_CENTROID_SPECTRUM;
                        });
                    if ( itCentroid != atts.end() ) {

                        // output spectrum(centroid) name
                        outf << adportable::utf::to_utf8( folium.name() + L",\t" + itCentroid->name() ) << std::endl;

                        if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid ) ) {
                            adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *centroid );
                            int fcn = 0;
                            for ( auto& ms: segments ) {
                                const adcontrols::annotations& annots = ms.get_annotations();
                                for ( size_t n = 0; n < ms.size(); ++n ) {
                                    outf << fcn << ",\t" << n << ",\t"
                                         << std::scientific << std::setprecision( 15 ) << ms.getTime( n ) << ",\t"
                                         << std::fixed << std::setprecision( 13 ) << ms.getMass( n ) << ",\t"
                                         << std::scientific << std::setprecision(7) << ms.getIntensity( n );
                                    
                                    auto it = std::find_if( annots.begin(), annots.end()
                                                            , [=]( const adcontrols::annotation& a ){ return a.index() == int(n); } );
                                    while ( it != annots.end() ) {
                                        outf << ",\t" << it->text();
                                        it = std::find_if( ++it, annots.end()
                                                           , [=]( const adcontrols::annotation& a ){ return a.index() == int(n); } );
                                    }
                                    outf << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void
MainWindow::handleImportChecked()
{
    QString filename = QFileDialog::getSaveFileName( 0
                                                     , tr( "Import checked data into a file")
                                                     , currentDir()
                                                     , tr( "QtPlatz files(*.adfs)" ) );
    if ( filename.isEmpty() )
        return;

    bool handled(false);
    do {
        adutils::adfile adfile;
        if ( adfile.open( boost::filesystem::path( filename.toStdWString() ) ) ) {
            handled = true;
            for ( auto& session : *SessionManager::instance() ) {
                if ( auto processor = session.processor() ) {
                    for ( auto& folder : processor->portfolio().folders() ) {
                        for ( auto& folium : folder.folio() ) {
                            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                                adfile.append( folium, processor->file() );
                            }
                        }
                    }
                }
            }
        }
    } while ( 0 );

    if ( handled )
        DataprocPlugin::instance()->handleFileCreated( filename );
}

void
MainWindow::actionApply()
{
    ADTRACE() << "dataproc::MainWindow::actionApply(" << currentFeature_ << ")";
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
MainWindow::proteinSelected( const adprot::digestedPeptides& digested )
{
    auto docks = dockWidgets();
    auto it = std::find_if( docks.begin(), docks.end(), []( QDockWidget * d ){ return d->objectName() == "PeptideMethod"; });
    if ( it != docks.end() ) {
        (*it)->raise();
        boost::any a( digested );
        if ( auto t = dynamic_cast< adwidgets::PeptideWidget *>( (*it)->widget() ) ) {
            t->setContents( a );
        }
    }
}

void
MainWindow::handleFeatureSelected( int value )
{
    currentFeature_ = static_cast< ProcessType >( value );

    const char * object_name = 0;

    if ( currentFeature_ == TargetingProcess )
        object_name = "TargetingMethod";
    else if ( currentFeature_ == CalibrationProcess )
        object_name = "MSCalibrationMethod";
    else if ( currentFeature_ == PeakFindProcess )
        object_name = "PeakFindMethod";

    if ( object_name ) {
        auto docks = dockWidgets();
        auto it = std::find_if( docks.begin(), docks.end(), [=]( QDockWidget * d ){	return d->objectName() == object_name; });
        if ( it != docks.end() )
            (*it)->raise();
    }
}

void
MainWindow::handleFeatureActivated( int value )
{
    currentFeature_ = static_cast< ProcessType >( value );
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

bool
MainWindow::editor_factories( iSequenceImpl& impl ) 
{
    impl << detail::iEditorFactoryImpl( [] ( QWidget * p )->QWidget*{ return new adwidgets::CentroidForm( p ); }
         , adextension::iEditorFactory::PROCESS_METHOD
         , "Centroid" );

     impl << detail::iEditorFactoryImpl( [] ( QWidget * p )->QWidget*{ return new adwidgets::TargetingWidget( p ); }
         , adextension::iEditorFactory::PROCESS_METHOD
         , "Targeting" );
     return true;
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

QString
MainWindow::makePrintFilename( const std::wstring& id, const std::wstring& insertor, const char * extension )
{
    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
        portfolio::Portfolio portfolio = dp->getPortfolio();

        boost::filesystem::path path( portfolio.fullpath() );
        path = path.parent_path() / path.stem();

        if ( portfolio::Folium folium = portfolio.findFolium( id ) ) {
            std::wstring name = folium.name();
            boost::algorithm::trim( name );
            path += ( insertor + name );

            boost::filesystem::path tpath = path;
            tpath += extension;
            int nnn = 0;
            while( boost::filesystem::exists( tpath ) ) {
				tpath = path.wstring() + ( boost::wformat(L"(%d)%s") % nnn++ % extension).str();
            }
            return QString::fromStdString( tpath.string() );
        }
    }
    return QString();
}

//static
QString
MainWindow::makeDisplayName( const std::wstring& id, const char * insertor, int nbsp )
{
    QString o;

    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {

        while ( nbsp-- )
            o += "&nbsp;";

        portfolio::Portfolio portfolio = dp->getPortfolio();
        boost::filesystem::path path( portfolio.fullpath() );

        QDir dir( QString::fromStdWString( path.wstring() ) );
        dir.cdUp();
        dir.cdUp();
        o += dir.relativeFilePath( path.string().c_str() );
        o += insertor;

        if ( portfolio::Folium folium = portfolio.findFolium( id ) ) {
            std::wstring name = folium.name();
            boost::algorithm::trim( name );
            o += QString::fromStdWString( name );
        }
        return o;
    }
    return QString();
    
}


//static
std::wstring
MainWindow::foliumName( const std::wstring& id )
{
    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
        portfolio::Portfolio portfolio = dp->getPortfolio();

        if ( portfolio::Folium folium = portfolio.findFolium( id ) ) {
            std::wstring name = folium.name();
            boost::algorithm::trim( name );
            return name;
        }
    }
    return std::wstring();
}

QString
MainWindow::currentDir()
{
    static QString dir = QString::fromStdWString( adportable::profile::user_data_dir<wchar_t>() );

    QString currentFile = Core::ICore::instance()->fileManager()->currentFile();
    if ( !currentFile.isEmpty() ) {
        const QFileInfo fi( currentFile );
        dir = fi.absolutePath();
    }
    return dir;
}

