/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "waveformwnd.hpp"
#include "constants.hpp"
#include "document.hpp"
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrscontrols/u5303a/tdcdoc.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#if XCHROMATOGRAMSMETHOD
# include <adcontrols/controlmethod/xchromatogramsmethod.hpp>
#endif
#if TOFCHROMATOGRAMSMETHOD
# include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
# include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#endif
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/trace.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spanmarker.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/zoomer.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/time_format.hpp>
#include <coreplugin/minisplitter.h>
#include <u5303a/digitizer.hpp>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_widget.h>
#include <QBoxLayout>
#include <QPen>
#include <QSplitter>
#include <QLocale>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/json.hpp>
#include <chrono>
#include <ratio>
#include <sstream>
#include <thread>

namespace accutof { namespace acquire {

        constexpr size_t widthFactor = 5;

        class LegendItem : public QwtPlotLegendItem {
        public:
            LegendItem() {
                setRenderHint( QwtPlotItem::RenderAntialiased );
                QColor color( Qt::white );
                setTextPen( color );
                setBorderPen( color );
                QColor c( Qt::gray );
                c.setAlpha( 200 );
                setBackgroundBrush( c );
            }
        };

        /////////////////////////////////////////

        class WaveformWnd::impl {
            std::mutex mutex_;
        public:
            std::atomic< bool > blockTraces_;
            std::pair< bool, bool > yAxes_;

            impl() : blockTraces_( false )
                   , yAxes_{ true, false } {
            }
        };

        /////////////////////////////////////////

    } // acquire
} // accutof

namespace {

    class delayed_execution : public std::enable_shared_from_this< delayed_execution > {
        std::chrono::milliseconds delay_;
        std::function< void() > callback_;
        std::thread thread_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::atomic< bool > stop_waiting_;
    public:
        delayed_execution( std::chrono::milliseconds t
                           , std::function< void() >&& f ) : delay_(t)
                                                           , callback_( std::move( f ) ) {}

        ~delayed_execution() {
            thread_.detach();
        }

        void cancel() {
            stop_waiting_.store( true );
            cv_.notify_one();
        }

        void run() {
            auto self( shared_from_this() );
            stop_waiting_.store( false );
            thread_ = std::thread{ [this,self](){
                std::unique_lock< std::mutex> lock( mutex_ );
                cv_.wait_for( lock, delay_, [this](){ return stop_waiting_.load(); } );
                if ( ! stop_waiting_ )
                    callback_();
            }};
        }
    };

}

using namespace accutof::acquire;

WaveformWnd::WaveformWnd( QWidget * parent ) : QWidget( parent )
                                             , tpw_( new adplot::ChromatogramWidget )
                                             , spw_( new adplot::SpectrumWidget )
                                             , hpw_( new adplot::SpectrumWidget )
                                             , tickCount_( 0 )
                                             , longTermHistogramEnabled_( true )
                                             , pkdSpectrumEnabled_( true )
                                             , elapsedTime_( 0 )
                                             , numberOfTriggersSinceInject_( 0 )
                                             , impl_( new impl() )
{
    hpw_->setViewId( 10 );

    for ( auto& tp: tp_ )
        tp = std::make_shared< adcontrols::Trace >();

    init();

    connect( document::instance(), &document::dataChanged, this, &WaveformWnd::handleDataChanged );
    connect( document::instance(), &document::traceChanged, this, &WaveformWnd::handleTraceChanged );
    connect( document::instance(), &document::drawSettingChanged, this, &WaveformWnd::handleDrawSettingChanged );
    connect( document::instance(), &document::traceSettingChanged, this, &WaveformWnd::handleTraceSettingChanged );

    auto zoomer = tpw_->zoomer();
    connect( zoomer, &adplot::Zoomer::zoomed, this, [&](const QRectF& rc){
        impl_->blockTraces_ = true;
        using namespace std::chrono_literals;
        std::make_shared< delayed_execution >( 3000ms
                                               , [&](){ impl_->blockTraces_ = false; } )->run();
    });
}

WaveformWnd::~WaveformWnd()
{
    fini();
    tof_markers_.clear();
    delete tpw_;
    delete hpw_;
    delete spw_;
    delete impl_;
}

