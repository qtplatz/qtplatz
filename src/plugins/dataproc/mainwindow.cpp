/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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
//#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "aboutdlg.hpp"
//#endif
#include "chromatogramwnd.hpp"
#include "document.hpp"
#include "dataprocconstants.hpp"

#include "dataprocessor.hpp"
#include "dataprocessworker.hpp"
#include "dataprocplugin.hpp"
// #include "dataprocfactory.hpp"
#include "elementalcompwnd.hpp"
#include "filepropertywidget.hpp"
#include "mspeaktable.hpp"

#include <adwidgets/mspeaktree.hpp>
#include <qtwrapper/make_widget.hpp>
#include <qtwrapper/settings.hpp>

#include "msprocessingwnd.hpp"
#include "mscalibrationwnd.hpp"
#include "mspeakswnd.hpp"
#include "msspectrawnd.hpp"
#include "mspropertyform.hpp"
#include "peaklist_export.hpp"
#include "rms_export.hpp"
#include "sessionmanager.hpp"
#include "contourwnd.hpp"

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
#include <adlog/logger.hpp>
#include <admtwidgets/moltablewidget.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/utf.hpp>
#include <adprocessor/processmediator.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptide.hpp>
#include <adprot/peptides.hpp>
#include <adutils/adfile.hpp>
#include <adwidgets/centroidwidget.hpp>
#include <adwidgets/lifecycle.hpp>
#include <adwidgets/peptidewidget.hpp>
#include <adwidgets/targetingwidget.hpp>
#include <adwidgets/mscalibratewidget.hpp>
#include <adwidgets/mschromatogramwidget.hpp>
#include <adwidgets/peakmethodform.hpp>
#include <adwidgets/mspeakwidget.hpp>
#include <adwidgets/mssimulatorwidget.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <qtwrapper/jsonhelper.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/plugin_manager.hpp>
#include <extensionsystem/pluginmanager.h>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>
#include <boost/optional/optional_io.hpp>

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

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>
#include <QtGui/QIcon>
#include <qdebug.h>
#include <qstackedwidget.h>
#include <boost/json.hpp>
#include <functional>
#include <fstream>

namespace dataproc {

    typedef boost::variant< MSProcessingWnd*, ElementalCompWnd*, MSCalibrationWnd* //, MSCalibSpectraWnd*
                            , ChromatogramWnd*, MSPeaksWnd*, ContourWnd*, MSSpectraWnd* > wnd_ptr_t;

}

using namespace dataproc;

namespace {
    // __cplusplus >= 201703L
    template<typename _Ty,  typename ... _Types>
    inline _Ty * make_stack_widget( QStackedWidget * stack, QString&& title, _Types&&... _Args )
    {
        if ( auto w = new _Ty( std::forward<_Types>(_Args)...) ) {
            w->setWindowTitle( title );
            stack->addWidget( w );
            return w;
        }
        return nullptr;
    }
}

namespace {
    // #if  __cplusplus >= 201703L
    template< typename ...Args > void scaleSpectrumConnector( MainWindow * w, QObject * p ) {
        ( QObject::connect( w, &MainWindow::onScaleYChanged, p->findChild<Args *>(), &Args::handleSpectrumYScale ) , ... );
    }

    template< typename ...Args > void scaleChromatogramConnector( MainWindow * w, QObject * p ) {
        ( QObject::connect( w, &MainWindow::onScaleChromatogramYChanged, p->findChild<Args *>(), &Args::handleChromatogramYScale ),...);
        ( QObject::connect( w, &MainWindow::onScaleChromatogramXChanged, p->findChild<Args *>(), &Args::handleChromatogramXScale ),...);
    }

    template< typename ...Args> void sessionAddedConnector( QObject * p ) {
        ( QObject::connect( SessionManager::instance(), &SessionManager::onSessionAdded
                            , p->findChild< Args *>(), &Args::handleSessionAdded ), ...);
    }

    template< typename ...Args> void removeSessionConnector( QObject * p ) {
        ( QObject::connect( SessionManager::instance(), &SessionManager::onRemoveSession
                            , p->findChild< Args *>(), &Args::handleRemoveSession ), ...);
    }

    template< typename ...Args> void sessionRemovedConnector( QObject * p ) {
        ( QObject::connect( SessionManager::instance(), &SessionManager::onSessionRemoved
                            , p->findChild< Args *>(), &Args::handleSessionRemoved ), ...);
    }

    template< typename ...Args> void selectionChangedConnector( QObject * p ) {
        ( QObject::connect( SessionManager::instance(), &SessionManager::signalSelectionChanged
                            , p->findChild< Args * >(), &Args::handleSelectionChanged ), ...);
    }

    template< typename ...Args> void processedConnector( QObject * p ) {
        ( QObject::connect( SessionManager::instance(), &SessionManager::onProcessed
                            , p->findChild< Args * >()
                            , &Args::handleProcessed ), ...);
    }

    template< typename ...Args> void applyMethodConnector( QObject * p ) {
        ( QObject::connect( DataprocPlugin::instance(), &DataprocPlugin::onApplyMethod
                            , p->findChild< Args * >()
                            , &Args::handleApplyMethod ), ...);
    }

    template< typename ...Args> void checkStateChangedConnector( QObject * p ) {
        ( QObject::connect( SessionManager::instance(), &SessionManager::signalCheckStateChanged
                            , p->findChild< Args * >()
                            , &Args::handleCheckStateChanged ), ...);
    }

