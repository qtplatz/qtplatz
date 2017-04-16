/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "rangemarker.hpp"
#include <qwt_scale_map.h>

using namespace adplot;

RangeMarker::RangeMarker() : color_( 128, 0, 0, 0x80 )
                           , range_( 0.0, 100.0 )
{
}

RangeMarker::RangeMarker( const QColor& color ) : color_( color )
                                                , range_( 0.0, 100.0 )
{
}

RangeMarker::~RangeMarker()
{
}

void
RangeMarker::setValue( double lower_value, double upper_value )
{
    range_ = std::make_pair( lower_value, upper_value );
}

void
RangeMarker::setValue( fence which, double value )
{
    if ( which == lower )
        range_.first = value;
    else
        range_.second = value;
}

void
RangeMarker::setColor( const QColor& color )
{
    color_ = color;
}

const QColor&
RangeMarker::color() const
{
    return color_;
}

void
RangeMarker::draw( QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &, const QRectF &canvasRect ) const
{
    int x1 = qRound( xMap.transform( range_.first ) );
    int x2 = qRound( xMap.transform( range_.second ) );

    painter->fillRect( QRect( x1, canvasRect.top(), x2 - x1, canvasRect.height() ), color_ );
};

