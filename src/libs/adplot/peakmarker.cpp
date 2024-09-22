/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "peakmarker.hpp"
#include <qwt_plot_marker.h>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/peak.hpp>
#include <QColor>

// enum { idPeakLeft, idPeakCenter, idPeakRight, idPeakBottom, idPeakThreshold, idPeakTop, numMarkers };

using namespace adplot;

PeakMarker::PeakMarker()
{
    for ( auto& marker: markers_ )
        marker = new QwtPlotMarker();

    markers_[ idPeakCenter ]->setLineStyle( QwtPlotMarker::VLine );
    markers_[ idPeakCenter ]->setLinePen( QColor( 0xff, 0, 0, 0x40 ), 0, Qt::DashDotLine );

    markers_[ idPeakBase ]->setLineStyle( QwtPlotMarker::HLine );
    markers_[ idPeakBase ]->setLinePen( Qt::darkGray, 0, Qt::DotLine );

    markers_[ idPeakThreshold ]->setLineStyle( QwtPlotMarker::HLine );
    markers_[ idPeakThreshold ]->setLinePen( Qt::darkGray, 0, Qt::DashDotLine );

    markers_[ idPeakTop ]->setLineStyle( QwtPlotMarker::HLine );
    markers_[ idPeakTop ]->setLinePen( Qt::darkGray, 0, Qt::DotLine );

    for ( auto id: { idPeakLeft, idPeakRight, idWidthLeft, idWidthRight } ) {
        markers_[ id ]->setLineStyle( QwtPlotMarker::VLine );
        markers_[ id ]->setLinePen( Qt::darkGray, 0, Qt::DotLine );
        // markers_[ idPeakRight ]->setLineStyle( QwtPlotMarker::VLine );
        // markers_[ idPeakRight ]->setLinePen( Qt::darkGray, 0, Qt::DotLine );
    }
}

QwtPlotMarker *
PeakMarker::marker( idAxis id )
{
    return markers_[ id ];
}

PeakMarker::~PeakMarker()
{
    // for ( auto marker: markers_ )
    //     delete marker;
}

void
PeakMarker::attach( QwtPlot * plot )
{
    for ( auto marker: markers_ )
        marker->attach( plot );
}

void
PeakMarker::detach()
{
    for ( auto marker: markers_ )
        marker->detach();
}

void
PeakMarker::setYAxis( int yAxis )
{
    for ( auto marker: markers_ )
        marker->setYAxis( yAxis );
}

void
PeakMarker::setValue( PeakMarker::idAxis idAxis, double x, double y )
{
    markers_[ idAxis ]->setValue( x, y );
}

void
PeakMarker::setPeak( const adcontrols::MSPeakInfoItem& pk, bool isTime, adcontrols::metric::prefix pfx )
{
    if ( isTime ) {
        markers_[ idPeakCenter ]->setValue( adcontrols::metric::scale_to( pfx, pk.time() ), 0 );
        markers_[ idPeakLeft ]->setValue( adcontrols::metric::scale_to( pfx, pk.centroid_left( true ) ), 0 );
        markers_[ idPeakRight ]->setValue( adcontrols::metric::scale_to( pfx, pk.centroid_right( true ) ), 0 );
    } else {
        markers_[ idPeakCenter ]->setValue( pk.mass(), 0 );
        markers_[ idPeakLeft ]->setValue( pk.centroid_left(), 0 );
        markers_[ idPeakRight ]->setValue( pk.centroid_right(), 0 );
    }
    markers_[ idPeakThreshold ]->setValue( 0, pk.centroid_threshold() );
    markers_[ idPeakBase ]->setValue( 0, pk.base_height() );
    markers_[ idPeakTop ]->setValue( 0, pk.height() + pk.base_height() );
}

void
PeakMarker::setPeak( const adcontrols::MassSpectrum& ms, int idx, bool isTime, adcontrols::metric::prefix pfx )
{
    if ( ms.size() > unsigned(idx) ) {
        if ( isTime ) {
            markers_[ idPeakCenter ]->setValue( adcontrols::metric::scale_to( pfx, ms.time( idx ) ), 0 );
        } else {
            markers_[ idPeakCenter ]->setValue( ms.mass( idx ), 0 );
        }
    }
    markers_[ idPeakLeft ]->setValue( 0, 0 );
    markers_[ idPeakRight ]->setValue( 0, 0 );
    markers_[ idPeakThreshold ]->setValue( 0, 0 );
    markers_[ idPeakBase ]->setValue( 0, 0 );
    markers_[ idPeakTop ]->setValue( 0, 0 );
}

void
PeakMarker::visible( bool v )
{
    if ( v ) {
        markers_[ idPeakCenter ]->setLinePen( QColor( 0xff, 0, 0, 0x40 ) );
    } else {
        markers_[ idPeakCenter ]->setLinePen( QColor( 0xff, 0, 0, 0x10 ) );
    }
}

// Chromatographic peak
void
PeakMarker::setPeak( const adcontrols::Peak& pk, adplot::constants::chromatogram_time_spec spec )
{
    auto const& tr = pk.retentionTime();

    if ( spec == constants::chromatogram_time_seconds ) {
        markers_[ idPeakCenter ]->setValue( pk.peakTime(), 0 );
        markers_[ idPeakLeft ]->setValue( pk.startTime(), 0 );
        markers_[ idPeakRight ]->setValue( pk.endTime(), 0 );
        markers_[ idWidthLeft ]->setValue( tr.boundary().first, 0 );
        markers_[ idWidthRight ]->setValue( tr.boundary().second, 0 );
    } else if ( spec == constants::chromatogram_time_minutes ) {
        markers_[ idPeakCenter ]->setValue( adcontrols::timeutil::toMinutes( pk.peakTime() ), 0 );
        markers_[ idPeakLeft ]->setValue( adcontrols::timeutil::toMinutes( pk.startTime() ), 0 );
        markers_[ idPeakRight ]->setValue( adcontrols::timeutil::toMinutes( pk.endTime() ), 0 );
    }

    //double hh = pk.topHeight() - pk.peakHeight() / 2;
    markers_[ idPeakThreshold ]->setValue( 0, tr.threshold().first );
    markers_[ idPeakBase ]->setValue( 0, std::min( pk.startHeight(), pk.endHeight() ) );
    markers_[ idPeakTop ]->setValue( 0, pk.topHeight() );

    //------>

}
