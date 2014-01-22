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

using namespace adwplot;

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
#include <QBrush>
#include <QEvent>
#include <QMouseEvent>
#include <iostream>

namespace detail {

    class zoomer : public QwtPlotZoomer {
    public:
        zoomer( QWidget * canvas ) : QwtPlotZoomer( canvas ) {
            setTrackerMode( AlwaysOn );
        }
        virtual QwtText trackerTextF( const QPointF &pos ) const {
            QColor bg( Qt::white );
            bg.setAlpha( 128 );
            
            QwtText text = QwtPlotZoomer::trackerTextF( pos );
			text.setBackgroundBrush( QBrush( bg ) );
            return text;
        }
    };

    class ColorMap: public QwtLinearColorMap {
    public:
        ColorMap(): QwtLinearColorMap( Qt::darkCyan, Qt::red ) {
            addColorStop( 0.1, Qt::cyan );
            addColorStop( 0.6, Qt::green );
            addColorStop( 0.95, Qt::yellow );
        }
    };

    class PickerMachine : public QwtPickerMachine {
    public:
        PickerMachine() : QwtPickerMachine( QwtPickerMachine::PointSelection ) {
        }
        virtual QList< QwtPickerMachine::Command > transition( const QwtEventPattern&, const QEvent * e ) {
            QList< QwtPickerMachine::Command > cmdList;
            if ( e->type() == QEvent::MouseMove )
                cmdList << Move;
            return cmdList;
        }
    };
    
    class Picker: public QwtPlotPicker {
    public:
        Picker(QWidget *canvas): QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, canvas) {
            setRubberBand(QwtPlotPicker::CrossRubberBand);
            QPen pen( QColor( 0xff, 0, 0, 0x80 ) ); // transparent darkRed
            setRubberBandPen( pen );
            setRubberBand(QwtPicker::CrossRubberBand);
            setTrackerMode( AlwaysOff );
            canvas->setMouseTracking(true);
        }

        void widgetMouseMoveEvent(QMouseEvent *e) {
            if ( !isActive() ) {
                begin();
                append( e->pos() );
            } else {
                move( e->pos() );
            }
            QwtPlotPicker::widgetMouseMoveEvent(e);
        }
        
        void widgetLeaveEvent(QEvent *) {
            end();
        }

        virtual QwtPickerMachine *stateMachine(int) const {
            return new PickerMachine;
        }
    };

} // namespace detail

SpectrogramWidget::SpectrogramWidget( QWidget *parent ) : QwtPlot(parent)
                                                        , spectrogram_( new QwtPlotSpectrogram() )
                                                        , zoomer_( new detail::zoomer( canvas() ) )
                                                        , panner_( new QwtPlotPanner( canvas() ) )
                                                        , data_(0)
{
    spectrogram_->setRenderThreadCount( 0 ); // use system specific thread count

    spectrogram_->setColorMap( new detail::ColorMap() );
    
    spectrogram_->setCachePolicy( QwtPlotRasterItem::PaintCache );

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
    
    detail::Picker * picker = new detail::Picker( canvas() );
    (void)picker;  // will delete by QWidget

    connect( this, SIGNAL( dataChanged() ), this, SLOT( handle_dataChanged() ) );
    connect( zoomer_.get(), SIGNAL( zoomed( const QRectF& ) ), this, SLOT( handleZoomed( const QRectF& ) ) );
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