    template< typename ...Args> void axisChangedConnector( QObject * p, QComboBox * choice ) {
        ( QObject::connect( choice, qOverload< int >( &QComboBox::currentIndexChanged )
                            , p->findChild< Args * >()
                            , [=](int id){
                                if ( auto w = p->findChild< Args * >() )
                                    w->handleAxisChanged( id == 0 ? adcontrols::hor_axis_mass : adcontrols::hor_axis_time );}), ...);
    }
}

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
        toolBarLayout->setContentsMargins( {} );
        toolBarLayout->setSpacing( 2 );
        Core::ActionManager * am = Core::ActionManager::instance();
        if ( am ) {

            Core::Context context( ( Utils::Id( Constants::C_DATAPROCESSOR ) ) );

            if ( auto p = new QAction( tr("MS Process"), this ) ) {
                connect( p, &QAction::triggered, [&](){ stack_->setCurrentIndex( idSelMSProcess ); } );
                am->registerAction( p, "dataproc.selMSProcess", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelMSProcess ) ) );
            }
            if ( auto p = new QAction( tr("Spectra"), this ) ) {
                connect( p, &QAction::triggered, [&](){ stack_->setCurrentIndex( idSelSpectra ); } );
                am->registerAction( p, "dataproc.selSpectra", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelSpectra ) ) );
            }
            if ( auto p = new QAction( tr("Simulation"), this ) ) {
                connect( p, &QAction::triggered, [&](){ stack_->setCurrentIndex( idSelElementalComp ); } );
                am->registerAction( p, "dataproc.selElementalComp", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelElementalComp ) ) );
            }
            if ( auto p = new QAction( tr("MS Calibration"), this ) ) {
                connect( p, &QAction::triggered, [&](){ stack_->setCurrentIndex( idSelMSCalibration ); } );
                am->registerAction( p, "dataproc.selMSCalibration", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelMSCalibration ) ) );
            }

            if ( auto p = new QAction( tr("Chromatogram"), this ) ) {
                connect( p, &QAction::triggered, [&](){ stack_->setCurrentIndex( idSelChromatogram ); } );
                am->registerAction( p, "dataproc.selChromatogram", context );
                toolBarLayout->addWidget( toolButton( p, QString("wnd.%1").arg( idSelChromatogram ) ) );
            }

            if ( auto p = new QAction( tr("Contour"), this ) ) {
                connect( p, &QAction::triggered, [&](){ stack_->setCurrentIndex( idSelSpectrogram ); } );
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

        // -- axis y-scale
        constexpr int i = 1;
        if ( auto cb = qtwrapper::make_widget< QCheckBox >( QString("cbY%1" ).arg(i), "Y-Auto" ) ) {
            cb->setCheckState( Qt::Checked ); // defalut start with auto
            toolBarLayout->addWidget( cb );
            connect( cb, &QCheckBox::toggled, [&](bool checked){ handleScaleYChanged( i ); } );
        }

        if ( auto sp = qtwrapper::make_widget< QDoubleSpinBox >( (boost::format( "spB%1%" ) % i ).str().c_str() ) ) {
            sp->setRange( -200000.0, 200000.0 );
            sp->setDecimals( 3 );
            sp->setSingleStep( 0.1 );
            toolBarLayout->addWidget( sp );
            connect( sp, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double){ handleScaleYChanged( 1 ); } );
        }

        if ( auto sp = qtwrapper::make_widget< QDoubleSpinBox >( (boost::format( "spH%1%" ) % i ).str().c_str() ) ) {
            sp->setRange( -200000.0, 200000.0 );
            sp->setDecimals( 3 );
            sp->setSingleStep( 0.1 );
            toolBarLayout->addWidget( sp );
            connect( sp, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double){ handleScaleYChanged( 1 ); } );
        }
        // <---- y-scale
        // toolBarLayout->addWidget( new QLabel( tr("Sequence:") ) );
        // toolBarLayout->addWidget( new QLineEdit );
    }
    return toolBar;
}

void
MainWindow::handleDataprocessor( Dataprocessor * dp )
{
    auto sp = ( dp ) ? dp->massSpectrometer() : nullptr;
    if ( auto w = findChild< adwidgets::MSSimulatorWidget * >( "MSSimulatorMethod" ) ) {
        w->setMassSpectrometer( sp );
    }
    if ( auto w = findChild< dataproc::MSPeakTable * >( "MSPeakTable" ) ) {
        w->setMassSpectrometer( sp );
    }
}

void
MainWindow::handleScaleYChanged( int i )
{
    auto cb = findChild< QCheckBox * >( QString( "cbY%1" ).arg( i ) );
    auto height = findChild< QDoubleSpinBox * >( QString( "spH%1" ).arg( i ) );
    auto bottom = findChild< QDoubleSpinBox * >( QString( "spB%1" ).arg( i ) );
    emit onScaleYChanged( cb->isChecked(), bottom->value(), height->value() );

    QJsonDocument doc( QJsonObject{{"autoY", cb->isChecked()}, {"bottom", bottom->value()}, {"height", height->value()}} );
    document::instance()->settings()->setValue( QString( "MainWindow/axes/%1" ).arg( i ), QString(doc.toJson()) );
}

void
MainWindow::handleScaleY2Changed( int r, int c )
{
    if ( auto cb = findChild< QCheckBox * >( QString( "cb%1Y%2" ).arg(r).arg(c) ) ) {
        if ( auto height = findChild< QDoubleSpinBox * >( QString( "sp%1H%2" ).arg(r).arg(c) ) ) {
            if ( auto bottom = findChild< QDoubleSpinBox * >( QString( "sp%1B%2" ).arg(r).arg(c) ) ) {
                emit onScaleChromatogramYChanged( cb->isChecked(), bottom->value(), height->value() );

                QJsonDocument doc( QJsonObject{{"autoY", cb->isChecked()}, {"bottom", bottom->value()}, {"height", height->value()}} );
                document::instance()->settings()->setValue( QString( "MainWindow/axes%1Y/%2" ).arg(r).arg(c), QString(doc.toJson()) );
            } else {
                ADDEBUG() << QString( "sp%1B%2" ).arg( r ).arg( c ).toStdString() << " ---- not found";
            }
        } else {
            ADDEBUG() << QString( "sp%1H%2" ).arg( r ).arg( c ).toStdString() << " ---- not found";
        }
    } else {
        ADDEBUG() << "## " << __FUNCTION__ << " ## " << QString( "cb%1Y%2 -- not found" ).arg( r, c ).toStdString() << std::make_pair( r, c );
    }
}

