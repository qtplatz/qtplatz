// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "chromatogramwidget.h"
#include "annotation.h"
#include "annotations.h"
#include "trace.h"
#include "traces.h"
#include "zoomer.h"
#include "peak.h"
#include "baseline.h"
#include "plotpicker.h"
#include "plotpanner.h"
#include "seriesdata.h"
#include <adcontrols/trace.h>
#include <adcontrols/chromatogram.h>
#include <adcontrols/peaks.h>
#include <adcontrols/peak.h>
#include <adcontrols/baselines.h>
#include <adcontrols/baseline.h>
#include <adcontrols/descriptions.h>
#include <adcontrols/description.h>
#include <boost/format.hpp>

using namespace adwplot;

ChromatogramWidget::ChromatogramWidget(QWidget *parent) :
    Dataplot(parent)
{
    setAxisTitle(QwtPlot::xBottom, "Time[min]");
    setAxisTitle(QwtPlot::yLeft, "Intensity[uV]");
}

void
ChromatogramWidget::setData( const adcontrols::Trace& d, int idx, bool yaxis2 )
{
    if ( d.size() < 2 )
        return;

    while ( int( traces_.size() ) <= idx )
        traces_.push_back( Trace( *this, L"trace" ) );

    Trace& trace = traces_[ idx ];
    if ( ! trace.getSeriesData() )
        trace.setSeriesData( new SeriesData );

    trace.getSeriesData()->setData( d );

    canvas()->invalidatePaintCache();
    canvas()->update( canvas()->contentsRect() );

/*
    std::pair<double, double> xrange( pX[0], pX[ d.size() - 1 ] );
    std::pair<double, double> yrange = d.range_y();
    display_range_x( xrange );
    display_range_y( yrange );

    adwidgets::ui::Trace trace = traces()[idx];

    trace.setXYDirect( d.size(), pX, pY );
    Markers markers = trace.markers();
    markers.style( MS_Circle );
    markers.visible( true );
    trace.visible(true);
    traces().visible(true);

    std::wostringstream o;
    o << L"Chromatogram: " << display_range_x().second << L" min";
*/
}

void
ChromatogramWidget::setData( const adcontrols::Chromatogram& c )
{
    std::wstring title;
    const adcontrols::Descriptions& desc_v = c.getDescriptions();
    for ( size_t i = 0; i < desc_v.size(); ++i ) {
        if ( ! title.empty() )
            title += L", ";
        title += desc_v[i].text();
    }
    setTitle( title );

    annotations_.clear();
    peaks_.clear();
    baselines_.clear();

    if ( traces_.empty() ) {
        traces_.push_back( Trace( *this, title ) );
        traces_.back().setSeriesData( new SeriesData );
    }
    Trace& trace = traces_.back();

    SeriesData * d = trace.getSeriesData();
    d->setData( c );

    QStack<QRectF> stack;
    stack.push_back( d->boundingRect() );
    zoomer1_->setZoomStack( stack );

#if 0
    const adcontrols::Baselines& baselines = c.baselines();
    for ( adcontrols::Baselines::vector_type::const_iterator it = baselines.begin(); it != baselines.end(); ++it )
        setBaseline( *it );
#endif

    const adcontrols::Peaks& peaks = c.peaks();
    for ( adcontrols::Peaks::vector_type::const_iterator it = peaks.begin(); it != peaks.end(); ++it )
        setPeak( *it );

    replot();

#if 0
    canvas()->invalidatePaintCache();
    //canvas()->update( canvas()->contentsRect() );
    canvas()->repaint( canvas()->contentsRect() );
#endif


}
    

void
ChromatogramWidget::setPeak( const adcontrols::Peak& peak )
{
    double tR = adcontrols::timeutil::toMinutes( peak.peakTime() );

    std::wstring label = peak.name();
    if ( label.empty() )
        label = ( boost::wformat( L"%.3lf" ) % tR ).str();

    Annotations annots( *this, annotations_ );

    Annotation anno = annots.add( tR, peak.peakHeight(), label );
    anno.setLabelAlighment( Qt::AlignTop | Qt::AlignCenter );

    peaks_.push_back( adwplot::Peak( *this, peak ) );
}

void
ChromatogramWidget::setBaseline( const adcontrols::Baseline& bs )
{
    baselines_.push_back( adwplot::Baseline( *this, bs ) );
}

