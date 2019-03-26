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

#include "baseline.hpp"
#include "plot.hpp"
#include <qwt_plot_curve.h>
#include <adcontrols/baseline.hpp>

using namespace adplot;

Baseline::Baseline( Baseline&& t ) : plot_( t.plot_ ), curve_( std::move( t.curve_ ) )
{
}

Baseline::Baseline( plot& plot, const adcontrols::Baseline& bs ) : plot_( &plot )
                                                                 , curve_( std::make_unique< QwtPlotCurve >() )
{
    QColor color( 0xcf, 0x00, 0, 0x60 );
	curve_->setPen( QPen( color ) );
    curve_->setItemAttribute( QwtPlotItem::Legend, false );
    curve_->setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    curve_->attach( plot_ );

    double x[2], y[2];
    x [ 0 ] = bs.startTime(); // adcontrols::timeutil::toMinutes( bs.startTime() );
    x [ 1 ] = bs.stopTime(); // adcontrols::timeutil::toMinutes( bs.stopTime() );
    y [ 0 ] = bs.startHeight();
    y [ 1 ] = bs.stopHeight();
    curve_->setSamples(  x, y, 2 );
}
