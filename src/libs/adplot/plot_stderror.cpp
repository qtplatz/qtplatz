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

#include "plot_stderror.hpp"
#include <adportable/float.hpp>
#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <tuple>

using namespace adplot;

plot_stderror::plot_stderror()
{
}

void
plot_stderror::title( const std::string& title )
{
    title_ = title;
}

void
plot_stderror::clear()
{
    curves_.clear();
    markers_.clear();
}

void
plot_stderror::operator()( const QVector< QPointF >& data, QwtPlot& plot )
{
    clear();

    const QPointF& dmax = *std::max_element( data.begin(), data.end(), []( const QPointF& a, const QPointF& b ){
            return std::abs(a.y()) < std::abs(b.y()); } );
    double dfs = std::abs(dmax.y()) * 1.5;
    plot.setAxisScale( QwtPlot::yRight, -dfs, dfs );
    
    std::vector< std::tuple< int, double, double > > bars;
    for ( auto& datum: data ) {
        std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >();
        markers_.push_back( marker );
        marker->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Cross ), Qt::NoBrush, QPen( QColor(0x00, 0x00, 0x20, 0x40) ), QSize(7, 7) ) );
        marker->setValue( datum.x(), datum.y() );
        marker->setYAxis( QwtPlot::yRight );
        marker->attach( &plot );
        auto it = std::find_if( bars.begin(), bars.end(), [=]( const std::tuple<int, double, double>& d ){
                return adportable::compare<double>::essentiallyEqual( std::get<1>(d), datum.x() );
            });
        if ( it == bars.end() )
            bars.push_back( std::make_tuple( 1, datum.x(), datum.y() ) );
        else {
            std::get<0>( *it )++; // N
            std::get<2>( *it ) += datum.y(); // sum(y) over x
        }
    }
    QVector< QPointF > bar;
    for ( auto& t: bars ) {
        if ( std::get<0>(t) > 0 )
            bar.push_back( QPointF( std::get<1>(t), std::get<2>(t) / std::get<0>(t) ) ); // x, avg(y)
    }
    
    std::shared_ptr< QwtPlotCurve > curve = std::make_shared< QwtPlotCurve >( QwtText(title_.c_str(), QwtText::RichText) );
    curves_.push_back( curve );
    curve->setPen( QColor( 0x00, 0x00, 0x20, 0x40 ), 3.0 );
    curve->setYAxis( QwtPlot::yRight );
    curve->setSamples( bar );
    curve->setStyle( QwtPlotCurve::Sticks );
    curve->attach( &plot );
    
    // horizontal center line for deviation plot
    std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >();
    markers_.push_back( marker );
    marker->setYAxis( QwtPlot::yRight );
    marker->setValue( 0.0, 0.0 );
    marker->setLineStyle( QwtPlotMarker::HLine );
    marker->setLinePen( Qt::gray, 0.0, Qt::DashDotLine );
    marker->attach( &plot );
}