void
MainWindow::handleScaleX2Changed( int r, int c )
{
    // ADDEBUG() << "### X2 " << __FUNCTION__ << "r,c=" << std::make_pair( r, c );
    if ( auto cb = findChild< QCheckBox * >( QString( "cb%1X%2" ).arg(r).arg(c) ) ) {
        if ( auto left = findChild< QDoubleSpinBox * >( QString( "sp%1L%2" ).arg(r).arg(c) ) ) {
            if ( auto right = findChild< QDoubleSpinBox * >( QString( "sp%1R%2" ).arg(r).arg(c) ) ) {
                emit onScaleChromatogramXChanged( cb->isChecked(), left->value(), right->value() );
                QJsonDocument doc( QJsonObject{{"autoX", cb->isChecked()}, {"left", left->value()}, {"right", right->value()}} );
                document::instance()->settings()->setValue( QString( "MainWindow/axes%1X/%2" ).arg(r).arg(c), QString(doc.toJson()) );
            } else {
                ADDEBUG() << QString( "sp%1L%2" ).arg(r).arg(c).toStdString() << " ---- not found";
            }
        } else {
            ADDEBUG() << QString( "sp%1R%2" ).arg(r).arg(c).toStdString() << " ---- not found";
        }
    } else {
        ADDEBUG() << "## " << __FUNCTION__ << " ## " << QString( "cb%1X%2 -- not found" ).arg(r).arg(c).toStdString() << std::make_pair( r, c );
    }
}

