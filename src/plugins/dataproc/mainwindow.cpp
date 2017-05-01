/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
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

#if defined __APPLE__
# include <compiler/disable_deprecated.h>
#endif
#include "mainwindow.hpp"
#include "aboutdlg.hpp"
#include "chromatogramwnd.hpp"
#include "document.hpp"
#include "dataprocessor.hpp"
#include "dataprocessworker.hpp"
#include "dataprocplugin.hpp"
#include "dataprocessorfactory.hpp"
#include "elementalcompwnd.hpp"
#include "filepropertywidget.hpp"
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
#include <adcontrols/constants.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/mssimulatormethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targeting.hpp>
#include <adextension/idataproc.hpp>
#include <adextension/ieditorfactory_t.hpp>
#include <adextension/isequenceimpl.hpp>
#include <adextension/iwidgetfactory.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycleaccessor.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
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
#include <adwidgets/mschromatogramwidget.hpp>
#include <adwidgets/peakmethodform.hpp>
#include <adwidgets/mspeakwidget.hpp>
#include <adwidgets/mssimulatorwidget.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <qtwrapper/qstring.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <extensionsystem/pluginmanager.h>
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
#include <coreplugin/documentmanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <utils/styledbar.h>

//#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QDir>
#include <QDockWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
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
            return this_->connect( sender_, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), receiver, [=] ( int idx ) {
                receiver->handleAxisChanged( idx == 0 ? adcontrols::hor_axis_mass : adcontrols::hor_axis_time ); } );
        }
    };
    // following 3 classes has no axis change handler
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
                                          , aboutDlg_(0)
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

Utils::StyledBar *
MainWindow::createStyledBarTop()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 2 );
        Core::ActionManager * am = Core::ActionManager::instance();
        if ( am ) {
            Core::Context context( ( Core::Id( "dataproc.MainView" ) ) );

            if ( auto p = new QAction( tr("MS Process"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelMSProcess ); } );
                am->registerAction( p, "dataproc.selMSProcess", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelMSProcess ) ) );
            }
            if ( auto p = new QAction( tr("Spectra"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelSpectra ); } );
                am->registerAction( p, "dataproc.selSpectra", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelSpectra ) ) );
            }
            if ( auto p = new QAction( tr("Simulation"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelElementalComp ); } );
                am->registerAction( p, "dataproc.selElementalComp", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelElementalComp ) ) );
            }
            if ( auto p = new QAction( tr("MS Calibration"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelMSCalibration ); } );
                am->registerAction( p, "dataproc.selMSCalibration", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelMSCalibration ) ) );
            }
            if ( auto p = new QAction( tr("MS Calib. Spectra"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelMSCalibSpectra ); } );
                am->registerAction( p, "dataproc.selMSCalibSpectra", context );
                toolBarLayout->addWidget( toolButton( p, QString("wnd.%1").arg( idSelMSCalibSpectra ) ) );
            }
            if ( auto p = new QAction( tr("Chromatogram"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelChromatogram ); } );
                am->registerAction( p, "dataproc.selChromatogram", context );
                toolBarLayout->addWidget( toolButton( p, QString("wnd.%1").arg( idSelChromatogram ) ) );
            }
            if ( auto p = new QAction( tr("TOF Plots"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelMSPeaks ); } );
                am->registerAction( p, "dataproc.selTOFPlots", context );
                toolBarLayout->addWidget( toolButton( p, QString("wnd.%1").arg( idSelMSPeaks ) ) );
            }

            if ( auto p = new QAction( tr("Spectrogram"), this ) ) {
                connect( p, &QAction::triggered, [=](){ stack_->setCurrentIndex( idSelSpectrogram ); } );
                am->registerAction( p, "dataproc.selSpectrogram", context );
                toolBarLayout->addWidget( toolButton( p, QString("wnd.%1").arg( idSelSpectrogram ) ) );
            }

        }


        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        
        axisChoice_ = new QComboBox;
        axisChoice_->addItem( tr("m/z") );
        axisChoice_->addItem( tr("time") );
        toolBarLayout->addWidget( new QLabel( tr("Axis:") ) );
        toolBarLayout->addWidget( axisChoice_ );
        
        toolBarLayout->addWidget( new QLabel( tr("Sequence:") ) );
        toolBarLayout->addWidget( new QLineEdit );
    }
    return toolBar;
}

