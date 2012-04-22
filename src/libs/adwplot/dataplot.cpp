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

#include "dataplot.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include "trace.hpp"
#include "traces.hpp"
#include "zoomer.hpp"
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_picker_machine.h>
#include <qtwrapper/qstring.hpp>

using namespace adwplot;

Dataplot::Dataplot(QWidget *parent) : QwtPlot(parent)
{
    setCanvasBackground( QColor( Qt::lightGray ) );

	// zoomer
    zoomer1_.reset( new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, canvas() ) );
	// zoomer2_.reset( new Zoomer( QwtPlot::xTop, QwtPlot::yRight, canvas() ) );

	// picker
    picker_.reset( new QwtPlotPicker( canvas() ) );
	// picker_->setStateMachine( new QwtPickerDragPointMachine() );
	// picker_->setStateMachine( new QwtPickerClickPointMachine() );
	picker_->setTrackerMode( QwtPicker::ActiveOnly );
	picker_->setTrackerPen( QColor( Qt::red ) );
	picker_->setMousePattern( QwtEventPattern::MouseSelect1,  Qt::LeftButton, Qt::ControlModifier );

	// panner
    panner_.reset( new QwtPlotPanner( canvas() ) );
	panner_->setMouseButton( Qt::LeftButton, Qt::AltModifier );
}

void
Dataplot::setTitle( const std::wstring& title )
{
    QwtPlot::setTitle( qtwrapper::qstring( title ) );
}

void
Dataplot::link( Dataplot * p )
{
    connect( zoomer1_.get(), SIGNAL( zoomed( const QRectF& ) ), p, SLOT( zoom( const QRectF& ) ) );
}

void
Dataplot::unlink( Dataplot * p )
{
    disconnect( zoomer1_.get(), SIGNAL( zoomed( const QRectF& ) ), p, SLOT( zoom( const QRectF& ) ) );
}

