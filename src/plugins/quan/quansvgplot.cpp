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
#include <adportable/debug.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/chromatogramwidget.hpp>
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
    static const std::pair< double, double > range( 0, 0 );
    if ( data.chromatogram )
        return plot_chromatogram( data, idx, fcn, dataSource );
    else
        return plot_spectrum( data, idx, fcn, dataSource, range );
    return false;
}

bool
QuanSvgPlot::plot( const QuanPlotData& data, size_t idx, int fcn, const std::string& dataSource, const std::pair<double,double>& range )
{
    if ( data.chromatogram )
        return plot_chromatogram( data, idx, fcn, dataSource );
    else
        return plot_spectrum( data, idx, fcn, dataSource, range );
    return false;
}

bool
QuanSvgPlot::plot_spectrum( const QuanPlotData& data
                            , size_t idx
                            , int fcn
                            , const std::string& dataSource
                            , const std::pair< double, double >& range )
{
    if ( ! data.profile )
        return false;

    auto tCentroid( std::make_shared< adcontrols::MassSpectrum >() );
    auto tProfile( std::make_shared< adcontrols::MassSpectrum >() );
    std::shared_ptr< adcontrols::MassSpectrum > tProfiledHist;

    adcontrols::MSPeakInfoItem pk;

    if ( ! data.pkinfo )
        return false;
    if ( ! data.profile )
        return false;
    
    if ( data.pkinfo ) {
        
        if ( auto pkinfo = data.pkinfo.get()->findProtocol( fcn ) ) {

            auto pkIt = data.pkinfo.get()->size() >= idx ? data.pkinfo.get()->begin() + idx : pkinfo->end();
            
            if ( pkIt == data.pkinfo.get()->end() )
                return false;

            pk = *pkIt;
            double mass = pkIt->mass();

            if ( range.first < mass && mass < range.second ) {
                if ( auto profile = data.profile.get()->findProtocol( fcn ) ) {
                    if ( !profile->trim( *tProfile, range ) ) {
                        *tProfile = *(data.profile.get());
                    }
                } else {
                    adcontrols::segment_wrapper<> vec( *(data.profile.get()) );
                    ADDEBUG() << "################## no data found for proto# " << fcn << "#################### " << vec.size();
                    for ( auto& ms: vec ) {
                        ADDEBUG() << "proto = " << ms.protocolId();
                    }
                    *tProfile = *(data.profile.get());
                }
                
                if ( data.profiledHist ) {
                    tProfiledHist = std::make_shared< adcontrols::MassSpectrum >();
                    data.profiledHist.get()->trim( *tProfiledHist, range );
                }

                if ( data.centroid ) {
                    if ( auto centroid = data.centroid.get()->findProtocol( fcn ) ) {
                        if ( !centroid->trim( *tCentroid, range ) )
                            *tCentroid = *data.centroid.get();
                    }
                } else {
                    if ( data.centroid ) {
                        ADDEBUG() << "################## no data found for proto# " << fcn << "#################### "
                                  << adcontrols::segment_wrapper<> (*data.centroid.get() ).size();
                        *tCentroid = *data.centroid.get();
                    }
                }
            }
        }
    }

    QSvgGenerator generator;

    svg_.clear();
    QBuffer buffer( &svg_ );
    generator.setOutputDevice( &buffer );
    generator.setTitle( "QtPlatz Generated SVG" );
    generator.setDescription( "Copyright (C) 2010-2017 MS-Cheminformataics, All rights reserved" );

    QRectF rect( 0, 0, 350, 300 );
    generator.setViewBox( rect );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    adplot::SpectrumWidget plot;
    plot.setData( tProfile, 0, false );
    if ( tProfiledHist )
        plot.setData( tProfiledHist, 2, false );
    plot.setData( tCentroid, 1, true );

    plot.setZoomBase( range, true );
    adplot::PeakMarker marker;

    marker.attach( &plot );
    // set color etc.
    for ( int id = 0; id < adplot::PeakMarker::numMarkers; ++id )
        marker.marker( adplot::PeakMarker::idAxis(id) )->setLinePen( QColor(0xff, 0, 0, 0x80), 0, Qt::DashLine );

    marker.setPeak( pk );
    marker.visible(true);

    if ( data.centroid )
        plot.setTitle( dataSource + ", " + adportable::utf::to_utf8( data.centroid.get()->getDescriptions().toString() ) );
    plot.setFooter( (boost::format( "FWHM=%.1fmDa (%.2fns)" ) % (pk.widthHH( false ) * 1000) % adcontrols::metric::scale_to_nano( pk.widthHH( true ) )).str() );

    QPainter painter;
    painter.begin( &generator );

    renderer.render( &plot, &painter, rect );
    
    painter.end();

    return true;
}

bool
QuanSvgPlot::plot_chromatogram( const QuanPlotData& data, size_t idx, int fcn, const std::string& dataSource )
{
    if ( ! data.chromatogram )
        return false;
    
    QSvgGenerator generator;

    svg_.clear();
    QBuffer buffer( &svg_ );
    generator.setOutputDevice( &buffer );
    generator.setTitle( "QtPlatz Generated SVG" );
    generator.setDescription( "Copyright (C) 2013-2015 MS-Cheminformataics, All rights reserved" );

    QRectF rect( 0, 0, 350, 300 );
    generator.setViewBox( rect );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    adplot::ChromatogramWidget plot;
    if ( data.chromatogram )
        plot.setData( data.chromatogram.get(), 0 );
    if ( data.pkResult )
        plot.setData( *data.pkResult.get() );

    adplot::PeakMarker marker;
    if ( data.pkResult ) {
        if ( idx < data.pkResult.get()->peaks().size() ) {
            
            // set color etc.
            for ( int id = 0; id < adplot::PeakMarker::numMarkers; ++id )
                marker.marker( adplot::PeakMarker::idAxis(id) )->setLinePen( QColor(0xff, 0, 0, 0x80), 0, Qt::DashLine );
        
            auto item = data.pkResult.get()->peaks().begin() + idx;
            marker.setPeak( *item );
            plot.drawPeakParameter( *item );

            marker.attach( &plot );
            marker.visible(true);
        }
    }

    if ( data.chromatogram )
        plot.setTitle( dataSource + ", " + adportable::utf::to_utf8( data.chromatogram.get()->getDescriptions().toString() ) );
    //plot.setFooter( (boost::format( "FWHM=%.1fmDa (%.2fns)" ) % (pk.widthHH( false ) * 1000) % adcontrols::metric::scale_to_nano( pk.widthHH( true ) )).str() );

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

    svg_.clear();
    QBuffer buffer( &svg_ );
    generator.setOutputDevice( &buffer );
    generator.setTitle( "QtPlatz Generated SVG" );
    generator.setDescription( "Copyright (C) 2014 MS-Cheminformataics, All rights reserved" );
    QRectF rect( 0, 0, 350, 300 );
    generator.setViewBox( rect );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    adplot::plot dplot;
    QuanPlot qplot; // QuanPlot must be declear after adplot::Dataplot (due to detach order)

    qplot.plot_calib_curve_yx( &dplot, calib );
    qplot.plot_response_marker_yx( &dplot, resp.intensity, resp.amount, std::make_pair(calib.min_x, calib.max_x) );

    QPainter painter;
    painter.begin( &generator );

    renderer.render( &dplot, &painter, rect );
    
    painter.end();

    return true;
}