void
WaveformWnd::init()
{
    const QColor colors[] = { QColor( 0x00, 0x00, 0xff, 0x80 ), QColor( 0xff, 0x00, 0x00, 0x80 ) };

    Core::MiniSplitter * top_splitter = new Core::MiniSplitter; // L|R
    do {
        top_splitter->setOrientation( Qt::Horizontal );
    } while ( 0 );

    Core::MiniSplitter * splitter_r = new Core::MiniSplitter;
    do {
        splitter_r->setOrientation( Qt::Vertical );
        for ( auto& closeup: closeups_ ) {
            closeup.enable = false;
            closeup.sp = std::make_unique< adplot::SpectrumWidget >();
            closeup.sp->setMinimumHeight( 20 );
            closeup.sp->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
            closeup.sp->setAutoAnnotation( false );
            closeup.sp->setKeepZoomed( true );
            closeup.sp->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 50 );
            closeup.sp->setAxisTitle( QwtPlot::yLeft, tr( "a.u." ) );
            closeup.marker = std::make_unique< adplot::SpanMarker >( QColor( 0x4b, 0x00, 0x62, 0x60 ), QwtPlotMarker::VLine, 1.5 );
            closeup.marker->attach( closeup.sp.get() );
            splitter_r->addWidget( closeup.sp.get() );
        }
    } while ( 0 );

    auto splitter_l = new Core::MiniSplitter;
    do {
        splitter_l->addWidget( tpw_ );
        splitter_l->addWidget( hpw_ );
        splitter_l->addWidget( spw_ );
        splitter_l->setStretchFactor( 0, 1 );
        splitter_l->setStretchFactor( 1, 3 );
        splitter_l->setOrientation( Qt::Vertical );
    } while(0);

    top_splitter->addWidget( splitter_l );
    top_splitter->addWidget( splitter_r );

    tpw_->setMinimumHeight( 80 );
    spw_->setMinimumHeight( 80 );
    hpw_->setMinimumHeight( 80 );
    spw_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 72 );
    hpw_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 72 );

    spw_->setAxisTitle( QwtPlot::yLeft, tr( "mV" ) );

    spw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    spw_->setKeepZoomed( false );

    hpw_->setAxisTitle( QwtPlot::yLeft, tr( "Counts" ) );

    hpw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    hpw_->setKeepZoomed( false );
    hpw_->setAutoAnnotation( false );

    spw_->link( hpw_ );

    auto font = tpw_->axisFont( QwtPlot::yLeft );
    font.setPointSize( 8 );
    tpw_->setAxisFont( QwtPlot::yLeft, font );
    tpw_->setAxisFont( QwtPlot::yRight, font );
    QwtText axisLabel;
    axisLabel.setFont( font );
    for ( auto label: { std::make_pair( QwtPlot::yLeft, "a.u." ), std::make_pair( QwtPlot::yRight, "Counts" ) } ) {
        axisLabel.setText( std::get<1>(label ) );
        tpw_->setAxisTitle( std::get<0>( label ), axisLabel );
    }

    if ( auto legend = new QwtLegend() ) {
        tpw_->insertLegend( legend, QwtPlot::LegendPosition( QwtPlot::RightLegend ) );
        QFont font = legend->font();
        font.setPointSize( 8 );
        legend->setFont( font );
    }

    QBoxLayout * layout = new QVBoxLayout( this );
    layout->setContentsMargins( {} );
    layout->setSpacing( 2 );
    layout->addWidget( top_splitter );
    top_splitter->setStretchFactor( 0, 2 );
    top_splitter->setStretchFactor( 1, 1 );

    for ( size_t i = 0; i < threshold_markers_.size(); ++i ) {
        threshold_markers_[ i ] = new QwtPlotMarker();
        threshold_markers_[ i ]->setLineStyle( QwtPlotMarker::HLine );
        threshold_markers_[ i ]->setLinePen( colors[ i ], 2, Qt::DotLine );
        threshold_markers_[ i ]->setYAxis( ( i == 1 ) ? QwtPlot::yRight : QwtPlot::yLeft );
        threshold_markers_[ i ]->setYValue( ( i + 1 ) * 100 );
        threshold_markers_[ i ]->attach( spw_ );
    }

    // Threshold action (fence) marker (indigo)
    if ( ( threshold_action_marker_
           = std::make_unique< adplot::SpanMarker >( QColor( 0x4b, 0x00, 0x62, 0x60 ), QwtPlotMarker::VLine, 1.5 ) ) ) {
        //threshold_action_marker_->visible( false );
        threshold_action_marker_->attach( spw_ );
    }
}