void
MainWindow::loadSettings()
{
    do {
        constexpr int i = 1;
        auto axes = QJsonDocument::fromJson( document::instance()->settings()->value( QString("MainWindow/axes/%1").arg(i), "{}").toByteArray() ).object();
        if ( ! axes.isEmpty() ) {
            if ( auto autoY = qtwrapper::JsonHelper::value<bool>( axes, "autoY" ) )
                ; // ignore settings (always auto Y scale)

            if ( auto height = qtwrapper::JsonHelper::value< double >( axes, "height" ) ) {
                if ( auto sbox = findChild< QDoubleSpinBox * >( QString("spH%1").arg(i) ) )
                    sbox->setValue( *height );
            }

            if ( auto bottom = qtwrapper::JsonHelper::value< double >( axes, "bottom" ) ) {
                if ( auto sbox = findChild< QDoubleSpinBox * >( QString("spB%1").arg(i) ) )
                    sbox->setValue( *bottom );
            }
        }
    } while ( 0 );

    constexpr int r = 2, c = 1;
    do {
        auto axesY = QJsonDocument::fromJson(
            document::instance()->settings()->value( QString("MainWindow/axes%1Y/%2").arg(r).arg(c), "{}").toByteArray() ).object();
        if ( ! axesY.isEmpty() ) {
            if ( auto autoY = qtwrapper::JsonHelper::value<bool>( axesY, "autoY" ) )
                ; // ignore settings (always auto Y scale)
            if ( auto height = qtwrapper::JsonHelper::value< double >( axesY, "height" ) ) {
                if ( auto sbox = findChild< QDoubleSpinBox * >( QString("sp%1H%2").arg(r).arg(c) ) )
                    sbox->setValue( *height );
            }

            if ( auto bottom = qtwrapper::JsonHelper::value< double >( axesY, "bottom" ) ) {
                if ( auto sbox = findChild< QDoubleSpinBox * >( QString("sp%1B%2").arg(r).arg(c) ) )
                    sbox->setValue( *bottom );
            }
        }
    } while ( 0 );
    do {
        auto axesX = QJsonDocument::fromJson(
            document::instance()->settings()->value( QString("MainWindow/axes%1X/%2").arg(r).arg(c), "{}").toByteArray() ).object();
        if ( ! axesX.isEmpty() ) {
            if ( auto autoY = qtwrapper::JsonHelper::value<bool>( axesX, "autoY" ) )
                ; // ignore settings (always auto Y scale)
            if ( auto left = qtwrapper::JsonHelper::value< double >( axesX, "left" ) ) {
                if ( auto sbox = findChild< QDoubleSpinBox * >( QString("sp%1L%2").arg(r).arg(c) ) )
                    sbox->setValue( *left );
            }

            if ( auto right = qtwrapper::JsonHelper::value< double >( axesX, "right" ) ) {
                if ( auto sbox = findChild< QDoubleSpinBox * >( QString("sp%1R%2").arg(r).arg(c) ) )
                    sbox->setValue( *right );
            }
        }
    } while ( 0 );
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

    QRegularExpression reg( "wnd\\.[0-9]+" );
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
        toolBarLayout->setContentsMargins( {} );
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
            features->addItem( tr("MS Chromatogr") );
            features->addItem( tr("Find peaks") );
            features->addItem( tr("MS Calibration") );
            toolBarLayout->addWidget( features );

            connect( features, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::handleFeatureSelected );
            connect( features, qOverload<int>(&QComboBox::activated), this, &MainWindow::handleFeatureActivated );
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

            constexpr int r = 2, c = 1;
            if ( auto cb = qtwrapper::make_widget< QCheckBox >( QString( "cb%1Y%2" ).arg(r).arg(c), "Y-Auto" ) ) {
                cb->setCheckState( Qt::Checked ); // defalut start with auto
                toolBarLayout->addWidget( cb );
                connect( cb, &QCheckBox::toggled, [&](bool checked){ handleScaleY2Changed( r, c ); } );
            }

            if ( auto sp = qtwrapper::make_widget< QDoubleSpinBox >( QString( "sp%1B%2" ).arg(r).arg(c) ) ) {
                sp->setRange( -200000.0, 200000.0 );
                sp->setDecimals( 3 );
                sp->setSingleStep( 0.1 );
                toolBarLayout->addWidget( sp );
                connect( sp, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double){ handleScaleY2Changed( r, c ); } );
            }

            if ( auto sp = qtwrapper::make_widget< QDoubleSpinBox >( QString( "sp%1H%2" ).arg(r).arg(c) ) ) {
                sp->setRange( -200000.0, 200000.0 );
                sp->setDecimals( 3 );
                sp->setSingleStep( 0.1 );
                toolBarLayout->addWidget( sp );
                connect( sp, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double){ handleScaleY2Changed( r, c ); } );
            }

            toolBarLayout->addWidget( new Utils::StyledSeparator );
            if ( auto cb = qtwrapper::make_widget< QCheckBox >( QString("cb%1X%2").arg(r).arg(c), "X-Auto" ) ) {
                cb->setCheckState( Qt::Checked ); // defalut start with auto
                toolBarLayout->addWidget( cb );
                connect( cb, &QCheckBox::toggled, [&](bool checked){ handleScaleX2Changed( r, c ); } );
            }
            if ( auto sp = qtwrapper::make_widget< QDoubleSpinBox >( QString( "sp%1L%2" ).arg(r).arg(c) ) ) {
                sp->setRange( -100.0, 9999.0 );
                sp->setDecimals( 0 );
                sp->setSingleStep( 1 );
                toolBarLayout->addWidget( sp );
                connect( sp, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double){ handleScaleX2Changed( r, c ); } );
            }
            if ( auto sp = qtwrapper::make_widget< QDoubleSpinBox >( QString( "sp%1R%2" ).arg(r).arg(c) ) ) {
                sp->setRange( 0.0, 9999.0 );
                sp->setDecimals( 0 );
                sp->setSingleStep( 1 );
                toolBarLayout->addWidget( sp );
                connect( sp, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double){ handleScaleX2Changed( r, c); } );
            }
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

        if ( auto pWnd = make_stack_widget< MSProcessingWnd >( stack_, tr( "MS Process" ) ) ) {
            wnd.emplace_back( pWnd );
            connect( this, &MainWindow::onDataMayCanged, pWnd, &MSProcessingWnd::handleDataMayChanged );
        }
        if ( auto pWnd = make_stack_widget< ElementalCompWnd >( stack_, tr("Elemental Comp.") ) ) {
            wnd.emplace_back( pWnd );
        }
        if ( auto pWnd = make_stack_widget< MSCalibrationWnd >( stack_, tr("MS Calibration") ) ) {
            wnd.emplace_back( pWnd );
        }
        if ( auto pWnd = make_stack_widget< ChromatogramWnd >( stack_, tr("Chromatogram") ) ) {
            wnd.emplace_back( pWnd );
        }
        if ( auto pWnd = make_stack_widget< MSPeaksWnd >( stack_, tr("TOF Peaks") ) ) {
            wnd.emplace_back( new MSPeaksWnd );
        }
        if ( auto pWnd = make_stack_widget< ContourWnd >( stack_, tr("Contour") ) ) {
            wnd.emplace_back( new ContourWnd );
        }
        if ( auto pWnd = make_stack_widget< MSSpectraWnd >( stack_, tr("Spectra") ) ) {
            wnd.emplace_back( new MSSpectraWnd );
        }
    }

    if ( auto pSrc = stack_->widget( idSelMSProcess ) ) {
        if ( auto pDst = stack_->widget( idSelSpectra ) )
            connect( dynamic_cast<MSProcessingWnd *>(pSrc), &MSProcessingWnd::dataChanged, dynamic_cast<MSSpectraWnd *>(pDst), &MSSpectraWnd::onDataChanged );
    }

    connect( SessionManager::instance(), &SessionManager::onDataprocessorChanged, this, &MainWindow::handleDataprocessor );
    connect( SessionManager::instance(), &SessionManager::onSessionAdded, this, &MainWindow::handleSessionAdded );
    connect( SessionManager::instance(), &SessionManager::onProcessed, this, &MainWindow::handleProcessed );

    // The handleSelectionChanged on MainWindow should be called in advance for all stacked child widgets.
    // This is significantly important for child widget has right QRectF for each QwtPlot.
    connect( SessionManager::instance(), &SessionManager::signalSelectionChanged, this, &MainWindow::handleSelectionChanged );

    connect( document::instance(), &document::scanLawChanged, [this] ( double length, double vaccl, double tdelay ) {
        if ( auto w = findChild< adwidgets::MSSimulatorWidget * >() ) {
            w->setTimeSquaredScanLaw( length, vaccl, tdelay );
        }
    } );
    // #if __cplusplus >= 201703L
    sessionAddedConnector    < MSProcessingWnd, ElementalCompWnd, MSCalibrationWnd, ChromatogramWnd, MSPeaksWnd, ContourWnd, MSSpectraWnd >( stack_ );
    removeSessionConnector   < MSProcessingWnd, MSSpectraWnd, ElementalCompWnd, ChromatogramWnd >( stack_ );
    sessionRemovedConnector  < ChromatogramWnd >( stack_ );
    selectionChangedConnector< MSProcessingWnd, ElementalCompWnd, MSCalibrationWnd, ChromatogramWnd, MSPeaksWnd, ContourWnd, MSSpectraWnd >( stack_ );
    processedConnector       < MSProcessingWnd, ElementalCompWnd, MSCalibrationWnd, ChromatogramWnd, MSPeaksWnd, ContourWnd, MSSpectraWnd >( stack_ );
    applyMethodConnector     < MSProcessingWnd, ElementalCompWnd, MSCalibrationWnd, ChromatogramWnd, MSPeaksWnd, ContourWnd, MSSpectraWnd >( stack_ );
    checkStateChangedConnector<MSProcessingWnd, MSPeaksWnd, ContourWnd, MSSpectraWnd, ChromatogramWnd >( stack_ );
    axisChangedConnector     < MSProcessingWnd, ElementalCompWnd, MSCalibrationWnd, MSSpectraWnd >( stack_, axisChoice_ );
    scaleChromatogramConnector < MSProcessingWnd, ChromatogramWnd, ContourWnd >( this, stack_ );
    scaleSpectrumConnector   < MSProcessingWnd, MSSpectraWnd >( this, stack_ );
    // #endif
    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setContentsMargins( {} );
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( toolBar1 );  // top most toolbar
    toolBarAddingLayout->addWidget( splitter3 ); // Spectra|chrmatogram pane
    toolBarAddingLayout->addWidget( toolBar2 );  // middle toolbar

    // Right-side window with editor, output etc.
    Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter;
