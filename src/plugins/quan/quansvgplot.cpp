/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "quansvgplot.hpp"
#include "quanplot.hpp"
#include "quanplotdata.hpp"
#include <adportable/utf.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adwplot/peakmarker.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>
#include <QSvgGenerator>
#include <QIODevice>
#include <QBuffer>
#include <boost/format.hpp>

using namespace quan;

static int count;

QuanSvgPlot::QuanSvgPlot()
{
}

bool
QuanSvgPlot::plot( const QuanPlotData& data, size_t idx, int fcn, const std::string& dataSource )
{
    auto tCentroid( std::make_shared< adcontrols::MassSpectrum >() );
    auto tProfile( std::make_shared< adcontrols::MassSpectrum >() );

    adcontrols::MSPeakInfoItem pk;

    if ( auto pkinfo = data.pkinfo->findProtocol( fcn ) ) {

        auto pkIt = pkinfo->size() >= idx ? pkinfo->begin() + idx : pkinfo->end();
        
        if ( pkIt == pkinfo->end() )
            return false;

        pk = *pkIt;

        double mass = pkIt->mass();
        std::pair< double, double > range = std::make_pair( mass - 2.0, mass + 2.0 );

        if ( !data.centroid->trim( *tCentroid, range ) )
            return false;

        if ( !data.profile->trim( *tProfile, range ) )
            return false;
    }

    QSvgGenerator generator;

    // generator.setFileName( ( boost::format( "C:/Users/Toshi/Documents/data/QUAN/quan-svg_%1%.svg" ) % count++ ).str().c_str() );
    svg_.clear();
    QBuffer buffer( &svg_ );
    generator.setOutputDevice( &buffer );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    QRectF rect( 0, 0, 350, 300 );

    adwplot::SpectrumWidget plot;
    plot.setData( tProfile, 0 );
    plot.setData( tCentroid, 1, true );
    adwplot::PeakMarker marker;

    marker.attach( &plot );
    // set color etc.
    for ( int id = 0; id < adwplot::PeakMarker::numMarkers; ++id )
        marker.marker( adwplot::PeakMarker::idAxis(id) )->setLinePen( QColor(0xff, 0, 0, 0x80), 0, Qt::DashLine );
    marker.setPeak( pk );
    marker.visible(true);

    plot.setTitle( dataSource + ", " + adportable::utf::to_utf8( data.centroid->getDescriptions().toString() ) );
    plot.setFooter( (boost::format( "FWHM=%.1fmDa (%.2fns)" ) % (pk.widthHH( false ) * 1000) % adcontrols::metric::scale_to_nano( pk.widthHH( true ) )).str() );

    QPainter painter;
    painter.begin( &generator );

    renderer.render( &plot, &painter, rect );
    
    painter.end();

    return true;
}

bool
QuanSvgPlot::plot( const QuanPublisher::resp_data& resp, const QuanPublisher::calib_curve& calib )
{
    QSvgGenerator generator;

    // generator.setFileName( ( boost::format( "C:/Users/Toshi/Documents/data/QUAN/quan-calib_svg_%1%.svg" ) % count++ ).str().c_str() );
    svg_.clear();
    QBuffer buffer( &svg_ );
    generator.setOutputDevice( &buffer );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    QRectF rect( 0, 0, 350, 300 );

    adwplot::Dataplot dplot;
    QuanPlot qplot; // QuanPlot must be declear after adwplot::Dataplot (due to detach order)

    qplot.plot_calib_curve_yx( &dplot, calib );
    qplot.plot_response_marker_yx( &dplot, resp.intensity, resp.amount, std::make_pair(calib.min_x, calib.max_x) );

    QPainter painter;
    painter.begin( &generator );

    renderer.render( &dplot, &painter, rect );
    
    painter.end();

    return true;
}

