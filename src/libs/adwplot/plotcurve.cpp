// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
					 , const std::wstring& title ) : curve_( new QwtPlotCurve( qstring(title) ) )
												   , series_( 0 ) 
{
    // curve_->setRenderHint( QwtPlotItem::RenderAntialiased );
    curve_->setPen( QPen( Qt::blue) );
    curve_->setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    curve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve_->attach( &plot );
}

PlotCurve::PlotCurve( const PlotCurve& t ) : curve_( t.curve_ )
                                           , series_( t.series_ ) 
{
}

PlotCurve::~PlotCurve()
{
}

PlotCurve&
PlotCurve::operator = ( const PlotCurve& t )
{
    curve_ = t.curve_;
    series_ = t.series_;
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

