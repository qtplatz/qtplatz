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

#include "quanplotwidget.hpp"
#include "quandocument.hpp"
#include "quanplotdata.hpp"
#include <adcontrols/descriptions.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/peakmarker.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <qwt_plot_marker.h>
#include <QBoxLayout>
#include <boost/format.hpp>

namespace quan { 
    namespace detail {

        template<typename T > struct widget_get {
            QuanPlotWidget& parent;
            widget_get( QuanPlotWidget& t ) : parent( t ) {
            }

            T* operator ()() {
                if ( auto t = dynamic_cast<T*>(parent.dataplot()) )
                    return t;
                auto pT = new T;
                parent.dataplot( pT );
                if ( auto layout = parent.findChild<QHBoxLayout *>() )
                    layout->addWidget( parent.dataplot() );
                return pT;
            }
        };
    }
}

using namespace quan;

QuanPlotWidget::~QuanPlotWidget()
{
}

QuanPlotWidget::QuanPlotWidget( QWidget * parent ) : QWidget( parent )
                                                   , dplot_( new adwplot::SpectrumWidget )
                                                   , marker_( new adwplot::PeakMarker )
{
    auto layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    
    QuanDocument::instance()->register_dataChanged( [this]( int id, bool f ){ handleDataChanged( id, f ); } );
    layout->addWidget( dplot_.get() );
    marker_->attach( dplot_.get() );

    for ( int id = 0; id < adwplot::PeakMarker::numMarkers; ++id )
        marker_->marker( adwplot::PeakMarker::idAxis(id) )->setLinePen( QColor(0xff, 0, 0, 0x80), 0, Qt::DashLine );

    marker_->visible( true );
}

void
QuanPlotWidget::handleDataChanged( int id, bool )
{
    //auto layout = findChild< QHBoxLayout * >();

    if ( id == idQuanMethod ) {
        auto& method = QuanDocument::instance()->quanMethod();
        (void)method;
    }
}

void
QuanPlotWidget::setData( const QuanPlotData * d, size_t idx, int fcn, const std::wstring& dataSource )
{
    if ( auto spw = detail::widget_get< adwplot::SpectrumWidget >( *this )() ) {

        spw->enableAxis( QwtPlot::yRight );

        if ( d->profile->protocolId() == fcn ) {

            spw->setTitle( dataSource + L", " + d->centroid->getDescriptions().toString() );

            spw->setData( d->profile, 0 );
            spw->setData( d->centroid, 1, true );

            double mass = d->centroid->getMass( idx );
            QRectF rc = spw->zoomer().zoomRect();
            rc.setLeft( mass - 2 );
            rc.setRight( mass + 2 );
            spw->zoomer().zoom( rc );

            auto item = d->pkinfo->begin() + idx;
            marker_->setPeak( *item );
            marker_->visible( true );

            spw->setFooter( (boost::format( "FWHM=%.1fmDa (%.2fns)" ) % (item->widthHH( false ) * 1000) % adcontrols::metric::scale_to_nano( item->widthHH( true ) )).str() );

        }
    }
}

