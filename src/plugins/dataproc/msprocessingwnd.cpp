/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
**
** Contact: info@ms-cheminfo.com
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

#include "datafolder.hpp"
#include "dataprocessor.hpp"
#include "dataprocessworker.hpp"
#include "dataprocplugin.hpp"
#include "document.hpp"
#include "mainwindow.hpp"
#include "msprocessingwnd.hpp"
#include "rms_export.hpp"
#include "sessionmanager.hpp"
#include "utility.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/histogram.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adlog/logger.hpp>
#include <adfs/sqlite.hpp>
#include <adplot/picker.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/tracewidget.hpp>
#include <adplot/zoomer.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/fft.hpp>
#include <adportable/float.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/xml_serializer.hpp>
#include <adpublisher/printer.hpp>
#include <adutils/processeddata.hpp>
#include <adwidgets/filedialog.hpp>
#include <adwidgets/scanlawdialog.hpp>
#include <adwidgets/scanlawdialog2.hpp>
#include <adwidgets/mspeaktree.hpp>
#include <adwidgets/datareaderchoicedialog.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/fft4g.hpp>
#include <qtwrapper/font.hpp>
#include <qtwrapper/make_widget.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/xmlformatter.hpp>
#include <pugixml.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QClipboard>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QSettings>
#include <QSlider>
#include <QStandardPaths>
#include <QSvgGenerator>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTableFormat>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/variant.hpp>
#include <boost/json.hpp>
#include <algorithm>
#include <array>
#include <complex>
#include <filesystem>
#include <functional>
#include <numeric>
#include <sstream>

using namespace dataproc;

namespace dataproc {

    class MSProcessingWnd::impl {
    public:
        impl() : ticPlot_(0)
               , profileSpectrum_(0)
               , processedSpectrum_(0)
               , pwplot_( new adplot::TraceWidget )
               , is_time_axis_( false )
               , hasHistogram_( false )
               , scaleYAuto_( true )
               , scaleY_( {0, 0} )
               , yRightEnabled_( false )
               , yScaleChromatogram_{ true, 0,  100.0 }
               , xScaleChromatogram_{ true, 0, 1000.0 }
               , axis_(adcontrols::hor_axis_mass)
               , drawIdx1_( 0 )  {
        }

        void currentChanged( const adcontrols::MSPeakInfoItem& pk ) {
            if ( profile_marker_ ) {
                profile_marker_->setPeak( pk, is_time_axis_, adcontrols::metric::micro );
                profileSpectrum_->replot();
            }
            if ( processed_marker_ ) {
                processed_marker_->setPeak( pk, is_time_axis_, adcontrols::metric::micro );
                processedSpectrum_->replot();
            }
        }

        void currentChanged( const adcontrols::MassSpectrum& ms, int idx ) {
            if ( profile_marker_ ) {
                profile_marker_->setPeak( ms, idx, is_time_axis_, adcontrols::metric::micro );
                profileSpectrum_->replot();
            }
            if ( processed_marker_ ) {
                processed_marker_->setPeak( ms, idx, is_time_axis_, adcontrols::metric::micro );
                processedSpectrum_->replot();
            }
        }

        void set_time_axis( bool isTime ) {
            is_time_axis_ = isTime;
        }

        void focusedFcn( int fcn ) {
            profileSpectrum_->setFocusedFcn( fcn );
            processedSpectrum_->setFocusedFcn( fcn );
        }

        bool ticTracker( const QPointF& pos, QwtText& text ) {
            if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
                if ( auto rawfile = dp->rawdata() ) {

                    size_t npos(0);
                    int fcn(0), rep(0), index(0);
                    double minutes(0);

                    if ( rawfile->dataformat_version() >= 3 ) {
                        return true;
                    } else {
                        ADDEBUG() << "V2 format data are no longer supported.";
                    }
                }
            }
            return false;
        }

        void clearCheckedChromatograms() {
            checkedChromatograms_.clear();
        }

        void setCheckedChromatogram( std::shared_ptr< adcontrols::Chromatogram >& ptr, int idx ) {
            checkedChromatograms_[ idx ] = ptr;
        }

        adplot::ChromatogramWidget * ticPlot_;
        adplot::SpectrumWidget * profileSpectrum_;
        adplot::SpectrumWidget * processedSpectrum_;
        adplot::TraceWidget * pwplot_;
        std::shared_ptr< adplot::PeakMarker > profile_marker_;
        std::shared_ptr< adplot::PeakMarker > processed_marker_;
        std::map< int, std::weak_ptr< adcontrols::Chromatogram > > checkedChromatograms_;
        bool is_time_axis_;
        bool hasHistogram_;
        bool scaleYAuto_;
        std::pair< double, double > scaleY_;
        bool yRightEnabled_;
        std::tuple< bool, double, double > yScaleChromatogram_;
        std::tuple< bool, double, double > xScaleChromatogram_;
        std::array< datafolder, 2 > datum_; // chromatogram, spectrum
        //
        std::pair< std::wstring, std::weak_ptr< adcontrols::MassSpectrum > > pProcessedSpectrum_;
        std::pair< std::wstring, std::weak_ptr< adcontrols::MassSpectrum > > pProfileSpectrum_;
        std::pair< std::wstring, std::weak_ptr< adcontrols::MassSpectrum > > pProfileHistogram_;
        std::pair< std::wstring, std::weak_ptr< adcontrols::MSPeakInfo > > pkinfo_;
        std::pair< std::wstring, std::weak_ptr< adcontrols::Targeting > > targeting_;
        std::wstring idActiveFolium_;
        std::wstring idChromatogramFolium_;
        std::wstring idSpectrumFolium_;
        adcontrols::hor_axis axis_;
        int drawIdx1_;
    };

}

MSProcessingWnd::~MSProcessingWnd()
{
    delete pImpl_;
}

MSProcessingWnd::MSProcessingWnd(QWidget *parent) : QWidget(parent)
                                                  , pImpl_( new impl() )
{
    init();
}

void
MSProcessingWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {

        if ( ( pImpl_->ticPlot_ = new adplot::ChromatogramWidget(this) ) ) {
            pImpl_->ticPlot_->setObjectName( "MSProcessingWnd.0" );
            pImpl_->ticPlot_->setMinimumHeight( 80 );
			connect( pImpl_->ticPlot_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnChromatogram( const QRectF& ) ) );
            pImpl_->ticPlot_->register_tracker( [&]( const QPointF& pos, QwtText& text ){ return pImpl_->ticTracker( pos, text ); } );
            pImpl_->ticPlot_->setItemLegendEnabled( false );
        }

        if ( ( pImpl_->profileSpectrum_ = new adplot::SpectrumWidget(this) ) ) {
            pImpl_->profileSpectrum_->setMinimumHeight( 80 );
            using adplot::SpectrumWidget;
            connect( pImpl_->profileSpectrum_, &SpectrumWidget::onSelected, this, &MSProcessingWnd::selectedOnProfile );
            pImpl_->profile_marker_ = std::make_shared< adplot::PeakMarker >();
            pImpl_->profile_marker_->attach( pImpl_->profileSpectrum_ );
            pImpl_->profile_marker_->visible( true );
            pImpl_->profile_marker_->setYAxis( QwtPlot::yLeft );
        }

        if ( ( pImpl_->processedSpectrum_ = new adplot::SpectrumWidget(this) ) ) {
            pImpl_->processedSpectrum_->setMinimumHeight( 80 );
            using adplot::SpectrumWidget;
            connect( pImpl_->processedSpectrum_, qOverload<const QRectF&>(&SpectrumWidget::onSelected)
                     , this, qOverload<const QRectF&>(&MSProcessingWnd::selectedOnProcessed) );
            adplot::Zoomer * zoomer = pImpl_->processedSpectrum_->zoomer();
            connect( zoomer, &adplot::Zoomer::zoomed, this, &MSProcessingWnd::handleZoomedOnSpectrum );

            pImpl_->processed_marker_ = std::make_shared< adplot::PeakMarker >();
            pImpl_->processed_marker_->attach( pImpl_->processedSpectrum_ );
            pImpl_->processed_marker_->visible( true );
            pImpl_->processed_marker_->setYAxis( QwtPlot::yLeft );
        }
		pImpl_->ticPlot_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
		pImpl_->profileSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
		pImpl_->processedSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );

        pImpl_->profileSpectrum_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );
		pImpl_->processedSpectrum_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );

        pImpl_->pwplot_->setMinimumHeight( 80 );
        pImpl_->pwplot_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
        pImpl_->pwplot_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
        pImpl_->pwplot_->xBottomTitle( "Frequency (MHz)" );
        pImpl_->pwplot_->yLeftTitle( "Power" );
        connect( pImpl_->pwplot_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnPowerPlot( const QRectF& ) ) );

		splitter->addWidget( pImpl_->ticPlot_ );
        splitter->addWidget( pImpl_->profileSpectrum_ );
        splitter->addWidget( pImpl_->processedSpectrum_ );

        splitter->addWidget( pImpl_->pwplot_ ); // power spectrum

        //////////// add 2019-AUG-19 //////////
        splitter->addWidget( qtwrapper::make_widget< adwidgets::MSPeakTree >("TargetingTree") );

        splitter->setOrientation( Qt::Vertical );

        splitter->setChildrenCollapsible( true );
        pImpl_->pwplot_->hide();

        pImpl_->profileSpectrum_->link( pImpl_->processedSpectrum_ );
        //pImpl_->processedSpectrum_->link( pImpl_->profileSpectrum_ );

        pImpl_->processedSpectrum_->setContextMenuPolicy( Qt::CustomContextMenu );
		connect( pImpl_->processedSpectrum_, SIGNAL( customContextMenuRequested( QPoint ) )
                 , this, SLOT( handleCustomMenuOnProcessedSpectrum( const QPoint& ) ) );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setContentsMargins( {} );
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );

    connect( SessionManager::instance(), &SessionManager::foliumChanged, this, &MSProcessingWnd::handleFoliumChanged );
}

