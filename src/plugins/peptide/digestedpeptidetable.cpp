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

#include "digestedpeptidetable.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adprot/protein.hpp>
#include <adprot/protease.hpp>
#include <adprot/peptide.hpp>
#include <adprot/peptides.hpp>
#include <adprot/digestedpeptides.hpp>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QModelIndex>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QPainter>
#include <QDebug>
#include <boost/format.hpp>

namespace peptide {
    namespace detail {

        class HeaderView : public QHeaderView {
		public:
			HeaderView(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = 0) : QHeaderView( orientation, parent ) {}

			void paintSection( QPainter * painter, const QRect& rect, int logicalIndex ) const override {
				if ( !rect.isValid() )
					return;

				if ( logicalIndex == 3 || logicalIndex == 4 ) {
					QStyleOptionHeader op;
					initStyleOption(&op);
					op.text = "";
					op.rect = rect;
                    op.textAlignment = Qt::AlignVCenter | Qt::AlignHCenter;
					// draw the section
					style()->drawControl( QStyle::CE_Header, &op, painter, this );
					// html paiting
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
        };

        class DigestedPeptideDelegate : public QItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                if ( index.column() == 0 ) {
                    render_sequence( painter, option, index.data().toString() );
                } else if ( index.column() == 1 ) {
                    render_formula( painter, option, index.data().toString() );
				} else if ( index.column() >= 2 ) {
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
            void render_html( QPainter * painter, const QStyleOptionViewItem& option, const QString& text ) const {
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

            void render_formula( QPainter * painter, const QStyleOptionViewItem& option, const QString& text ) const {
                std::string formula = adcontrols::ChemicalFormula::formatFormula( text.toStdString() );
                render_html( painter, option, QString::fromStdString( formula ) );
            }

            void render_sequence( QPainter * painter, const QStyleOptionViewItem& option, const QString& text ) const {
                render_html( painter, option, text );
            }

        };

    }
}


using namespace peptide;

DigestedPeptideTable::DigestedPeptideTable(QWidget *parent) :  QTableView(parent)
                                            , model_( new QStandardItemModel )
{
	setHorizontalHeader( new detail::HeaderView );
    setModel( model_ );
	setItemDelegate( new detail::DigestedPeptideDelegate );
    init( *model_ );
}

DigestedPeptideTable::~DigestedPeptideTable()
{
    delete model_;
}

void
DigestedPeptideTable::init( QStandardItemModel& model )
{
    model.setColumnCount( 5 );
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr("sequence") );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr("formula") );
    model.setHeaderData( 2, Qt::Horizontal, QObject::tr("M") );
    model.setHeaderData( 3, Qt::Horizontal, QObject::tr("M+H<sup>+</sup>") );
    model.setHeaderData( 4, Qt::Horizontal, QObject::tr("M+H<sup>+</sup>(<sup>18</sup>O)") );
    setSortingEnabled( true );
	setColumnWidth( 0, 200 );
    QFont font;
    font.setFamily( "Consolas" );
    setFont( font );
	setWordWrap( true );
}

void
DigestedPeptideTable::setData( const adprot::digestedPeptides& digested )
{
    QStandardItemModel& model = *model_;

    if ( auto formulaParser = formulaParser_.lock() ) {

        double water = formulaParser->getMonoIsotopicMass( "H2O" );
        double electron = formulaParser->getElectronMass();
		double proton = formulaParser->getMonoIsotopicMass( "H" ) - electron;
		double heavyWater = formulaParser->getMonoIsotopicMass( "H2 18O" );

        model.setRowCount( static_cast<int>( digested.peptides().size() ) );
        
        int row = 0;
        for ( auto& peptide: digested.peptides() ) {
            
            const std::string& sequence = peptide.sequence();
            const std::string& stdFormula = peptide.formula();
            double mass = peptide.mass();

            model.setData( model.index( row, 0 ), QString::fromStdString( sequence ) );
            model.setData( model.index( row, 1 ), QString::fromStdString( stdFormula ) );
            model.setData( model.index( row, 2 ), mass );
            model.setData( model.index( row, 3 ), mass + proton ); 
			model.setData( model.index( row, 4 ), mass - water + heavyWater + proton ); // 18O
            
            ++row;
        }
    }
}

void
DigestedPeptideTable::setData( const std::shared_ptr< adcontrols::ChemicalFormula >& ptr )
{
    formulaParser_ = ptr;
}

void
DigestedPeptideTable::selectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
	QTableView::selectionChanged( selected, deselected );

    QModelIndexList list = selectionModel()->selectedIndexes();
    qSort( list );
    if ( list.size() < 1 )
        return;
    QVector< QString > formulae;
    for ( auto index: list )
        formulae.push_back( model_->index( index.row(), 1 ).data( Qt::EditRole ).toString() );
    
    emit selectedFormulae( formulae );
}