#if QTC_VERSION <= 0x03'02'81
    QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
#else
    QWidget * outputPane = new Core::OutputPanePlaceHolder( mode->id(), mainWindowSplitter );
#endif
    outputPane->setObjectName( QLatin1String( "OutputPanePlaceHolder" ) );
    mainWindowSplitter->addWidget( this );
    mainWindowSplitter->addWidget( outputPane );
    mainWindowSplitter->setStretchFactor( 0, 10 );
    mainWindowSplitter->setStretchFactor( 1, 0 );
    mainWindowSplitter->setOrientation( Qt::Vertical );

    // Navigation and right-side window
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
#if QTC_VERSION <= 0x03'02'81
    splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
#else
    splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode->id(), Core::Side::Left) );
#endif
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
        if ( npos++ >= 1 )
            tabifyDockWidget( widgets[0], widget );
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
    dockWidget->titleBarWidget()->setHidden(true);
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
        { tr( "Centroid" ),         "CentroidMethod",    [] (){ return new adwidgets::CentroidWidget; } }
        , { tr( "MS Peaks" ),       "MSPeakTable",       [] (){ return new dataproc::MSPeakTable; } }
        , { tr( "MS Simulator" ),   "MSSimulatorMethod", [] (){ return new adwidgets::MSSimulatorWidget; } }
        , { tr( "Targeting" ),      "TargetingMethod",   [] (){ return new adwidgets::TargetingWidget; } }
        , { tr( "MS Chromatogr." ), "MSChromatogrMethod",[] (){ return new adwidgets::MSChromatogramWidget; } }
        , { tr( "Peak Find" ),      "PeakFindMethod",    [] (){ return new adwidgets::PeakMethodForm; } }
        , { tr( "MS Calibration" ), "MSCalibrateWidget", [] (){ return new adwidgets::MSCalibrateWidget; } }
        , { tr( "Data property" ),  "DataProperty",      [] (){ return new dataproc::MSPropertyForm; } }
    };
#if QTC_VERSION <= 0x03'02'81
    auto list = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iDataproc >();
#else
    auto list = qtwrapper::plugin_manager::getObjects< adextension::iDataproc >();
