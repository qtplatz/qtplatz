// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "annotations.hpp"
#include "annotation.hpp"
#include "dataplot.hpp"
#include <qwt_plot_marker.h>
#include <QFontMetrics>


using namespace adwplot;

Annotations::Annotations( Dataplot& plot, vector_type& vec ) : vec_(vec), plot_(plot)
{
}

Annotations::Annotations( const Annotations& t ) : vec_(t.vec_), plot_( t.plot_ )
{
}

void
Annotations::clear()
{
    return vec_.clear();
}

Annotation&
Annotations::add( double x, double y, const std::wstring& text )
{
    QwtText label( QString::fromStdWString( text ), QwtText::RichText );
    label.setFont( Annotation::font() );
    label.setColor( Qt::darkGreen );
    vec_.push_back( Annotation( plot_, label, QPointF( x, y ) ) );
    return vec_.back();
}

bool
Annotations::interference( double x, double y, const QwtText& label, Qt::Alignment align ) const
{
	QRectF rc = boundingRect( x, y, label, align );

	auto it = std::find_if( vec_.begin(), vec_.end(), [=]( const Annotation& a ){
            bool res = rc.intersects( this->boundingRect( a, align ) );
            return res;
        });

	if ( it == vec_.end() )
		return false;

	return true;
}

bool
Annotations::insert( double x, double y, const QwtText& label, Qt::Alignment align )
{
	if ( interference( x, y, label, align ) )
		return false;

    vec_.push_back( Annotation( plot_, label, QPointF( x, y ), align ) );

	return true;
}

void
Annotations::adjust( QRectF& rc, Qt::Alignment align ) const
{
    int xoffs(0), yoffs(0);
    
    if ( align & Qt::AlignLeft ) {
        xoffs = 0;
    } else if ( align & Qt::AlignHCenter ) {
        xoffs = -rc.width() / 2;
    } else if ( align & Qt::AlignRight ) {
        xoffs = -rc.width();
    }
    if ( align & Qt::AlignTop ) {
        yoffs = 0;
    } else if ( align & Qt::AlignVCenter ) {
        yoffs = -rc.height() / 2;
    } else if ( align & Qt::AlignBottom ) {
        yoffs = -rc.height();
    }
    rc.moveTo( rc.x() + xoffs, rc.y() + yoffs );
	rc.adjust( 1, 1, 1, 1 );
}

QRectF
Annotations::boundingRect( const Annotation& a, Qt::Alignment align ) const
{
    QwtPlotMarker * marker = a.getPlotMarker();
    return boundingRect( marker->xValue(), marker->yValue(), marker->label(), align );
}

QRectF
Annotations::boundingRect( double x, double y, const QwtText& label, Qt::Alignment align ) const
{
	QSizeF sz = label.textSize();
    
	const QwtScaleMap xMap = plot_.canvasMap( QwtPlot::xBottom );
	const QwtScaleMap yMap = plot_.canvasMap( QwtPlot::yLeft );
#if 0
	double p1 = xMap.p1(); // device left
	double p2 = xMap.p2(); // device right
	double s1 = xMap.s1(); // axis left
	double s2 = xMap.s2(); // axis right
#endif	
	QPointF pt( QwtScaleMap::transform( xMap, yMap, QPointF( x, y ) ) );
	QRectF rc( QwtScaleMap::transform( xMap, yMap, QRectF( pt, sz ) ) );

    adjust( rc, align );

    return rc;
}

