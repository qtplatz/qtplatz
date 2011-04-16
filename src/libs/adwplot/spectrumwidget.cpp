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

#include "spectrumwidget.h"
#include "trace.h"
#include "traces.h"
#include "zoomer.h"
#include "plotpicker.h"
#include "plotpanner.h"
#include "annotation.h"
#include <adcontrols/massspectrum.h>


using namespace adwplot;

SpectrumWidget::SpectrumWidget(QWidget *parent) : Dataplot(parent)
{
    setAxisTitle(QwtPlot::xBottom, "m/z");
    setAxisTitle(QwtPlot::yLeft, "Intensity[uV]");
}


void
SpectrumWidget::setData( const adcontrols::MassSpectrum& )
{
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum&, const adcontrols::MassSpectrum& )
{
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum& ms, int idx, bool yaxis2 )
{
/*
    while ( int(traces().size() ) <= idx )
        traces().add();
    
    Trace trace = traces()[ idx ];
    if ( ! trace.getSeriesData() )
        trace.setSeriesData( new SeriesDataMSImpl );

    SeriesDataMSImpl * data = dynamic_cast<SeriesDataMSImpl *>( trace.getSeriesData() );
    data->setData( ms );
    // trace.getSeriesData()->setData( ms );
*/
/*
    this->display_range_x( xrange );
	if ( yaxis2 ) {
		display_range_y2( drange );
		trace.YAxis( Trace::Y2 );
		axisY2().visible( true );
	} else {
		display_range_y( drange );
		trace.YAxis( Trace::Y1 );
	}
*/
    /*
    trace.colorIndex( 1 + idx );
    if ( ms.isCentroid() ) {
        trace.traceStyle( Trace::TS_Stick );
        trace.autoAnnotation( true );
        adwidgets::ui::Annotations anno = trace.annotations();
        anno.annotateX( true );
        anno.annotateY( true );
        anno.decimalsX( 5 );
        anno.decimalsY( 0 );
        anno.visible( true );
    } else { 
        trace.traceStyle( Trace::TS_Connected );
        trace.autoAnnotation( false );
    }
    trace.setXYDirect( ms.size(), ms.getMassArray(), ms.getIntensityArray() );

    const unsigned char * pColors = ms.getColorArray();
    if ( pColors ) {
        const size_t count = ms.size();
        boost::scoped_array< short > pColorIndices( new short [ count ] );
        for ( size_t i = 0; i < count; ++i ) {
            short color = pColors[ i ] ? pColors[ i ] + getColorIndex( adwidgets::ui::CI_MSTarget ) : idx;
            pColorIndices[ i ] = color;
        }
        trace.setColorIndicesDirect( count, pColorIndices.get() );
    }

    trace.visible( true );
    traces().visible( true );
*/
}