void
WaveformWnd::fini()
{
    threshold_action_marker_.reset();
}

void
WaveformWnd::onInitialUpdate()
{
    spw_->setKeepZoomed( false );
    hpw_->setKeepZoomed( false );

    handle_method( QString() );
    handle_threshold_method( 0 );

    handleDrawSettingChanged();
}

void
WaveformWnd::handle_threshold_action()
{
    if ( auto am = document::instance()->tdc()->threshold_action() ) {

        auto massSpectrometer( document::instance()->massSpectrometer() ); // lock pointer
        auto mass_assign = [=](double time, int){ return massSpectrometer->assignMass( time, 0 ); };

        double lower(0), upper(0);
        if ( spw_->axis() == adplot::SpectrumWidget::HorizontalAxisTime ) {
            lower = ( am->delay - am->width / 2 ) * std::micro::den; // us
            upper = ( am->delay + am->width / 2 ) * std::micro::den; // us
        } else {
            lower = mass_assign( am->delay - am->width / 2, 0 );
            upper = mass_assign( am->delay + am->width / 2, 0 );
        }

        bool replot( false );
        auto x = threshold_action_marker_->xValue();
        if ( ! (adportable::compare<double>::approximatelyEqual( x.first, lower ) &&
                adportable::compare<double>::approximatelyEqual( x.second, upper ) ) ) {
            threshold_action_marker_->setXValue( lower, upper );
            replot = true;
        }

        if ( am->enable != threshold_action_marker_->isVisible() ) {
            threshold_action_marker_->setVisible( am->enable );
            replot = true;
        }

        if ( replot )
            spw_->replot();
    }
}

void
WaveformWnd::handle_threshold_method( int ch )
{
    if ( auto th = document::instance()->tdc()->threshold_method( ch ) ) {

        bool replot( false );
        double level_mV = th->threshold_level * 1.0e3;

        if ( !adportable::compare<double>::approximatelyEqual( threshold_markers_[ ch ]->yValue(), level_mV ) ) {
            threshold_markers_[ ch ]->setYValue( level_mV );
            replot = true;
        }
        if ( th->enable != threshold_markers_[ ch ]->isVisible() ) {
            threshold_markers_[ ch ]->setVisible( th->enable );
            replot = true;
        }
        if ( replot )
            spw_->replot();
    }
}

void
WaveformWnd::handle_method( const QString& )
{
    if ( auto ptr = document::instance()->method() ) {
        if ( ( ptr->channels() & 0x01 ) == 0 && sp_[ 0 ] ) {
            sp_[ 0 ]->resize( 0 );
            spw_->setData( sp_[ 0 ], 0, QwtPlot::yLeft );
        }
        if ( ( ptr->channels() & 0x02 ) == 0 && sp_[ 1 ] ) {
            sp_[ 1 ]->resize( 0 );
            spw_->setData( sp_[ 1 ], 1, QwtPlot::yRight );
        }
        spw_->setAxisAutoScale( QwtPlot::yLeft, true );// ptr->threshold_.autoScale );
        //spw_->setAxisAutoScale( QwtPlot::yRight, ptr->ch2_.autoScale );
    }
}

void
WaveformWnd::handleTraceSettingChanged( int idx, bool enable )
{
    if ( !enable ) {
        tpw_->removeData( idx, false );
    }
}

