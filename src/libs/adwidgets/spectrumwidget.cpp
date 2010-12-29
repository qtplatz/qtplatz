//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "spectrumwidget.h"
#include <adwidgets/axis.h>
#include <adwidgets/traces.h>
#include <adwidgets/trace.h>
#include <adwidgets/titles.h>
#include <adwidgets/title.h>
#include <adwidgets/colors.h>
#include <adwidgets/color.h>
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

	if ( idx < 3 ) {
		adwidgets::ui::Title title = titles()[idx];
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

    trace.colorIndex(2 + idx);
	trace.traceStyle( ms.isCentroid() ? Trace::TS_Stick : Trace::TS_Connected );
    trace.setXYDirect( ms.size(), ms.getMassArray(), ms.getIntensityArray() );
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
