/**************************************************************************
** Copyright (C) 2010-     Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "mass_assignor.hpp"
#include "moleculeswidget.hpp"
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrscontrols/u5303a/tdcdoc.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/scanlaw.hpp>
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

    }
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
{
    hpw_->setViewId( 10 );

    for ( auto& tp: tp_ )
        tp = std::make_shared< adcontrols::Trace >();

    init();

    connect( document::instance(), &document::dataChanged, this, &WaveformWnd::handleDataChanged );
    connect( document::instance(), &document::traceChanged, this, &WaveformWnd::handleTraceChanged );
    connect( document::instance(), &document::drawSettingChanged, [&]{ handleDrawSettings(); } );
}

WaveformWnd::~WaveformWnd()
{
    fini();
    tof_markers_.clear();
    delete tpw_;
    delete hpw_;
    delete spw_;
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
    //spw_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 72 );
    //hpw_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 72 );

    spw_->setAxisTitle( QwtPlot::yLeft, tr( "<i>mV</i>" ) );
    //spw_->setAxisTitle( QwtPlot::yRight, tr( "<i>mV</i>" ) );
    //spw_->enableAxis( QwtPlot::yRight, true );

    spw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    spw_->setKeepZoomed( false );

    hpw_->setAxisTitle( QwtPlot::yLeft, tr( "Counts" ) );
    //hpw_->setAxisTitle( QwtPlot::yRight, tr( "<i>Counts</i>" ) );
    //hpw_->enableAxis( QwtPlot::yRight, true );

    hpw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    hpw_->setKeepZoomed( false );
    hpw_->setAutoAnnotation( false );

    spw_->link( hpw_ );

    tpw_->setAxisTitle( QwtPlot::yLeft, tr( "Counts" ) );
    //tpw_->setAxisTitle( QwtPlot::yRight, tr( "<i>Counts</i>" ) );
    //tpw_->enableAxis( QwtPlot::yRight, true );

    if ( auto legend = new QwtLegend() ) {
        tpw_->insertLegend( legend, QwtPlot::LegendPosition( QwtPlot::RightLegend ) );
        QFont font = legend->font();
        font.setPointSize( 9 );
        legend->setFont( font );
    }

    QBoxLayout * layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
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
}

void
WaveformWnd::handle_threshold_action()
{
    if ( auto am = document::instance()->tdc()->threshold_action() ) {

        double lower(0), upper(0);
        if ( spw_->axis() == adplot::SpectrumWidget::HorizontalAxisTime ) {
            lower = ( am->delay - am->width / 2 ) * std::micro::den; // us
            upper = ( am->delay + am->width / 2 ) * std::micro::den; // us
        } else {
            lower = mass_assignor()( am->delay - am->width / 2, 0 );
            upper = mass_assignor()( am->delay + am->width / 2, 0 );
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
WaveformWnd::handleTraceChanged( const boost::uuids::uuid& /* uuid = pkkd_trace_obsserver */ )
{
    std::vector< std::shared_ptr< adcontrols::Trace > > traces;

    document::instance()->getTraces( traces );

    QString footer;
    QString runname;

    size_t idx( 0 );
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
            tpw_->setTrace( trace, idx, QwtPlot::yLeft );
            if ( auto plotItem = tpw_->getPlotItem( idx ) ) {
                plotItem->setTitle( QwtText( QString::fromStdString( trace->legend() ) ) );
            }
        }
        // tpw_->setData( trace, idx, false );
        ++idx;
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
            tpw_->setTrace( trace, idx, QwtPlot::yLeft );
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
                    const auto yAxis = pkdSpectrumEnabled_ ? QwtPlot::yRight : QwtPlot::yLeft;
                    for ( auto& closeup: closeups_ ) {
                        if ( closeup.enable ) {
                            closeup.sp->setData( sp, 1, yAxis );
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
                                closeup.sp->setData( sp, 0, QwtPlot::yLeft );
                            }
                        }
                    }
                }
            } else if ( uuid == acqrscontrols::u5303a::pkd_coadd_spectrum ) { // soft pkd
                numberOfTriggersSinceInject_ = sp->getMSProperty().numAverage();
                if ( longTermHistogramEnabled_ ) {
                    // title may set at waveform (PKD) draw
                    hpw_->setData( sp, 1, QwtPlot::yRight );
                }

                for ( auto& closeup: closeups_ ) {
                    if ( closeup.enable )
                        closeup.sp->setData( sp, 1, QwtPlot::yLeft ); // left axis
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
                        closeup.sp->setData( sp, 1, QwtPlot::yLeft ); // left axis
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

void
WaveformWnd::setSpanMarker( unsigned int row, unsigned int index /* 0 = time, 1 = window */, double value )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    // ADDEBUG() << "--------------- setSpanMarker ------------- <-- from Chrmatograms";

    if ( row < closeups_.size() ) {
        auto& closeup = closeups_.at( row );

        auto range = closeup.marker->xValue();

        double width = range.second - range.first;  // us
        double cx = range.first + ( width / 2 );  // us
        if ( index == 0 ) {
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
WaveformWnd::handleDrawSettings()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    pkdSpectrumEnabled_ = document::instance()->pkdSpectrumEnabled();
    longTermHistogramEnabled_ = document::instance()->longTermHistogramEnabled();

    //hpw_->enableAxis( QwtPlot::yLeft, pkdSpectrumEnabled_ );
    //hpw_->enableAxis( QwtPlot::yRight, longTermHistogramEnabled_ );

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

    // ADDEBUG() << "setAxis(" << idView << ", " << axis << ")";

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
    if ( auto mols = MoleculesWidget::json_to_moltable( json.toStdString() ) ) {
        for ( auto& mol: mols->data() ) {
            ADDEBUG() << "mol.formula: " << mol.formula() << ", " << mol.mass();
        }

        while ( tof_markers_.size() < mols->data().size() ) {
            tof_markers_.emplace_back( std::make_unique< QwtPlotMarker >() );
            auto& marker = tof_markers_.back();
            marker->attach( spw_ );
            marker->setLineStyle( QwtPlotMarker::VLine );
            marker->setLinePen( QColor( 0xdd, 0xa0, 0xdd ), 2.0, Qt::DotLine ); // plum
            marker->setLabelOrientation( Qt::Vertical );
            marker->setLabelAlignment( Qt::AlignRight );
        }

        size_t i(0);
        for ( const auto& mol: mols->data() ) {
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
        if ( tof_markers_.size() > mols->data().size() ) {
            std::for_each( tof_markers_.begin() + mols->data().size(), tof_markers_.end()
                           , []( const auto& marker ){ marker->setVisible( false ); });
        }
        spw_->replot();
    }
}
