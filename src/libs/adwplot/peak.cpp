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
#include "dataplot.hpp"
#include <qwt_plot_curve.h>
#include <adcontrols/peak.hpp>

using namespace adwplot;

Peak::Peak( const Peak& t ) : curve_( t.curve_ )
                            , bar_( t.bar_ )
{
}

Peak::Peak( Dataplot& plot, const adcontrols::Peak& peak ) : plot_( &plot )
                                                           , curve_( new QwtPlotCurve() ) 
														   , bar_( new QwtPlotCurve() ) 
{
    curve_->setPen( QPen( Qt::red) );
    curve_->setStyle( QwtPlotCurve::Lines ); // continuum (or Stics)
    curve_->attach( plot_ );

    double x[2], y[2];
/*
	x[0] = adcontrols::timeutil::toMinutes( peak.startTime() );
    x[1] = adcontrols::timeutil::toMinutes( peak.endTime() );
    y[0] = peak.startHeight();
    y[1] = peak.endHeight();
*/
	x[0] = x[1] = adcontrols::timeutil::toMinutes( peak.peakTime() );
    y[0] = 0; 
	y[1] = peak.topHeight();
    
    curve_->setSamples(  x, y, 2 );
}
