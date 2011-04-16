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

#include "spectrumwidget.h"
#include "seriesdata.h"
#include "trace.h"
#include "traces.h"
#include "zoomer.h"
#include "plotpicker.h"
#include "plotpanner.h"
#include "annotation.h"
#include <adcontrols/massspectrum.h>
#include <boost/foreach.hpp>
#include <qwt_plot_curve.h>

using namespace adwplot;

namespace adwplot { namespace internal {

    static Qt::GlobalColor color_table[] = {
        Qt::blue,
        Qt::red,
        Qt::green,
        Qt::cyan,
        Qt::magenta,
        Qt::yellow,
        Qt::darkRed,
        Qt::darkGreen,
        Qt::darkBlue,
        Qt::darkCyan,
        Qt::darkMagenta,
        Qt::darkYellow,
        Qt::darkGray,
        Qt::gray,
        Qt::lightGray,
    };


    class TraceData {
    public:
        void setData( Dataplot& plot, const adcontrols::MassSpectrum& ms );
        const QRectF& boundingRect() const { return rect_; }
    private:
        adcontrols::MassSpectrum ms_;
        QRectF rect_;
        std::vector< Trace > traces_;
        typedef std::map< int, SeriesData * > map_type;
        map_type data_;
    };

}
}

SpectrumWidget::SpectrumWidget(QWidget *parent) : Dataplot(parent)
{
    setAxisTitle(QwtPlot::xBottom, "m/z");
    setAxisTitle(QwtPlot::yLeft, "Intensity[uV]");

    // picker_->setRubberBand( QwtPicker::CrossRubberBand );
    // zoomer1_->setRubberBandPen( QColor(Qt::green) );
    zoomer1_->setRubberBand( QwtPicker::CrossRubberBand );
    zoomer1_->autoYScale( true );

    zoomer2_.reset(); //  new Zoomer( QwtPlot::xTop, QwtPlot::yRight, canvas() ) );
}

void
SpectrumWidget::zoom( const QRectF& rect )
{
/*
    double m1, y1, m2, y2;
    rect.getCoords( &m1, &y1, &m2, &y2 );

    QRectF rc = zoomer1_->zoomRect();
    double cx1, cy1, cx2, cy2;
    rc.getCoords( &cx1, &cy1, &cx2, &cy2 );
    rc.setCoords( m1, cy1, m2, cy2 );

*/
    QRectF rc = zoomer1_->zoomRect();
    rc.setLeft( rect.left() );
    rc.setRight( rect.right() );
    zoomer1_->zoom( rc );
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum& )
{
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum&, const adcontrols::MassSpectrum& )
{
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum& ms, int idx, bool yaxis2 )
{
    using internal::TraceData;

    if ( int( traces_.size() ) <= idx )
        traces_.resize( idx + 1 );

    TraceData& trace = traces_[ idx ];
    trace.setData( *this, ms );

    QStack<QRectF> stack;
    stack.push_back( trace.boundingRect() );
    zoomer1_->setZoomStack( stack );

    replot();
/*
    this->display_range_x( xrange );
	if ( yaxis2 ) {
		display_range_y2( drange );
		trace.YAxis( Trace::Y2 );
		axisY2().visible( true );
	} else {
		display_range_y( drange );
		trace.YAxis( Trace::Y1 );
	}
*/
    /*
    trace.colorIndex( 1 + idx );
    if ( ms.isCentroid() ) {
        trace.traceStyle( Trace::TS_Stick );
        trace.autoAnnotation( true );
        adwidgets::ui::Annotations anno = trace.annotations();
        anno.annotateX( true );
        anno.annotateY( true );
        anno.decimalsX( 5 );
        anno.decimalsY( 0 );
        anno.visible( true );
    } else { 
        trace.traceStyle( Trace::TS_Connected );
        trace.autoAnnotation( false );
    }
    trace.setXYDirect( ms.size(), ms.getMassArray(), ms.getIntensityArray() );

    const unsigned char * pColors = ms.getColorArray();
    if ( pColors ) {
        const size_t count = ms.size();
        boost::scoped_array< short > pColorIndices( new short [ count ] );
        for ( size_t i = 0; i < count; ++i ) {
            short color = pColors[ i ] ? pColors[ i ] + getColorIndex( adwidgets::ui::CI_MSTarget ) : idx;
            pColorIndices[ i ] = color;
        }
        trace.setColorIndicesDirect( count, pColorIndices.get() );
    }

    trace.visible( true );
    traces().visible( true );
*/
}



//
using namespace adwplot::internal;

void
TraceData::setData( Dataplot& plot, const adcontrols::MassSpectrum& ms )
{
    traces_.clear();

    ms_ = ms;

    const double * intens = ms.getIntensityArray();
    const double * masses = ms.getMassArray();
    const size_t size = ms.size();

    if ( ms.isCentroid() ) {
        const unsigned char * colors = ms.getColorArray();
        for ( size_t i = 0; i < size; ++i ) {
            int color = colors ? colors[i] : 0;
            if ( data_.find( color ) == data_.end() ) {
                SeriesData * d = new SeriesData;
                d->set_range_x( ms.getAcquisitionMassRange() );
                d->set_range_y( std::pair<double, double>( ms.getMinIntensity(), ms.getMaxIntensity() ) );
                data_[ color ] = d;
            }
            data_[ color ]->push_back( QPointF( masses[i], intens[i] ) );
        }
        BOOST_FOREACH( const map_type::value_type& pair, data_ ) {
            Trace trace( plot, L"" );
            QwtPlotCurve * curve = trace;
            if ( pair.first != 0 && pair.first < sizeof( color_table ) / sizeof( color_table[0] ) )
                curve->setPen( QPen( color_table[ pair.first ] ) );
            trace.setSeriesData( pair.second );
            trace.setStyle( Trace::Sticks );
            traces_.push_back( trace );
            rect_.setCoords( ms.getAcquisitionMassRange().first, ms.getMinIntensity(), ms.getAcquisitionMassRange().second, ms.getMaxIntensity() );
        }
    } else {
        for ( size_t i = 0; i < size; ++i ) {
            if ( data_.find( 0 ) == data_.end() ) {
                SeriesData * d = new SeriesData;
                d->set_range_x( ms.getAcquisitionMassRange() );
                d->set_range_y( std::pair<double, double>( ms.getMinIntensity(), ms.getMaxIntensity() ) );
                data_[ 0 ] = d;
            }
            data_[ 0 ]->push_back( QPointF( masses[i], intens[i] ) );
        }
        BOOST_FOREACH( const map_type::value_type& pair, data_ ) {
            Trace trace( plot, L"" );
            QwtPlotCurve * curve = trace;
            if ( pair.first != 0 && pair.first < sizeof( color_table ) / sizeof( color_table[0] ) )
                curve->setPen( QPen( color_table[ pair.first ] ) );
            trace.setSeriesData( pair.second );
            trace.setStyle( Trace::Lines );
            traces_.push_back( trace );
            rect_.setCoords( ms.getAcquisitionMassRange().first, ms.getMinIntensity(), ms.getAcquisitionMassRange().second, ms.getMaxIntensity() );
        }
    }
}