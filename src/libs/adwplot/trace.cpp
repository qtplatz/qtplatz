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

#include "trace.hpp"
#include "dataplot.hpp"
#include "seriesdata.hpp"
#include <qwt_plot_curve.h>
#include <qtwrapper/qstring.hpp>

using namespace adwplot;
using qtwrapper::qstring;

Trace::Trace( Dataplot& plot
             , const std::wstring& title ) : plot_( &plot )
                                           , curve_( new QwtPlotCurve( qstring(title) ) )
                                           , data_(0) 
{
    curve_->setRenderHint( QwtPlotItem::RenderAntialiased );
    curve_->setPen( QPen( Qt::blue) );
    curve_->setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    curve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve_->attach( plot_ );
}

Trace::Trace( const Trace& t ) : plot_( t.plot_ ), curve_( t.curve_ ), data_( t.data_ )
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
    data_ = t.data_;
    return *this;
}

void
Trace::setStyle( Trace::CurveStyle style )
{
    curve_->setStyle( static_cast<QwtPlotCurve::CurveStyle>( style ) );
}

void
Trace::setData( const double * xData, const double * yData, int size )
{
    curve_->setSamples( xData, yData, size );
}

void
Trace::setSeriesData( SeriesData* d )
{
    data_ = d;  // don't delete data_.  QwtPlotCurve will delete.
    curve_->setData( data_ );
}

SeriesData *
Trace::getSeriesData()
{
    return data_;
}

