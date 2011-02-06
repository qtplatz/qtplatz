/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <adwidgets/axis.h>
#include <adwidgets/traces.h>
#include <adwidgets/trace.h>
#include <adwidgets/titles.h>
#include <adwidgets/title.h>
#include <adwidgets/colors.h>
#include <adwidgets/color.h>
#include <adwidgets/font.h>
#include <adwidgets/annotations.h>
#include <adwidgets/colorindices.h>
#include <adcontrols/massspectrum.h>
#include <adcontrols/descriptions.h>
#include <adcontrols/description.h>

using namespace adwidgets::ui;

SpectrumWidget::~SpectrumWidget()
{
}

SpectrumWidget::SpectrumWidget(QWidget *parent) :  TraceWidget(parent)
{
    this->axisX().text( L"m/z" );
    this->axisY().text( L"Intensity" );
	this->title( 0, L"Spectrum" );
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum& ms )
{
	setData( ms, 0 );
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum& ms1, const adcontrols::MassSpectrum& ms2 )
{
	setData( ms1, 0, false );
	setData( ms2, 1, ms1.isCentroid() != ms2.isCentroid() );
}

void
SpectrumWidget::setData( const adcontrols::MassSpectrum& ms, int idx, bool yaxis2 )
{
	while ( int(traces().size()) <= idx )
		traces().add();

    if ( ms.isCentroid() ) { // workaround for annotation conflict with peak
        titles()[ 0 ].text( L"." );  titles()[ 0 ].visible( true );
        titles()[ 1 ].text( L"." );  titles()[ 1 ].visible( true );
        titles()[ 2 ].text( L"." );  titles()[ 2 ].visible( true );
    }

	if ( idx < 3 ) {
		adwidgets::ui::Title title = titles()[idx];
        title.font().size( 100000 );
        title.font().bold( false );
		const adcontrols::Descriptions& desc_v = ms.getDescriptions();
		std::wstring ttext;
		for ( size_t i = 0; i < desc_v.size(); ++i ) {
			if ( i != 0 )
				ttext += L" :: ";
			ttext += desc_v[i].text();
		}
        title.text( ttext );
        title.visible( true );
	}

    adwidgets::ui::Trace trace = traces()[idx];
    trace.font().size( 80000 );

	std::pair<double, double> xrange = ms.getAcquisitionMassRange();

    std::pair<double, double> yrange( ms.getMinIntensity(), ms.getMaxIntensity() );
    std::pair<double, double> drange;
    drange.first = yrange.first < 0 ? yrange.first : 0;
	drange.second = yrange.second - drange.first < 50 ? 50 : yrange.second;

    traces().visible(false);

    this->display_range_x( xrange );
	if ( yaxis2 ) {
		display_range_y2( drange );
		trace.YAxis( Trace::Y2 );
		axisY2().visible( true );
	} else {
		display_range_y( drange );
		trace.YAxis( Trace::Y1 );
	}

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
}

void
SpectrumWidget::handleZoomXAutoscaleY( double x1, double x2 )
{
    DataplotWidget::handleZoomXAutoscaleY( x1, x2 );
}

void
SpectrumWidget::handleZoomXY( double x1, double /* y1 */, double x2, double /* y2 */ )
{
    DataplotWidget::handleZoomXAutoscaleY( x1, x2 );
}

void
SpectrumWidget::handleOnRButtonClick( double x, double y )
{
}

void
SpectrumWidget::handleOnRButtonRange( double x1, double x2, double y1, double y2 )
{
}