void
MainWindow::setSpectrumAxisChoice( adcontrols::hor_axis axis )
{
    if ( axisChoice_ )
        axisChoice_->setCurrentIndex( axis ); // 0 = mass, 1 = time
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
        if ( auto p = dynamic_cast<MSSpectraWnd *>(stack_->widget( idx )) ) 
            p->onPageSelected();
    }

    QRegExp reg( "wnd\\.[0-9]+" );
    auto list = findChildren<QToolButton *>( reg );
    for ( auto btn : list )
        btn->setStyleSheet( QString( "color: lightGray; border: 2px; border-color: gray; border-style: groove" ) );

    if ( auto sel = findChild<QToolButton *>( QString( "wnd.%1" ).arg( QString::number( idx ) ) ) )
        sel->setStyleSheet( QString( "color: ivory; border: 2px; border-color: darkGray; border-style: inset;" ) );
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
        Core::ActionManager * am = Core::ActionManager::instance();
        if ( am ) {
            // print, method file open & save buttons
            toolBarLayout->addWidget(toolButton(am->command(Constants::PRINT_CURRENT_VIEW)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::METHOD_OPEN)->action()));
            toolBarLayout->addWidget(toolButton(am->command(Constants::METHOD_SAVE)->action()));
			toolBarLayout->addWidget(toolButton(am->command(Constants::CALIBFILE_APPLY)->action()));
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //----------
            toolBarLayout->addWidget( toolButton( am->command( Constants::METHOD_APPLY )->action() ) );

            QComboBox * features = new QComboBox;
            features->addItem( tr("Centroid") );
            features->addItem( tr("Targeting") ); // Centroid + find targets
            features->addItem( tr("MS Calibration") );
            features->addItem( tr("Find peaks") );
            toolBarLayout->addWidget( features );

            connect( features, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::handleFeatureSelected );
            connect( features, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MainWindow::handleFeatureActivated );
            features->setContextMenuPolicy( Qt::CustomContextMenu );

            connect( features, &QComboBox::customContextMenuRequested, [=] ( QPoint pt ){
                    QMenu menu;
                    menu.addAction( am->command( Constants::PROCESS_ALL_CHECKED )->action() );
                    menu.exec( features->mapToGlobal( pt ) );
                });

            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("Process Method:" ) ) );
            auto edit = new QLineEdit;
            edit->setObjectName( Constants::EDIT_PROCMETHOD );
            edit->setEnabled( false );

            toolBarLayout->addWidget( edit );

            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

            toolBarLayout->addWidget( toolButton( am->command( Constants::HIDE_DOCK )->action() ) );
            
        }
    }
    return toolBar2;
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
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

        if ( auto pWnd = new MSProcessingWnd ) {
            wnd.push_back( pWnd );
            stack_->addWidget( boost::apply_visitor( wnd_set_title( tr( "MS Process" ) ), wnd.back() ) );
            connect( this, &MainWindow::onDataMayCanged, pWnd, &MSProcessingWnd::handleDataMayChanged );
        }

        wnd.push_back( new ElementalCompWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( tr("Elemental Comp.") ), wnd.back() ) );

        wnd.push_back( new MSCalibrationWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( tr("MS Calibration") ), wnd.back() ) );

        wnd.push_back( new MSCalibSpectraWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( tr("MS Calibration(2)") ), wnd.back() ) );

        wnd.push_back( new ChromatogramWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( tr("Chromatogram") ), wnd.back() ) );

        wnd.push_back( new MSPeaksWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( tr("TOF Peaks") ), wnd.back() ) );

        wnd.push_back( new SpectrogramWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( tr("Spectrogram") ), wnd.back() ) );

        wnd.push_back( new MSSpectraWnd );
        stack_->addWidget( boost::apply_visitor( wnd_set_title( tr("Spectra") ), wnd.back() ) );
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

    connect( document::instance(), &document::scanLawChanged, [this] ( double length, double vaccl, double tdelay ) {
        if ( auto w = findChild< adwidgets::MSSimulatorWidget * >() ) {
            w->setTimeSquaredScanLaw( length, vaccl, tdelay );
        }
    } );

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
    if ( widget->windowTitle().isEmpty() ) // avoid QTC_CHECK warning on console
        widget->setWindowTitle( title );
    if ( widget->objectName().isEmpty() )
        widget->setObjectName( pageName );


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

    struct widget { 
        QString title;
        QString pageName;
        std::function<QWidget *()> factory;
        void operator = (const widget& t) {
            title = t.title;
            pageName = t.pageName;
            factory = t.factory;
        }
        widget( const QString& t, const QString& p, std::function<QWidget *()> f ) : title( t ), pageName( p ), factory( f ) {}
    };

    std::vector< widget > widgets = {
        { tr( "Centroid" ), "CentroidMethod", [] (){ return new adwidgets::CentroidForm; } } // should be first
        , { tr( "MS Peaks" ), "MSPeakTable", [] () { return new adwidgets::MSPeakTable; } }
        , { tr( "MS Simulator" ), "MSSimulatorMethod", [] () { return new adwidgets::MSSimulatorWidget; } }
        , { tr( "Targeting" ), "TargetingMethod", [] () { return new adwidgets::TargetingWidget; } }
        , { tr( "MS Chromatogr." ), "MSChromatogrMethod", [] (){ return new adwidgets::MSChromatogramWidget; } }
        , { tr( "Peak Find" ), "PeakFindMethod", [] () { return new adwidgets::PeakMethodForm; } }
        , { tr( "MS Calibration" ), "MSCalibrateWidget", [] () { return new adwidgets::MSCalibrateWidget; } }
        , { tr( "Data property" ), "DataProperty", [] () { return new dataproc::MSPropertyForm; } }
        , { tr( "TOF Peaks" ), "TOFPeaks", [] (){ return new adwidgets::MSPeakWidget; } }
    };

    auto list = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iDataproc >();
    for ( auto v : list ) {
        for ( auto& wf : *v )
            widgets.push_back( widget( wf.title(), wf.objname(), [&wf] (){ return wf(); } ) );
    }

    for ( auto& widget: widgets ) {

        if ( QWidget * pWidget = widget.factory() ) {

            if ( widget.pageName == "TOFPeaks" ) {
                // TOFPeaks
                connect( this, SIGNAL( onAddMSPeaks( const adcontrols::MSPeaks& ) ), pWidget, SLOT( handle_add_mspeaks( const adcontrols::MSPeaks& ) ) );
                adplugin::LifeCycleAccessor accessor( pWidget );
                if ( adplugin::LifeCycle * p = accessor.get() ) {
                    if ( auto wnd = findChild< MSPeaksWnd *>() ) {
                        p->setContents( boost::any( static_cast<QWidget *>(wnd) ) );
                    }
                }
            }

            if ( qobject_cast< adwidgets::CentroidForm * >( pWidget ) == nullptr &&
                 qobject_cast<adwidgets::MSSimulatorWidget *>( pWidget ) == nullptr &&
                 qobject_cast<adwidgets::TargetingWidget *>( pWidget ) == nullptr &&
                 qobject_cast<adwidgets::MSChromatogramWidget *>( pWidget ) == nullptr &&
                 qobject_cast<adwidgets::PeakMethodForm *>( pWidget ) == nullptr &&
                 qobject_cast<adwidgets::MSCalibrateWidget *>( pWidget ) == nullptr &&
                 qobject_cast<dataproc::MSPropertyForm *>( pWidget ) == nullptr &&
                 qobject_cast<adwidgets::MSPeakWidget *>( pWidget ) == nullptr ) {

                // all MSPeakTable variants 
                if ( auto wnd = findChild< MSProcessingWnd *>() ) {
                    connect( pWidget, SIGNAL( currentChanged( int, int ) ), wnd, SLOT( handleCurrentChanged( int, int ) ) ); // idx, fcn
                    connect( pWidget, SIGNAL( formulaChanged( int, int ) ), wnd, SLOT( handleFormulaChanged( int, int ) ) );
                    connect( pWidget, SIGNAL( triggerLockMass( const QVector<QPair<int, int>>& ) ), wnd, SLOT( handleLockMass( const QVector<QPair<int, int>>& ) ) );
                    connect( pWidget, SIGNAL( estimateScanLaw( const QVector<QPair<int, int>>& ) ), wnd, SLOT( handleScanLawEst( const QVector<QPair<int, int>>& ) ) );
                }
                connect( this, SIGNAL( onZoomedOnSpectrum( const QRectF& ) ), pWidget, SLOT( handleZoomedOnSpectrum( const QRectF& ) ) );

                if ( qobject_cast<adwidgets::MSPeakTable *>( pWidget ) == nullptr )
                    connect( this, SIGNAL( onZoomedOnChromatogram( const QRectF& ) ), pWidget, SLOT( handleZoomedOnChromatogram( const QRectF& ) ) );
            }

            if ( qobject_cast< dataproc::MSPropertyForm * >( pWidget ) == nullptr &&
                 qobject_cast< adwidgets::MSPeakTable * >( pWidget ) == nullptr &&
                 qobject_cast< adwidgets::MSPeakWidget * >( pWidget ) == nullptr ) {

                connect( pWidget, SIGNAL( triggerProcess( const QString& ) ), this, SLOT( handleProcess( const QString& ) ) );
            }

            if ( auto p = qobject_cast< adwidgets::PeptideWidget *>( pWidget ) ) {
                connect( p, &adwidgets::PeptideWidget::triggerFind, this, &MainWindow::handlePeptideTarget );
            }

            createDockWidget( pWidget, widget.title, widget.pageName );
        } else {
            QMessageBox::critical(0, QLatin1String("dataprocmanager"), widget.pageName );
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
MainWindow::toolButton( QAction * action, const QString& objectName )
{
    if ( auto button = new QToolButton ) {
        button->setDefaultAction( action );
        if ( !objectName.isEmpty() ) {
            button->setObjectName( objectName );
            button->setCheckable( true );
        }
        return button;
    }
    return 0;
}

// static
QToolButton * 
MainWindow::toolButton( const char * id )
{
    return toolButton( Core::ActionManager::instance()->command(id)->action() );
}

void
MainWindow::zoomedOnSpectrum( const QRectF& rc )
{
    emit onZoomedOnSpectrum( rc );
}

void
MainWindow::handleWarningMessage( const QString& msg )
{
    QMessageBox::warning( this, "Dataproc", msg );
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
	if ( portfolio::Folder folder = folium.parentFolder() ) {

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
        } else if ( folder.name() == L"Chromatograms" ) {
            if ( stack_->currentIndex() != idSelMSProcess )
                selPage( idSelChromatogram );
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
        for ( auto widget: dockWidgets() ) {
            if ( auto pLifeCycle = qobject_cast<adplugin::LifeCycle *>( widget->widget() ) ) {
                pLifeCycle->setContents( boost::any( folium ) );
                pLifeCycle->setContents( boost::any( centroid ) );
                if ( targeting )
                    pLifeCycle->setContents( boost::any( targeting ) );
            }
        }
    }
}

void
MainWindow::selectionChanged( std::shared_ptr< adcontrols::MassSpectrum > centroid, std::function<adwidgets::MSPeakTable::callback_t> f )
{
    // this is for Spectrogram; selected centroid data review on MSPeakTable
    if ( auto pktable = findChild< adwidgets::MSPeakTable * >( "MSPeakTable" ) ) {
        pktable->setContents( centroid, f );
    }
}

void
MainWindow::handleProcess( const QString& origin )
{
    if ( origin == "MSChromatogramWidget" ) {
        // generate chromatograms
        auto pm = std::make_shared< adcontrols::ProcessMethod >();
        getProcessMethod( *pm );
        if ( auto cm = pm->find< adcontrols::MSChromatogramMethod >() ) {
            if ( ! cm->molecules().data().empty() ) {
                if ( auto processor = SessionManager::instance()->getActiveDataprocessor() )
                    DataprocessWorker::instance()->createChromatogramsByMethod( processor, pm, origin );
            }
        }
    } else if ( origin == "MSSimulatorWidget" ) {
        auto pm = std::make_shared< adcontrols::ProcessMethod >();
        getProcessMethod( *pm );
        if ( auto m = pm->find< adcontrols::MSSimulatorMethod >() ) {
            if ( auto wnd = findChild< ElementalCompWnd * >() ) {
                wnd->simulate( *m );
                stack_->setCurrentIndex( idSelElementalComp );                
            }
        }
    } else if ( origin == "MSCalibrateWidget" ) {
        // peak identification, and then compute calibration equation
        auto pm = std::make_shared< adcontrols::ProcessMethod >();
        getProcessMethod( *pm );
        document::instance()->setProcessMethod( *pm );
        if ( auto processor = SessionManager::instance()->getActiveDataprocessor() )
            processor->applyCalibration( *pm );
    } else if ( origin == "TargetingWidget" ) {
        auto pm = std::make_shared< adcontrols::ProcessMethod >();
        getProcessMethod( *pm );
        document::instance()->setProcessMethod( *pm );
        if ( auto processor = SessionManager::instance()->getActiveDataprocessor() )
            processor->applyProcess( *pm, TargetingProcess );
    }
}

void
MainWindow::lockMassHandled( const std::shared_ptr< adcontrols::MassSpectrum >& ptr )
{
    if ( auto pktable = findChild< adwidgets::MSPeakTable * >( "MSPeakTable" ) ) {

        pktable->onUpdate( boost::any( ptr ) );

    }
}

void
MainWindow::dataMayChanged()
{
    if ( auto pktable = findChild< adwidgets::MSPeakTable * >( "MSPeakTable" ) ) {

        pktable->dataMayChanged();

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
		QMessageBox::information( 0, "Dataproc", tr("No data exist for peptide targeting") );
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
        if ( auto lifeCycle = qobject_cast<adplugin::LifeCycle *>( widget->widget() ) )
            lifeCycle->getContents( any );
    }
}

void
MainWindow::setProcessMethod( const adcontrols::ProcessMethod& pm )
{
	for ( auto widget: dockWidgets() ) {
        if ( auto lifeCycle = qobject_cast<adplugin::LifeCycle *>( widget->widget() ) )
            lifeCycle->setContents( boost::any( pm ) );
	}
}

void
MainWindow::OnInitialUpdate()
{
    using adcontrols::ProcessMethod;

    createDockWidgets();

    QList< QDockWidget *> widgets = dockWidgets();
  
    for ( auto widget: widgets ) {
        QWidget * obj = widget->widget();
		adplugin::LifeCycleAccessor accessor( obj );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle )
			pLifeCycle->OnInitialUpdate();
    }

    if ( auto pm = document::instance()->processMethod() ) {
        setProcessMethod( *pm ); // write to UI
        getProcessMethod( *pm ); // read back from UI
    } else {
        adcontrols::ProcessMethod m;
        getProcessMethod( m );
        document::instance()->setProcessMethod( m );
    }
    connect( document::instance(), &document::onProcessMethodChanged, this, &MainWindow::handleProcessMethodChanged );

    setSimpleDockWidgetArrangement();

    currentPageChanged( 0 );

#if defined Q_OS_LINUX
    for ( auto dock: dockWidgets() )
        dock->widget()->setStyleSheet( "* { font-size: 9pt; }" );

    for ( auto tabbar: findChildren< QTabBar * >() )
        tabbar->setStyleSheet( "QTabBar { font-size: 9pt; }" );
#endif
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
                                adfile.append( folium, *processor->file() );
                            }
                        }
                    }
                }
            }
        }
    } while ( 0 );

    if ( handled )
        document::instance()->handle_portfolio_created( filename );
}

