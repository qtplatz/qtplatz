// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "peak.hpp"
#include "plot.hpp"
#include <qwt_plot_curve.h>
#include <adcontrols/peak.hpp>
#include <adcontrols/baseline.hpp>
#include <QPen>

using namespace adplot;

Peak::Peak( Peak&& t ) : plot_( t.plot_ ), curve_( std::move( t.curve_ ) )
{
}

Peak::Peak( plot& plot
            , const adcontrols::Peak& peak
            , const adcontrols::Baseline& bs ) : plot_( &plot )
                                               , curve_( std::make_unique< QwtPlotCurve >() )
{
    QColor color( 0x7f, 0, 0, 0x60 );
    curve_->setPen( QPen( color ) );
    curve_->setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    curve_->setItemAttribute( QwtPlotItem::Legend, false );
    curve_->attach( plot_ );

    double x[2], y[2];

    //x [ 0 ] = peak.startTime();
    x [ 0 ] = peak.endTime();
    x [ 1 ] = peak.endTime();

    y [ 0 ] = bs.height( peak.endPos() );
    y [ 1 ] = peak.endHeight();

    // force horizontal
    // y [ 0 ] = y [ 1 ] = std::min( peak.startHeight(), peak.endHeight() ); // - ( rc.height() / 20 ); // <-- 5% offsset

    curve_->setSamples(  x, y, 2 );
}