void
MSProcessingWnd::draw_histogram( portfolio::Folium& folium, adutils::MassSpectrumPtr& hist )
{
    pImpl_->hasHistogram_ = true;

    std::shared_ptr< adcontrols::MassSpectrum > profile;

    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {

        pImpl_->pProfileSpectrum_ = std::make_pair( folium.id(), hist ); // sticked

        if ( auto att = dp->findProfiledHistogram( folium, true ) ) {
            profile = portfolio::get< adcontrols::MassSpectrumPtr >( att );
            pImpl_->pProfileHistogram_ = std::make_pair( att.id(), profile ); // profiled
        } else {
            pImpl_->pProfileHistogram_.second.reset();
        }
    }

    if ( pImpl_->axis_ == adcontrols::hor_axis_mass ) {
        if ( hist->size() > 0
             && adportable::compare<double>::approximatelyEqual( hist->mass( hist->size() - 1 ), hist->mass( 0 ) ) ) {
            // Spectrum has no mass assigned
            MainWindow::instance()->setSpectrumAxisChoice( adcontrols::hor_axis_time );
        }
    }

    pImpl_->profileSpectrum_->setData( hist, pImpl_->drawIdx1_ + 1, QwtPlot::yLeft );
    pImpl_->profileSpectrum_->setData( profile, pImpl_->drawIdx1_, QwtPlot::yLeft );
    pImpl_->drawIdx1_ += 2;
    pImpl_->profileSpectrum_->setAxisTitle( QwtPlot::yLeft, QwtText( "Counts" ) );

    QString title = QString("[%1]").arg( MainWindow::makeDisplayName( pImpl_->idSpectrumFolium_ ) );
	for ( auto text: hist->getDescriptions() )
		title += QString::fromStdWString( std::wstring( text.text<wchar_t>() ) + L", " );

	pImpl_->profileSpectrum_->setTitle( title );
    pImpl_->processedSpectrum_->clear();
}

void
MSProcessingWnd::draw_profile( const std::wstring& guid, adutils::MassSpectrumPtr& ptr )
{
    pImpl_->pProfileSpectrum_ = std::make_pair( guid, ptr );
    pImpl_->pProfileHistogram_.second.reset();

    if ( pImpl_->hasHistogram_ ) {
        pImpl_->profileSpectrum_->clear();
        pImpl_->hasHistogram_ = false;
    }

    if ( pImpl_->axis_ == adcontrols::hor_axis_mass ) {
        if ( ptr->size() > 0
             && adportable::compare<double>::approximatelyEqual( ptr->mass( ptr->size() - 1 ), ptr->mass( 0 ) ) ) {
            // Spectrum has no mass assigned
            MainWindow::instance()->setSpectrumAxisChoice( adcontrols::hor_axis_time );
        }
    }

    pImpl_->profileSpectrum_->setData( ptr, static_cast<int>(pImpl_->drawIdx1_++), QwtPlot::yLeft );
    QString title = QString("[%1]").arg( MainWindow::makeDisplayName( pImpl_->idSpectrumFolium_ ) );
	for ( auto text: ptr->getDescriptions() )
		title += QString::fromStdWString( std::wstring( text.text<wchar_t>() ) + L", " );
	pImpl_->profileSpectrum_->setTitle( title );
    pImpl_->processedSpectrum_->clear();
}

void
MSProcessingWnd::draw1()
{
    if ( auto ptr = pImpl_->pProfileSpectrum_.second.lock() ) {
        if ( pImpl_->drawIdx1_ )
            --pImpl_->drawIdx1_;
        pImpl_->profileSpectrum_->setData( ptr, static_cast<int>(pImpl_->drawIdx1_++), QwtPlot::yLeft );

        QString title = QString("[%1]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;").arg( MainWindow::makeDisplayName( pImpl_->idSpectrumFolium_ ) );
        for ( auto text: ptr->getDescriptions() )
            title += QString::fromStdWString( std::wstring( text.text<wchar_t>() ) + L", " );

        pImpl_->profileSpectrum_->setTitle( title );
        pImpl_->processedSpectrum_->clear();
    }
}

void
MSProcessingWnd::draw( adutils::ChromatogramPtr ptr, int idx )
{
    pImpl_->ticPlot_->setData( ptr, idx, QwtPlot::yLeft );
    if ( auto label = ptr->axisLabel( adcontrols::plot::yAxis ) ) {
        pImpl_->ticPlot_->setAxisTitle( QwtPlot::yLeft, QwtText( QString::fromStdString( *label ) ) );
    } else {
        pImpl_->ticPlot_->setAxisTitle( QwtPlot::yLeft, QwtText( "Intensity" ) );
    }
}

void
MSProcessingWnd::idSpectrumFolium( const std::wstring& id )
{
    pImpl_->idSpectrumFolium_ = id;
}

void
MSProcessingWnd::idChromatogramFolium( const std::wstring& id )
{
    pImpl_->idChromatogramFolium_ = id;
}

void
MSProcessingWnd::handleRemoveSession( Dataprocessor * processor )
{
#ifndef NDEBUG
    ADDEBUG() << "handleRemoveSession(" << processor->filename() << ")"
              << "\n\t" << pImpl_->datum_[ 0 ].filename()
              << "\n\t" << pImpl_->datum_[ 1 ].filename();
#endif
    if ( pImpl_->datum_[ 0 ].filename() == processor->filename<char>() ) {
        pImpl_->ticPlot_->clear();
        pImpl_->ticPlot_->replot();
    }
    if ( pImpl_->datum_[ 1 ].filename() == processor->filename<char>() ) {
        for ( auto plot: { pImpl_->profileSpectrum_, pImpl_->processedSpectrum_ } ) {
            plot->setTitle( QString{} );
            plot->clear();
            plot->replot();
        }
    }
}

