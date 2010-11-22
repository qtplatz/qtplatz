//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "chromatogramwidget.h"
#include <adcontrols/chromatogram.h>
#include <adcontrols/descriptions.h>
#include <adwidgets/axis.h>
#include <adwidgets/traces.h>
#include <adwidgets/trace.h>
#include <adwidgets/titles.h>
#include <adwidgets/title.h>


using namespace adwidgets::ui;

ChromatogramWidget::~ChromatogramWidget()
{
}

ChromatogramWidget::ChromatogramWidget(QWidget *parent) : TraceWidget(parent)
{
    this->axisX().text( L"Time(min)");
    this->axisY().text( L"(uV)");
    this->title( 0, L"Chromatogram" );
}

void
ChromatogramWidget::setData( const adcontrols::Chromatogram& c )
{
    setData( c, 0 );
}

void
ChromatogramWidget::setData( const adcontrols::Chromatogram& c, int idx, bool yaxis1 )
{
	while ( int(traces().size()) <= idx )
		traces().add();

	if ( idx < 3 ) {
		adwidgets::ui::Title title = titles()[idx];
        const adcontrols::Descriptions& desc_v = c.getDescriptions();
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

    std::pair<double, double> xrange(c.maxTime(), c.minTime());
    // std::pair<double, double> yrange( ms.getMinIntensity(), ms.getMaxIntensity() );
#if 0
    std::pair<double, double> drange;
    drange.first = yrange.first < 0 ? yrange.first : 0;
	drange.second = yrange.second - drange.first < 50 ? 50 : yrange.second;
#endif
    traces().visible(false);
#if 0
    this->display_range_x( xrange );
	if ( yaxis2 ) {
		display_range_y2( drange );
		trace.YAxis( Trace::Y2 );
		axisY2().visible( true );
	} else {
		display_range_y( drange );
		trace.YAxis( Trace::Y1 );
	}
#endif
    trace.colorIndex(2 + idx);
    trace.traceStyle( Trace::TS_Connected );
    // trace.setXYDirect( c.size(), ms.getMassArray(), ms.getIntensityArray() );
    trace.visible( true );
    traces().visible( true );
}
