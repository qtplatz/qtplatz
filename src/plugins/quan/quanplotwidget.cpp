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
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>


#include <QBoxLayout>

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
{
    auto layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    
    QuanDocument::instance()->register_dataChanged( [this]( int id, bool f ){ handleDataChanged( id, f ); } );
    layout->addWidget( dplot_.get() );
}

void
QuanPlotWidget::handleDataChanged( int id, bool )
{
    auto layout = findChild< QHBoxLayout * >();

    if ( id == idQuanMethod ) {
        auto& method = QuanDocument::instance()->quanMethod();
        if ( method.isChromatogram() ) {
            if ( dynamic_cast< adwplot::SpectrumWidget * >( dplot_.get() ) ) {
                // replace to chromatogram
                dplot_.reset( new adwplot::ChromatogramWidget );
                layout->addWidget( dplot_.get() );
            }
        } else {
            if ( dynamic_cast< adwplot::ChromatogramWidget * >( dplot_.get() ) ) {
                // replace to spectrum
                dplot_.reset( new adwplot::SpectrumWidget );
                layout->addWidget( dplot_.get() );
            }
        }
    }
}

void
QuanPlotWidget::setData( const QuanPlotData * d, size_t idx, int fcn )
{
    if ( auto spw = detail::widget_get< adwplot::SpectrumWidget >( *this )() ) {

        spw->enableAxis( QwtPlot::yRight );

        if ( d->profile->protocolId() == fcn ) {

            spw->setData( d->profile, 0 );
            spw->setData( d->centroid, 1, true );

            double mass = d->centroid->getMass( idx );
            QRectF rc = spw->zoomer().zoomRect();
            rc.setLeft( mass - 2 );
            rc.setRight( mass + 2 );
            spw->zoomer().zoom( rc );

        }
    }
}

