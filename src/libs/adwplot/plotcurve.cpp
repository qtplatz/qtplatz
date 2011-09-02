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

#include "plotcurve.hpp"
#include "dataplot.hpp"
#include "seriesdata.hpp"
#include <qwt_plot_curve.h>
#include <qtwrapper/qstring.hpp>

using namespace adwplot;
using qtwrapper::qstring;

PlotCurve::PlotCurve( Dataplot& plot
             , const std::wstring& title ) : ownership_( true ) 
                                           , curve_( new QwtPlotCurve( qstring(title) ) )
{
    // curve_->setRenderHint( QwtPlotItem::RenderAntialiased );
    curve_->setPen( QPen( Qt::blue) );
    curve_->setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    curve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve_->attach( &plot );
}

PlotCurve::PlotCurve( const PlotCurve& t ) : ownership_( t.ownership_ ), curve_( t.curve_ )
{
    if ( ownership_ )
        const_cast<PlotCurve&>(t).ownership_ = false;
}

PlotCurve::~PlotCurve()
{
    if ( ownership_ )
        delete curve_;
}

PlotCurve&
PlotCurve::operator = ( const PlotCurve& t )
{
    curve_ = t.curve_;
    if ( ownership_ = t.ownership_ )
        const_cast<PlotCurve&>(t).ownership_ = false;
    return *this;
}

void
PlotCurve::setStyle( QwtPlotCurve::CurveStyle style )
{
    curve_->setStyle( style );
}

void
PlotCurve::setData( const double * xData, const double * yData, size_t size )
{
    curve_->setSamples( xData, yData, size );
}