void
MSProcessingWnd::handleSessionAdded( Dataprocessor * processor )
{
    portfolio::Portfolio portfolio = processor->getPortfolio();
    auto noise_filter = std::shared_ptr< adprocessor::noise_filter >();

    if ( const adcontrols::LCMSDataset * dset = processor->rawdata() ) {

        portfolio::Folder folder = portfolio.findFolder( L"Chromatograms" );
        if ( folder.nil() ) {
            folder = processor->getPortfolio().addFolder( L"Chromatograms" );
        }

        if ( dset->dataformat_version() >= 3 ) {

            adcontrols::ProcessMethod m;
            MainWindow::instance()->getProcessMethod( m );

            auto vec = dset->dataReaders();
            for ( auto& reader : vec ) {
                // ADDEBUG() << "reader->trace_method" << reader->trace_method();
                for ( int fcn = 0; fcn < reader->fcnCount(); ++fcn ) {

                    if ( auto tic = reader->TIC( fcn ) ) {
                        // name before v5.5
                        auto name = ( boost::wformat( L"%1%/%2%.%3%" )
                                      % adcontrols::Chromatogram::make_folder_name<wchar_t>( tic->getDescriptions() )
                                      % L"TIC" % ( fcn + 1 ) ).str();

                        auto query = ( boost::format( "./folium[contains(@name,'TIC.%d')]" ) % ( fcn + 1 ) ).str();
                        auto folium = folder.findFoliumByRegex( query );

                        if ( folium.nil() ) {
                            auto c = std::make_shared< adcontrols::Chromatogram >(*tic);
                            c->addDescription( adcontrols::description(
                                                   {"acquire.title", ( boost::format( "TIC.%1%" ) % ( fcn + 1 ) ).str() }) );
                            portfolio::Folium folium = processor->addChromatogram( c, m, noise_filter );
                            SessionManager::instance()->updateDataprocessor( processor, folium ); // added 2022-11-22
                        }
                        processor->setCurrentSelection( folium );
                    }
                }
                if ( reader->trace_method() == adacquire::SignalObserver::eTRACE_TRACE ) {
                    for ( size_t idx = 0; idx < 8; ++idx ) {
                        if ( auto pChro = reader->getChromatogram( idx ) ) {
                            std::string legend;

                            auto trace = adportable::json_helper::find( pChro->generatorProperty(), "trace" );
                            if ( trace.is_object() ) {
                                auto enable = boost::json::value_to< bool >( trace.at( "enable" ) );

                                if ( enable ) {
                                    pChro->addDescription( adcontrols::description({"acquire.title"
                                                , ( boost::format( "ADC.%1%" ) % ( idx + 1 ) ).str()}) );
                                    std::wstring name = adcontrols::Chromatogram::make_folder_name<wchar_t>( pChro->getDescriptions() );
                                    auto folium = folder.findFoliumByName( name );
                                    if ( folium.nil() ) {
                                        ADDEBUG() << "---------- addChromatogram ---------------";
                                        folium = processor->addChromatogram( pChro, m, noise_filter ); //, true );
                                        processor->setCurrentSelection( folium );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else { // v2 format data
            size_t nfcn = dset->getFunctionCount();
            for ( size_t fcn = 0; fcn < nfcn; ++fcn ) {
                std::wstring title = ( boost::wformat( L"TIC.%1%" ) % ( fcn + 1 ) ).str();

                portfolio::Folium folium = folder.findFoliumByName( std::wstring( L"TIC/" ) + title );
                if ( folium.nil() ) {   // add TIC if not yet added
                    auto c = std::make_shared< adcontrols::Chromatogram >();
                    if ( dset->getTIC( static_cast<int>( fcn ), *c ) ) {
                        // if ( c->isConstantSampledData() )
                        //     c->getTimeArray();
                        c->addDescription( adcontrols::description( L"acquire.title", title ) );
                        adcontrols::ProcessMethod m;
                        MainWindow::instance()->getProcessMethod( m );
                        folium = processor->addChromatogram( c, m, noise_filter );
                    }
                }
                if ( folium.attribute( L"protoId" ).empty() )
                    folium.setAttribute( L"protoId", ( boost::wformat( L"%d" ) % fcn ).str() );
            }
            if ( portfolio::Folium folium = folder.findFoliumByName( L"TIC/TIC.1" ) ) {
                qtwrapper::waitCursor wait;
                if ( folium.empty() )
                    processor->fetch( folium );
                processor->setCurrentSelection( folium );
            }
        }
    }

    // show first spectrum on tree by default
    portfolio::Folder spectra = portfolio.findFolder( L"Spectra" );
    if ( !spectra.nil() ) {
        portfolio::Folio folio = spectra.folio();
        if ( !folio.empty() ) {
            processor->fetch( folio [ 0 ] );
            processor->setCurrentSelection( folio [ 0 ] );
        }
    }
}

void
MSProcessingWnd::handleZoomedOnSpectrum( const QRectF& rc )
{
    if ( pImpl_->axis_ == adcontrols::hor_axis_time ) {
        MainWindow::instance()->zoomedOnSpectrum( QRectF( rc.x()/std::micro::den, rc.y(), rc.width()/std::micro::den, rc.height()), pImpl_->axis_ );
    } else {
        MainWindow::instance()->zoomedOnSpectrum( rc, pImpl_->axis_ );
    }
}

void
MSProcessingWnd::handleProcessed( Dataprocessor* processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MSProcessingWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    // ScopedDebug(x);  x << " ========== ";

    pImpl_->drawIdx1_ = 0;

    if ( portfolio::Folder folder = folium.parentFolder() ) {

        if ( folder.name() == L"Spectra" ) { //|| folder.name() == L"Chromatograms" ) {

            // ADDEBUG() << "------- selection changed for Spectra --------";
            if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {

                pImpl_->pProcessedSpectrum_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::MassSpectrum >( 0 ) );
                pImpl_->pProfileSpectrum_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::MassSpectrum >( 0 ) );
                pImpl_->pkinfo_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::MSPeakInfo >( 0 ) );
                pImpl_->targeting_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::Targeting >( 0 ) );

                if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {

                    pImpl_->idActiveFolium_ = folium.id();
                    pImpl_->idSpectrumFolium_ = folium.id();
                    pImpl_->datum_[ 1 ] = datafolder( processor, folium );

                    pImpl_->processedSpectrum_->clear();
                    pImpl_->processedSpectrum_->replot();

                    if ( ptr->isHistogram() ) // a.k.a. pkd waveform
                        draw_histogram( folium, ptr ); // draw counting histogram
                    else
                        draw_profile( folium.id(), ptr );

                    if ( auto f = portfolio::find_first_of( folium.attachments(), []( const portfolio::Folium& a ){
                        return a.name() == Constants::F_DFT_FILTERD; }) ) {
                        if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( f ) ) {
                            pImpl_->processedSpectrum_->setData( ptr, 1, QwtPlot::yRight ); // overlay DFT low pass filterd
                            pImpl_->processedSpectrum_->setAlpha( 1, 0x20 );
                        }
                    }

                    if ( auto fcentroid = portfolio::find_first_of( folium.attachments()
                                                                    , []( const portfolio::Folium& a ){
                                                                        return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
                        if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( fcentroid ) ) {
                            pImpl_->processedSpectrum_->setData( centroid, 0, QwtPlot::yLeft );
                            pImpl_->pProcessedSpectrum_ = std::make_pair( fcentroid.id(), centroid );
                        }

                        if ( auto fmethod = portfolio::find_first_of( fcentroid.attachments(), []( portfolio::Folium& a ){
                                    return portfolio::is_type< adcontrols::ProcessMethodPtr >( a ); }) ) {

                            if ( auto method = portfolio::get< adcontrols::ProcessMethodPtr >( fmethod ) )
                                MainWindow::instance()->setProcessMethod( *method );
                        }

                        if ( auto fpkinfo = portfolio::find_first_of( fcentroid.attachments(), []( portfolio::Folium& a ){
                                    return portfolio::is_type< adcontrols::MSPeakInfoPtr >( a ); } ) ) {
                            pImpl_->pkinfo_ = std::make_pair( fpkinfo.id(), portfolio::get< adcontrols::MSPeakInfoPtr >( fpkinfo ) );
                        }

                        if ( auto ftgt = portfolio::find_first_of( fcentroid.attachments(), []( portfolio::Folium& a ){
                                    return portfolio::is_type< adcontrols::TargetingPtr >( a ); } ) ) {
                            pImpl_->targeting_ = std::make_pair( ftgt.id(), portfolio::get< adcontrols::TargetingPtr >( ftgt ) );

                            // set corresponding targeting method to UI
                            if ( auto fmth = portfolio::find_first_of(
                                     ftgt.attachments(), [] ( portfolio::Folium& a ){
                                         return portfolio::is_type< adcontrols::ProcessMethodPtr >( a ); } ) ) {
                                if ( auto mth = portfolio::get< adcontrols::ProcessMethodPtr >( fmth ) )
                                    MainWindow::instance()->setProcessMethod( *mth );
                            }
                        }

                    } else {
                        pImpl_->processedSpectrum_->clear();
                    }

                }
            }
        }
        else if ( folder.name() == L"Chromatograms" ) {

            int idx = 0;
            pImpl_->ticPlot_->clear();
            pImpl_->ticPlot_->replot();
            if ( portfolio::is_type< adcontrols::ChromatogramPtr >( folium ) ) {

                boost::any data = folium.data();
                auto p = boost::any_cast< adcontrols::ChromatogramPtr >( data );

                if ( auto ptr = portfolio::get< adcontrols::ChromatogramPtr > ( folium ) ) {
                    idx = std::max( idx, ptr->protocol() );
                    draw( ptr, ptr->protocol() );
                    pImpl_->idActiveFolium_ = folium.id();
                    if ( processor->getPortfolio().findFolium( folium.id() ) ) { // if not searchable
                        idChromatogramFolium( folium.id() );
                    }

                    pImpl_->datum_[ 0 ] = datafolder( processor, folium );
                    if ( auto f = portfolio::find_first_of( folium.attachments(), []( portfolio::Folium& a ){
                        return portfolio::is_type< adcontrols::PeakResultPtr >( a ); }) ) {
                        auto pkresults = portfolio::get< adcontrols::PeakResultPtr >( f );
                        pImpl_->ticPlot_->setPeakResult( *pkresults, 0, QwtPlot::yLeft );
                    }
                } else {
                    ADDEBUG() << "\t############# failed to get chromatogram ptr (null_ptr)";
                }
                pImpl_->ticPlot_->setNormalizedY( QwtPlot::yLeft, std::get< 0 >( pImpl_->yScaleChromatogram_ ) );  // auto scale y?

                pImpl_->clearCheckedChromatograms();
                // redraw all chromatograms with check marked
#if 0
                auto folio = folder.folio();
                for ( auto& f: folio ) {
                    if ( ( f.uuid() != folium.uuid() ) && ( f.attribute( L"isChecked" ) == L"true" ))  {
                        processor->fetch(f);
                        if ( auto cptr = portfolio::get< adcontrols::ChromatogramPtr >( f ) ) {
                            ++idx;
                            pImpl_->setCheckedChromatogram( cptr, idx );
                            pImpl_->ticPlot_->setData( cptr, idx, QwtPlot::yLeft );
                            pImpl_->ticPlot_->setAlpha( idx, 0x40 );
                        }
                    }
                }
#endif
            }
        }
    }
}

void
MSProcessingWnd::handleSelections( const std::vector< portfolio::Folium >& folio )
{
    int idx(0);
    for ( auto it = folio.rbegin(); it != folio.rend(); ++it ) {
        if ( it->attribute("dataType") == "Chromatogram" ) {
            if ( auto dp = SessionManager::instance()->find_processor( it->filename<char>() ) ) {
                auto self( dp->shared_from_this() );
                auto folium( *it );
                dp->fetch( folium );
                if ( auto cptr = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
                    ++idx;
                    pImpl_->setCheckedChromatogram( cptr, idx );
                    pImpl_->ticPlot_->setData( cptr, idx, QwtPlot::yLeft );
                    pImpl_->ticPlot_->setAlpha( idx, 0x40 );
                }
            }
        }
    }
}

void
MSProcessingWnd::handleAxisChanged( adcontrols::hor_axis axis )
{
    using adplot::SpectrumWidget;

    pImpl_->axis_ = axis;
    pImpl_->set_time_axis( axis == adcontrols::hor_axis_mass ? false : true );
    auto plot_axis = ( axis == adcontrols::hor_axis_mass ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime );

    std::shared_ptr< adcontrols::MassSpectrometer > spectrometer;

    if ( auto processor = SessionManager::instance()->getActiveDataprocessor() )
        spectrometer = processor->massSpectrometer();

    pImpl_->processedSpectrum_->setAxis( plot_axis, true );

    pImpl_->profileSpectrum_->setAxis( plot_axis, true, [&](const QRectF& z
                                                            , const adcontrols::MassSpectrum& ms
                                                            , adplot::SpectrumWidget::HorizontalAxis axis ){
        if ( axis == adplot::SpectrumWidget::HorizontalAxisMass ) { // mass --> time
            auto range = spectrometer->timeFromMass( std::make_pair( z.left(), z.right() ), ms );
            return QRectF( range.first * std::micro::den, z.bottom(), ( range.second - range.first ) * std::micro::den, z.height() );
        } else { // time --> mass
            auto range = spectrometer->massFromTime( std::make_pair( z.left() / std::micro::den, z.right() / std::micro::den ), ms );
            return QRectF( range.first, z.bottom(), range.second - range.first, z.height() );
        }
    });
}

void
MSProcessingWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSProcessingWnd::handleCustomMenuOnProcessedSpectrum( const QPoint& )
{
	// This is conflicting with picker's action, so it has moved to range selection slots
}

void
MSProcessingWnd::handleCurrentChanged( int idx, int fcn )
{
    if ( auto pkinfo = pImpl_->pkinfo_.second.lock() ) {

        if ( pkinfo->numSegments() > 1 )
            pImpl_->focusedFcn( fcn );

        adcontrols::segment_wrapper< const adcontrols::MSPeakInfo > fpks( *pkinfo );
        if ( fpks[ fcn ].size() > idx ) {
            auto pk = fpks[ fcn ].begin() + idx;
            pImpl_->currentChanged( *pk );
        }

    } else if ( auto ms = pImpl_->pProcessedSpectrum_.second.lock() ) {

        if ( ms->numSegments() > 1 )
            pImpl_->focusedFcn( fcn );

        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *ms );
        if ( segs.size() > unsigned( fcn ) ) {
            pImpl_->currentChanged( segs[ fcn ], idx );
        }
    }
}

void
MSProcessingWnd::handleFoliumChanged( Dataprocessor * processor, const portfolio::Folium& folium )
{
}

void
MSProcessingWnd::handleModeChanged( int idx, int fcn, int mode )
{
    if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
        if ( auto sp = dp->massSpectrometer() ) {
            if ( auto pkinfo = pImpl_->pkinfo_.second.lock() ) {
                // nothing to be done
            }
            if ( auto ms = pImpl_->pProcessedSpectrum_.second.lock() ) {
                if ( ms->isCentroid() && !ms->isHistogram() ) {
                    auto& fms = adcontrols::segment_wrapper< adcontrols::MassSpectrum >( *ms )[ fcn ];
                    auto it = std::find_if( fms.annotations().begin(), fms.annotations().end()
                                            , [&]( const auto& a ){
                                                return a.index() == idx && a.dataFormat() == adcontrols::annotation::dataJSON; } );
                    boost::json::object jobj;
                    if ( it != fms.annotations().end() ) {
                        auto jv = adportable::json_helper::parse( it->json() );
                        if ( jv.is_object() )
                            jobj = jv.as_object();
                    }
                    jobj[ "peak" ] = boost::json::object{{ "peak", {{ "mode", mode }, {"mass", sp->assignMass( fms.time( idx ), mode ) }}}};
                    fms.addAnnotation( { jobj } );
                    dp->setModified( true );
                }
            } else {
                ADDEBUG() << "------- no processing spectrum can be locked ---------";
            }
        }
    }
}

void
MSProcessingWnd::handleFormulaChanged( int idx, int fcn )
{
	pImpl_->processedSpectrum_->update_annotation();
    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() )
        dp->formulaChanged(); // this makes processor dirty (setModified())

    emit dataChanged( QString::fromStdWString( pImpl_->pProfileSpectrum_.first ), QString::fromStdWString( pImpl_->pProcessedSpectrum_.first ), idx, fcn );
}

void
MSProcessingWnd::handleScanLawEst( const QVector< QPair<int, int> >& refs )
{
    if ( auto ms = pImpl_->pProcessedSpectrum_.second.lock() ) {

        if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
            std::vector< std::pair< int, int> > crefs( refs.size() );
            std::transform( refs.begin(), refs.end(), crefs.begin(), []( const auto& a ){ return std::make_pair( a.first, a.second ); } );
            if ( dp->estimateScanLaw( ms, crefs ) )
                return;
        }
    }

    estimateScanLaw( adcontrols::iids::adspectrometer_uuid );
}

