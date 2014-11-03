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

#pragma once

class QPainter;
class QStyleOptionViewItem;
class QString;
class QModelIndex;
#include <QSize>
#include <QHeaderView>
#include <QPainter>
#include <QTextDocument>
#include "adwidgets_global.hpp"

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT HtmlHeaderView : public QHeaderView {
    public:
        HtmlHeaderView(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = 0) : QHeaderView( orientation, parent ) {
            setSectionsClickable( true );
        }
            
        void paintSection( QPainter * painter, const QRect& rect, int logicalIndex ) const override {
            
            if ( rect.isValid() ) {
                if ( logicalIndex > 0 ) {
                    QStyleOptionHeader op;
                    initStyleOption(&op);
                    op.text = "";
                    op.rect = rect;
                    op.textAlignment = Qt::AlignVCenter | Qt::AlignHCenter;
                    // draw the section
                    style()->drawControl( QStyle::CE_Header, &op, painter, this );
                    // html painting
                    painter->save();
                    QRect textRect = style()->subElementRect( QStyle::SE_HeaderLabel, &op, this );
                    painter->translate( textRect.topLeft() );
                    QTextDocument doc;
                    doc.setTextWidth( textRect.width() );
                    doc.setDefaultTextOption( QTextOption( Qt::AlignHCenter ) );
                    doc.setDocumentMargin(0);
                    doc.setHtml( model()->headerData( logicalIndex, Qt::Horizontal ).toString() );
                    doc.drawContents( painter, QRect( QPoint( 0, 0 ), textRect.size() ) );
                    painter->restore();
                } else {
                    QHeaderView::paintSection( painter, rect, logicalIndex );
                }
            }
        }

    };

}


