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

#include "trace.h"
#include "dataplot.h"
#include <qwt_plot_curve.h>

using namespace adwplot;

Trace::Trace( Dataplot& plot, const QString& title ) : plot_( &plot ), curve_( new QwtPlotCurve( title ) )
{
    curve_->setRenderHint( QwtPlotItem::RenderAntialiased );
    curve_->setPen( QPen( Qt::blue) );
    curve_->setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    curve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve_->attach( plot_ );
}

Trace::Trace( const Trace& t ) : plot_( t.plot_ ), curve_( t.curve_ )
{
}

Trace::~Trace()
{
}

Trace&
Trace::operator = ( const Trace& t )
{
    plot_ = t.plot_;
    curve_ = t.curve_;
    return *this;
}

void
Trace::setData( const double * xData, const double * yData, size_t size )
{
    curve_->setSamples( xData, yData, size );
}

void
Trace::setStyle( Trace::CurveStyle style )
{
    curve_->setStyle( static_cast<QwtPlotCurve::CurveStyle>( style ) );
}