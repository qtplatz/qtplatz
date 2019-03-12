/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
//#include <ads54j/waveformobserver.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/spanmarker.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/debug.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_widget.h>
#include <QSplitter>
#include <QBoxLayout>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <sstream>

using namespace acquire;

WaveformWnd::WaveformWnd( QWidget * parent ) : QWidget( parent )
                                             , spw_( new adplot::SpectrumWidget )
                                             , hpw_( new adplot::SpectrumWidget )
                                             , pkw_( new adplot::SpectrumWidget )
                                             , tpw_( new adplot::ChromatogramWidget )
                                             , tickCount_( 0 )
{
    for ( auto& tp: tp_ )
        tp = std::make_shared< adcontrols::Trace >();

    // for ( auto& marker : peak_marker_ )
    //     marker = std::make_unique< adplot::PeakMarker >();

    init();
    connect( document::instance(), &document::dataChanged, this, &WaveformWnd::dataChanged );
}

WaveformWnd::~WaveformWnd()
{
    fini();
    delete tpw_;
    delete pkw_;
    delete hpw_;
    delete spw_;
}

void
WaveformWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    do {
        splitter->addWidget( tpw_ );
        splitter->addWidget( hpw_ ); // long-term
        splitter->addWidget( pkw_ );
        splitter->addWidget( spw_ );
        splitter->setStretchFactor( 0, 1 );
        splitter->setStretchFactor( 1, 3 );
        splitter->setStretchFactor( 2, 3 );
        splitter->setOrientation( Qt::Vertical );
    } while(0);

    tpw_->setMinimumHeight( 8 );
    spw_->setMinimumHeight( 80 );
    hpw_->setMinimumHeight( 80 );
    pkw_->setMinimumHeight( 80 );
    spw_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
    hpw_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
    pkw_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
    spw_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 40 );
    hpw_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 40 );
    pkw_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 40 );

    spw_->setAxisTitle( QwtPlot::yLeft, tr( "<i>mV</i>" ) );
    spw_->setAxisTitle( QwtPlot::yRight, tr( "<i>mV</i>" ) );
    spw_->enableAxis( QwtPlot::yRight, true );

    spw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    spw_->setKeepZoomed( false );

    hpw_->setAxisTitle( QwtPlot::yLeft, tr( "<i>Counts</i>" ) );
    hpw_->setAxisTitle( QwtPlot::yRight, tr( "<i>mV</i>" ) );
    hpw_->enableAxis( QwtPlot::yRight, true );

    hpw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    hpw_->setKeepZoomed( false );
    hpw_->setAutoAnnotation( false );

    //------------
    pkw_->setAxisTitle( QwtPlot::yLeft, tr( "<i>Counts</i>" ) );
    pkw_->setAxisTitle( QwtPlot::yRight, tr( "<i>mV</i>" ) );
    pkw_->enableAxis( QwtPlot::yRight, true );

    pkw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    pkw_->setKeepZoomed( false );
    pkw_->setAutoAnnotation( false );
    //------------

    spw_->link( hpw_ );
    hpw_->link( pkw_ );

    tpw_->setAxisTitle( QwtPlot::yLeft, tr( "<i>mV</i>" ) );
    tpw_->setAxisTitle( QwtPlot::yRight, tr( "<i>mV</i>" ) );
    tpw_->enableAxis( QwtPlot::yRight, true );

    QBoxLayout * layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 2 );
    layout->addWidget( splitter );

    const QColor colors[] = { QColor( 0x00, 0x00, 0xff, 0x80 ), QColor( 0xff, 0x00, 0x00, 0x80 ) };

    for ( size_t i = 0; i < threshold_markers_.size(); ++i ) {
        threshold_markers_[ i ] = new QwtPlotMarker();
        threshold_markers_[ i ]->setLineStyle( QwtPlotMarker::HLine );
        threshold_markers_[ i ]->setLinePen( colors[ i ], 2, Qt::DotLine );
        threshold_markers_[ i ]->setYAxis( ( i == 1 ) ? QwtPlot::yRight : QwtPlot::yLeft );
        threshold_markers_[ i ]->setYValue( ( i + 1 ) * 100 );
        threshold_markers_[ i ]->attach( spw_ );
    }

    // Histogram window marker
    int viewId = 0;
    for ( auto& histogram_window_marker : histogram_window_markers_ ) {
        int idx = 0;
        for ( auto& marker: histogram_window_marker ) {
            QColor color( tpw_->color( idx++ ) );
            color.setAlpha( 0x20 );
            marker = std::make_unique< adplot::SpanMarker >( color, QwtPlotMarker::VLine, 2.0 );
            marker->setVisible( false );
            marker->attach( viewId == 0 ? spw_ : hpw_ );
        }
        ++viewId;
    }

    // Threshold action (fence) marker (indigo)
    if ( ( threshold_action_marker_
           = std::make_unique< adplot::SpanMarker >( QColor( 0x4b, 0x00, 0x62, 0x60 ), QwtPlotMarker::VLine, 1.5 ) ) ) {
        threshold_action_marker_->attach( spw_ );
    }
}

void
WaveformWnd::fini()
{
    for ( auto& histogram_window_marker : histogram_window_markers_ ) {
        for ( auto& marker: histogram_window_marker )
            marker->detach();
    }
    threshold_action_marker_.reset();
}

