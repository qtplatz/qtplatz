/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "moltabledelegate.hpp"
#include <QPainter>
#include <QApplication>
#include <QMenu>
#include <QComboBox>
#include <QSvgRenderer>
#include <QByteArray>
#include <QDebug>

using namespace chemistry;

MolTableDelegate::MolTableDelegate(QObject *parent) :  QItemDelegate(parent)
{
}

void
MolTableDelegate::paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	if ( index.column() == 1 ) {
        render_svg( painter, option.rect, option.palette, index.data().toByteArray() );
    } else {
		QItemDelegate::paint( painter, option, index );
    }
}

void
MolTableDelegate::render_svg( QPainter * painter, const QRect& rect, const QPalette&, const QByteArray& svg ) const
{
    painter->save();

	QSvgRenderer renderer( svg );

	painter->translate( rect.x(), rect.y() );
	QRectF viewport = painter->viewport();
    painter->scale( 1.0, 1.0 );

	QRect target( 0, 0, rect.width(), rect.height() );
	renderer.render( painter, target );

    painter->restore();
}