void
MSProcessingWnd::estimateScanLaw( const boost::uuids::uuid& iid_spectrometer )
{
    // this will be relocate into 'CalibScanLaw' class defined in adtofprocessor
    adwidgets::ScanLawDialog2 dlg;

    if ( auto ms = pImpl_->pProcessedSpectrum_.second.lock() ) {

        for ( auto& fms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms ) ) {
            int mode = fms.getMSProperty().mode();
            for ( const auto& a: fms.annotations() ) {
                if ( a.dataFormat() == adcontrols::annotation::dataFormula && a.index() >= 0 ) {
                    dlg.addPeak( a.index()
                                 , QString::fromStdString( a.text() )
                                 , fms.time( a.index() )    // observed time-of-flight
                                 , fms.mass( a.index() )    // matched mass
                                 , mode );
                }
            }
        }

        dlg.commit();
    }

    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {

        if ( auto db = dp->db() ) { // sqlite shared_ptr
            {
                adfs::stmt sql( *db );
                sql.prepare( "SELECT id,description,fLength FROM Spectrometer" );
                if ( sql.step() == adfs::sqlite_row ) {
                    dlg.setSpectrometerData( sql.get_column_value< boost::uuids::uuid >( 0 )
                                             , QString::fromStdWString( sql.get_column_value< std::wstring >( 1 ) )
                                             , sql.get_column_value< double >( 2 ) ); // fLength
                } else {
                    dlg.setSpectrometerData( iid_spectrometer, "", 0 );
                }
            }
            {
                adfs::stmt sql( *db );
                sql.prepare( "SELECT objuuid,objtext,acclVoltage,tDelay FROM ScanLaw" );
                while( sql.step() == adfs::sqlite_row ) {
                    dlg.addObserver( sql.get_column_value< boost::uuids::uuid >( 0 )
                                     , QString::fromStdString( sql.get_column_value< std::string >(1) )
                                     , sql.get_column_value< double >( 2 )
                                     , sql.get_column_value< double >( 3 )
                                     , dlg.peakCount() > 0 ? true : false );
                }
            }
        }

        if ( dlg.exec() != QDialog::Accepted )
            return;

        double t0 = dlg.tDelay() / std::micro::den;
        double acclV = dlg.acceleratorVoltage();
        adportable::TimeSquaredScanLaw law( acclV, t0, dlg.length() ); // <- todo: get 'L' from Spectrometer on db

        // update database
        auto list = dlg.checkedObservers();
        for ( auto& obj: list ) {
            adfs::stmt sql( *(dp->db()) );
            sql.prepare( "UPDATE ScanLaw SET acclVoltage=?,tDelay=? WHERE objtext=?" );
            sql.bind( 1 ) = acclV;
            sql.bind( 2 ) = t0;
            sql.bind( 3 ) = obj.toStdString();
            while ( sql.step() == adfs::sqlite_row )
                ;
        }

        // assign masses for processed peak
        if ( auto ms = pImpl_->pProcessedSpectrum_.second.lock() ) {
            for ( auto& fms: adcontrols::segment_wrapper< adcontrols::MassSpectrum >( *ms ) ) {
                fms.getMSProperty().setAcceleratorVoltage( acclV );
                fms.getMSProperty().setTDelay( t0 );
                fms.assign_masses( [&]( double time, int mode ){ return law.getMass( time, mode ); } );
            }
        }

        // assign masses for profile spectrum
        if ( auto ms = pImpl_->pProfileSpectrum_.second.lock() ) {
            for ( auto& fms: adcontrols::segment_wrapper< adcontrols::MassSpectrum >( *ms ) ) {
                fms.getMSProperty().setAcceleratorVoltage( acclV );
                fms.getMSProperty().setTDelay( t0 );
                fms.assign_masses( [&]( double time, int mode ){ return law.getMass( time, mode ); } );
            }
        }
        handleDataMayChanged();
    }
}

void
MSProcessingWnd::handleLockMass( const QVector< QPair<int, int> >& refs )
{
    if ( auto ms = pImpl_->pProcessedSpectrum_.second.lock() ) {

        // Qt -> std
        std::vector< std::pair< int, int > > refList;  // vector of { idx, fcn }
        std::for_each( refs.begin(), refs.end(), [&](const auto ref){ refList.emplace_back( ref.first, ref.second ); } );

        if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
            if ( auto folium = dp->getPortfolio().findFolium( pImpl_->idSpectrumFolium_ ) ) {

                if ( auto mslock = dp->doMSLock( folium, ms, refList ) ) {
                    // ADDEBUG() << "###\n" << boost::json::value_from( mslock );
                    dp->setAttribute( folium, { "mslock", "true" } );

                    pImpl_->processedSpectrum_->update_annotation();

                    MainWindow::instance()->lockMassHandled( ms ); // update MSPeakTable
                    pImpl_->processedSpectrum_->replot();
                    pImpl_->profileSpectrum_->replot();
                    emit dataChanged( QString::fromStdWString( pImpl_->idSpectrumFolium_ ), QString(), -1, -1 );
                }
            }
        }
    }
}

void
MSProcessingWnd::handleDataMayChanged()
{
    pImpl_->profileSpectrum_->update();
    pImpl_->processedSpectrum_->update();
}

void
MSProcessingWnd::handleFoliumDataChanged( const QString& id )
{
    if ( id == QString::fromStdWString( pImpl_->idSpectrumFolium_ ) ) {
        pImpl_->profileSpectrum_->replot();
        pImpl_->processedSpectrum_->replot();
    }
}

void
MSProcessingWnd::handleCheckStateChanged( Dataprocessor* processor, portfolio::Folium& folium, bool isChecked )
{
    // nothing to do
    // check state change will be handled on handleSelectionchanged method
}

void
MSProcessingWnd::handleRescaleY( int proto )
{
    pImpl_->profileSpectrum_->rescaleY( proto );
    pImpl_->processedSpectrum_->rescaleY( proto );
}

void
MSProcessingWnd::selectedOnChromatogram( const QRectF& rect )
{
	double x0 = pImpl_->ticPlot_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->ticPlot_->transform( QwtPlot::xBottom, rect.right() );

    QMenu menu;

	if ( int( std::abs( x1 - x0 ) ) <= 2 ) {

        if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
            if ( auto rawfile = dp->rawdata() ) {
                if ( rawfile->dataformat_version() >= 3 ) {
                    // v3 data
                    auto readers = rawfile->dataReaders();
                    for ( auto& reader : readers ) {
                        if ( auto it = reader->findPos( rect.left() ) )
                            menu.addAction( QString::fromStdString(
                                                ( boost::format( "Select spectrum (%s) @ %.3lfs" ) % reader->display_name() % rect.left() ).str() )
                                            , [=] () { document::instance()->onSelectSpectrum_v3( dp, rect.left(), it ); } );
                    }
                } else {
                    QMessageBox::information( 0, "Dataprocessor"
                                              , "QtPlatz 6.0 and later versions do not support v2 format data file.  Use QtPlatz 5.4" );
                }
            }
            auto folium = dp->getPortfolio().findFolium( pImpl_->idChromatogramFolium_ );
            menu.addAction( tr( "Relative Abundances" ), [dp,folium,rect]{
                dp->relativeAbundances( folium, rect.left() );
            } );
        }


        menu.addAction( tr("Copy image to clipboard"), [&] () { adplot::plot::copyToClipboard( pImpl_->ticPlot_ ); } );
        menu.addAction( tr( "Save as SVG File..." ), [&] () {
            utility::save_image_as<SVG>()( pImpl_->ticPlot_, pImpl_->idChromatogramFolium_ );
        });

        menu.addAction( tr("Clear overlay" ), [&]{
            pImpl_->clearCheckedChromatograms();
            pImpl_->ticPlot_->replot();
        })->setEnabled( pImpl_->checkedChromatograms_.size() );

        menu.addAction( tr("Frequency analysis"), [&] () {
            if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
                auto folium = dp->getPortfolio().findFolium( pImpl_->idChromatogramFolium_ );
                if ( auto chr = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
                    power_spectrum( *chr, 0 );
                }
            }
        } );

        menu.addAction( tr("Low pass filter"), [&] () {
            if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
                auto folium = dp->getPortfolio().findFolium( pImpl_->idChromatogramFolium_ );

                if ( auto chr = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
                    dp->dftFilter( folium, MainWindow::instance()->processMethod() );
                }
            }
        } );

        menu.addAction( tr("RMS to clipboard"), [&]() {
            auto range = ( (x0 - x1) >= 2 ) ?
                std::make_pair( rect.left(), rect.right() ) :
                std::make_pair( pImpl_->ticPlot_->zoomer()->zoomRect().left(), pImpl_->ticPlot_->zoomer()->zoomRect().right() );
            if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
                auto folium = dp->getPortfolio().findFolium( pImpl_->idChromatogramFolium_ );
                if ( auto chr = portfolio::get< adcontrols::ChromatogramPtr >( folium ) )
                    compute_rms( *chr, range, folium.fullpath() );
            }
        });

        menu.addAction( tr( "Find single peak (FI; DI-PTR)" ), [&]() {
            if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
                auto folium = dp->getPortfolio().findFolium( pImpl_->idChromatogramFolium_ );
                dp->findSinglePeak( folium );
            }
        });

        menu.exec( QCursor::pos() );

    } else {
        document::instance()->handleSelectTimeRangeOnChromatogram( rect.x(), rect.x() + rect.width() );
    }

}