void
WaveformWnd::handleTraceChanged( const boost::uuids::uuid& /* uuid = pkkd_trace_obsserver */ )
{
    if ( impl_->blockTraces_ )
        return;
    std::vector< std::shared_ptr< adcontrols::Trace > > traces;

    document::instance()->getTraces( traces );

    QString footer;
    QString runname;

    size_t idx( 0 );
    bool yRight( false );
    for ( const auto& trace: traces ) {
        if ( idx == 0 ) {
            double t_inject = trace->x( trace->size() - 1 ) - trace->injectTime(); // time since injection trigger
            auto up = adportable::time_format::elapsed_time( int( trace->x( trace->size() - 1 ) ) );
            footer += QString( "Elapsed time: %1s (up %2)" ).arg( QString::number( t_inject, 'f', 3 ), up.c_str() );

            if ( auto run = document::instance()->activeSampleRun() ) {
                footer += QString( "   <AccuTOF> %1 [method time: %2]" )
                    .arg( QString::fromStdWString( run->filename() ), QString::number( run->methodTime(), 'f', 1 ) );
            }

            tpw_->setTitle( footer );
        }

        if ( trace->enable() ) {
            yRight |= trace->isCountingTrace();
            tpw_->setTrace( trace, idx, trace->isCountingTrace() ? QwtPlot::yRight : QwtPlot::yLeft );
            if ( auto plotItem = tpw_->getPlotItem( idx ) ) {
                plotItem->setTitle( QwtText( QString::fromStdString( trace->legend() ) ) );
            }
        }
        ++idx;
    }
    if ( impl_->yAxes_.second != yRight ) {
        impl_->yAxes_.second = yRight;
        tpw_->enableAxis( QwtPlot::yRight, yRight );
    }
}

void
WaveformWnd::thresholdTraceChanged()
{
    std::vector< std::shared_ptr< adcontrols::Trace > > traces;

    document::instance()->getTraces( traces );

    QString footer;

    size_t idx(0);
    for ( const auto& trace: traces ) {
        if ( idx == 0 ) { // TIC
            double t_inject = trace->x( trace->size() - 1 ) - trace->injectTime(); // time since injection trigger
            auto up = adportable::time_format::elapsed_time( int( trace->x( trace->size() - 1 ) ) );

            footer += QString( "Elapsed time: %1s (up %2)" ).arg( QString::number( t_inject, 'f', 3 ), up.c_str() );

            if ( auto run = document::instance()->activeSampleRun() ) {
                footer += QString( "   Count(V<sub>th</sub>) %1 [method time: %2]" )
                    .arg( QString::fromStdWString( run->filename() ), QString::number( run->methodTime(), 'f', 1 ) );
            }

            tpw_->setTitle( footer );
        }

        if ( trace->enable() ) {
            tpw_->setTrace( trace, idx, trace->isCountingTrace() ? QwtPlot::yRight : QwtPlot::yLeft );
            if ( auto plotItem = tpw_->getPlotItem( idx ) ) {
                plotItem->setTitle( QwtText( QString::fromStdString( trace->legend() ) ) );
            }
        }
        ++idx;
    }
}

