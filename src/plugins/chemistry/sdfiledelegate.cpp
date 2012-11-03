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

#include "sdfiledelegate.hpp"
#include "svgitem.hpp"

using namespace chemistry;

SDFileDelegate::SDFileDelegate(QObject *parent) : QItemDelegate(parent)
{
}


QWidget *
SDFileDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	if ( index.column() != 0 )
		return QItemDelegate::createEditor( parent, option, index );
	return 0; // edit not allowed
}

void
SDFileDelegate::paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	if ( index.column() == 0 && qVariantCanConvert< SvgItem >( index.data() ) ) {
		SvgItem svg = qVariantValue< SvgItem >( index.data() );
		svg.paint( painter, option.rect, option.palette );
	} else {
		QItemDelegate::paint( painter, option, index );
	}
}

void
SDFileDelegate::setEditorData( QWidget * editor, const QModelIndex& index ) const
{
	QItemDelegate::setEditorData( editor, index );
}

void
SDFileDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const
{
	QItemDelegate::setModelData( editor, model, index );
}

void
SDFileDelegate::updateEditorGeometry( QWidget * editor, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	QItemDelegate::updateEditorGeometry( editor, option, index );
}

