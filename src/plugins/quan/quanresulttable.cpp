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
#include <QDebug>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QVariant>
#include <QPainter>
#include <boost/filesystem.hpp>

namespace quan {
    namespace {

        class ItemDelegate : public QStyledItemDelegate {
            boost::filesystem::path ppath_;
        public:
            void clear() {
                ppath_.clear();
            }

            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );

                // if ( auto sqlModel = qobject_cast< const QSqlQueryModel * >( index.model() ) ) {
                //     auto field = sqlModel->record().field( index.column() );

                if ( index.data().metaType() == QMetaType::fromType< double >() ) { // QVariant::Double ) {
                    double value = index.data().toDouble();
                    if ( ( value <= std::numeric_limits< double >::epsilon() ) || value >= 0.01 )
                        painter->drawText( op.rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( index.data().toDouble(), 'f', 5 ) );
                    else
                        painter->drawText( op.rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( index.data().toDouble(), 'e', 5 ) );
                } else if ( index.data().metaType() == QMetaType::fromType< QString >() ) {
                    std::string formula = adcontrols::ChemicalFormula::formatFormula( index.data().toString().toStdString() );
                    if ( !formula.empty() ) {
                        adwidgets::DelegateHelper::render_html( painter, op, QString::fromStdString( formula ) );
                        return;
                    }
                    boost::filesystem::path path( index.data().toString().toStdWString() );
                    if ( path.is_absolute() ) {
                        if ( ppath_.empty() )
                            const_cast< ItemDelegate * >(this)->ppath_ = path.parent_path();
                        painter->drawText( op.rect, Qt::AlignRight | Qt::AlignVCenter, QString::fromStdString( boost::filesystem::relative( path, ppath_ ).string() ) );
                        return;
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
                } else if ( index.data().metaType() == QMetaType::fromType< double >() ) {
                    QFontMetricsF fm = op.fontMetrics;
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
                                                  , model_( new QSqlQueryModel )
{
    setAllowDelete( false );
    setModel( model_.get() );
    setItemDelegate( new ItemDelegate );
    setHorizontalHeader( new adwidgets::HtmlHeaderView );

    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows); // Selecting only rows
}

void
QuanResultTable::currentChanged( const QModelIndex& current, const QModelIndex& index )
{
    scrollTo( index, QAbstractItemView::EnsureVisible );
    emit onCurrentChanged( current );
}

QAbstractItemModel*
QuanResultTable::model()
{
    return model_.get();
}

/////////////////////
void
QuanResultTable::setQuery( QSqlQuery&& sqlQuery, const std::vector<QString>& hidelist )
{
    if ( auto delegate = dynamic_cast< ItemDelegate *>(itemDelegate()) )
        delegate->clear();

    if ( auto model = qobject_cast< QSqlQueryModel * >( model_.get() ) ) {
        model->setQuery( std::move( sqlQuery ) );
    }

    for ( auto& hide: hidelist ) {
        int col = sqlQuery.record().indexOf( hide );
        if ( col >= 0 )
            setColumnHidden( col, true );
    }
}