void
WaveformWnd::handleDataChanged( const boost::uuids::uuid& uuid, int idx )
{
    // ADDEBUG() << uuid
    //           << "\t"  << bool( uuid == acqrscontrols::u5303a::waveform_observer )
    //           << "\t" << idx;
    std::lock_guard< std::mutex > lock( mutex_ );
    QLocale loc;

    if ( uuid == trace_observer ) {

        thresholdTraceChanged();

    } else {
        // idx == 0 --> avg.waveform
        // idx == 1 --> pkd.waveform
        if ( auto sp = document::instance()->recentSpectrum( uuid, idx ) ) {

            if ( uuid == acqrscontrols::u5303a::waveform_observer ) {
                if ( idx == 0 ) { // avg.waveform
                    // waveform (analog)
                    double seconds = sp->getMSProperty().timeSinceInjection();
                    QString title = QString( "U5303A: Elapsed time: %1s, Trig# %2" ).arg( loc.toString( seconds, 'f', 4 )
                                                                                          , loc.toString( sp->getMSProperty().trigNumber() ) );

                    // todo.....
                    if ( document::instance()->tdc()->threshold_action()->enable ) {
                        auto counts = document::instance()->tdc()->threshold_action_counts( 0 );

                        double rate = counts.second ? double(counts.first * 100.0 / counts.second) : 0.0;
                        title += QString( "    Count rate: <font color=blue>%1 / %2 (%3%)" )
                            .arg( loc.toString( counts.first ), loc.toString( counts.second ), loc.toString( rate, 'f', 3 ) );
                    }
                    //.........
                    spw_->setTitle( title );
                    spw_->setData( sp, idx, QwtPlot::yLeft );
                    spw_->setKeepZoomed( true );

                    uint32_t id(0);
                    // const auto yAxis = pkdSpectrumEnabled_ ? QwtPlot::yRight : QwtPlot::yLeft;
                    const uint32_t tid = pkdSpectrumEnabled_ ? 1 : 0;
                    for ( auto& closeup: closeups_ ) {
                        if ( closeup.enable ) {
                            closeup.sp->setData( sp, tid, QwtPlot::yLeft );
                            double rate = document::instance()->countRate( id );
                            QString title = QString( "%1 Count rate: %2%" ).arg( closeup.formula, QString::number( rate * 100, 'f', 3 ) );
                            closeup.sp->setTitle( title );
                        }
                        ++id;
                    }

                } else {
                    // pkd.waveform (PKD)
                    elapsedTime_ = sp->getMSProperty().timeSinceInjection();
                    QString title = QString( "U5303A: Elapsed time: %1s, Total <font color=blue>%2 <font color=black>triggers acquired." )
                        .arg( loc.toString( elapsedTime_, 'f', 3 )
                              , loc.toString( numberOfTriggersSinceInject_ ) );
                    hpw_->setTitle( title );
                    hpw_->setData( sp, 0, QwtPlot::yLeft );
                    hpw_->setKeepZoomed( true );
                    if ( pkdSpectrumEnabled_ ) {
                        for ( auto& closeup: closeups_ ) {
                            if ( closeup.enable ) {
                                closeup.sp->setData( sp, 0, QwtPlot::yRight );
                            }
                        }
                    }
                }
            } else if ( uuid == acqrscontrols::u5303a::pkd_coadd_spectrum ) { // soft pkd
                numberOfTriggersSinceInject_ = sp->getMSProperty().numAverage();
                if ( longTermHistogramEnabled_ ) {
                    // title may set at waveform (PKD) draw
                    hpw_->setData( sp, 1, QwtPlot::yRight );

                    for ( auto& closeup: closeups_ ) {
                        if ( closeup.enable )
                            closeup.sp->setData( sp, 2, QwtPlot::yRight ); // left axis
                    }
                }

            } else if ( uuid == acqrscontrols::u5303a::histogram_observer ) { // Vth counting
                // histogram
                double rate = document::instance()->triggers_per_second();
                QString title = QString( "U5303A: %1 samples since trig# %2, at rate %3/s" )
                    .arg( loc.toString( sp->getMSProperty().numAverage() )
                          , loc.toString( sp->getMSProperty().trigNumber() )
                          , QString::number( rate, 'f', 2 ) );

                hpw_->setTitle( title );
                hpw_->setData( sp, idx, bool( idx ) ? QwtPlot::yRight : QwtPlot::yLeft );
                hpw_->setKeepZoomed( true );

                for ( auto& closeup: closeups_ ) {
                    if ( closeup.enable )
                        closeup.sp->setData( sp, 3, QwtPlot::yRight ); // left axis
                }

            } else {
                ADDEBUG() << "Unhandled observer";
            }

        } else {
            if ( uuid == acqrscontrols::u5303a::waveform_observer ) {
                spw_->setData( nullptr, idx, bool( idx ) ? QwtPlot::yRight : QwtPlot::yLeft );

            } else if ( uuid == acqrscontrols::u5303a::histogram_observer ) {
                hpw_->setData( nullptr, idx, bool( idx ) ? QwtPlot::yRight : QwtPlot::yLeft );
            }
        }
    }
}

#if TOFCHROMATOGRAMSMETHOD
void
WaveformWnd::setMethod( const adcontrols::TofChromatogramsMethod& m )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    hpw_->setData( nullptr, 1, QwtPlot::yRight ); // clear co-added pkd

    for ( size_t i = 0; i < m.size() && i < closeups_.size(); ++i ) {

        const auto& tofm = m.begin() + i;
        auto& closeup = closeups_.at( i );

        closeup.enable = tofm->enable();
        closeup.formula = QString::fromStdString( adcontrols::ChemicalFormula::formatFormula( tofm->formula() ) );

        double width( 1 ), cx(0);

        if ( closeup.sp->axis() == adplot::SpectrumWidget::HorizontalAxisTime ) {
            cx = tofm->time() * std::micro::den;
            width = ( tofm->timeWindow() * std::micro::den );
        } else {
            cx = tofm->mass();
            width = tofm->massWindow();
        }

        closeup.marker->setXValue( cx - ( width / 2 ), cx + width / 2 );

        auto zoom = closeup.sp->zoomer()->zoomBase();
        zoom.setLeft( cx - ( width * widthFactor ) / 2 );
        zoom.setWidth( width * widthFactor );
        if ( closeup.enable ) {
            closeup.sp->show();
            closeup.sp->setZoomStack( zoom );
            closeup.sp->replot();
        } else {
            closeup.sp->hide();
        }
    }
}
#endif

