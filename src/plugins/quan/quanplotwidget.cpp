/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanplotwidget.hpp"
#include "quandocument.hpp"
#include "quanplotdata.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/quanmethod.hpp>

#include <adplot/chromatogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/zoomer.hpp>
#include <adportable/debug.hpp>

#include <qwt_plot_marker.h>
#include <QBoxLayout>
#include <boost/format.hpp>

using namespace quan;

QuanPlotWidget::~QuanPlotWidget()
{
}

QuanPlotWidget::QuanPlotWidget( QWidget * parent, bool isChromatogram ) : QWidget( parent )
                                                                        , isChromatogram_( isChromatogram )
                                                                        , marker_( new adplot::PeakMarker )
{
    auto layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    if ( isChromatogram_ )
        dplot_.reset( new adplot::ChromatogramWidget );
    else
        dplot_.reset( new adplot::SpectrumWidget );
    
    QuanDocument::instance()->connectDataChanged( [this]( int id, bool f ){ handleDataChanged( id, f ); } );

    layout->addWidget( dplot_.get() );

    marker_->attach( dplot_.get() );

    for ( int id = 0; id < adplot::PeakMarker::numMarkers; ++id )
        marker_->marker( adplot::PeakMarker::idAxis(id) )->setLinePen( QColor(0xff, 0, 0, 0x80), 0, Qt::DashLine );

    marker_->visible( true );
}

void
QuanPlotWidget::handleDataChanged( int id, bool )
{
    if ( id == idQuanMethod ) {
        auto& method = QuanDocument::instance()->quanMethod();
        (void)method;
    }
}

void
QuanPlotWidget::setData( const QuanPlotData * d, size_t idx, int fcn, const std::wstring& dataSource )
{
    if ( d->profile ) {
        setSpectrum( d, idx, fcn, dataSource );
    } else if ( d->chromatogram ) {
        setChromatogram( d, idx, fcn, dataSource );        
    }
}

void
QuanPlotWidget::setSpectrum( const QuanPlotData * d, size_t idx, int fcn, const std::wstring& dataSource )
{
    if ( auto spw = dynamic_cast<adplot::SpectrumWidget *>( dplot_.get() ) ) {
        
        spw->enableAxis( QwtPlot::yRight );
        
        spw->setTitle( dataSource + L", " + d->centroid->getDescriptions().toString() );
        
        if ( d->filterd ) {
            spw->setData( d->filterd, 0, true );
            spw->setData( d->profile, 2, true );
            spw->setData( d->centroid, 1, false );
        } else {
            spw->setData( d->profile, 0, true );
            spw->setData( d->centroid, 1, false );
        }

        auto pkinfo = d->pkinfo->findProtocol( fcn );

        if ( pkinfo && pkinfo->size() > idx ) {
            auto item = pkinfo->begin() + idx;

            double mass = item->mass();
            QRectF rc = spw->zoomer()->zoomRect();

            double width = pkinfo->size() > idx ? item->widthHH() * 5 : 1.0;
            
            rc.setLeft( mass - width );
            rc.setRight( mass + width );
            spw->zoomer()->zoom( rc );

            marker_->setYAxis( QwtPlot::yRight );
            marker_->setPeak( *item );
            marker_->visible( true );
            
            spw->setFooter( ( boost::format( "W=%.2fmDa (%.2fns)" )
                              % ( item->widthHH( false ) * 1000 )
                              % adcontrols::metric::scale_to_nano( item->widthHH( true ) ) ).str() );
        }
    }
}

void
QuanPlotWidget::setChromatogram( const QuanPlotData * d, size_t idx, int fcn, const std::wstring& dataSource )
{
    if ( auto pw = dynamic_cast<adplot::ChromatogramWidget *>( dplot_.get() ) ) {

        pw->setTitle( dataSource + L", " + d->chromatogram->getDescriptions().toString() );
        pw->setData( d->chromatogram, 0, true );

        if ( d->pkResult ) {
            pw->setData( *d->pkResult );
            
            if ( idx < d->pkResult->peaks().size() ) {
                auto item = d->pkResult->peaks().begin() + idx;
                marker_->setPeak( *item );
                marker_->visible( true );
                pw->drawPeakParameter( *item );
            }

        }
        
    }

}
