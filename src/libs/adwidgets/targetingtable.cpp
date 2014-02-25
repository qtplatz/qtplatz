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

#include "targetingtable.hpp"
#include "delegatehelper.hpp"
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptides.hpp>
#include <adprot/peptide.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <boost/format.hpp>
#include <qtwrapper/font.hpp>

using namespace adwidgets;

namespace adwidgets {
    class TargetingDelegate : public QStyledItemDelegate {
        
        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.column() == 1 ) {
				std::string formula = adcontrols::ChemicalFormula::formatFormula( index.data().toString().toStdString() );
                DelegateHelper::render_html( painter, option, QString::fromStdString( formula ) );
            } else if ( index.column() == 2 ) {
                QStyleOptionViewItemV2 op = option;
                op.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                QStyledItemDelegate::paint( painter, op, index );
            } else {
                QStyledItemDelegate::paint( painter, option, index );
            }
        }

        QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.column() == 1 ) {
                return DelegateHelper::html_size_hint( option, index );
            } else {
                return QStyledItemDelegate::sizeHint( option, index );
            }
        }
    };
}

TargetingTable::TargetingTable(QWidget *parent) : TableView(parent)
                                                , model_( new QStandardItemModel() )
{
    setModel( model_ );
	setItemDelegate( new TargetingDelegate );
    setSortingEnabled( true );

    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );
	model_->setColumnCount( 3 );
    model_->setRowCount( 1 );
}

TargetingTable::~TargetingTable()
{
    delete model_;
}

QStandardItemModel&
TargetingTable::model()
{
    return *model_;
}

void
TargetingTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;
    
    model.setColumnCount( 4 );

    model.setHeaderData( 0,  Qt::Horizontal, QObject::tr( "peptide" ) );
    model.setHeaderData( 1,  Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( 2,  Qt::Horizontal, QObject::tr( "mass" ) );
    model.setHeaderData( 3,  Qt::Horizontal, QObject::tr( "#" ) );

    resizeColumnsToContents();
    resizeRowsToContents();

    horizontalHeader()->setResizeMode( QHeaderView::Stretch );
}

void
TargetingTable::setContents( const adprot::digestedPeptides& digested )
{
    QStandardItemModel& model = *model_;
    const adprot::peptides& peptides = digested.peptides();

    model.setRowCount( static_cast< int >( peptides.size() ) );

    int row = 0;
    for ( auto& p: peptides ) {
        model.setData( model.index( row, 0 ), QString::fromStdString( p.sequence() ) );
        model.setData( model.index( row, 1 ), QString::fromStdString( p.formula() ) );
        model.setData( model.index( row, 2 ), p.mass(), Qt::EditRole );
        model.setData( model.index( row, 2 ), QString::fromStdString( (boost::format("%.7f")%p.mass()).str() ), Qt::DisplayRole );
        model.setData( model.index( row, 3 ), row );
		++row;
    }
    if ( row > 1 ) {
        --row;
        model.setData( model.index( 0, 3 ), "N" );
        model.setData( model.index( row, 3 ), "C" );
    }
}

void
TargetingTable::setContents( const adcontrols::TargetingMethod& method )
{
}

void
TargetingTable::getContents( adcontrols::TargetingMethod& method )
{
}