#if XCHROMATOGRAMSMETHOD
void
WaveformWnd::setMethod( const adcontrols::XChromatogramsMethod& m )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    hpw_->setData( nullptr, 1, QwtPlot::yRight ); // clear co-added pkd

    for ( size_t i = 0; i < m.size() && i < closeups_.size(); ++i ) {

        const auto& xm = m.xics().at( i );
        auto& closeup = closeups_.at( i );

        closeup.enable = xm.enable();
        closeup.formula = QString::fromStdString( adcontrols::ChemicalFormula::formatFormula( xm.formula() ) );

        double width( 1 ), cx(0);

        if ( closeup.sp->axis() == adplot::SpectrumWidget::HorizontalAxisTime ) {
            cx = xm.time() * std::micro::den;
            width = xm.time_window() * std::micro::den;
        } else {
            cx = xm.mass();
            width = xm.mass_window();
        }

        closeup.marker->setXValue( cx - ( width / 2 ), cx + width / 2 );

        auto zoom = closeup.sp->zoomer()->zoomBase();
        zoom.setLeft( cx - ( width * widthFactor ) / 2 );
        zoom.setWidth( width * widthFactor );
        if ( closeup.enable ) {
            closeup.sp->show();
            closeup.sp->setZoomStack( zoom );
            closeup.sp->replot();
        } else {
            closeup.sp->hide();
        }
    }
}
#endif

void
WaveformWnd::setSpanMarker( unsigned int row, unsigned int index /* 0 = mass, 1 = window */, double value )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( row < closeups_.size() ) {
        auto& closeup = closeups_.at( row );

        auto range = closeup.marker->xValue();

        double width = range.second - range.first;
        double cx = range.first + ( width / 2 );
        if ( index == 0 ) { // mass
            cx = value * std::micro::den;         // -> us
        } else if ( index == 1 ) {
            width = value * std::micro::den;        // -> us
        }
        double vwidth = width * widthFactor;

        closeup.marker->setXValue( cx - ( width / 2 ), cx + width / 2 );

        auto zoom = closeup.sp->zoomer()->zoomBase();
        zoom.setLeft( cx - vwidth / 2 );
        zoom.setWidth( vwidth );

        closeup.sp->setZoomStack( zoom );
        closeup.sp->replot();
    }
}

void
WaveformWnd::handleDrawSettingChanged()
{
    std::lock_guard< std::mutex > lock( mutex_ );

    pkdSpectrumEnabled_ = document::instance()->pkdSpectrumEnabled();
    longTermHistogramEnabled_ = document::instance()->longTermHistogramEnabled();

    for ( auto& closeup: closeups_ ) {
        closeup.sp->enableAxis( QwtPlot::yRight, pkdSpectrumEnabled_ );
        closeup.sp->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 20 );
        for ( uint32_t i = 0; i < 4; ++i )
            closeup.sp->setData( nullptr, i, QwtPlot::yRight );
    }

    if ( ! pkdSpectrumEnabled_ ) {
        hpw_->setData( nullptr, 0, QwtPlot::yLeft );
        // hpw_->axisWidget( QwtPlot::yLeft )->hide(); //scaleDraw()->setMinimumExtent( 50 );
    }
    if ( ! longTermHistogramEnabled_ ) {
        hpw_->setData( nullptr, 1, QwtPlot::yRight );
    }
}

