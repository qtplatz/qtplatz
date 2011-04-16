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

#include "dataplot.h"
#include "annotation.h"
#include "annotations.h"
#include "trace.h"
#include "traces.h"
#include "zoomer.h"
#include "plotpicker.h"
#include "plotpanner.h"
#include <qtwrapper/qstring.h>

using namespace adwplot;

Dataplot::Dataplot(QWidget *parent) : QwtPlot(parent)
{
    setMargin(5);
    setCanvasBackground( QColor( Qt::lightGray ) );
    zoomer1_.reset( new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, canvas() ) );
    zoomer2_.reset( new Zoomer( QwtPlot::xTop, QwtPlot::yRight, canvas() ) );
    picker_.reset( new PlotPicker( canvas() ) );
    panner_.reset( new PlotPanner( canvas() ) );
}

void
Dataplot::setTitle( const std::wstring& title )
{
    QwtPlot::setTitle( qtwrapper::qstring( title ) );
}

void
Dataplot::zoom( const QRectF& rect )
{
    if ( zoomer1_ )
        zoomer1_->zoom( rect );
}

void
Dataplot::link( Dataplot * p )
{
    connect( zoomer1_.get(), SIGNAL( zoomed( const QRectF& ) ), p, SLOT( zoom( const QRectF& ) ) );}

void
Dataplot::unlink( Dataplot * p )
{
    disconnect( zoomer1_.get(), SIGNAL( zoomed( const QRectF& ) ), p, SLOT( zoom( const QRectF& ) ) );}

