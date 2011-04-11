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

#include "zoomer.h"

using namespace adwplot;

Zoomer::Zoomer( int xAxis, int yAxis, QwtPlotCanvas * canvas ) : QwtPlotZoomer( xAxis, yAxis, canvas )
{
    setTrackerMode(QwtPicker::AlwaysOff);
    setRubberBand(QwtPicker::NoRubberBand);

    // RightButton: zoom out by 1
    // Ctrl+RightButton: zoom out to full size

    setMousePattern( QwtEventPattern::MouseSelect2,  Qt::RightButton, Qt::ControlModifier );
    setMousePattern( QwtEventPattern::MouseSelect3,  Qt::RightButton );

    setRubberBand( QwtPicker::RectRubberBand );
    setRubberBandPen( QColor(Qt::green) );
    setTrackerMode( QwtPicker::ActiveOnly );
    setTrackerPen( QColor( Qt::white ) );
}
