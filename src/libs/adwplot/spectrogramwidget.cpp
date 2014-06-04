/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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
#include "picker.hpp"
#include <adcontrols/spectrogram.hpp> // clusters
#include <adportable/debug.hpp>
#include <qtwrapper/font.hpp>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_painter.h>
#include <qwt_plot_item.h>
#include <QBrush>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <iostream>
#include <boost/format.hpp>
#include <cmath>

namespace adwplot {
    namespace spectrogram {
        
        class ColorMap: public QwtLinearColorMap {
        public:
            ColorMap(): QwtLinearColorMap( Qt::darkBlue, Qt::red ) {
                addColorStop( 0.1, Qt::cyan );
                addColorStop( 0.7, Qt::green );
                addColorStop( 0.95, Qt::yellow );
            }
        };

        class ClusterMarker : public QwtPlotItem {
        public:
            ClusterMarker() {
                setZ( 10 );
            }
            void draw( QPainter * painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect ) const override {
                if ( auto clusters = clusters_.lock() ) {

					QRectF scaleRect = this->scaleRect( xMap, yMap );
					if ( scaleRect.height() > 100 )
						return;

                    painter->save();
                    painter->setPen( QColor( 0xff, 0xff, 0xff, 0x60 ) );

					for ( auto& cluster: *clusters ) {
						QRectF rc;
						rc.setCoords( cluster.time_interval().first, cluster.mass_interval().first
							, cluster.time_interval().second, cluster.mass_interval().second );
						QRectF xrc = QwtScaleMap::transform( xMap, yMap, rc );
						if ( canvasRect.contains( xrc ) )
							painter->drawRect( xrc ); 
					}
                    painter->restore();
                    
                } else {
                    int x0 = xMap.transform( 5.0 );
                    int x1 = xMap.transform( 5.1 );
                    int y0 = yMap.transform( 400.0 );
                    int y1 = yMap.transform( 401.0 );
                    QRect rc( x0, y0, x1 - x0, y1 - y0 );
                    painter->fillRect( rc, QColor( 0xff, 0x00, 0x00, 0x80 ) ); 
                }
            }
            std::weak_ptr< adcontrols::SpectrogramClusters > clusters_;
        };

    } // namespace spectrogram
}

using namespace adwplot;

SpectrogramWidget::SpectrogramWidget( QWidget *parent ) : QwtPlot(parent)
                                                        , spectrogram_( new QwtPlotSpectrogram() )
                                                        , zoomer_( new Zoomer( xBottom, yLeft, canvas() ) )
                                                        , picker_( new Picker( canvas() ) )
                                                        , data_(0)
                                                        , clusterMarker_( new spectrogram::ClusterMarker() )
{
    spectrogram_->setRenderThreadCount( 0 ); // use system specific thread count
    spectrogram_->setColorMap( new spectrogram::ColorMap() );
    spectrogram_->setCachePolicy( QwtPlotRasterItem::PaintCache );

    if ( Zoomer * zoomer = dynamic_cast< Zoomer * >( zoomer_ ) ) {
        using namespace std::placeholders;
        zoomer->tracker1( std::bind( &SpectrogramWidget::tracker1, this, _1 ) );
        zoomer->tracker2( std::bind( &SpectrogramWidget::tracker2, this, _1, _2 ) );
        zoomer_->setRubberBandPen( QPen( QColor( 0xff, 0xff, 0x80, 0x80 ) ) ); // Ivory, transparency = 50%
    }

    if ( picker_ ) {
        connect( picker_, SIGNAL( selected( const QPointF& ) ), this, SLOT( handleSelected( const QPointF& ) ) );
        connect( picker_, SIGNAL( selected( const QRectF& ) ), this, SLOT( handleSelected( const QRectF& ) ) );
        // picker_->setStateMachine( new QwtPickerClickPointMachine );
        picker_->setTrackerMode( QwtPicker::AlwaysOff );
        picker_->setEnabled( true );
    }

	setData( new SpectrogramData() );
    spectrogram_->attach( this );
    clusterMarker_->attach( this );
    clusterMarker_->setZ( spectrogram_->z() + 10 );

    const QwtInterval zInterval = spectrogram_->data()->interval( Qt::ZAxis );
    // A color bar on the right axis
    QwtScaleWidget *rightAxis = axisWidget( QwtPlot::yRight );
    rightAxis->setTitle( QwtText( "Intensity", QwtText::RichText ) );
    rightAxis->setColorBarEnabled( true );
	rightAxis->setColorMap( zInterval, new spectrogram::ColorMap );

    setAxisScale( QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );
    enableAxis( QwtPlot::yRight );

    axisWidget( QwtPlot::xBottom )->setTitle( QwtText( "Time[min]", QwtText::RichText ) );
    axisWidget( QwtPlot::yLeft )->setTitle( QwtText( "<i>m/z</i>", QwtText::RichText ) );

    QwtScaleWidget *yAxis = axisWidget( QwtPlot::yLeft );    
    yAxis->setTitle( "m/z" );

    QFont font;
    qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontAxisLabel );

    setAxisFont( QwtPlot::xBottom, font );
    setAxisFont( QwtPlot::yLeft, font );
    setAxisFont( QwtPlot::yRight, font );

    plotLayout()->setAlignCanvasToScales( true );
    replot();

    const QFontMetrics fm( axisWidget( QwtPlot::yLeft )->font() );
    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( fm.width( "8000.0" ) ); // width for yLeft axis

    connect( zoomer_, SIGNAL( zoomed( const QRectF& ) ), this, SLOT( handleZoomed( const QRectF& ) ) );
}

void
SpectrogramWidget::setData( SpectrogramData * data )
{
    data_ = data;

    const QwtInterval xInterval = data->interval( Qt::XAxis );
    setAxisScale( QwtPlot::xBottom, xInterval.minValue(), xInterval.maxValue() );

    const QwtInterval yInterval = data->interval( Qt::YAxis );
    setAxisScale( QwtPlot::yLeft, yInterval.minValue(), yInterval.maxValue() );

    const QwtInterval zInterval = data->interval( Qt::ZAxis );
    setAxisScale( QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );
    // A color bar on the right axis
    axisWidget( QwtPlot::yRight )->setColorMap( zInterval, new spectrogram::ColorMap() );

    spectrogram_->setData( data );
    replot();

    zoomer_->setZoomBase( false );
}

void
SpectrogramWidget::setData( adcontrols::SpectrogramClusters * clusters )
{
    auto ptr = clusters->shared_from_this();
    clusterMarker_->clusters_ = ptr;
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
            spectrogram_->invalidateCache();
			const QwtInterval zInterval = data_->interval( Qt::ZAxis );
            axisWidget( QwtPlot::yRight )->setColorMap( zInterval, new spectrogram::ColorMap );
            setAxisScale( QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );
            replot();
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

void
SpectrogramWidget::handleSelected( const QPointF& pos )
{
    emit onSelected( pos );
}

void
SpectrogramWidget::handleSelected( const QRectF& rect )
{
    QRect rc( QPoint( transform( xBottom, rect.left() ), transform( yLeft, rect.top() ) )
              , QPoint( transform( xBottom, rect.right() ), transform( yLeft, rect.bottom() ) ) );
    rc = rc.normalized();
    if ( rc.width() < 2 && rc.height() < 2 )
        emit onSelected( QPointF( rect.left(), rect.top() ) );
    else
        emit onSelected( rect );
}

