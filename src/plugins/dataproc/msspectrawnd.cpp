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

#include "msspectrawnd.hpp"
#include "datafolder.hpp"
#include "dataprocessor.hpp"
#include "document.hpp"
#include "mainwindow.hpp"
#include "qtwidgets_name.hpp"
#include "selchanged.hpp"
#include "sessionmanager.hpp"
#include "utility.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msqpeaks.hpp>
#include <adlog/logger.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spectrogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adportable/array_wrapper.hpp> // std::span
#include <adportable/configuration.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adutils/processeddata.hpp>
#include <adwidgets/mspeaktree.hpp>
#include <adwidgets/msquantable.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <adportable/unique_ptr.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <algorithm>
#include <bitset>
#include <cmath>
#include <deque>
#include <filesystem>
#include <numeric>

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMenu>
#include <QPainter>
#include <QPrinter>
#include <functional>

namespace dataproc {

    class MSSpectraWnd::impl {
    public:
        impl( MSSpectraWnd * p ) : pThis_( p )
                                 , isTimeAxis_( false )
                                 , dirty_( false )
                                 , selProcessed_( false ) {

            for ( size_t i = 0; i < plots_.size(); ++i ) {

                plots_[ i ] = std::make_unique< adplot::SpectrumWidget >();
                plots_[ i ]->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
                plots_[ i ]->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );

                markers_[ i ] = std::make_unique< adplot::PeakMarker >();

            }

        }

        ~impl() {
        }

        MSSpectraWnd * pThis_;

        std::vector< datafolder > data_; // reference spectra on plot[0] (expecting overlay)
        datafolder currData_;

        // std::pair< std::wstring, datafolder > profile_;
        std::array< std::unique_ptr< adplot::SpectrumWidget >, 2 > plots_;
        std::array< std::unique_ptr< adplot::PeakMarker >, 2 > markers_;
        bool isTimeAxis_;
        bool dirty_;
        bool selProcessed_;
        std::vector< datafolder > overlays_; // last = priority
    };

}

namespace dataproc {

    struct massExtractor {
        std::vector< double > operator()( adcontrols::MassSpectrum ms ) const {
            double th = ms.maxIntensity() * 0.001; // 1% base peak
            std::vector< double > a;
            for ( size_t idx = 0; idx < ms.size(); ++idx ) {
                if ( ms.color( idx ) || ( ms.intensity( idx ) > th ) ) {
                    a.emplace_back( ms.mass( idx ) );
                }
            }
            return a;
        }
    };

    // --------------------------------
    // -------------------------------
    struct findCentroid {
        std::shared_ptr< adcontrols::MassSpectrum > operator()( portfolio::Folium&& folium ) const {
            ADDEBUG() << "findCentroid: " << folium.name() << ", " << portfolio::is_type< adcontrols::MassSpectrumPtr >( folium );
            if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {
                if ( auto fi = portfolio::find_last_of( folium.attachments()
                                                        , [](const auto& a){ return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
                    if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( fi ) ) {
                        return ptr;
                    }
                }
                if ( auto ms = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
                    if ( ms->isCentroid() && !ms->isHistogram() )
                        return ms;
                }
            }
            return nullptr;
        }
    };

    // -------------------------------
    struct globalSetIntersection {
        portfolio::Folium find( const boost::uuids::uuid& uuid ) const {
            for ( auto session: *dataproc::SessionManager::instance() ) {
                if ( auto dp = session.processor() ) {
                    if ( auto folium = dp->getPortfolio().findFolium( uuid ) )
                        return folium;
                }
            }
            return {};
        }

        void foliumChanged( const boost::uuids::uuid& uuid ) const {
            for ( auto session: *dataproc::SessionManager::instance() ) {
                if ( auto dp = session.processor() ) {
                    auto folium = dp->getPortfolio().findFolium( uuid );
                    dp->setModified( true );
                    emit SessionManager::instance()->foliumChanged( dp, folium );
                }
            }
        }

