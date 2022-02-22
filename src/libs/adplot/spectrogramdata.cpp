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

#include "spectrogramdata.hpp"
#include <QRectF>

using namespace adplot;

SpectrogramData::~SpectrogramData()
{
}

SpectrogramData::SpectrogramData() : interval_{
        QwtInterval( 0, 100.0, QwtInterval::ExcludeMaximum )
        , QwtInterval( 0, 1000.0, QwtInterval::ExcludeMaximum )
        , QwtInterval( 0.0, 100.0 ) }
{
    // setInterval( Qt::XAxis, QwtInterval( 0, 100.0, QwtInterval::ExcludeMaximum ) );   // time
    // setInterval( Qt::YAxis, QwtInterval( 0, 1000.0, QwtInterval::ExcludeMaximum ) ); // m/z
    // setInterval( Qt::ZAxis, QwtInterval( 0.0, 100.0 ) );
}

double
SpectrogramData::value( double x, double y ) const
{
    return 0; //x < 1.0 ? 0.0 : x - 1.0;
}

QRectF
SpectrogramData::boundingRect() const
{
    return QRectF( 0.0, 0.0, 10.0, 1000.0 ); // x, y, w, h
}

QwtInterval
SpectrogramData::interval( Qt::Axis axis ) const
{
    switch ( axis ) {
    case Qt::XAxis: return std::get< 0 >( interval_ );
    case Qt::YAxis: return std::get< 1 >( interval_ );
    case Qt::ZAxis: return std::get< 2 >( interval_ );
    }
    return {};
}

void
SpectrogramData::setInterval( Qt::Axis axis, QwtInterval && interval )
{
    switch ( axis ) {
    case Qt::XAxis: std::get< 0 >( interval_ ) = std::move( interval ); break;
    case Qt::YAxis: std::get< 1 >( interval_ ) = std::move( interval ); break;
    case Qt::ZAxis: std::get< 2 >( interval_ ) = std::move( interval ); break;
    }
}
