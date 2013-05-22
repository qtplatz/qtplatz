// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "plot.hpp"


Plot::Plot(QWidget *parent) :  QwtPlot(parent)
{
    curve0_.setRenderHint( QwtPlotItem::RenderAntialiased ); //, false );
    curve0_.setYAxis( QwtPlot::yLeft );
    curve0_.setPen( QPen( Qt::blue ) );
    curve0_.attach( this );

    curve1_.setRenderHint( QwtPlotItem::RenderAntialiased, false );
    curve1_.setYAxis( QwtPlot::yLeft );
    curve1_.setPen( QPen( Qt::red ) );
    curve1_.attach( this );

    curve2_.setRenderHint( QwtPlotItem::RenderAntialiased, false );
    curve2_.setYAxis( QwtPlot::yLeft );
    curve2_.setPen( QPen( Qt::green ) );
    curve2_.attach( this );

    curve2_.setRenderHint( QwtPlotItem::RenderAntialiased, false );
    curve2_.setYAxis( QwtPlot::yLeft );
    curve2_.setPen( QPen( Qt::black ) );
    curve2_.attach( this );
}

void
Plot::setData( const double * x, const double * y, size_t count, int id )
{
    switch( id ) {
    case 0:
        curve0_.setSamples( x, y, count );
        break;
    case 1:
        curve1_.setSamples( x, y, count );
        break;
    case 2:
        curve2_.setSamples( x, y, count );
        break;
    case 3:
        curve3_.setSamples( x, y, count );
        break;
    }
    //setAxisScale( QwtPlot::xBottom, time_range.first, time_range.second );
    //setAxisScale( QwtPlot::yLeft, intens[ c.min_element() ], intens[ c.max_element() ] );
}