void
WaveformWnd::onInitialUpdate()
{
    spw_->setKeepZoomed( false );
    hpw_->setKeepZoomed( false );
    pkw_->setKeepZoomed( false );
    //handle_method( QString() );
    //handle_threshold_method( 0 );
}


void
WaveformWnd::dataChanged( const boost::uuids::uuid& uuid, int idx )
{
    // if ( auto sp = document::instance()->recentSpectrum( uuid, idx ) ) {
    //     if ( uuid == WaveformObserver::__objid__ ) {
    //         //         // waveform (analog)
    //         double seconds = sp->getMSProperty().timeSinceInjection();
    //         size_t nacc = sp->getMSProperty().numAverage();
    //         QString title = QString( "ACQUIRE[AVG]: Elapsed time: %1s, <font color=blue>#%2, N=%3" ).arg( QString::number( seconds, 'f', 4 )
    //                                                                                          , QString::number( sp->getMSProperty().trigNumber() )
    //                                                                                          , QString::number( nacc )  );
    //         spw_->setTitle( title );
    //         spw_->setData( sp, idx, bool( idx ) );
    //         spw_->setKeepZoomed( true );

    //     } else if ( uuid == pkd_observer ) {
}

void
WaveformWnd::setMethod( const adcontrols::TofChromatogramsMethod& m )
{
    int idx = 0;
    for ( auto& item : m ) {

        if ( idx < histogram_window_markers_[0].size() ) {

            if ( item.formula() == "TIC" || ( item.time() < 1.0e-9 ) ) {

                for ( size_t i = 0; i < histogram_window_markers_.size(); ++i ) {
                    auto& marker = histogram_window_markers_[ i ][ idx ];
                    marker->setVisible( false );
                }

            } else {
                using adcontrols::metric::scale_to_micro;

                double lower = item.time() - item.timeWindow() / 2;
                double upper = item.time() + item.timeWindow() / 2;

                for ( size_t i = 0; i < histogram_window_markers_.size(); ++i ) {
                    auto& marker = histogram_window_markers_[ i ][ idx ];
                    marker->setXValue( scale_to_micro( lower ), scale_to_micro( upper ) );
                    marker->setVisible( true );
                }
            }
        }
        ++idx;
    }
    spw_->replot();
    hpw_->replot();
    pkw_->replot();
}

void
WaveformWnd::handle_threshold_action( const QJsonDocument& doc )
{
    // ADDEBUG() << doc.toJson( QJsonDocument::Compact ).toStdString();
    if ( doc.object().contains( "threshold_action" ) ) {
        auto action = doc.object()[ "threshold_action" ].toObject();
        bool enable = action[ "enable" ].toBool();
        auto delay = action[ "delay" ].toDouble();
        auto width = action[ "width" ].toDouble();
        // ADDEBUG() << __FUNCTION__ << " enable: " << enable << ", delay: " << delay << ", width: " << width;

        double lower = ( delay - width / 2 ) * std::micro::den; // us
        double upper = ( delay + width / 2 ) * std::micro::den; // us

        bool replot( false );
        auto x = threshold_action_marker_->xValue();
        if ( ! (adportable::compare<double>::approximatelyEqual( x.first, lower ) &&
                adportable::compare<double>::approximatelyEqual( x.second, upper ) ) )
            replot = true;

        threshold_action_marker_->setXValue( lower, upper );

        if ( enable != threshold_action_marker_->isVisible() )
            replot = true;

        threshold_action_marker_->setVisible( enable );

        if ( replot )
            spw_->replot();
    }
}

void
WaveformWnd::handle_threshold_method( const QJsonDocument& doc )
{
    // ADDEBUG() << doc.toJson( QJsonDocument::Compact ).toStdString();

    if ( doc.object().contains( "threshold_method" ) ) {
        auto method = doc.object()[ "threshold_method" ].toObject();
        auto enable = method[ "enable" ].toBool();
        auto threshold = method[ "threshold_level" ].toDouble();
        auto title = method[ "title" ].toString();
        int ch = title == "CH2" ? 1 : 0;

        // ADDEBUG() << __FUNCTION__ << " enable: " << enable << ", threshold: " << threshold << ", title: " << title.toStdString();

        bool replot( false );

        double level_mV = threshold * 1.0e3;

        if ( !adportable::compare<double>::approximatelyEqual( threshold_markers_[ ch ]->yValue(), level_mV ) )
            replot = true;

        threshold_markers_[ ch ]->setYValue( level_mV );

        if ( enable != threshold_markers_[ ch ]->isVisible() )
            replot = true;

        threshold_markers_[ ch ]->setVisible( enable );

        if ( replot )
            spw_->replot();
    }
}

void
WaveformWnd::handle_threshold_level( double level_mV )
{
    const int ch = 0;
    bool replot( false );

    if ( !adportable::compare<double>::approximatelyEqual( threshold_markers_[ ch ]->yValue(), level_mV ) )
        replot = true;

    threshold_markers_[ ch ]->setYValue( level_mV );

    if ( !threshold_markers_[ ch ]->isVisible() ) {
        threshold_markers_[ ch ]->setVisible( true );
        replot = true;
    }

    if ( replot )
        spw_->replot();
}
