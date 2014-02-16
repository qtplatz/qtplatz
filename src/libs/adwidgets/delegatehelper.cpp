/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "delegatehelper.hpp"
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QString>
#include <QTextDocument>
#include <QSize>

using namespace adwidgets;

DelegateHelper::DelegateHelper()
{
}

void
DelegateHelper::render_html( QPainter * painter, const QStyleOptionViewItem& option, const QString& text )
{
    painter->save();
    QStyleOptionViewItemV4 op = option;
    QTextDocument document;
    document.setDefaultTextOption( QTextOption( op.displayAlignment ) );
    document.setDefaultFont( op.font );
    document.setHtml( text );
    op.displayAlignment = Qt::AlignVCenter;
    op.widget->style()->drawControl( QStyle::CE_ItemViewItem, &op, painter );
    painter->translate( op.rect.topLeft() );
    QRect clip( 0, 0, op.rect.width(), op.rect.height() );
    document.drawContents( painter, clip );
    painter->restore();
}

//static
QSize
DelegateHelper::html_size_hint( const QStyleOptionViewItem& option, const QModelIndex& index )
{
    QTextDocument document;
	document.setDefaultTextOption( QTextOption( option.displayAlignment ) );
	document.setDefaultFont( option.font );
    document.setHtml( index.data().toString() );
    return QSize( document.size().width(), document.size().height() );  
}
