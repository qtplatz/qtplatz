/**************************************************************************
** Copyright (C) 2010- Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "spanmarker.hpp"
#include <qwt_plot_marker.h>
#include <qwt_plot.h>

using namespace adplot;

SpanMarker::SpanMarker() : markers_( { {new QwtPlotMarker(), new QwtPlotMarker()} } )
{
}

SpanMarker::SpanMarker( const QColor& color
                        , QwtPlotMarker::LineStyle style
                        , double lineWidth
                        , Qt::PenStyle penStyle ) : markers_( { {new QwtPlotMarker(), new QwtPlotMarker()} } )
{
    for ( auto marker : markers_ ) {
        marker->setLineStyle( style );
        marker->setLinePen( color, lineWidth, penStyle );
    }
}


QwtPlotMarker *
SpanMarker::marker( fence id )
{
    return markers_[ id ];
}

SpanMarker::~SpanMarker()
{
    detach();
    for ( auto marker : markers_ )
        delete marker;
}

void
SpanMarker::attach( QwtPlot * plot )
{
    for ( auto marker: markers_ )
        marker->attach( plot );
}

void
SpanMarker::detach()
{
    for ( auto marker: markers_ )
        marker->detach();
}

void
SpanMarker::setYAxis( int yAxis )
{
    for ( auto marker : markers_ )
        marker->setYAxis( yAxis );
}

void
SpanMarker::setValue( fence id, double x )
{
    markers_[ id ]->setXValue( x );
}

void
SpanMarker::setValue( double lower_value, double upper_value )
{
    markers_[ 0 ]->setXValue( lower_value );
    markers_[ 1 ]->setXValue( upper_value );
}

void
SpanMarker::setXValue( fence id, double x )
{
    markers_[ id ]->setXValue( x );
}

void
SpanMarker::setXValue( double lower_value, double upper_value )
{
    markers_[ 0 ]->setXValue( lower_value );
    markers_[ 1 ]->setXValue( upper_value );
}

void
SpanMarker::visible( bool v )
{
    setVisible( v );
}

void
SpanMarker::setVisible( bool v )
{
    for ( auto marker : markers_ )
        marker->setVisible( v );
}

bool
SpanMarker::isVisible() const
{
    return markers_[ 0 ]->isVisible();
}

double
SpanMarker::xValue( fence id ) const
{
    return markers_[ id ]->xValue();
}

std::pair< double, double >
SpanMarker::xValue() const
{
    return { markers_[ lower ]->xValue(), markers_[ upper ]->xValue() };
}

void
SpanMarker::replot()
{
    if ( auto plot = markers_[ 0 ]->plot() )
        plot->replot();
}