void
WaveformWnd::setAxis( int idView, int axis ) // 0: mass, 1: time
{
    std::lock_guard< std::mutex > lock( mutex_ );

    auto haxis = ( axis == 0 ? adplot::SpectrumWidget::HorizontalAxisMass : adplot::SpectrumWidget::HorizontalAxisTime );

    std::vector< adplot::SpectrumWidget * > views;
    if ( idView == 0 ) {
        views = { spw_, hpw_ };
    } else if ( idView == 1 ) {
        for ( auto& closeup: closeups_ )
            views.emplace_back( closeup.sp.get() );
    }

    for ( auto& spw: views ) {
        spw->setAxis( haxis, true
                      , []( const QRectF& z, const adcontrols::MassSpectrum& vms, adplot::SpectrumWidget::HorizontalAxis axis ){
                          auto sp = document::instance()->massSpectrometer();
                          auto scanLaw = sp->scanLaw();
                          if ( axis == adplot::SpectrumWidget::HorizontalAxisMass ) { // mass --> time
                              for ( auto& ms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( vms ) ) {
                                  if ( ms.getAcquisitionMassRange().first < z.left() && z.right() < ms.getAcquisitionMassRange().second ) {
                                      auto range = std::make_pair( scanLaw->getTime( z.left(), ms.mode() ), scanLaw->getTime( z.right(), ms.mode() ) );
                                      return QRectF( range.first * std::micro::den, z.bottom(), ( range.second - range.first ) * std::micro::den, z.height() );
                                  }
                              }
                          } else { // time --> mass
                              auto sp = document::instance()->massSpectrometer();
                              auto scanLaw = sp->scanLaw();
                              for ( auto& ms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( vms ) ) {
                                  auto left = z.left() / std::micro::den;
                                  auto right = z.right() / std::micro::den;
                                  if ( ms.getMSProperty().instTimeRange().first < left && right < ms.getMSProperty().instTimeRange().second ) {
                                      auto range = std::make_pair( scanLaw->getMass( left, ms.mode() ), scanLaw->getMass( right, ms.mode() ) );
                                      return QRectF( range.first, z.bottom(), range.second - range.first, z.height() );
                                  }
                              }
                          }
                          return QRectF();
                      });
    }

    handle_threshold_action();
}

void
WaveformWnd::handleScaleY( int which, bool autoScale, double top, double bottom )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( which == 0 ) {
        if ( autoScale )
            spw_->setYScale( 0, 0, QwtPlot::yLeft );
        else
            spw_->setYScale( top, bottom, QwtPlot::yLeft );
    } else {
        for ( auto& closeup: closeups_ ) {
            if ( closeup.enable ) {
                // ADDEBUG() << "handleScaleY which=" << which << ", autoScale=" << autoScale << ", top:bottom=" << top << ", " << bottom;
                if ( autoScale )
                    closeup.sp->setYScale(0, 0, QwtPlot::yLeft );
                else
                    closeup.sp->setYScale(top, bottom, QwtPlot::yLeft );
            }
        }
    }
}

void
WaveformWnd::handleMolecules( const QString & json )
{
    boost::system::error_code ec;
    auto jv = boost::json::parse( json.toStdString(), ec );
    if ( !ec ) {
        auto mols = boost::json::value_to< adcontrols::moltable >( jv );

        while ( tof_markers_.size() < mols.data().size() ) {
            tof_markers_.emplace_back( std::make_unique< QwtPlotMarker >() );
            auto& marker = tof_markers_.back();
            marker->attach( spw_ );
            marker->setLineStyle( QwtPlotMarker::VLine );
            marker->setLinePen( QColor( 0xdd, 0xa0, 0xdd ), 2.0, Qt::DotLine ); // plum
            marker->setLabelOrientation( Qt::Vertical );
            marker->setLabelAlignment( Qt::AlignRight );
        }

        size_t i(0);
        for ( const auto& mol: mols.data() ) {
            if ( spw_->axis() == adplot::SpectrumWidget::HorizontalAxisMass ) {
                tof_markers_[i]->setXValue( mol.mass() );
            } else {
                if ( auto sp = document::instance()->massSpectrometer() ) {
                    double time = sp->timeFromMass( mol.mass() );
                    tof_markers_[i]->setXValue( time * std::micro::den );
                }
            }
            QString label = QString::fromStdString( mol.synonym() );
            if ( label.isEmpty() ) {
                label = QString::fromStdString( adcontrols::ChemicalFormula::formatFormula( mol.formula() ) );
            }

            tof_markers_[i]->setLabel( QwtText( label, QwtText::RichText ) );
            tof_markers_[i]->setVisible( mol.enable() );
            ++i;
        }
        if ( tof_markers_.size() > mols.data().size() ) {
            std::for_each( tof_markers_.begin() + mols.data().size(), tof_markers_.end()
                           , []( const auto& marker ){ marker->setVisible( false ); });
        }
        spw_->replot();
    }
}
