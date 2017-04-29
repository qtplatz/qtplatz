/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adwidgets/delegatehelper.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <qtwrapper/font.hpp>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QVariant>
#include <QPainter>
#include <boost/filesystem.hpp>

namespace quan {
    namespace quanresulttable {
        
        class ItemDelegate : public QStyledItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );
                if ( index.data().type() == QVariant::Double ) {
                    double value = index.data().toDouble();
                    if ( value < 0.01 )
                        painter->drawText( op.rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( index.data().toDouble(), 'e', 5 ) );
                    else
                        painter->drawText( op.rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( index.data().toDouble(), 'f', 5 ) );
                } else if ( index.data().type() == QVariant::String ) {
                    std::string formula = adcontrols::ChemicalFormula::formatFormula( index.data().toString().toStdString() );
                    if ( !formula.empty() )
                        adwidgets::DelegateHelper::render_html( painter, op, QString::fromStdString( formula ) );
                    else {
                        boost::filesystem::path path( index.data().toString().toStdWString() );
                        if ( path.is_complete() )
                            op.textElideMode = Qt::ElideLeft; // elide left for filename, otherwise stay default
                        QStyledItemDelegate::paint( painter, op, index );
                    }
                } else
                    QStyledItemDelegate::paint( painter, op, index );
            }
            void setEditorData( QWidget * editor, const QModelIndex& index ) const override {
                QStyledItemDelegate::setEditorData( editor, index );
            }

            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
                QStyledItemDelegate::setModelData( editor, model, index );
            }
            
            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );
                if ( index.data().isNull() ) {
                    return QSize();
                } else if ( index.data().type() == QVariant::Double ) {
                    QFont font;
                    qtwrapper::font::setFont( font, qtwrapper::fontSizeNormal, qtwrapper::fontTableBody );
                    QFontMetricsF fm( font );
                    double value = index.data().toDouble();
                    double width;
                    if ( value < 0.01 )
                        width = fm.boundingRect( op.rect, Qt::AlignJustify | Qt::AlignVCenter, QString::number( value, 'f', 5 ) ).width();
                    else
                        width = fm.boundingRect( op.rect, Qt::AlignJustify | Qt::AlignVCenter, QString::number( value, 'e', 5 ) ).width();
                    QSize sz = QStyledItemDelegate::sizeHint( option, index );
                    sz.setWidth( width );
                    return sz;
                } else
                    return QStyledItemDelegate::sizeHint( option, index );
            }
        
        };
    }
}

using namespace quan;

QuanResultTable::~QuanResultTable()
{
}

QuanResultTable::QuanResultTable(QWidget *parent) : adwidgets::TableView(parent)
                                                  , model_( new QStandardItemModel )
{
    setAllowDelete( false );
    setModel( model_.get() );
    setItemDelegate( new quanresulttable::ItemDelegate );
    setHorizontalHeader( new adwidgets::HtmlHeaderView );

    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows); // Selecting only rows
}

void
QuanResultTable::prepare( const QuanQuery& q )
{
    model_->clear();
    model_->setColumnCount( int( q.column_count() ) );

    for ( int col = 0; col < int( q.column_count() ); ++col  ) {
        model_->setHeaderData( col, Qt::Horizontal, QuanQuery::column_name_tr( q.column_name( col ) ) );
        if ( hideColumns_.find( q.column_name( col ).toStdString() ) != hideColumns_.end() )
            setColumnHidden( col, true );
    }

    for ( int col = 0; col < int( q.column_count() ); ++col ) {
        QTextDocument document;
        // document.setDefaultFont( option.font );
        document.setHtml( QuanQuery::column_name_tr( q.column_name( col ) ) );
        QSize size( document.size().width(), document.size().height() );
        horizontalHeader()->model()->setHeaderData( col, Qt::Horizontal, QVariant( size ), Qt::SizeHintRole );
    }    

}

void
QuanResultTable::addRecord( const QuanQuery& q )
{
    int row = model_->rowCount();

    if ( model_->insertRow( row ) ) {
        for ( int col = 0; col < int( q.column_count() ); ++col ) {
            model_->setData( model_->index( row, col ), q.column_value( col ) );
            model_->itemFromIndex( model_->index( row, col ) )->setEditable( false );
        }
    }
    resizeColumnsToContents();
    resizeRowsToContents();
}

void
QuanResultTable::setColumnHide( const std::string& hide )
{
    hideColumns_.insert( hide );
}

void
QuanResultTable::clear()
{
    hideColumns_.clear();
}

void
QuanResultTable::currentChanged( const QModelIndex& current, const QModelIndex& index )
{
    scrollTo( index, QAbstractItemView::EnsureVisible );
    emit onCurrentChanged( current );
}

int
QuanResultTable::findColumn( const QString& name )
{
    int nColumn = model_->columnCount();
    for ( int col = 0; col < nColumn; ++col ) {
        if ( model_->headerData( col, Qt::Horizontal, Qt::EditRole ).toString() == name )
            return col;
    }
    return -1;
}