#endif
    for ( auto v : list ) {
        for ( auto& wf : *v )
            widgets.push_back( widget( wf.title(), wf.objname(), [&wf] (){ return wf(); } ) );
    }

    for ( auto& widget: widgets ) {

        if ( QWidget * pWidget = widget.factory() ) {

            if ( widget.pageName == "TOFPeaks" ) {
                // TOFPeaks
                connect( this, SIGNAL( onAddMSPeaks( const adcontrols::MSPeaks& ) )
                         , pWidget, SLOT( handle_add_mspeaks( const adcontrols::MSPeaks& ) ) );

                if ( auto p = qobject_cast< adplugin::LifeCycle * >( pWidget ) ) {
                    if ( auto wnd = findChild< MSPeaksWnd *>() ) {
                        p->setContents( boost::any( static_cast<QWidget *>(wnd) ) );
                    }
                }
            }

            if ( qobject_cast< adwidgets::CentroidWidget * >( pWidget ) == nullptr &&
                 qobject_cast< adwidgets::MSSimulatorWidget *>( pWidget ) == nullptr &&
                 qobject_cast< adwidgets::TargetingWidget *>( pWidget ) == nullptr &&
                 qobject_cast< adwidgets::MSChromatogramWidget *>( pWidget ) == nullptr &&
                 qobject_cast< adwidgets::PeakMethodForm *>( pWidget ) == nullptr &&
                 qobject_cast< adwidgets::MSCalibrateWidget *>( pWidget ) == nullptr &&
                 qobject_cast< dataproc::MSPropertyForm *>( pWidget ) == nullptr &&
                 qobject_cast< adwidgets::MSPeakWidget *>( pWidget ) == nullptr ) {

                // all MSPeakTable variants
                if ( auto wnd = findChild< MSProcessingWnd *>() ) {
                    connect( pWidget, SIGNAL( currentChanged( int, int ) ), wnd, SLOT( handleCurrentChanged( int, int ) ) ); // idx, fcn
                    connect( pWidget, SIGNAL( formulaChanged( int, int ) ), wnd, SLOT( handleFormulaChanged( int, int ) ) );
                    connect( pWidget, SIGNAL( modeChanged( int, int, int ) ), wnd, SLOT( handleModeChanged( int, int, int ) ) );
                    connect( pWidget, SIGNAL( triggerLockMass( const QVector<QPair<int, int>>& ) )
                             , wnd, SLOT( handleLockMass( const QVector<QPair<int, int>>& ) ) );
                    connect( pWidget, SIGNAL( estimateScanLaw( const QVector<QPair<int, int>>& ) )
                             , wnd, SLOT( handleScanLawEst( const QVector<QPair<int, int>>& ) ) );
                    connect( pWidget, SIGNAL( rescaleY( int ) ), wnd, SLOT( handleRescaleY( int ) ) ); // fcn (a.k.a protocol)
                }
                connect( this, SIGNAL( onZoomedOnSpectrum( const QRectF&, int ) ), pWidget, SLOT( handleZoomedOnSpectrum( const QRectF&, int ) ) );

                if ( qobject_cast<adwidgets::MSPeakTable *>( pWidget ) == nullptr )
                    connect( this, SIGNAL( onZoomedOnChromatogram( const QRectF& ) )
                             , pWidget, SLOT( handleZoomedOnChromatogram( const QRectF& ) ) );
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
    hbox->setContentsMargins( {} );
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
MainWindow::zoomedOnSpectrum( const QRectF& rc, adcontrols::hor_axis axis )
{
    emit onZoomedOnSpectrum( rc, int(axis) );
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
            if ( stack_->currentIndex() != idSelMSCalibration ) // && stack_->currentIndex() != idSelMSCalibSpectra )
                selPage( idSelMSCalibration );
        } else if ( folder.name() == L"Spectra" ) {
            if ( stack_->currentIndex() != idSelMSProcess &&
                 stack_->currentIndex() != idSelElementalComp &&  stack_->currentIndex() != idSelSpectra )
                selPage( idSelMSProcess );
        } else if ( folder.name() == L"Spectrograms" ) { // Contours
            if ( stack_->currentIndex() != idSelSpectrogram )
                selPage( idSelSpectrogram );
        } else if ( folder.name() == L"Contours" ) { // Contours
            if ( stack_->currentIndex() != idSelSpectrogram )
                selPage( idSelSpectrogram );
        } else if ( folder.name() == L"Chromatograms" ) {
            if ( stack_->currentIndex() != idSelMSProcess )
                selPage( idSelChromatogram );
        }

        adcontrols::MassSpectrumPtr centroid;
        adcontrols::TargetingPtr targeting;
        adcontrols::MSPeakInfoPtr pkinfo;
        std::string dataSource;

        const auto docks = dockWidgets();

        if ( folder.name() == L"Spectra" ) {

            if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium.data() ) ) {
                if ( auto f = portfolio::find_first_of( folium.attachments(), []( auto& a ){ return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
					try {
						centroid = portfolio::get< adcontrols::MassSpectrumPtr >( f );
					} catch ( boost::bad_any_cast& ex ) {
						ADERROR() << boost::diagnostic_information( ex );
					}

                    if ( auto t = portfolio::find_first_of( f.attachments(), []( auto& a){ return portfolio::is_type< adcontrols::MSPeakInfoPtr >( a ); }) ) {
                        try {
                            pkinfo = portfolio::get< adcontrols::MSPeakInfoPtr >( t );
                        } catch ( boost::bad_any_cast& ex ) {
                            ADERROR() << boost::diagnostic_information( ex );
                        }
                    }

                    if ( auto t = portfolio::find_first_of( f.attachments(), []( auto& a) { return a.name() == Constants::F_TARGETING;}) ) {
                        try {
                            targeting = portfolio::get< adcontrols::TargetingPtr >( t );
                        } catch ( boost::bad_any_cast& ex ) {
                            ADERROR() << boost::diagnostic_information( ex );
                        }
                    }
                } else {
                    centroid = std::make_shared< adcontrols::MassSpectrum >();  // empty data for clear table
                }

                if ( auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) { // for ms-simulator, added 2021-02-27
                    auto json = folium.attributes_json();
                    std::for_each( docks.begin(), docks.end(), [=]( auto dock ){
                        if ( auto pLifeCycle = qobject_cast< adplugin::LifeCycle * >( dock->widget() ) )
                            pLifeCycle->setContents( boost::any( profile ), json );
                    });
                }
            }

			auto it = std::find_if( docks.begin(), docks.end(), []( QDockWidget * d ){	return d->objectName() == "MSPeakTable"; });
			if ( it != docks.end() )
				(*it)->raise();

            for ( auto widget: dockWidgets() ) {
                if ( auto pLifeCycle = qobject_cast<adplugin::LifeCycle *>( widget->widget() ) ) {
                    if ( centroid && pkinfo ) {
                        pLifeCycle->setContents( boost::any( std::make_pair( centroid, pkinfo ) ) );
                    } else {
                        if ( centroid )
                            pLifeCycle->setContents( boost::any( centroid ) );
                        if ( pkinfo )
                            pLifeCycle->setContents( boost::any( pkinfo ) );
                    }
                    if ( targeting ) {
                        pLifeCycle->setContents( boost::any( targeting ) );
                    }
                    pLifeCycle->setContents( boost::any( folium ) );
                }
            }

            if ( auto tree = findChild< adwidgets::MSPeakTree * >( "TargetingTree" ) ) {
                if ( pkinfo )
                    tree->setContents( { pkinfo, centroid, targeting } );
                else
                    tree->setContents( { centroid, targeting } );
            }
        }
    }
}

void
MainWindow::repaintTable()
{
}

void
MainWindow::selectionChanged( std::shared_ptr< adcontrols::MassSpectrum > centroid, std::function<adwidgets::MSPeakTable::callback_t> f )
{
    // this is for Spectrogram; selected centroid data review on MSPeakTable
    if ( auto pktable = findChild< adwidgets::MSPeakTable * >( "MSPeakTable" ) ) {
        pktable->setCentroidSpectrum( centroid, f );
    }
}

void
MainWindow::handleProcess( const QString& origin )
{
    auto pm = processMethod();
    document::instance()->setProcessMethod( *pm );

    if ( origin == "MSChromatogramWidget" ) {
        // generate chromatograms
        if ( auto cm = pm->find< adcontrols::MSChromatogramMethod >() ) {
            if ( cm->enableAutoTargeting() ) { // check tR & protocol
                for ( const auto& mol: cm->molecules().data() ) {
                    if ( mol.enable() &&  mol.mass() > 0 && !mol.tR() ) {
                        QMessageBox::critical(0, QLatin1String("MS Chromatograms"), "Set tR and protocol# for auto-targeting" );
                        return;
                    }
                }
            }
            if ( ! cm->molecules().data().empty() ) {
                if ( auto processor = SessionManager::instance()->getActiveDataprocessor() )
                    DataprocessWorker::instance()->createChromatogramsByMethod( processor, pm, origin );
            } else {
                QMessageBox::critical(0, QLatin1String("MS Chromatograms"), "No molecule" );
            }
        }
    } else if ( origin == "MSSimulatorWidget" ) {
        if ( auto w = findChild< adwidgets::MSSimulatorWidget * >( "MSSimulatorMethod" ) ) {
            if ( auto wnd = findChild< ElementalCompWnd * >() ) {
                wnd->setSimulatedSpectrum( w->massSpectrum() );
                stack_->setCurrentIndex( idSelElementalComp );
            }
        }
    } else if ( origin == "MSCalibrateWidget" ) {
        // peak identification, and then compute calibration equation
        if ( auto processor = SessionManager::instance()->getActiveDataprocessor() )
            processor->applyCalibration( *pm );
    } else if ( origin == "TargetingWidget" ) {
        if ( auto processor = SessionManager::instance()->getActiveDataprocessor() )
            processor->applyProcess( *pm, TargetingProcess );
    } else if ( origin == "PeakFind" ) {
        if ( auto processor = SessionManager::instance()->getActiveDataprocessor() ) {
            processor->applyProcess( *pm, PeakFindProcess );
        }
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

std::shared_ptr< adcontrols::ProcessMethod >
MainWindow::processMethod() const
{
    auto pm = std::make_shared< adcontrols::ProcessMethod >();
    getProcessMethod( *pm );
    return pm;
}

void
MainWindow::getProcessMethod( adcontrols::ProcessMethod& pm ) const
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
		if ( auto pLifeCycle = qobject_cast< adplugin::LifeCycle * >( obj ) )
			pLifeCycle->OnInitialUpdate();
    }

    if ( auto wnd = findChild< MSProcessingWnd * >() )
        wnd->onInitialUpdate();

    if ( auto wnd = findChild< MSSpectraWnd * >() )
        wnd->onInitialUpdate();

    if ( auto wnd = findChild< ElementalCompWnd * >() )
        wnd->onInitialUpdate();

    if ( auto pm = document::instance()->processMethod() ) {
        setProcessMethod( *pm ); // write to UI
        getProcessMethod( *pm ); // read back from UI
    } else {
        adcontrols::ProcessMethod m;
        getProcessMethod( m );
        document::instance()->setProcessMethod( m );
    }
    connect( document::instance(), &document::onProcessMethodChanged, this, &MainWindow::handleProcessMethodChanged );
    connect( document::instance(), &document::onMergeSelection, this, &MainWindow::handleMergeSelection );

    adprocessor::ProcessMediator::instance()->registerProcessMethodProvider( [this]( adcontrols::ProcessMethod& pm ){
            getProcessMethod( pm );
        } );

    setSimpleDockWidgetArrangement();

    currentPageChanged( 0 );
    loadSettings();

#if defined Q_OS_LINUX
    for ( auto dock: dockWidgets() ) {
        dock->widget()->setStyleSheet( "* { font-size: 9pt; }"
                                       "QHeaderView::section { font-size: 9pt; }" );
    }

    for ( auto tabbar: findChildren< QTabBar * >() )
        tabbar->setStyleSheet( "QTabBar { font-size: 9pt; }" );

    for ( int i = 0; i < stack_->count(); ++i ) {
        for ( auto tab: stack_->widget( i )->findChildren< QTableView * >() ) {
            tab->setStyleSheet( "QTableView { font-size: 9pt; }"
                                "QHeaderView::section { font-size: 9pt; }" );
        }
    }
    for ( auto tree: findChildren< QTreeView * >() ) {
        tree->setStyleSheet( "* { font-size: 9pt; }"
                             "QHeaderView::section { font-size: 9pt; }" );
    }
#endif
}

void
MainWindow::OnFinalClose()
{
    using adcontrols::ProcessMethod;

    QList< QDockWidget *> widgets = dockWidgets();

    for ( auto widget: widgets ) {
        QWidget * obj = widget->widget();

        if ( auto pLifeCycle = qobject_cast< adplugin::LifeCycle * >( obj ) ) {
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
    ADDEBUG() << "## " << __FUNCTION__ << " ##";
    QFileDialog dlg( this, tr( "Save peak list for all checked spectra/chromatograms") );
    //dlg.setDirectory( currentDir() );
    dlg.setDirectory( document::instance()->recentFile( Constants::GRP_EXPORT_FILES ) );

    dlg.setAcceptMode( QFileDialog::AcceptSave );
    dlg.setFileMode( QFileDialog::AnyFile );
    QStringList filter;
    filter << "SQLite(*.db)" << "Text files(*.txt)" << "All files(*)";
    dlg.setNameFilters( filter );

    if ( !dlg.exec() )
        return;

    auto files = dlg.selectedFiles();
    if ( files.isEmpty() )
        return;

    boost::filesystem::path path( files.at(0).toStdString() );

    if ( path.extension().empty() ) {
        if ( dlg.selectedNameFilter().contains( "SQLite" ) )
            path.replace_extension( ".db" );
        else
            path.replace_extension( ".txt" );
    }

    if ( boost::filesystem::exists( path ) ) {
        boost::system::error_code ec;
        if ( !boost::filesystem::remove( path,ec ) ) {
            QMessageBox::critical( this, "QtPlatz::dataproc::mainwindow", QString("Cannot delete existing file: %1").arg( path.string().c_str() ) );
            return;
        }
    }

    if ( path.extension() == ".db" ) {
        peaklist_export::sqlite_export( path );
    } else {
        peaklist_export::text_export( path );
    }

    document::instance()->addToRecentFiles( QString::fromStdString( path.string() ), Constants::GRP_EXPORT_FILES );
}

void
MainWindow::handleExportRMSAllChecked()
{
    QFileDialog dlg( this, tr( "Save RMS for all checked spectra") );
    // dlg.setDirectory( currentDir() );
    dlg.setDirectory( document::instance()->recentFile( Constants::GRP_EXPORT_FILES ) );
    dlg.setAcceptMode( QFileDialog::AcceptSave );
    dlg.setFileMode( QFileDialog::AnyFile );
    QStringList filter;
    filter << "SQLite(*.db)" << "Text files(*.txt)" << "All files(*)";
    dlg.setNameFilters( filter );

    if ( !dlg.exec() )
        return;

    auto files = dlg.selectedFiles();
    if ( files.isEmpty() )
        return;

    boost::filesystem::path path( files.at(0).toStdString() );

    if ( path.extension().empty() ) {
        if ( dlg.selectedNameFilter().contains( "SQLite" ) )
            path.replace_extension( ".db" );
        else
            path.replace_extension( ".txt" );
    }
#if 0
    if ( boost::filesystem::exists( path ) ) {
        boost::system::error_code ec;
        if ( !boost::filesystem::remove( path,ec ) ) {
            QMessageBox::critical( this, "QtPlatz::dataproc::mainwindow", QString("Cannot delete existing file: %1").arg( path.string().c_str() ) );
            return;
        }
    }
#endif

    if ( auto pWnd = findChild< MSProcessingWnd * >() ) {
        QRectF rect;
        adcontrols::hor_axis axis;
        std::tie( rect, axis ) = pWnd->profileRect();
        auto isTime = (axis == adcontrols::hor_axis_time);

        const std::pair< double, double > range( isTime ? range_t<true>()(rect) : range_t<false>()(rect) );

        if ( path.extension() == ".db" ) {
            rms_export< adcontrols::MassSpectrum >::sqlite_export( path, range, axis == adcontrols::hor_axis_time );
        } else {
            QMessageBox::information(0, QLatin1String("QtPlatz export RMS"), "Sorry, text format export not supported." );
        }
    }

    if ( auto pWnd = findChild< MSProcessingWnd * >() ) {
        auto rect = pWnd->chromatogrRect();
        if ( path.extension() == ".db" ) {
            rms_export< adcontrols::Chromatogram >::sqlite_export( path, std::make_pair( rect.left(), rect.right() ) );
        } else {
            QMessageBox::information(0, QLatin1String("QtPlatz export RMS"), "Sorry, text format export not supported." );
        }
    }

}

void
MainWindow::handleExportAllChecked()
{
//     ADDEBUG() << "########################### TODO ###################################";
    QString dataPath;
    if ( auto dp = SessionManager::instance()->getActiveDataprocessor() )
        dataPath = QString::fromStdString( boost::filesystem::path( dp->filename() ).parent_path().string() );

    QFileDialog dlg( this, tr("Select directory for export" ) );
    dlg.setDirectory( dataPath );
    dlg.setAcceptMode( QFileDialog::AcceptSave );
    //dlg.setFileMode( QFileDialog::DirectoryOnly );
    dlg.setOption( QFileDialog::ShowDirsOnly, true );
    if ( dlg.exec() ) {
        auto files = dlg.selectedFiles();
        if ( files.empty() )
            return;
    }
    boost::filesystem::path dir( dlg.selectedFiles().at(0).toStdString() );


    ADDEBUG() << "## " << __FUNCTION__ << " dir: " << dir.string();
    if ( !boost::filesystem::exists( dir ) )
        boost::filesystem::create_directory( dir );

    for ( auto& session : *SessionManager::instance() ) {
        if ( auto processor = session.processor() ) {
            auto spectra = processor->getPortfolio().findFolder( L"Spectra" );
            uint32_t ident(1);
            auto basename = dir / boost::filesystem::path( processor->filename() ).stem();
            for ( auto& folium: spectra.folio() ) {
                if ( folium.attribute( L"isChecked" ) == L"true" ) {
                    auto outname = basename.string() + ( boost::format( "_spectrum_%d.txt" ) % ident++ ).str();
                    auto outf = std::ofstream( outname );

                    if ( folium.empty() )
                        processor->fetch( folium );
                    processor->export_text( folium, outf );
#if 0
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
#endif
                }
            }
        }
    }
//#endif
}

void
MainWindow::handleImportChecked()
{
//     ADDEBUG() << "########################### TODO ###################################";
    QString filename = QFileDialog::getSaveFileName( 0
                                                     , tr( "Import checked data into a file")
                                                     , currentDir()
                                                     , tr( "QtPlatz files(*.adfs)" ) );
    if ( filename.isEmpty() )
        return;
    boost::filesystem::path path( filename.toStdString() );
    path.replace_extension( "adfs" );

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
        document::instance()->handle_portfolio_created( QString::fromStdString( path.string() ) );
//#endif
}

void
MainWindow::handleMergeSelection( const std::map< Dataprocessor *, std::vector< portfolio::Folium > >& merge )
{
    for ( auto data: merge ) {
        ADDEBUG() << "merge: " << std::make_pair( data.first->filename(), data.second.size() );
    }
}

void
MainWindow::actionApply()
{
    qtwrapper::waitCursor wait;

    adcontrols::ProcessMethod pm;

    getProcessMethod( pm );  // update by what values GUI holds

    document::instance()->setProcessMethod( pm );

//     ADDEBUG() << "########################### TODO ###################################";
    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {

        if ( currentFeature_ == CalibrationProcess )
            processor->applyCalibration( pm );
        else if ( currentFeature_ == MSChromatogrProcess )
            handleProcess( "MSChromatogramWidget" );
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
    else if ( currentFeature_ == MSChromatogrProcess )
        object_name = "MSChromatogrMethod";
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

    if ( auto p = std::make_shared< adextension::iEditorFactoryT< adwidgets::CentroidWidget > >( "Centroid",  iEditorFactory::PROCESS_METHOD ) ) {
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
        processor->createContour();
    }
}

void
MainWindow::actClusterSpectrogram()
{
    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
        processor->clusterContour();
    }
}

void
MainWindow::addPrintFileToSettings( const QString& name )
{
    if ( auto settings = document::instance()->settings() ) {
        auto dir = boost::filesystem::path( name.toStdString() ).parent_path();
        settings->beginGroup( Constants::GRP_SPECTRUM_IMAGE );
        settings->setValue( Constants::KEY_IMAGE_SAVE_DIR, QString::fromStdString( dir.string() ) );
        settings->endGroup();
    }
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
//     ADDEBUG() << "########################### TODO ###################################";
    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
        portfolio::Portfolio portfolio = dp->getPortfolio();

        if ( portfolio::Folium folium = portfolio.findFolium( id ) ) {
            std::wstring name = folium.name();
            boost::algorithm::trim( name );
            return name;
        }
    }
    return {};
}

QString
MainWindow::currentDir()
{
    static QString dir = QString::fromStdWString( adportable::profile::user_data_dir<wchar_t>() );
    ADDEBUG() << "########################### TODO ###################################";
#if QTC_VERSION <= 0x03'02'81
    QString currentFile = Core::DocumentManager::currentFile(); // Core::ICore::instance()->fileManager()->currentFile();
    if ( !currentFile.isEmpty() ) {
        const QFileInfo fi( currentFile );
        dir = fi.absolutePath();
    }
#endif
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