void
MainWindow::actionApply()
{
    qtwrapper::waitCursor wait;

    adcontrols::ProcessMethod pm;

    getProcessMethod( pm );  // update by what values GUI holds

    document::instance()->setProcessMethod( pm );

    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
        
        if ( currentFeature_ == CalibrationProcess )
            processor->applyCalibration( pm );
        else
            processor->applyProcess( pm, currentFeature_ );
    }
}

void
MainWindow::applyCalibration( const adcontrols::MSAssignedMasses& assigned )
{
    adcontrols::ProcessMethod pm;
        
    getProcessMethod( pm );

    document::instance()->setProcessMethod( pm );

    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {

        processor->applyCalibration( pm, assigned );

    }
}

void
MainWindow::applyCalibration( const adcontrols::MSAssignedMasses& assigned, portfolio::Folium& folium )
{
    adcontrols::ProcessMethod pm;

    getProcessMethod( pm );
    
    document::instance()->setProcessMethod( pm );

    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {

        processor->applyCalibration( pm, assigned, folium );

    }
}

void
MainWindow::proteinSelected( const adprot::digestedPeptides& digested )
{
    auto docks = dockWidgets();
    auto it = std::find_if( docks.begin(), docks.end(), []( QDockWidget * d ){ return d->objectName() == "PeptideMethod"; });
    if ( it != docks.end() ) {

        (*it)->raise();

        if ( auto t = dynamic_cast< adwidgets::PeptideWidget *>( (*it)->widget() ) )
            t->setContents( boost::any( digested ) );
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

void
MainWindow::getEditorFactories( adextension::iSequenceImpl& impl )
{
    using namespace adextension;// ::iEditorFactory;
    
    if ( auto p = std::make_shared< adextension::iEditorFactoryT< adwidgets::CentroidForm > >( "Centroid",  iEditorFactory::PROCESS_METHOD ) ) {
        impl << p;
    };
    if ( auto p = std::make_shared< adextension::iEditorFactoryT< adwidgets::TargetingWidget > >( "Targeting",  iEditorFactory::PROCESS_METHOD ) ) {
        impl << p;
    };
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
            std::replace( name.begin(), name.end(), '/', '_' );
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

    QString currentFile = Core::DocumentManager::currentFile(); // Core::ICore::instance()->fileManager()->currentFile();
    if ( !currentFile.isEmpty() ) {
        const QFileInfo fi( currentFile );
        dir = fi.absolutePath();
    }
    return dir;
}

void
MainWindow::aboutQtPlatz()
{
    if ( !aboutDlg_ ) {
        aboutDlg_ = new AboutDlg(this);
        connect( aboutDlg_, static_cast<void(AboutDlg::*)(int)>(&AboutDlg::finished), this, [&](){ aboutDlg_->deleteLater(); aboutDlg_ = 0; });
    }
    aboutDlg_->show();
}

void
MainWindow::handleProcessMethodChanged( const QString& filename )
{
    for ( auto& edit : findChildren< QLineEdit * >( Constants::EDIT_PROCMETHOD ) )
        edit->setText( filename );

    if ( auto pm = document::instance()->processMethod() ) {
        setProcessMethod( *pm ); // update UI
    }

}

void
MainWindow::hideDock( bool hide )
{
    for ( auto& w :  dockWidgets() ) {
        if ( hide )
            w->hide();
        else
            w->show();
    }
}
