/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "svgitem.hpp"
#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qsvgrenderer.h>
#include <QtCore/QDebug>
#include <algorithm>

using namespace chemistry;

SvgItem::SvgItem()
{
}

SvgItem::SvgItem( const SvgItem& t ) : svg_( t.svg_ )
{
}

template<class T> void debug_draw_rect( QPainter * painter, const T& rect, const QBrush& brush )
{
	painter->setPen( Qt::SolidLine );
	painter->setBrush( brush );
	painter->drawRect( rect );
}


void
SvgItem::paint( QPainter * painter, const QRect& rect, const QPalette& palette ) const 
{
	painter->save();

	// painter->setRenderHint( QPainter::Antialiasing, true );
	QSvgRenderer renderer( svg_ );
	QRectF source = renderer.boundsOnElement( "topsvg" );
	QSize sz = renderer.defaultSize();
	double factor = sz.width() / renderer.viewBox().width();
	painter->translate( rect.x(), rect.y() );
    
	double sf = std::min( double( rect.width() ) / source.width(), double( rect.height() ) / source.height() );
	painter->scale( sf / factor, sf / factor );

	QRect target( 0, 0, rect.width(), rect.height() );
	// painter->setViewport( target );
	// debug_draw_rect( painter, source, palette.foreground() );

	renderer.setViewBox( source );

	renderer.render( painter, target );

	painter->restore();
}
