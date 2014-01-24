/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "spectrogramwidget.hpp"
#include "spectrogramdata.hpp"
#include "zoomer.hpp"

#include <adportable/debug.hpp>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_painter.h>
#include <QBrush>
#include <QEvent>
#include <QMouseEvent>
#include <iostream>
#include <boost/format.hpp>
#include <cmath>

namespace adwplot {
    namespace detail {
        
        class ColorMap: public QwtLinearColorMap {
        public:
            ColorMap(): QwtLinearColorMap( Qt::darkCyan, Qt::red ) {
                addColorStop( 0.1, Qt::cyan );
                addColorStop( 0.6, Qt::green );
                addColorStop( 0.95, Qt::yellow );
            }
        };

    } // namespace detail
}

using namespace adwplot;

SpectrogramWidget::SpectrogramWidget( QWidget *parent ) : QwtPlot(parent)
                                                        , spectrogram_( new QwtPlotSpectrogram() )
                                                        , zoomer_( new Zoomer( xBottom, yLeft, canvas() ) )
                                                        , panner_( new QwtPlotPanner( canvas() ) )
                                                        , data_(0)
{
    spectrogram_->setRenderThreadCount( 0 ); // use system specific thread count
    spectrogram_->setColorMap( new detail::ColorMap() );
    spectrogram_->setCachePolicy( QwtPlotRasterItem::PaintCache );

    if ( Zoomer * zoomer = dynamic_cast< Zoomer * >( zoomer_ ) ) {
        using namespace std::placeholders;
        zoomer->tracker1( std::bind( &SpectrogramWidget::tracker1, this, _1 ) );
        zoomer->tracker2( std::bind( &SpectrogramWidget::tracker2, this, _1, _2 ) );
    }

	setData( new SpectrogramData() );
    spectrogram_->attach( this );

    const QwtInterval zInterval = spectrogram_->data()->interval( Qt::ZAxis );
    // A color bar on the right axis
    QwtScaleWidget *rightAxis = axisWidget( QwtPlot::yRight );
    rightAxis->setTitle( QwtText( "Intensity", QwtText::RichText ) );
    rightAxis->setColorBarEnabled( true );
	rightAxis->setColorMap( zInterval, new detail::ColorMap );

    setAxisScale( QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );
    enableAxis( QwtPlot::yRight );

    axisWidget( QwtPlot::xBottom )->setTitle( QwtText( "Time (min)", QwtText::RichText ) );
    axisWidget( QwtPlot::yLeft )->setTitle( QwtText( "<i>m/z</i>", QwtText::RichText ) );

    QwtScaleWidget *yAxis = axisWidget( QwtPlot::yLeft );    
    yAxis->setTitle( "m/z" );

    plotLayout()->setAlignCanvasToScales( true );
    replot();

    // LeftButton for the zooming
    // Alt+LeftButton for the panning
    // RightButton: zoom out by 1
    // Shift+LeftButton: zoom out to full size

    zoomer_->setMousePattern( QwtEventPattern::MouseSelect2, Qt::LeftButton, Qt::ShiftModifier );
    zoomer_->setMousePattern( QwtEventPattern::MouseSelect3, Qt::RightButton );

    panner_->setAxisEnabled( QwtPlot::yRight, false );
    panner_->setMouseButton( Qt::LeftButton, Qt::AltModifier );

    // Avoid jumping when labels with more/less digits
    // appear/disappear when scrolling vertically

    const QFontMetrics fm( axisWidget( QwtPlot::yLeft )->font() );
    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( fm.width( "1.00" ) );

    const QColor c( Qt::darkBlue );
    zoomer_->setRubberBandPen( c );
    zoomer_->setTrackerPen( c );
    
    // detail::Picker * picker = new detail::Picker( canvas() );
    // (void)picker;  // will delete by QWidget

    connect( this, SIGNAL( dataChanged() ), this, SLOT( handle_dataChanged() ) );
    connect( zoomer_, SIGNAL( zoomed( const QRectF& ) ), this, SLOT( handleZoomed( const QRectF& ) ) );
	//    model::instance()->signal( std::bind(&SpectrogramWidget::handle_signal, this) );
}

void
SpectrogramWidget::setData( SpectrogramData * data )
{
    data_ = data;

    // A color bar on the right axis
    const QwtInterval zInterval = data->interval( Qt::ZAxis );
    axisWidget( QwtPlot::yRight )->setColorMap( zInterval, new detail::ColorMap() );
    setAxisScale( QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );

    const QwtInterval xInterval = data->interval( Qt::XAxis );
    setAxisScale( QwtPlot::xBottom, xInterval.minValue(), xInterval.maxValue() );

    const QwtInterval yInterval = data->interval( Qt::YAxis );
    setAxisScale( QwtPlot::yLeft, yInterval.minValue(), yInterval.maxValue() );

    spectrogram_->setData( data );
    replot();

    zoomer_->setZoomBase( false );
}

void
SpectrogramWidget::handleDataChanged()
{
	spectrogram_->invalidateCache();
 	replot();
}

void
SpectrogramWidget::handleShowContour( bool on )
{
    spectrogram_->setDisplayMode( QwtPlotSpectrogram::ContourMode, on );
    replot();
}

void
SpectrogramWidget::handleShowSpectrogram( bool on )
{
    spectrogram_->setDisplayMode( QwtPlotSpectrogram::ImageMode, on );
    spectrogram_->setDefaultContourPen( on ? QPen( Qt::black, 0 ) : QPen( Qt::NoPen ) );

    replot();
}

void
SpectrogramWidget::handleSetAlpha( int alpha )
{
    spectrogram_->setAlpha( alpha );
    replot();
}

void
SpectrogramWidget::handle_signal()
{
    emit dataChanged();
}

void
SpectrogramWidget::handleZoomed( const QRectF& rc )
{
    if ( data_ ) {
        if ( data_->zoomed( rc ) ) {
			const QwtInterval zInterval = data_->interval( Qt::ZAxis );
            axisWidget( QwtPlot::yRight )->setColorMap( zInterval, new detail::ColorMap );
            setAxisScale( QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );
            spectrogram_->invalidateCache();
        }
    }
}

QwtText
SpectrogramWidget::tracker1( const QPointF& pos )
{
    return QwtText( (boost::format("<i>m/z</i> %.4f @ %.3fmin") % pos.y() % pos.x() ).str().c_str(), QwtText::RichText );
}

QwtText
SpectrogramWidget::tracker2( const QPointF& p1, const QPointF& pos )
{
    double dm = pos.y() - p1.y();
    if ( std::abs( dm ) < 1.0 ) {
        dm *= 1000.0;
        return QwtText( (boost::format("<i>m/z</i> %.4f(&delta;=%.3fmDa)@ %.3fmin") % pos.y() % dm % pos.x() ).str().c_str(), QwtText::RichText );
    } else {
        return QwtText( (boost::format("<i>m/z</i> %.4f(&delta;=%.3fDa)@ %.3fmin") % pos.y() % dm % pos.x() ).str().c_str(), QwtText::RichText );
    }
}