void
MSProcessingWnd::selectedOnProfile( const QRectF& rect )
{
	double x0 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.right() );

    QMenu menu;

	if ( int( std::abs( x1 - x0 ) ) > 2 ) {

        using namespace adcontrols::metric;

        QString left = QString::number( rect.left(), 'f', 3 );
        QString right = QString::number( rect.right(), 'f', 3 );

        std::pair<size_t, size_t> range;

        ADDEBUG() << "--------- " << __FUNCTION__ << " -----------------";

        if ( auto ms = pImpl_->pProfileSpectrum_.second.lock() ) {

            ADDEBUG() << "--------- " << __FUNCTION__ << " --------------" << ms->size() << ", " << ms->dataReaderUuid();
            if ( ms->dataReaderUuid() != boost::uuids::uuid( {{0}} ) ) {
                // v3 data
                if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
                    if ( auto rd = dp->rawdata() ) {
                        if ( auto reader = rd->dataReader( ms->dataReaderUuid()) ) {
                            auto display_name = QString::fromStdString( reader->display_name() );
                            ADDEBUG() << "--------- " << __FUNCTION__ << " -----------------" << reader->display_name();

                            // todo: chromatogram creation by m/z|time range
                            if ( pImpl_->axis_ == adcontrols::hor_axis_mass ) {
                                auto title = ( boost::format( "Make chromatogram from %s in m/z range %.3lf -- %.3lf" )
                                               % reader->display_name() % rect.left() % rect.right() ).str();
                                menu.addAction( QString::fromStdString(title.c_str() )
                                                , [=,this] () { make_chromatogram( reader, ms, pImpl_->axis_, rect.left(), rect.right() ); } );
                            } else {
                                auto title = ( boost::format( "Make chromatogram from %ss in range %.3lf -- %.3lf(us)" )
                                               % reader->display_name() % rect.left() % rect.right() ).str();
                                menu.addAction( QString::fromStdString( title.c_str() )
                                                , [=,this] () { make_chromatogram( reader, ms, pImpl_->axis_, rect.left() * 1.0e-6, rect.right() * 1.0e-6 ); } );
                            }
                        }
                    }
                }
            }

            if ( pImpl_->axis_ == adcontrols::hor_axis_time )
                range = std::make_pair( ms->getIndexFromTime( scale_to_base( rect.left(), micro ) )
                                        , ms->getIndexFromTime( scale_to_base( rect.right(), micro ) ) );
            else {
                const double * masses = ms->getMassArray();
                range = std::make_pair( std::distance( masses, std::lower_bound( masses, masses + ms->size(), rect.left() ) )
                                        , std::distance( masses, std::lower_bound( masses, masses + ms->size(), rect.right() ) ) );
            }

            const auto f_rms = ( pImpl_->axis_ == adcontrols::hor_axis_time ) ?
                tr("RMS in range %1 -- %2(us)") : tr("RMS in m/z range %1 -- %2");
            const auto f_maxval = ( pImpl_->axis_ == adcontrols::hor_axis_time ) ?
                tr("Max value in range %1 -- %2(us)") : tr("Max value in m/z range %1 -- %2");
            const auto f_count = ( pImpl_->axis_ == adcontrols::hor_axis_time ) ?
                tr("Count/Area in range %1 -- %2(us)") : tr("Count/Area in m/z range %1 -- %2");

            menu.addAction( tr( "y-zoom" )
                            , [rect,this](){ pImpl_->profileSpectrum_->yZoom( rect.left(), rect.right() ); } );

            menu.addAction( QString( f_rms ).arg( left, right ), [&](){
                    if ( compute_rms( rect.left(), rect.right() ) )
                        draw1();
                });

            menu.addAction( QString( f_maxval ).arg( left, right ), [&](){
                    compute_minmax( rect.left(), rect.right() );
                    draw1();
                });

            menu.addAction( QString( f_count ).arg( left, right ), [&](){
                    compute_count( rect.left(), rect.right() );
                    draw1();
                });

            menu.addAction( QString( tr("Frequency analysis") ).arg( left, right ), [&](){
                    power_spectrum( *ms, range );
                });
        }

        menu.exec( QCursor::pos() );

	} else {
        using adplot::plot;

        bool isHistogram( false );
        if ( auto ms = pImpl_->pProfileSpectrum_.second.lock() )
            isHistogram = ms->isCentroid();

        auto rect = pImpl_->profileSpectrum_->zoomRect();

        menu.addAction( tr( "Correct baseline" ),    [this] () { correct_baseline(); draw1(); } );
        menu.addAction( tr( "Copy to clipboard" ),   [this] () { plot::copyToClipboard( pImpl_->profileSpectrum_ ); } );
        menu.addAction( tr( "Frequency analysis" ),  [this] () { frequency_analysis(); } );
        menu.addAction( tr( "Zero filling" ),        [this] () { zero_filling(); } );
        menu.addAction( tr( "Save as SVG File..." ), [this] () {
            utility::save_image_as<SVG>()( pImpl_->profileSpectrum_, pImpl_->idSpectrumFolium_ );
        });
        menu.addAction( tr( "Save image file..." ),  [this] () { save_image_file(); } );
        menu.addAction( tr( "RMS to clipboard" ),    [this,rect] () { compute_rms( rect.left(), rect.right() ); draw1(); } );

        // menu.actions()[4]->setCheckable( true );
        // menu.actions()[4]->setChecked( pImpl_->profileSpectrum_->zoomer()->autoYScale() );

        if ( isHistogram )
            menu.actions()[2]->setEnabled( false ); // Frequency analysis

        auto iid_spectrometers = adcontrols::MassSpectrometerBroker::installed_uuids();

        for ( auto model : iid_spectrometers ) {
            menu.addAction( QString( tr( "Re-assign masses using %1 scan law" ) ).arg( QString::fromStdString( model.second ) )
                            , [&](){
                                assign_masses_to_profile( model );
                                pImpl_->profileSpectrum_->replot();
                            });
        }

        menu.exec( QCursor::pos() );
    }
}

void
MSProcessingWnd::selectedOnPowerPlot( const QRectF& rect )
{
    QMenu menu;

    menu.addAction( tr( "Copy to Clipboard" ), [&](){ adplot::plot::copyToClipboard( pImpl_->pwplot_ ); } );

    menu.addAction( tr( "Save as SVG File..." ), [&](){
        utility::save_image_as<SVG>()( pImpl_->pwplot_, pImpl_->idSpectrumFolium_ );
    });

    menu.addAction( tr( "Dismiss" ), [&](){ pImpl_->pwplot_->hide(); } );

    menu.exec( QCursor::pos() );
}

void
MSProcessingWnd::selectedOnProcessed( const QRectF& rect )
{
	double x0 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.right() );
    bool hasRange = int( std::abs( x1 - x0 ) ) > 2;
    auto ptr = pImpl_->pProcessedSpectrum_.second.lock();

    QMenu menu;

    // [0]
    menu.addAction( tr( "y-zoom" ), [&](){ pImpl_->processedSpectrum_->yZoom( rect.left(), rect.right() ); } );
    // [1]
    if ( hasRange ) {
        menu.addAction( tr( "Make mass chromatograms" )
                        , [&]{ make_chromatograms_from_peaks( pImpl_->pProcessedSpectrum_.second.lock()
                                                              , pImpl_->axis_, rect.left(), rect.right() ); } );
    } else {
        QRectF rc = pImpl_->profileSpectrum_->zoomRect();
        menu.addAction( tr( "Make mass chromatograms (%1--%2)" )
                        .arg( QString::number(rc.left(),'g',5) ).arg( QString::number(rc.right(),'g',5) )
                        , [=,this]{ make_chromatograms_from_peaks( pImpl_->pProcessedSpectrum_.second.lock()
                                                                   , pImpl_->axis_, rc.left(), rc.right() ); } );
    }

    // [2]
    menu.addAction( tr( "Mark masses with checked chromatograms" )
                    , [&]{
                        auto dp = SessionManager::instance()->getActiveDataprocessor();
                        dp->markupMassesFromChromatograms( dp->getPortfolio().findFolium( pImpl_->idSpectrumFolium_ ) );
                    });
    // [3]
    menu.addAction( tr( "Clear color on masses" )
                    , [&]{
                        auto dp = SessionManager::instance()->getActiveDataprocessor();
                        dp->clearMarkup( dp->getPortfolio().findFolium( pImpl_->idSpectrumFolium_ ) );
                    });
    // [4]
    menu.addAction( tr( "Copy to clipboard" ), [&]{ adplot::plot::copyToClipboard( pImpl_->processedSpectrum_ ); } );
    // [5]
    menu.addAction( tr( "Save as SVG File..." ), [&]{
        utility::save_image_as< SVG >()( pImpl_->processedSpectrum_, pImpl_->idSpectrumFolium_ ); //, ",processed;" );
    });

    auto actions = menu.actions();
    if ( actions.size() >= 4 ) {
        actions[ 0 ]->setEnabled( hasRange );
        actions[ 1 ]->setEnabled( ptr && ptr->isCentroid() );
        actions[ 2 ]->setEnabled( !hasRange && ptr && ptr->isCentroid() );
        actions[ 3 ]->setEnabled( !hasRange && ptr && ptr->isCentroid() );
    }

    if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
        dp->addContextMenu( adprocessor::ContextMenuOnProcessedMS, menu, ptr
                            , { rect.left(), rect.right() }, pImpl_->axis_ == adcontrols::hor_axis_time );
    }

    menu.exec( QCursor::pos() );
}

