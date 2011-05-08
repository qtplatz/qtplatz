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

#include "annotation.hpp"
#include "dataplot.hpp"
#include <qtwrapper/qstring.hpp>
#include <qwt_plot_marker.h>
#include <qwt_text.h>

using namespace adwplot;

Annotation::Annotation( Dataplot& plot
                       , const std::wstring& label
                       , double x, double y) : plot_( &plot )
                                                     , marker_( new QwtPlotMarker )
{
    marker_->setValue( x, y );
    marker_->setLineStyle( QwtPlotMarker::NoLine );
    marker_->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
    // marker_->setLinePen( QPen( Qt::green, 0, Qt::DashDotLine) )
    QwtText text( qtwrapper::qstring::copy( label ) );
    text.setFont( QFont("Helvetica", 9, QFont::Normal ) );
    text.setColor( Qt::green );
    marker_->setLabel( text );
    marker_->attach( plot_ );
}

Annotation::Annotation( const Annotation& t ) : plot_( t.plot_ )
                                              , marker_( t.marker_ )
{
} 

void
Annotation::setLabelAlighment( Qt::Alignment align )
{
    marker_->setLabelAlignment( align );
}