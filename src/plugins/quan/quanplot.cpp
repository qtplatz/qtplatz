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

#include "quanplot.hpp"
#include <adportable/float.hpp>
#include <adportable/polfit.hpp>
#include <adplot/plot.hpp>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <sstream>
#include <iomanip>

using namespace quan;

QuanPlot::QuanPlot() : curves_( curves_holder_ )
                     , markers_( markers_holder_ )
{
}

QuanPlot::QuanPlot( std::vector< std::shared_ptr< QwtPlotCurve > > & curves
                    , std::vector< std::shared_ptr< QwtPlotMarker > >& markers ) : curves_( curves )
                                                                                 , markers_( markers )
{
}

void
QuanPlot::plot_response_marker_yx( adplot::plot* plot, double intensity, double amount, const std::pair<double,double>& xrange )
{
    markers_.clear();

    auto marker = std::make_shared< QwtPlotMarker >();
    markers_.push_back( marker );
    marker->attach ( plot );

    QColor color( Qt::darkGreen );
    double lwidth = 0.0;
    if ( intensity < xrange.first || xrange.second < intensity ) {
        color = Qt::red;
        lwidth = 2.0;
    }
    
    marker->setValue( amount, intensity );
    marker->setLinePen( color, lwidth, Qt::DashLine );

    if ( adportable::compare<double>::approximatelyEqual( 0.0, amount ) ) // std has no amout value
        marker->setLineStyle( QwtPlotMarker::HLine );
    else
        marker->setLineStyle( QwtPlotMarker::Cross );

    plot->replot();
}


void
<<<<<<< HEAD
QuanPlot::plot_calib_curve_yx( adwplot::Dataplot* plot
=======
QuanPlot::plot_calib_curve_yx( adplot::plot* plot
>>>>>>> origin/v3.1.3
                             , const QuanPublisher::calib_curve& calib )
{
    plot->setAxisTitle( QwtPlot::xBottom, tr( "amounts" ) );
    plot->setAxisTitle( QwtPlot::yLeft, tr( "response" ) );
    plot->setTitle( QString( tr( "Calibration curve for %1" ) ).arg( QString::fromStdWString( calib.description ) ) );

    std::ostringstream o;
    o << tr( "Amounts = " ).toStdString();
    if ( calib.coeffs.size() == 1 )
        o << std::setprecision( 6 ) << calib.coeffs[ 0 ] << "&times;I";
    else if ( calib.coeffs.size() >= 2 ) { 
        o << std::setprecision( 6 ) << calib.coeffs[ 0 ] << "+" << calib.coeffs[ 1 ] << "&times;<i>I</i>";
        for ( int i = 2; i < calib.coeffs.size(); ++i )
            o << std::setprecision( 6 ) << "+" << calib.coeffs[ i ] << "&times;I<sup>" << i << "</sup>";
    }
    o << tr( "&nbsp;&nbsp;where I is the response" ).toStdString();
    
    plot->setFooter( o.str() );

    curves_.clear();
    curves_.push_back( std::make_shared< QwtPlotCurve >() );
    auto curve = curves_.back();

    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
	QPen pen( Qt::red );
	curve->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Ellipse ), Qt::NoBrush, pen, QSize(5, 5) ) );
	curve->setPen( pen );
    curve->setStyle( QwtPlotCurve::NoCurve );
    do {
        QVector< QPointF > yx;
        for ( auto& xy: calib.xy )
            yx.push_back( QPointF( xy.second, xy.first ) );
        curve->setSamples( yx ); 
    } while(0);

    curve->attach( plot );

    ///////////////// plot regression ///////////////
    curves_.push_back( std::make_shared< QwtPlotCurve >() );
    auto regression = curves_.back();

    QVector< QPointF > yx;
    double max_x = calib.max_x * 1.2;

    if ( calib.coeffs.size() == 1 ) {
        yx.push_back( QPointF( 0, 0 ) );
        yx.push_back( QPointF( max_x * calib.coeffs[ 0 ], max_x ) );
    }
    else if (calib.coeffs.size() == 2) {
        double y0 = adportable::polfit::estimate_y( calib.coeffs, 0 );
        double y1 = adportable::polfit::estimate_y( calib.coeffs, max_x );
        yx.push_back( QPointF( y0, 0 ) );
        yx.push_back( QPointF( y1, max_x ) );
    }
    else if ( calib.coeffs.size() >= 3 ) {
        for ( int i = 0; i < 100; ++i ) {
            double x = (max_x * i / 100);
            double y = adportable::polfit::estimate_y( calib.coeffs, x );
            yx.push_back( QPointF( y, x ));
        }
    }
    regression->setSamples( yx );
    regression->attach( plot );
    
    plot->replot();
}