void
MSProcessingWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
    QSizeF sizeMM( 180, 80 );

    int resolution = 96;
	const double mmToInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * mmToInch * resolution;

	QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPageSize( QPageSize( QPageSize::A4 ) );
    printer.setPageMargins( QMarginsF( 10.0, 10.0, 10.0, 10.0 ), QPageLayout::Millimeter);
    printer.setFullPage( true );

	portfolio::Folium folium;
    printer.setDocName( "QtPlatz Process Report" );
	if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
        folium = dp->getPortfolio().findFolium( pImpl_->idActiveFolium_ );
    }

    //printer.setOutputFormat( QPrinter::PdfFormat );
    printer.setOutputFileName( pdfname );
    printer.setResolution( resolution );

    QPainter painter( &printer );

    QRectF boundingRect;
    QRectF drawRect( 0.0, 0.0, printer.width(), (12.0/72)*printer.resolution() );

    painter.drawText( drawRect, Qt::TextWordWrap, folium.fullpath().c_str(), &boundingRect );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    drawRect.setTop( boundingRect.bottom() );
    drawRect.setWidth( size.width() );
    drawRect.setHeight( size.height() );
    renderer.render( pImpl_->ticPlot_, &painter, drawRect );

    drawRect.setTop( drawRect.bottom() );
    drawRect.setHeight( size.height() );
    renderer.render( pImpl_->profileSpectrum_, &painter, drawRect );

    drawRect.setTop( drawRect.bottom() );
    drawRect.setHeight( size.height() );
    renderer.render( pImpl_->processedSpectrum_, &painter, drawRect );

    // -- starting method print
    //drawRect.setTop( drawRect.bottom() + 0.5 * resolution );
    //drawRect.setHeight( printer.height() - drawRect.top() );

    QTextDocument doc;
    QTextBlockFormat blockFormat;
    QTextCursor cursor(&doc);
    auto pageSize = printer.pageLayout().paintRectPixels( printer.resolution() ).size();
    // auto pageSize = printer.pageRect().size();
    doc.setPageSize( pageSize );
    const QRectF textRect( 10, 10, pageSize.width() - 2 * 10, pageSize.height() - 2 * 10 );

    {
        // Targeting results
        portfolio::Folio attachments = folium.attachments();
        auto it = std::find_if( attachments.begin(), attachments.end()
                                , []( auto& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; } );
        if ( it != attachments.end() ) {

            if ( adportable::a_type< std::shared_ptr< adcontrols::MassSpectrum > >::is_a( it->data() ) ) {
                auto centroid = boost::any_cast< std::shared_ptr< adcontrols::MassSpectrum > >( it->data() );

                auto atts = it->attachments();
                auto tgtIt = std::find_if( atts.begin(), atts.end(), []( auto f ){ return f.name() == Constants::F_TARGETING; } );
                if ( tgtIt != atts.end() ) {
                    if ( adportable::a_type< std::shared_ptr< adcontrols::Targeting > >::is_a( tgtIt->data() ) ) {

                        if ( auto targeting = boost::any_cast< std::shared_ptr< adcontrols::Targeting > >( tgtIt->data() ) ) {

                            std::ostringstream html;
                            html << "<html>"
                                "<head>"
                                "<meta name=\"qrichtext\" content='1' />"
                                "<style>table{ border-collapse:collapse; } tr{ border-bottom:1px solid blue;}</style>"
                                "</head>"
                                "<body style=\" font-size:8pt; font-weight:400; font-style:normal; text-decoration:none;\">"
                                ;
                            html << "<table border=\"1\" align=\"center\" width=\"90%\" cellspacing=\"0\" cellpadding=\"4\">";
                            html << "<tr>";
                            html << "<th>Formula</th> <th>Charge</th> <th>Exact m/z</th> <th>Exact ratio</th> "
                                      "<th>m/z</th> <th>Error(mDa)</th> <th>Abundance</th> <th>Ratio</th> <th>Error(%)</th>";
                            html << "</tr>";

                            for ( const auto& c: targeting->candidates() ) {

                                auto tms = adcontrols::segment_wrapper<>( *centroid )[ c.fcn ];
                                html << "<tr>";
                                html << "<td>"
                                     << "<span style=\" font-weight:600;\">"
                                     << adcontrols::ChemicalFormula::formatFormulae( c.formula ) << "</span></td>";
                                html << "<td style=\"text-align:center\">" << c.charge << "</td>";
                                html << "<td style=\"text-align:right\">" << boost::format( "%.4lf" ) % c.exact_mass << "</td>";
                                html << "<td style=\"text-align:right\">" << "1.0" << "</td>";
                                html << "<td style=\"text-align:right\">" << boost::format( "%.4lf" ) % c.mass << "</td>";
                                html << "<td style=\"text-align:right\">" << boost::format( "%.4lf" ) % ((c.mass - c.exact_mass) * 1000) << "</td>";
                                html << "<td style=\"text-align:right\">" << boost::format( "%.3lf" ) % tms.intensity( c.idx ) << "</td>";
                                html << "<td style=\"text-align:right\">" << "1.000" << "</td>";
                                html << "<td style=\"text-align:right\">" << "n/a"   << "</td>";
                                html << "</tr>";
                                // ADDEBUG() << c.idx << c.fcn << c.charge << c.mass_error << c.formula << c.score;
                                for ( const auto& i: c.isotopes ) {
                                    html << "<tr>";
                                    html << "<td colspan=\"2\">" << "</td>"; // Formula
                                    html << "<td style=\"text-align:right\">" << boost::format( "%.4lf" ) % i.exact_mass << "</td>";
                                    html << "<td style=\"text-align:right\">" << boost::format( "%.4g" ) % i.exact_abundance << "</td>";
                                    if ( i.idx >= 0 ) {
                                        html << "<td style=\"text-align:right\">" << boost::format( "%.4lf" ) % i.mass << "</td>";
                                        html << "<td style=\"text-align:right\">" << boost::format( "%.4lf" ) % ((i.mass - i.exact_mass)*1000) << "</td>";
                                        html << "<td style=\"text-align:right\">" << boost::format( "%.3lf" ) % tms.intensity( i.idx ) << "</td>";
                                        html << "<td style=\"text-align:right\">" << boost::format( "%.4g" )  % i.abundance_ratio << "</td>";
                                        html << "<td style=\"text-align:right\">" << boost::format( "%.1lf" ) % (100 * i.abundance_ratio_error) << "</td>";
                                    } else {
                                        html << "<td style=\"text-align:right\">" << "n.d." << "</td>";
                                        html << "<td>" << "--" << "</td>";
                                        html << "<td>" << "--" << "</td>";
                                        html << "<td style=\"text-align:right\">" << "--" << "</td>";
                                    }
                                    html << "</tr>";
                                }
                            }
                            html << "</table></font></body></html>";

                            doc.setHtml( QString::fromStdString( html.str() ) );
                            //doc.setDefaultStyleSheet( "table{ border-collapse:collapse; } th,td{ border-style: none;}");
                            doc.setDefaultStyleSheet( "table{ border-collapse:collapse; } th,td{ border: solid;}");

                            {
                                for ( auto download: QStandardPaths::standardLocations( QStandardPaths::DownloadLocation ) ) {
                                    auto name = std::filesystem::path( download.toStdString() ) / "debug.html";
                                    std::ofstream of( name );
                                    of << html.str();
                                    break;
                                }
                            }


                            ADDEBUG() << "--------------> " << doc.pageCount();
                            for ( size_t i = 0; i < doc.pageCount(); ++i ) {
                                printer.newPage();
                                painter.save();
                                const QRectF textPageRect( 0, i * doc.pageSize().height(), doc.pageSize().width(), doc.pageSize().height() );
                                painter.translate( 0, -textPageRect.top() );
                                painter.translate( textPageRect.left(), textRect.top() );
                                doc.drawContents( &painter );
                                painter.restore();
                            }
                        }
                    }
                }
            }
        }

        //doc.print( &printer );
//#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        it = portfolio::Folium::find<adcontrols::MassSpectrumPtr>( attachments.begin(), attachments.end() );
        if ( it != attachments.end() ) {
            auto atts = it->attachments();
            auto pf = portfolio::Folium::find< adcontrols::ProcessMethodPtr >( atts.begin(), atts.end() );
            if ( pf != atts.end() ) {
                adcontrols::ProcessMethodPtr pm;
                if ( portfolio::Folium::get< adcontrols::ProcessMethodPtr >( pm, *pf ) ) {
                    std::wostringstream o;
                    adcontrols::ProcessMethod::xml_archive( o, *pm );
                    pugi::xml_document dom;
                    auto result = dom.load_string( pugi::as_utf8( o.str() ).c_str() );
                    if ( result ) {
                        ADDEBUG() << "########################### TODO ###################################";
                        adpublisher::printer::print( printer, painter, drawRect, dom, "process-method-html.xsl" );
                    }
                }
            }
        }
//#endif
    }

    ///////////
    if ( auto p = MainWindow::instance()->findChild< adwidgets::MSPeakTable * >( "MSPeakTable" ) ) {
        p->handlePrint( printer, painter );
    }

    QDesktopServices::openUrl( QUrl( "file://" + pdfname ) );
}

bool
MSProcessingWnd::assign_masses_to_profile( const std::pair< boost::uuids::uuid, std::string >& iid_spectrometer )
{
    if ( iid_spectrometer.first == adcontrols::iids::adspectrometer_uuid ) {
        estimateScanLaw( iid_spectrometer.first );
        return true;
    } else {
        estimateScanLaw( iid_spectrometer.first );
        return true;
    }
    return false;
}

bool
MSProcessingWnd::assign_masses_to_profile()
{
	adportable::TimeSquaredScanLaw law;

    std::pair< double, double > mass_range;

    if ( auto x = pImpl_->pProfileSpectrum_.second.lock() ) {

        adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *x );

        for ( auto& ms: segments ) {
            for ( size_t idx = 0; idx < ms.size(); ++idx ) {
                double m = law.getMass( ms.time( idx ), 0 );
                ms.setMass( idx, m );
                if ( idx == 0 )
                    mass_range.first = std::min( mass_range.first, m );
                if ( idx == ms.size() - 1 )
                    mass_range.second = std::max( mass_range.second, m );
            }
        }
        x->setAcquisitionMassRange( mass_range.first, mass_range.second );

        adcontrols::MSProperty prop( x->getMSProperty() );
		prop.setAcceleratorVoltage( law.kAcceleratorVoltage() );
		prop.setTDelay( law.tDelay() );
		x->setMSProperty( prop );

        return true;
    }
    return false;
}


double
MSProcessingWnd::correct_baseline()
{
    double tic = 0;

    if ( auto x = pImpl_->pProfileSpectrum_.second.lock() ) {

        QString name;
        if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
            auto folium = dp->getPortfolio().findFolium( pImpl_->idActiveFolium_ );
            name = QString::fromStdString( std::filesystem::path( folium.fullpath() ).filename().string() );
        }

        std::wostringstream o;
        o << L"Baseline corrected";

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *x );

        QString text( QString("%1\tt_min,h_min,t_max,h_max,RMS,nAVG/" ).arg(name) );

        boost::format fmt( "\t%1%\t%2%\t%3%\t%4%\t%5%\t%6%\t;" );

        size_t proto(0);
		for ( auto& ms: segments ) {
            auto nAvg = ms.getMSProperty().numAverage();

            double dbase(0), rms(0);
            const double * data = ms.getIntensityArray();
            tic += adportable::spectrum_processor::tic( static_cast< unsigned int >( ms.size() ), data, dbase, rms );
            for ( size_t idx = 0; idx < ms.size(); ++idx )
                ms.setIntensity( idx, data[ idx ] - dbase );
            auto mm = std::minmax_element( ms.getIntensityArray(), ms.getIntensityArray() + ms.size() );
            o << boost::wformat( L" p%d H=%.2f/RMS=%.2f(navg=%d)" ) % proto % (*mm.second) % rms % nAvg;

            auto t_max = ms.time( std::distance(ms.getIntensityArray(), mm.second) );
            auto t_min = ms.time( std::distance(ms.getIntensityArray(), mm.first) );

            text.append( QString::fromStdString( (fmt % t_min % *mm.first % t_max % *mm.second % rms % nAvg).str() ) );
            ++proto;
		}
		x->addDescription( adcontrols::description( L"process", o.str() ) );
        QApplication::clipboard()->setText( text );
	}
	return tic;
}

bool
MSProcessingWnd::compute_rms( const adcontrols::Chromatogram& chr
                              , const std::pair<double,double>& time_range
                              , const std::string& name )
{
    QString text( QString("\"%1\"\tmin(x,y),max(x,y),rms,mean,n" ).arg( QString::fromStdString( name )) );

    if ( auto res = dataproc::rms_calculator::compute_rms( chr, time_range ) ) {
        std::pair< double, double > xrange;
        size_t n;
        double avg, rms, min_time, min_value, max_time, max_value;
        std::tie( xrange, n, rms, avg, min_time, min_value, max_time, max_value ) = *res;
        boost::format fmt( "\t%1%\t%2%\t%3%\t%4%\t%5%\t%6%\t%7%\t" );
        text.append( QString::fromStdString( (fmt % min_time % min_value % max_time % max_value % rms % avg % n ).str() ) );
        QApplication::clipboard()->setText( text );
        return true;
    }
    QApplication::clipboard()->setText( text + "RMS calculation has failed");
    return false;
}