        void operator()( const boost::uuids::uuid& idfolium, std::vector< double >&& reference ) const {
            ScopedDebug(__t);
            if ( auto ptr = findCentroid()( find( idfolium ) ) ) {
                qtwrapper::waitCursor wait;
                adportable::array_wrapper<const double> a( ptr->getMassArray(), ptr->size() );
                auto it = a.begin();
                for ( auto m: reference ) {
                    it = std::lower_bound( it, a.end(), m, [](const auto& a, const auto& b){ return a < b; });
                    if ( it != a.end() ) {
                        auto beg = it != a.begin() ? it - 1 : it;
                        auto end = (it + 1) != a.end() ? it + 1 : it;
                        auto tIt = std::min_element( beg, end, [&](auto a, auto b){ return std::abs( a - m ) < std::abs( b - m );} );
                        if ( std::abs( *tIt - m ) < 0.005 ) { // 5 mDa
                            ADDEBUG() << "intersection found: " << m << ", " << *tIt << "\tdm=" << ((*tIt - m) * 1000) << "mDa";
                            ptr->setColor( std::distance( a.begin(), tIt ), 10 ); // dark gray
                        }
                    }
                }
                foliumChanged( idfolium );
            } else {
                ADDEBUG() << "specified folium: '" << idfolium << "' does not holds spectrum";
            }
        }
    };
}


using namespace dataproc;

MSSpectraWnd::~MSSpectraWnd()
{
    delete impl_;
}

MSSpectraWnd::MSSpectraWnd( QWidget *parent ) : QWidget(parent)
                                              , impl_( new impl( this ) )
{
    init();
}

void
MSSpectraWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {

        for ( size_t i = 0; i < impl_->plots_.size(); ++i ) {

            auto& plot = impl_->plots_[i];
            auto& marker = impl_->markers_[i];

            connect( plot.get()
                     , static_cast< void(adplot::SpectrumWidget::*)(const QRectF&)>(&adplot::SpectrumWidget::onSelected)
                     , [&plot,i,this]( const QRectF& rc ) { handleSelected( rc, plot.get(), i ); } );

            connect( SessionManager::instance(), &SessionManager::foliumChanged, this, &MSSpectraWnd::handleFoliumChanged );

            // plot->enableAxis( QwtPlot::yRight );
            plot->setMinimumHeight( 80 );
            marker->attach( plot.get() );
            marker->visible( true );
            marker->setYAxis( QwtPlot::yRight );

            if ( i )
                impl_->plots_[ 0 ]->link( plot.get() );

            splitter->addWidget( plot.get() );

        }

        // splitter->addWidget( impl_->table_.get() );
        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setContentsMargins(0, 0, 0, 0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
MSSpectraWnd::onInitialUpdate()
{
}

void
MSSpectraWnd::handleSpectrumYScale( bool autoY, double base, double height )
{
    // ADDEBUG() << std::make_tuple( autoY, base, height );
    for ( auto& plot: impl_->plots_ ) {
        if ( autoY ) {
            plot->setYScale( 0, 0, QwtPlot::yLeft );
        } else {
            plot->setYScale( base + height, base, QwtPlot::yLeft );
        }
        plot->replotYScale();
    }
}

void
MSSpectraWnd::handleSessionAdded( Dataprocessor * processor )
{
    // if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
    //     return;
}

void
MSSpectraWnd::handleRemoveSession( Dataprocessor * processor )
{
    for ( auto& datum: impl_->data_ ) {
        if ( processor->filename<char>() == datum.filename() ) {
            datum = {};
            impl_->plots_[ 1 ]->setTitle( QString{} );
            impl_->plots_[ 1 ]->clear();
            impl_->plots_[ 1 ]->replot();
        }
    }
    impl_->data_.erase( std::remove_if( impl_->data_.begin(), impl_->data_.end(), [](const auto& d){ return d.filename().empty(); } )
                        , impl_->data_.end() );
    if ( processor->filename<char>() == impl_->currData_.filename() ) {
        impl_->currData_ = {};
        impl_->plots_[ 0 ]->setTitle( QString{} );
        impl_->plots_[ 0 ]->clear();
        impl_->plots_[ 0 ]->replot();
    }
}

void
MSSpectraWnd::handleSelectionChanged( Dataprocessor * processor, portfolio::Folium& folium )
{
    if ( ! portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) )
        return;

    auto pfolium = folium.is_attachment() ? folium.parentFolium() : folium;  // parent folium if child selected
    auto datum = datafolder( processor, pfolium );

    auto& plot = impl_->plots_[ datum.isChecked() ? 1 : 0 ];

    plot->setNormalizedY( QwtPlot::yLeft, false );

    if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) { // selected, whether folium or attached
        impl_->selProcessed_ = folium.is_attachment(); // selected node is an attachment

        plot->clear();
        plot->setTitle( datum.display_name() );

        int idx( 0 );
        if ( ptr->isHistogram() ) {
            if ( auto ppkd = datum.get_profiled_histogram() )
                plot->setData( ppkd, idx++, QwtPlot::yLeft ); // bar spectrum always draw blue by default
        }
        plot->setData( ptr, idx, QwtPlot::yLeft );
        plot->setAxisTitle( QwtPlot::yLeft, datum.isCounting() ? QwtText("Counts") : QwtText( "Intensity (a.u.)" ) );
    }
    for ( auto& p: impl_->plots_ )
        p->enableAxis( QwtPlot::yRight, false );

    if ( datum.isChecked() ) {
        impl_->data_.clear();
        impl_->data_.emplace_back( datum );
        impl_->plots_[ 0 ]->clear();
        impl_->plots_[ 0 ]->replot();
    }
    impl_->currData_ = datum;
}

void
MSSpectraWnd::handleSelections( const std::vector< portfolio::Folium >& folio )
{
    if ( folio.empty() )
        return;
    std::vector< datafolder > data;
    for ( auto folium: folio ) {
        if ( folium.attribute( "dataType" ) == "MassSpectrum" ) {
            if ( auto dp = SessionManager::instance()->find_processor( folium.filename<char>() ) ) {
                auto self( dp->shared_from_this() );
                data.emplace_back( datafolder( dp, folium ) );
            }
        }
    }

    auto& plot = impl_->plots_[ 0 ];

    std::bitset< 2 > yAxes{ 0 };
    size_t nPKD = std::accumulate( data.begin(), data.end(), 0, [](size_t a, const auto& d){ return a + (d.isCounting() ? 1 : 0); } );
    yAxes[0] = nPKD;
    yAxes[1] = data.size() - nPKD;

    impl_->overlays_ = std::move( data );
    int idx(0);

    for ( auto it = impl_->overlays_.rbegin(); it != impl_->overlays_.rend(); ++it ) {
        if ( auto d = it->get_spectrum_for_overlay() ) {
            auto [ms,isCounting] = *d;
            if ( yAxes == 0b011 ) { // use yRight
                plot->setData( ms, idx++, it->isCounting() ? QwtPlot::yRight : QwtPlot::yLeft );
                plot->setData( ms, idx++, it->isCounting() ? QwtPlot::yRight : QwtPlot::yLeft );
            } else {
                plot->setData( ms, idx++, QwtPlot::yLeft );
            }
        }
    }
    if ( yAxes.count() == 2 ) {
        plot->enableAxis( QwtPlot::yRight, true );
        plot->setAxisTitle( QwtPlot::yLeft, QwtText("Counts") );
        plot->setAxisTitle( QwtPlot::yRight, QwtText( "Intensity (a.u.)" ) );
        // impl_->plots_[1]->enableAxis( QwtPlot::yRight, true );
    } else {
        plot->enableAxis( QwtPlot::yRight, false );
        plot->setAxisTitle( QwtPlot::yLeft, yAxes[0] ? QwtText( "Counts" ) : QwtText( "Intensity (a.u.)" ) );
        // impl_->plots_[1]->enableAxis( QwtPlot::yRight, false );
    }

    plot->replot();
}

void
MSSpectraWnd::handleFoliumChanged( Dataprocessor * processor, const portfolio::Folium& folium )
{
    if ( impl_->currData_ && impl_->currData_.uuid() == folium.uuid() ) {
        auto& data = impl_->currData_;
        auto& plot = impl_->plots_[ 0 ];
        plot->clear();
        if ( auto ms = ( impl_->selProcessed_ ? data.get_processed() : data.get_primary() ) ) {
            plot->setData( ms->first, 0, QwtPlot::yLeft );
            plot->setAxisTitle( QwtPlot::yLeft, ms->second ? QwtText("Counts") : QwtText( "Intensity (a.u.)" ) );
        }
    }
    if ( !impl_->data_.empty() && impl_->data_.back().uuid() == folium.uuid() ) {
        auto& data = impl_->data_.back();
        auto& plot = impl_->plots_[ 1 ];
        if ( auto ms = ( impl_->selProcessed_ ? data.get_processed() : data.get_primary() ) ) {
            plot->setData( ms->first, 0, QwtPlot::yLeft );
            plot->setAxisTitle( QwtPlot::yLeft, ms->second ? QwtText("Counts") : QwtText( "Intensity (a.u.)" ) );
        }
    }
}

void
MSSpectraWnd::handleSelected( const QRectF& rc, adplot::SpectrumWidget * plot, int id )
{
    QMenu menu;

    menu.addAction( tr("Copy image to clipboard"), [&] () { adplot::plot::copyToClipboard( plot ); } );

    menu.addAction( tr( "Save as SVG File" ), [&] () {
        utility::save_image_as< SVG >()( plot );
    });

    auto action
        = menu.addAction( tr("set intersection (common ions)"), [&]() {
            if ( impl_->currData_ && !impl_->data_.empty() ) {
                if ( auto const ms1 = impl_->data_.at( 0 ).get_processed() ) { // reference
                    if ( auto const ms2 = impl_->currData_.get_processed() ) {
                        globalSetIntersection()( impl_->currData_.uuid(), massExtractor()( *ms1->first ) );
                    }
                }
            }
        });
    action->setEnabled( impl_->selProcessed_ && !impl_->data_.empty() );

    menu.exec( QCursor::pos() );
}

void
MSSpectraWnd::redraw()
{
#if 0
    QString title;

    size_t idx(0);
    for ( auto& data: impl_->data_ ) {
        auto traceid = idx++;

        if ( !title.isEmpty() ) {
            title += ";";
        }

        title += data.display_name();

        QColor color = impl_->plots_[ 1 ]->index_color( traceid );
        if ( auto profile = data.get_primary() ) {
            auto [ ms, isCounts ] = *profile;
            if ( impl_->data_.size() == 1 ) {
                impl_->plots_[ 1 ]->setData( ms, traceid, QwtPlot::yLeft );
                impl_->plots_[ 1 ]->setColor( traceid, color );
                impl_->plots_[ 1 ]->setAxisTitle( QwtPlot::yLeft, isCounts ? "Counts" : "Intensity (a.u.)");
            } else {
                // overlay
                impl_->plots_[ 1 ]->setData( data.overlaySpectrum_, traceid, QwtPlot::yLeft );
                impl_->plots_[ 1 ]->setColor( traceid, color );
                impl_->plots_[ 1 ]->setAxisTitle( QwtPlot::yLeft, "Intensity (R.A.)" );
            }
        }
    }
    impl_->plots_[ 1 ]->setTitle( title );
    impl_->dirty_ = false;
#endif
}

void
MSSpectraWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSSpectraWnd::handleAxisChanged( adcontrols::hor_axis axis )
{
    impl_->isTimeAxis_ = ( axis == adcontrols::hor_axis_time );
    for ( auto& plot: impl_->plots_ )
        plot->setAxis( adplot::SpectrumWidget::HorizontalAxis( axis ), true );
}

void
MSSpectraWnd::handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool )
{
    // check state on navigator
    // will handle at handleSelectionChanged
}

