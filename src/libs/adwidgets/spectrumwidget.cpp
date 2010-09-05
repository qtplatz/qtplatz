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
    if ( traces().size() == 0 )
        traces().add();

    adwidgets::ui::Title title = titles()[0];
    const adcontrols::Descriptions& desc_v = ms.getDescriptions();
    if ( desc_v.size() )
        title.text( desc_v[0].text() );

    adwidgets::ui::Trace trace;
    if ( traces().size() < 1 ) {
        trace = traces().add();
    } else {
        trace = traces()[0];
    }
    std::pair<double, double> xrange = ms.range_x();
    std::pair<double, double> yrange = ms.range_y();
    std::pair<double, double> drange;
    drange.first = yrange.first < 0 ? yrange.first : 0;
    drange.second = yrange.second - drange.first < 250 ? 250 : yrange.second;

    traces().visible(false);

    this->display_range_y( drange );
    this->display_range_x( xrange );

    trace.colorIndex(2);
    trace.setXYDirect( ms.size(), ms.getMassArray(), ms.getIntensityArray() );
    trace.visible( true );
    traces().visible( true );
}