bool
MSProcessingWnd::compute_rms( double s, double e )
{
	if ( auto ptr = pImpl_->pProfileSpectrum_.second.lock() ) {

        namespace pfx = adcontrols::metric;

        QString name;
        if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
            auto folium = dp->getPortfolio().findFolium( pImpl_->idActiveFolium_ );
            name = QString::fromStdString( std::filesystem::path( folium.fullpath() ).filename().string() );
        }

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *ptr );

        QString text( QString("%1\trms(t0,t1,N,rms,t_min,h_min,t_max,h_max,nAvg)").arg( name ) );
        //                   t0     t1      N   rms  t_min h_min t_max   hmax nAVG
        boost::format fmt("\t%1%\t%2%\t%3%\t%4%\t%5%\t%6%\t%7%\t%8%\t%9%\t;");

        for ( auto& ms: segments ) {

            std::pair<double, double> xrange(pImpl_->is_time_axis_ ? range_t<true>()( s, e ) : range_t<false>()( s, e ));

            if ( auto res = dataproc::rms_calculator::compute_rms( ms, xrange, pImpl_->is_time_axis_ ) ) {
                std::pair<double, double> time_range;
                size_t N;
                double rms, min_time, min_value, max_time, max_value;
                std::tie( time_range, N, rms, min_time, min_value, max_time, max_value ) = res.get();
                ADDEBUG() << "compute_rms: " << time_range;

                ptr->addDescription( adcontrols::description( L"process"
                                                              , (boost::wformat(L"RMS[%.3lf-%.3lf(&mu;s),N=%d]=%.3lf(%d)")
                                                                 % (time_range.first * std::micro::den )
                                                                 % (time_range.second * std::micro::den )
                                                                 % N
                                                                 % rms
                                                                 % ms.getMSProperty().numAverage()
                                                                  ).str() ) );
                // ---
                text.append( QString::fromStdString(
                                 ( fmt
                                   % time_range.first
                                   % time_range.second
                                   % N
                                   % rms
                                   % min_time
                                   % min_value
                                   % max_time
                                   % max_value
                                   % ms.getMSProperty().numAverage()
                                     ).str() ) );
            }
        }
        QApplication::clipboard()->setText( text );
        return true;
    }
	return false;
}

std::pair< double, double >
MSProcessingWnd::compute_minmax( double s, double e )
{
    using namespace adcontrols::metric;

	if ( auto ptr = pImpl_->pProfileSpectrum_.second.lock() ) {

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *ptr );

        for ( auto& ms: segments ) {

            std::pair< size_t, size_t > range;
            if ( pImpl_->is_time_axis_ ) {
                range.first = ms.getIndexFromTime( scale_to_base( s, micro ), false );
                range.second = ms.getIndexFromTime( scale_to_base( e, micro), true );
            } else {
                const double * masses = ms.getMassArray();
                range.first = std::distance( masses, std::lower_bound( masses, masses + ms.size(), s ) );
                range.second = std::distance( masses, std::lower_bound( masses, masses + ms.size(), e ) );
            }

            size_t n = range.second - range.first + 1;
            if ( n >= 5 ) {

                adportable::array_wrapper<const double> data( ms.getIntensityArray(), ms.size() );
                auto pair = std::minmax_element( data.begin() + range.first, data.begin() + range.second );

                std::pair<double, double> result = std::make_pair( *pair.first, *pair.second );
                std::pair< size_t, size_t > index = std::make_pair( std::distance( data.begin(), pair.first )
                                                                    , std::distance( data.begin(), pair.second ) );

                std::pair< double, double > time = std::make_pair(
                    adcontrols::MSProperty::toSeconds( index.first, ms.getMSProperty().samplingInfo() )
                    , adcontrols::MSProperty::toSeconds( index.second, ms.getMSProperty().samplingInfo() )
                    );

                ptr->addDescription( adcontrols::description( L"process"
                                                              , ( boost::wformat(L"min at %.4us=%.3f; max at %.4us=%3f" )
                                                                  % scale_to_micro(time.first) % result.first
                                                                  % scale_to_micro(time.second) % result.second ).str() ) );

                std::ostringstream o;
                o << boost::format("min @\t%d\t%.14lf\t%.7f") % index.first % scale_to_micro(time.first) % result.first
                  << boost::format("\tmax @\t%d\t%.14lf\t%.7f") % index.second % scale_to_micro(time.second) % result.second;

                QApplication::clipboard()->setText( QString::fromStdString( o.str() ) );

				return result;
            }
        }

    }
	return std::make_pair(0,0);
}

double
MSProcessingWnd::compute_count( double s, double e )
{
	if ( auto ptr = pImpl_->pProfileSpectrum_.second.lock() ) {

        using namespace adcontrols::metric;

        QString clipboard;

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *ptr );

        int idx = 0;

        for ( auto& ms: segments ) {

            bool found( false );
            std::pair< size_t, size_t > range = { 0, 0 };

            if ( pImpl_->is_time_axis_ ) {

                s = scale_to_base( s, micro );
                e = scale_to_base( e, micro );
                if ( ms.time( 0 ) <= e && ms.time( ms.size() - 1 ) >= s ) {
                	if ( const double * times = ms.getTimeArray() ) {
                		range.first = std::distance( times, std::lower_bound( times, times + ms.size(), s));
                		range.second = std::distance( times, std::lower_bound( times, times + ms.size(), e));
                		found = true;
                	}
                }

            } else {
            	if ( ms.mass( 0 ) <= e && ms.mass( ms.size() - 1 ) >= s ) {
            		if ( const double * masses = ms.getMassArray() ) {
            			range.first = std::distance( masses, std::lower_bound( masses, masses + ms.size(), s ) );
            			range.second = std::distance( masses, std::lower_bound( masses, masses + ms.size(), e ) );
            			found = true;
            		}
            	}
            }

            if ( found ) {

                // ADDEBUG() << "data range: " << range.first << ", " << range.second;

                const double * data = ms.getIntensityArray();
                double count = std::accumulate( data + range.first, data + range.second, 0.0 );

                auto maxIdx = std::distance( data, std::max_element( data + range.first, data + range.second ) );

                double apex = ( pImpl_->is_time_axis_ ) ? ms.time( maxIdx ) : ms.mass( maxIdx );
                double height = ms.intensity( maxIdx );

                char fmt = ( pImpl_->is_time_axis_ ) ? 'e' : 'f';

                clipboard.append(
                    QString("#%1\tCount,height,apex(m/z|time),N,n-trig\t%2\t%3\t%4\t%5\t%6\n").arg(
                        QString::number( idx )
                	    , QString::number( count )
                        , QString::number( height )
                        , QString::number( apex, fmt, 7 )
                        , QString::number( range.second - range.first )
                        , QString::number( ms.getMSProperty().numAverage() )
                        )
                    );
            }
            ++idx;
        }
        QApplication::clipboard()->setText( clipboard );
    }
	return 0;
}

bool
MSProcessingWnd::find_single_peak( const adcontrols::Chromatogram& chr )
{
    return true;
}

bool
MSProcessingWnd::power_spectrum( const adcontrols::MassSpectrum& ms
                                 , const std::pair< size_t, size_t >& range )
{
    const size_t size = range.second - range.first + 1;
    if ( size < 8 )
        return false;
    size_t n = 1;
    while ( size >> n )
        ++n;
    size_t N = 1LL << ( n - 1 );

    std::vector< std::complex< double > > spc( N ), fft( N );
	const double * intens = ms.getIntensityArray() + range.first;
	for ( size_t i = 0; i < N && i < size; ++i )
        spc[ i ] = std::complex< double >( intens[ i ] );

    adportable::fft::fourier_transform( fft, spc, false );

	const double T = N * ms.getMSProperty().samplingInfo().fSampInterval();
    std::vector< double > x( N / 2 ), y( N / 2 );
    for ( size_t i = 0; i < N / 2; ++i ) {
        y[i] = ( ( fft[ i ].real() * fft[ i ].real() ) + ( fft[ i ].imag() * fft[ i ].imag() ) ) / ( double(N) * N );
        x[i] = double( i ) / T * 1e-6; // MHz
    }
    double dc = fft[0].real();
    double nyquist = fft[ N / 2 ].real();

    //----- draw
    std::ostringstream o;
    o << boost::format( "N=%d Power: DC=%.7g Nyquist=%.7g" ) % (x.size() * 2) % dc % nyquist;
    QString title = QString("[%1] %2").arg( MainWindow::makeDisplayName( pImpl_->idSpectrumFolium_ ), QString::fromStdString( o.str() ) );
    pImpl_->pwplot_->setData( x.size() - 1, x.data() + 1, y.data() + 1 );
    pImpl_->pwplot_->setTitle( title );
    pImpl_->pwplot_->show();
    pImpl_->pwplot_->xBottomTitle( "Frequency (MHz)" );
	return true;
}

void
MSProcessingWnd::power_spectrum( const adcontrols::Chromatogram& c, int algo )
{
    const size_t size = c.size();
    if ( size >= 8 ) {
        uint32_t n = 1;
        while ( size >> n )
            ++n;
        uint32_t N = 1 << n;
        ADDEBUG() << "power_spectrum : n = " << size << " --> " << N << ", algo=" << algo;

        std::vector< std::complex< double > > spc(N), fft(N);
        std::transform( c.getIntensityArray(), c.getIntensityArray() + c.size()
                        , spc.begin()
                        , [](const auto& a){ return std::complex<double>(a); } );
        std::fill( spc.begin() + c.size(), spc.end(), std::complex<double>( c.intensity( c.size() - 1 ) ) );

        // unsigned idx( 0 );
        // for ( auto it = c.begin(); it != c.end() && idx < N; ++it )
        //     spc[ idx++ ] = std::complex< double >( it.intensity() );

        const double fInterval = c.sampInterval();
        const double T = N * fInterval;
        std::vector< double > x( N / 2 ), y( N / 2 ); // plot data

        if ( algo == 0 ) {
            adportable::fft::fourier_transform( fft, spc, false );
        } else {
            fft = spc;
            adportable::fft4g().cdft( 1, fft );
        }

        std::transform( fft.begin(), fft.begin() + N / 2, y.begin(), [&]( const std::complex<double>& d ){
            return ( ( d.real() * d.real() ) + ( d.imag() * d.imag() ) ) / ( double(N) * N );
        } );

        //std::iota( x.begin(), x.end(), frequency(T) );
        for ( size_t i = 0; i < N / 2; ++i )
            x [ i ] = double( i ) / T; // Hz

        double dc = fft[0].real();
        double nyquist = fft[ N / 2 ].real();

        std::ostringstream o;
        o << boost::format( "N=%d Power: DC=%.7g Nyquist=%.7g" ) % (x.size() * 2) % dc % nyquist;
        QString title = QString("[%1]&nbsp;&nbsp;&nbsp;&nbsp;%2").arg( MainWindow::makeDisplayName( pImpl_->idChromatogramFolium_ ), QString::fromStdString( o.str() ) );
        pImpl_->pwplot_->setData( x.size() - 1, x.data() + 1, y.data() + 1 ); // skip DC component from plot
        pImpl_->pwplot_->setTitle( title );
        pImpl_->pwplot_->show();
        pImpl_->pwplot_->xBottomTitle( "Frequency (Hz)" );
    }
}

