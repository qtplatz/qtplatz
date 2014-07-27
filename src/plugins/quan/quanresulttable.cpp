/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "quanresulttable.hpp"
#include "quanquery.hpp"
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QVariant>

namespace quan {
    namespace quanresulttable {
        
        class ItemDelegate : public QStyledItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );
                //if ( index.data().type() == QVariant::String )
                op.textElideMode = Qt::ElideLeft;
                QStyledItemDelegate::paint( painter, op, index );
            }
            void setEditorData( QWidget * editor, const QModelIndex& index ) const {
                QStyledItemDelegate::setEditorData( editor, index );
            }

            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const {
                QStyledItemDelegate::setModelData( editor, model, index );
            }
            
            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                return QStyledItemDelegate::sizeHint( option, index );
            }
        
        };
    }
}

using namespace quan;

QuanResultTable::QuanResultTable(QWidget *parent) : adwidgets::TableView(parent)
                                                  , model_( new QStandardItemModel )
{
    setModel( model_.get() );
    setItemDelegate( new quanresulttable::ItemDelegate );
}

void
QuanResultTable::prepare( const QuanQuery& q )
{
    model_->clear();
    model_->setColumnCount( int( q.column_count() ) );

    for ( int col = 0; col < int( q.column_count() ); ++col  )
        model_->setHeaderData( col, Qt::Horizontal, q.column_name( col ) );
}

void
QuanResultTable::addRecord( const QuanQuery& q )
{

    int row = model_->rowCount();

    if ( model_->insertRow( row ) ) {
        for ( int col = 0; col < int( q.column_count() ); ++col )
            model_->setData( model_->index( row, col ), q.column_value( col ) );
    }

}
