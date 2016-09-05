/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#include "waveformview.hpp"
#include "xyseriesdata.hpp"
#include "waveform.hpp"
#include <qwt_plot_curve.h>
#include <ratio>
#include <memory>

WaveformView::WaveformView( QWidget * parent ) : QwtPlot( parent )
                                               , curve_( std::make_unique< QwtPlotCurve >() )
{
    // setCanvasBackground( QColor( "#d0d0d0" ) );
    curve_->setRenderHint( QwtPlotItem::RenderAntialiased );
    curve_->setPen( Qt::cyan );
    curve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve_->setYAxis( QwtPlot::yRight );
    curve_->attach( this );

    setAxisAutoScale( QwtPlot::yLeft, true );
    setAxisAutoScale( QwtPlot::xBottom, true );
}

WaveformView::~WaveformView()
{
}

void
WaveformView::setTitle( const QString& text )
{
	QwtText qwtText( text, QwtText::RichText );
    QwtPlot::setTitle( qwtText );
}

void
WaveformView::setFooter( const QString& text )
{
	QwtText qwtText( text, QwtText::RichText );
    QwtPlot::setFooter( qwtText );
}

void
WaveformView::setData( std::shared_ptr< const waveform > d )
{
    // data_ = std::make_unique< XYSeriesData >( d );
    auto data = new XYSeriesData( d );
    curve_->setSamples( data );

    auto rect = data->boundingRect();

    setAxisScale( QwtPlot::yLeft, rect.bottom(), rect.top() );
    setAxisScale( QwtPlot::xBottom, rect.left(), rect.right() );
    
    replot();
}
