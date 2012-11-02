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
SvgItem::paint( QPainter * painter, const QRect& rect, const QPalette& /* palette */ ) const 
{
	painter->save();

	QSvgRenderer renderer( svg_ );
	// QRectF source = renderer.boundsOnElement( "topsvg" );

	painter->translate( rect.x(), rect.y() );
	QRectF viewport = painter->viewport();
	painter->scale( 100 / viewport.width(), 100 / viewport.height() );  // aspect 1:2 := 100x100 vbox

	QRect target( 0, 0, rect.width(), rect.height() );
	// renderer.setViewBox( target ); // <-- this will fix size on device
	renderer.render( painter, target );

	painter->restore();
}