void
MSProcessingWnd::frequency_analysis()
{
    if ( auto ms = pImpl_->pProfileSpectrum_.second.lock() ) {
        auto range = std::make_pair( size_t( 0 ), ms->size() - 1 );
        power_spectrum( *ms, range );
    }
}

void
MSProcessingWnd::zero_filling()
{
    if ( auto ms = pImpl_->pProfileSpectrum_.second.lock() ) {

        auto dp = SessionManager::instance()->getActiveDataprocessor();
        if ( auto spectrometer = dp ? dp->massSpectrometer() : nullptr ) {

            if ( auto law = spectrometer->scanLaw() ) {
                for ( auto& fms: adcontrols::segment_wrapper< adcontrols::MassSpectrum >( *ms ) ) {
                    using adcontrols::waveform_filter;
                    waveform_filter::fft4c::zero_filling( fms, 100.0e6, [&]( double t ){
                            return law->getMass( t, fms.mode() );
                        } );
                }
            }
        }
    }
}

// process on profile spectrum
void
MSProcessingWnd::make_chromatogram( const adcontrols::DataReader * reader
                                    , std::shared_ptr< const adcontrols::MassSpectrum > ms
                                    , adcontrols::hor_axis axis
                                    , double s, double e )
{
    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
        auto pm = std::make_shared< adcontrols::ProcessMethod >();
        MainWindow::instance()->getProcessMethod( *pm );
        // [3]
        DataprocessWorker::instance()->createChromatogramByAxisRange3( processor, pm, axis, std::make_pair( s, e ), reader );
    }
}


/* make chromatograms from centroid data
 */
void
MSProcessingWnd::make_chromatograms_from_peaks( std::shared_ptr< const adcontrols::MassSpectrum > ptr
                                                , adcontrols::hor_axis axis
                                                , double left
                                                , double right )
{
    if ( ptr && ptr->isCentroid() ) {

        const int h_threshold = left < 0 ? 1000 : 100;

        std::shared_ptr< adcontrols::MSPeakInfo > xpkinfo;

        if ( auto pkinfo = pImpl_->pkinfo_.second.lock() ) {

            for ( const auto& pkseg: adcontrols::segment_wrapper< const adcontrols::MSPeakInfo >( *pkinfo ) ) {

                adcontrols::MSPeakInfo xInfo;

                auto beg = std::lower_bound( pkseg.begin()
                                             , pkseg.end()
                                             , left, [&]( const adcontrols::MSPeakInfoItem& a, const double& left ) {
                    return ( axis == adcontrols::hor_axis_mass ) ? a.mass() < left : a.time() < left;  });

                if ( beg != pkseg.end() ) {
                    auto end = std::lower_bound( beg, pkseg.end(), right, [&]( const adcontrols::MSPeakInfoItem& a, const double& right ) {
                        return ( axis == adcontrols::hor_axis_mass ) ? a.mass() < right : a.time() < right; });

                    auto bp = std::max_element( beg, end, []( const auto a, const auto b ){ return a.area() < b.area();} );

                    xInfo.setMode( pkseg.mode() );
                    xInfo.setProtocol( pkseg.protocolId(), pkseg.nProtocols() );

                    std::for_each( beg, end, [&]( const adcontrols::MSPeakInfoItem& a ){
                        if ( a.height() > bp->height() / h_threshold ) // 1 or 0.1% above for base peak
                            xInfo << a;
                    });
                    if ( xInfo.size() > 0 ) {
                        if ( !xpkinfo )
                            xpkinfo = std::make_shared< adcontrols::MSPeakInfo >( xInfo );
                        else
                            xpkinfo->addSegment( xInfo );
                    }
                }
            }
        }

        if ( xpkinfo ) {

            if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {

                if ( auto file = processor->rawdata() ) {
                    if ( file->dataformat_version() >= 3 ) {
                        //--------->
                        auto pm = MainWindow::instance()->processMethod();

                        adwidgets::DataReaderChoiceDialog dlg( file->dataReaders() );
                        dlg.setProtocolHidden( true );
                        if ( auto tm = pm->find< adcontrols::MSChromatogramMethod >() ) {
                            dlg.setMassWidth( tm->width( tm->widthMethod() ) );
                            dlg.setTimeWidth( 4e-9 ); // 4ns
                        }
                        if ( dlg.exec() == QDialog::Accepted ) {
                            auto reader_params = dlg.toJson();
                            for ( auto& sel: dlg.selection() ) {
                                auto rdpara = QJsonDocument::fromJson( reader_params.at( sel.first ) ).object();
                                auto enableTime = rdpara[ "enableTime" ].toBool();
                                double massWidth = rdpara[ "massWidth" ].toDouble();
                                double timeWidth = rdpara[ "timeWidth" ].toDouble();
                                if ( auto reader = file->dataReaders().at( sel.first ) ) {
                                    DataprocessWorker::instance()->createChromatogramsByPeakInfo3( processor, pm, axis, xpkinfo, reader.get() );
                                }
                            }
                        }
                    } else {
                        ADDEBUG() << "unsupported data file format (too old)";
                    }
                }
            }
        }
    }
}

void
MSProcessingWnd::save_image_file()
{
    auto settings = document::instance()->settings();
    using namespace dataproc::Constants;
    settings->beginGroup( GRP_SPECTRUM_IMAGE );
    QString fmt = settings->value( KEY_IMAGEE_FORMAT, "svg" ).toString();
    bool compress = settings->value( KEY_COMPRESS, true ).toBool();
    int dpi = settings->value( KEY_DPI, 300 ).toInt();
    auto lastDir = settings->value( KEY_IMAGE_SAVE_DIR, QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) ).toString();
    settings->endGroup();

    std::string dfmt = "." + fmt.toStdString();

    if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
        auto folium = dp->getPortfolio().findFolium( pImpl_->idSpectrumFolium_ );
        auto name = make_filename< SVG >()( folium, ",", document::instance()->recentFile( Constants::GRP_SVG_FILES ) );

        adwidgets::FileDialog dlg( MainWindow::instance(), tr( "Save Image File" ) );
        dlg.setDirectory( name );
        dlg.setAcceptMode( QFileDialog::AcceptSave );
        dlg.setFileMode( QFileDialog::AnyFile );
        dlg.setNameFilters( QStringList{ "SVG(*.svg)"} );
        dlg.setVectorCompression( tr( "Compress vector graphics" ), compress, fmt, dpi );

        if ( dlg.exec() == QDialog::Accepted ) {
            auto result = dlg.selectedFiles();
            std::filesystem::path path( result.at( 0 ).toStdWString() );
            const char * format = "svg";
            if ( path.extension() == ".pdf" )
                format = "pdf";

            settings->beginGroup( GRP_SPECTRUM_IMAGE );
            settings->setValue( KEY_IMAGEE_FORMAT, format );
            settings->setValue( KEY_COMPRESS, dlg.vectorCompression() );
            settings->setValue( KEY_DPI, dlg.dpi() );
            settings->setValue( KEY_IMAGE_SAVE_DIR, QString::fromStdString( path.parent_path().string() ) );
            settings->endGroup();
            adplot::plot::copyImageToFile( pImpl_->profileSpectrum_, result.at( 0 ), format, dlg.vectorCompression(), dlg.dpi() );
            document::instance()->addToRecentFiles( name, Constants::GRP_SVG_FILES );
        }
    }
}

void
MSProcessingWnd::autoYScale( adplot::plot * plot )
{
    if ( auto zoomer = plot->zoomer() )
        zoomer->autoYScale( ! zoomer->autoYScale() );
}

void
MSProcessingWnd::autoYZoom( adplot::plot * plot, double xmin, double xmax )
{

}

void
MSProcessingWnd::onInitialUpdate()
{
    if ( auto tree = findChild< adwidgets::MSPeakTree *>() ) {

        tree->OnInitialUpdate();

        connect( tree, &adwidgets::MSPeakTree::generateChromatogram, this
                 , []( const QByteArray& json ){
                       auto pm = std::make_shared< adcontrols::ProcessMethod >();
                       MainWindow::instance()->getProcessMethod( *pm );
                       if ( auto dp = SessionManager::instance()->getActiveDataprocessor() )
                           DataprocessWorker::instance()->genChromatograms( dp, pm, json );
                 });
    }
}

void
MSProcessingWnd::handleSpectrumYScale( bool autoScale, double base, double height )
{
    pImpl_->scaleYAuto_ = autoScale;
    pImpl_->scaleY_ = std::make_pair( base, height );

    if ( autoScale )
        pImpl_->profileSpectrum_->setYScale( 0, 0, QwtPlot::yLeft );
    else
        pImpl_->profileSpectrum_->setYScale( base + height, base, QwtPlot::yLeft );

    pImpl_->profileSpectrum_->replotYScale();
}

std::pair< QRectF, adcontrols::hor_axis >
MSProcessingWnd::profileRect() const
{
    return std::make_pair( pImpl_->profileSpectrum_->zoomRect(), pImpl_->axis_ );
}

QRectF
MSProcessingWnd::chromatogrRect() const
{
    return pImpl_->ticPlot_->zoomRect();
}

void
MSProcessingWnd::handleChromatogramYScale( bool checked, double bottom, double top ) const
{
    pImpl_->yScaleChromatogram_ = { checked, bottom, top };
    pImpl_->ticPlot_->setNormalizedY( QwtPlot::yLeft, std::get< 0 >( pImpl_->yScaleChromatogram_ ) );  // auto scale y?
    pImpl_->ticPlot_->setYScale( std::make_tuple( checked, bottom, top ), true );
}

void
MSProcessingWnd::handleChromatogramXScale( bool checked, double left, double right ) const
{
    pImpl_->xScaleChromatogram_ = { checked, left, right };
    pImpl_->ticPlot_->setXScale( std::make_tuple( checked, left, right ), true );
}
