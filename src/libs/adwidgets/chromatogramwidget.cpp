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

#include "chromatogramwidget.h"
#include <adcontrols/chromatogram.h>
#include <adcontrols/trace.h>
#include <adcontrols/descriptions.h>
#include <adcontrols/peaks.h>
#include <adcontrols/peak.h>
#include <adcontrols/baselines.h>
#include <adcontrols/baseline.h>
#include <adwidgets/axis.h>
#include <adwidgets/traces.h>
#include <adwidgets/trace.h>
#include <adwidgets/titles.h>
#include <adwidgets/title.h>
#include <adwidgets/peaks.h>
#include <adwidgets/peak.h>
#include <adwidgets/baselines.h>
#include <adwidgets/baseline.h>
#include <adwidgets/font.h>
#include <adwidgets/colorindices.h>
#include <adwidgets/annotations.h>
#include <adwidgets/annotation.h>
#include <adwidgets/markers.h>
#include <adwidgets/marker.h>
#include <adutils/dataplothelper.h>
#include <sstream>

using namespace adwidgets::ui;

namespace adwidgets {
    namespace internal {
        class ChromatogramWidgetImpl { 
        public:
            static void setTitle( adwidgets::ui::Dataplot& plot, int idx, const adcontrols::Descriptions& desc_v ) {
                plot.titles()[0].text( L"." ); plot.titles()[0].visible( true );
                plot.titles()[1].text( L"." ); plot.titles()[1].visible( true );
                if ( idx < 3 ) {
                    adwidgets::ui::Title title = plot.titles()[idx];
                    std::wstring ttext;
                    for ( size_t i = 0; i < desc_v.size(); ++i ) {
                        if ( i != 0 )
                            ttext += L" :: ";
                        ttext += desc_v[i].text();
                    }
                    title.text( ttext );
                    title.visible( true );
                }
            }

            static void setTitle( adwidgets::ui::Dataplot& plot, int idx, const std::wstring& text ) {
                if ( idx < 3 ) {
                    adwidgets::ui::Title title = plot.titles()[idx];
                    title.text( text );
                    title.visible( true );
                }
            }

        };
    }
}


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
ChromatogramWidget::setData( const adcontrols::Chromatogram& c)
{
    setData( c, 0 );
}

void
ChromatogramWidget::setData( const adcontrols::Trace& d, int idx, bool /* yaxis2 */ )
{
    if ( d.size() < 2 )
        return;

	while ( int(traces().size()) <= idx )
		traces().add();

    const double * pX = d.getTimeArray();
    const double * pY = d.getIntensityArray();
    if ( pX == 0 || pY == 0 )
        return;

    std::pair<double, double> xrange( pX[0], pX[ d.size() - 1 ] );
    display_range_x( xrange );

    adwidgets::ui::Trace trace = traces()[idx];

    trace.setXYDirect( d.size(), pX, pY );
    Markers markers = trace.markers();
    markers.style( MS_Circle );
    markers.visible( true );
    trace.visible(true);
    traces().visible(true);

    std::wostringstream o;
    o << L"Chromatogram: " << display_range_x().second << L" min";

    adwidgets::internal::ChromatogramWidgetImpl::setTitle( *this, idx, o.str() );
}

void
ChromatogramWidget::setData( const adcontrols::Chromatogram& c, int idx, bool yaxis2 )
{
	while ( int(traces().size()) <= idx )
		traces().add();

    adwidgets::internal::ChromatogramWidgetImpl::setTitle( *this, idx, c.getDescriptions() );

    adwidgets::ui::Trace trace = traces()[idx];

    std::pair<double, double> xrange( adcontrols::Chromatogram::toMinutes( c.timeRange() ) );
    std::pair<double, double> yrange( c.getMinIntensity(), c.getMaxIntensity() );

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
    trace.traceStyle( Trace::TS_Connected );
    if ( c.getTimeArray() ) {
        trace.setXYDirect( c.size(), c.getTimeArray(), c.getIntensityArray() );
    } else {
        boost::scoped_array<double> timearray( new double [ c.size() ] );
        for ( size_t i = 0; i < c.size(); ++i )
            timearray[i] = adcontrols::Chromatogram::toMinutes( c.timeFromDataIndex( i ) );
        trace.setXYDirect( c.size(), timearray.get(), c.getIntensityArray() );
    }

    trace.visible( true );
    traces().visible( true );

    setPeaks( c.peaks(), c.baselines(), trace );
    setAnnotations( c.peaks(), trace );
}

void
ChromatogramWidget::setPeaks(  const adcontrols::Peaks& peaks
                             , const adcontrols::Baselines& baselines
                             , Trace& trace )
{
    trace.font().size( 80000 );
    trace.font().bold( false );

    ui::Peaks pks = trace.peaks();
    pks.clear();
    for ( adcontrols::Peaks::vector_type::const_iterator it = peaks.begin(); it != peaks.end(); ++it ) {
        ui::Peak uipk = pks.add();
        adutils::DataplotHelper::copy( uipk, *it );
        uipk.colorIndex( this->getColorIndex( adwidgets::ui::CI_PeakMark ) );
    }
    pks.visible(true);

    ui::Baselines bss = trace.baselines();
    bss.clear();
    for ( adcontrols::Baselines::vector_type::const_iterator it = baselines.begin(); it != baselines.end(); ++it ) {
        ui::Baseline bs = bss.add();
        adutils::DataplotHelper::copy( bs, *it );
        bs.colorIndex( this->getColorIndex( adwidgets::ui::CI_BaseMark ) );
    }
    bss.visible( true );

}

void
ChromatogramWidget::setAnnotations( const adcontrols::Peaks& peaks, Trace& trace )
{
    trace.autoAnnotation( false );
    adwidgets::ui::Annotations annos = trace.annotations();
    annos.clear();

    for ( adcontrols::Peaks::vector_type::const_iterator it = peaks.begin(); it != peaks.end(); ++it ) {
        adwidgets::ui::Annotation anno = annos.add();
        if ( ! it->name().empty() )
            anno.value( it->name() );
        else
            anno.value( adcontrols::timeutil::toMinutes( it->peakTime() ).minutes );
        anno.x( adcontrols::timeutil::toMinutes( it->peakTime() ) );
        anno.y( it->peakHeight() );
        anno.colorIndex( 2 );
    }
    annos.centreHorizontal( true );
    annos.centreVertical( true );
    annos.visible( true );
    annos.decimalsX( 3 );
}