void
MSSpectraWnd::handleCurrentChanged( const QString& guid, int idx, int fcn )
{
    // deprecated -- used to work with quan table
}

void
MSSpectraWnd::onPageSelected()
{
    if ( impl_->dirty_ ) {
        redraw();
    }
}

void
MSSpectraWnd::handleProcessed( Dataprocessor * processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MSSpectraWnd::onDataChanged( const QString& foliumGuid, const QString& attGuid, int idx, int fcn )
{
    // data changed on MSPeakTable via MSProcessingWnd
}

void
MSSpectraWnd::handleDataChanged( const QString& dataGuid, int idx, int fcn, int column, const QVariant& data )
{
}

///////////////////////////

void
MSSpectraWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
    QSizeF sizeMM( 180, 80 );

    int resolution = 85;
	const double mmToInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * mmToInch * resolution;

	QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPageSize( QPageSize( QPageSize::A4 ) );
    printer.setFullPage( false );

    printer.setDocName( "QtPlatz Process Report" );
    printer.setOutputFileName( pdfname );
    printer.setResolution( resolution );

    QPainter painter( &printer );

	QRectF boundingRect;
	QRectF drawRect( 0.0, 0.0, printer.width(), (12.0/72)*printer.resolution() );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

	drawRect.setTop( boundingRect.bottom() );
	drawRect.setHeight( size.height() );
	drawRect.setWidth( size.width() );

    for ( auto& plot : impl_->plots_ ) {

        renderer.render( plot.get(), &painter, drawRect );

        drawRect.setTop( drawRect.bottom() );
        drawRect.setHeight( size.height() );
    }

}
