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

#include "digestedpeptidetable.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adprot/protein.hpp>
#include <adprot/protease.hpp>
#include <adprot/peptide.hpp>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QModelIndex>
#include <QItemDelegate>
#include <QPainter>
#include <QDebug>
#include <boost/format.hpp>

namespace peptide {
    namespace detail {

        class DigestedPeptideDelegate : public QItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                if ( index.column() == 0 ) {
                    render_sequence( painter, option, index.data().toString() );
                } else if ( index.column() == 1 ) {
                    render_formula( painter, option, index.data().toString() );
                } else if ( index.column() == 2 ) {
                    QStyleOptionViewItemV2 op = option;
					op.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    drawDisplay( painter, op, op.rect, (boost::format("%.7lf") % index.data().toDouble()).str().c_str() );
                } else {
                    QItemDelegate::paint( painter, option, index );
                }
            }

            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                if ( index.column() == 0 || index.column() == 1 ) {
                    QTextDocument document;
                    document.setHtml( index.data().toString() );
					return QSize( document.size().width(), document.size().height() );
                }
                return QItemDelegate::sizeHint( option, index );
            }
            
        private:
            void render_formula( QPainter * painter, const QStyleOptionViewItem& option, const QString& text ) const {
                painter->save();
                QStyleOptionViewItemV4 op = option;
                QTextDocument document;
                document.setHtml( text );
                op.widget->style()->drawControl( QStyle::CE_ItemViewItem, &op, painter );
                painter->translate( op.rect.topLeft() );
                QRect clip( 0, 0, op.rect.width(), op.rect.height() );
                document.drawContents( painter, clip );
                painter->restore();
            }

            void render_sequence( QPainter * painter, const QStyleOptionViewItem& option, const QString& text ) const {
                painter->save();
                QStyleOptionViewItemV4 op = option;
                QTextDocument document;
                QFont font;
                font.setFamily( "Consolas" );
                document.setDefaultFont( font );
                document.setHtml( text );
                op.widget->style()->drawControl( QStyle::CE_ItemViewItem, &op, painter );
                painter->translate( op.rect.topLeft() );
                QRect clip( 0, 0, op.rect.width(), op.rect.height() );
                document.drawContents( painter, clip );
                painter->restore();
            }

        };

    }
}


using namespace peptide;

DigestedPeptideTable::DigestedPeptideTable(QWidget *parent) :  QTableView(parent)
                                            , model_( new QStandardItemModel )
                                            , delegate_( new detail::DigestedPeptideDelegate )
{
    setModel( model_ );
    setItemDelegate( delegate_ );
    init( *model_ );
}

DigestedPeptideTable::~DigestedPeptideTable()
{
    delete delegate_;
    delete model_;
}

void
DigestedPeptideTable::init( QStandardItemModel& model )
{
    model.setColumnCount( 3 );
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr("sequence") );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr("formula") );
    model.setHeaderData( 2, Qt::Horizontal, QObject::tr("mass") );
	setColumnWidth( 0, 200 );
    QFont font;
    font.setFamily( "Consolas" );
    setFont( font );
	setWordWrap( true );
}

void
DigestedPeptideTable::setData( const adprot::protein& prot )
{
    QStandardItemModel& model = *model_;
    adcontrols::ChemicalFormula formulaParser;

    if ( auto enzyme = protease_.lock() ) {

        std::vector< std::string > peptides;
        if ( adprot::protease::digest( *enzyme, prot.sequence(), peptides ) ) {
            model.setRowCount( peptides.size() );

            int row = 0;
            for ( auto& peptide: peptides ) {
                std::string formula = adprot::peptide::formula( peptide );
                std::string stdFormula = adcontrols::ChemicalFormula::standardFormula( formula );
                model.setData( model.index( row, 0 ), QString::fromStdString( peptide ) );
                model.setData( model.index( row, 1 ), QString::fromStdString( adcontrols::ChemicalFormula::formatFormula( stdFormula ) ) );
                model.setData( model.index( row, 2 ), formulaParser.getMonoIsotopicMass( stdFormula ) );
                // model.setData( model.index( row, 2 ), Qt::AlignRight + Qt::AlignVCenter, Qt::TextAlignmentRole);
                ++row;
            }
            
        }
    }
}

void
DigestedPeptideTable::setData( const std::shared_ptr< adprot::protease >& enzyme )
{
    protease_ = enzyme;
}

