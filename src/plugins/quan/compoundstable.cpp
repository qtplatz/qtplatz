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

#include "compoundstable.hpp"
#include "quandocument.hpp"
#include <adwidgets/delegatehelper.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>

#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <boost/format.hpp>
#include <qtwrapper/font.hpp>
#include <functional>

namespace quan {
    namespace compounds_table {

        enum {
            c_formula
            , c_mass
            , c_tR
            , c_isISTD
            , c_idISTD
            , c_level_0
            , c_level_1
            , c_level_2
            , c_level_3
            , c_level_4
            , c_level_5
            , c_level_6
            , c_level_7
            , c_level_8
            , c_level_9
            , c_level_10
            , c_level_11
            , c_level_12
            , c_level_13
            , c_level_14
            , c_level_15
            , c_level_16
            , c_level_17
            , c_level_18
            , c_level_19
            , c_level_20
            , c_level_21
            , c_level_22
            , c_level_23
            , c_level_24
            , c_level_25
            , c_level_26
            , c_level_27
            , c_level_28
            , c_level_29
            , c_level_last
            , c_description
            , nbrColums
        };

        class HeaderView : public QHeaderView {
        public:
			HeaderView(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = 0) : QHeaderView( orientation, parent ) {}
            
			void paintSection( QPainter * painter, const QRect& rect, int logicalIndex ) const override {
				if ( !rect.isValid() )
					return;
				if ( logicalIndex == c_mass || logicalIndex == c_tR ) {
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

        class CompoundsDelegate : public QStyledItemDelegate {
        
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {

                QStyleOptionViewItem opt(option);
                initStyleOption( &opt, index );
                opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

                if ( index.column() == c_formula ) {

                    std::string formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                    adwidgets::DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );

                } else if ( index.column() == c_mass ) {

                    QStyledItemDelegate::paint( painter, opt, index );

                } else {
                    
                    QStyledItemDelegate::paint( painter, opt, index );

                }
            }

            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
                QStyledItemDelegate::setModelData( editor, model, index );
                if ( valueChanged_ )
                    valueChanged_( index );
            }
            
        public:
            void register_handler( std::function< void( const QModelIndex& ) > f ) {
                valueChanged_ = f;
            }
        private:
            std::function< void( const QModelIndex& ) > valueChanged_;
        };

    }
}

using namespace quan;
using namespace quan::compounds_table;

CompoundsTable::CompoundsTable(QWidget *parent) : TableView(parent)
                                                , model_( new QStandardItemModel() )
                                                , levels_( 1 )
{
    setModel( model_ );
    auto delegate = new CompoundsDelegate;
    delegate->register_handler( [=]( const QModelIndex& index ){ handleValueChanged( index ); } );
	setItemDelegate( delegate );
    setSortingEnabled( true );
    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QTableView::customContextMenuRequested, this, &CompoundsTable::handleContextMenu );

	model_->setColumnCount( nbrColums );
    model_->setRowCount( 1 );

    onInitialUpdate();
}

CompoundsTable::~CompoundsTable()
{
    delete model_;
}

void
CompoundsTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

    setHorizontalHeader( new HeaderView );

    // horizontalHeader()->setResizeMode( QHeaderView::Stretch );

    model.setColumnCount( nbrColums );
    model.setHeaderData( c_formula,  Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_mass,  Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
    model.setHeaderData( c_tR,  Qt::Horizontal, QObject::tr( "t<sub>R</sub>(min)" ) );
    model.setHeaderData( c_isISTD,  Qt::Horizontal, QObject::tr( "ISTD" ) );
    model.setHeaderData( c_idISTD,  Qt::Horizontal, QObject::tr( "ISTD ID" ) );
    model.setHeaderData( c_description, Qt::Horizontal, QObject::tr( "memo" ) );

    for ( int col = c_level_0; col < c_level_last; ++col )
        model.setHeaderData( col, Qt::Horizontal, QString( "amounts [%1]" ).arg( col - c_level_0 + 1 ) );

    handleQuanMethod( QuanDocument::instance()->quanMethod() );

    resizeColumnsToContents();
    resizeRowsToContents();
}

void
CompoundsTable::handleValueChanged( const QModelIndex& index )
{
    if ( index.column() == c_formula ) {

        std::string formula = index.data( Qt::EditRole ).toString().toStdString();

        // if ( auto item = model_->item( index.row(), c_formula ) ) {
        //     if ( !(item->flags() & Qt::ItemIsUserCheckable) ) {
        //         item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        //         item->setEditable( true );
        //     }
        // }
        // model_->setData( index, formula.empty() ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole );

        adcontrols::ChemicalFormula cformula;
        double exactMass = cformula.getMonoIsotopicMass( formula );
        model_->setData( model_->index( index.row(), c_mass ), exactMass );

        // set default values for amounts (avoid zero value in the table)
        for ( int col = c_level_0; col < int(c_level_0 + levels_); ++col ) {
            if ( model_->index( index.row(), col ).data().isNull() || 
                 model_->index( index.row(), col ).data().toDouble() <= 1.0e-30 )
                model_->setData( model_->index( index.row(), col ), double( col - c_level_0 + 1 ) );
        }
    }
    if ( index.row() == model_->rowCount() - 1 )
        model_->insertRow( index.row() + 1 );

    resizeColumnsToContents();
    resizeRowsToContents();
}

void
CompoundsTable::handleContextMenu( const QPoint& pt )
{
    QMenu menu;
#if 0
    typedef std::pair< QAction *, std::function< void() > > action_type;

    std::vector< action_type > actions;
    actions.push_back( std::make_pair( menu.addAction( "Enable all" ), [=] (){ enable_all( true ); } ) );
    actions.push_back( std::make_pair( menu.addAction( "Disable all" ), [=] (){ enable_all( false ); } ) );

    if ( QAction * selected = menu.exec( mapToGlobal( pt ) ) ) {
        auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){
                return t.first == selected;
            });
        if ( it != actions.end() )
            (it->second)();
    }
#endif
}

bool
CompoundsTable::getContents( adcontrols::QuanCompounds& c )
{
    QStandardItemModel& model = *model_;

    c.clear();
    for ( int row = 0; row < model.rowCount(); ++row ) {
        adcontrols::QuanCompound a;
        a.formula( model.index( row, c_formula ).data().toString().toStdWString().c_str() );
        if ( std::wstring( a.formula() ).empty() )
            continue;
        a.description( model.index( row, c_description ).data().toString().toStdWString().c_str() );
        a.mass( model.index( row, c_mass ).data().toDouble() );
        a.tR( model.index( row, c_tR ).data().toDouble() );
        a.isISTD( model.index( row, c_isISTD ).data().toBool() );
        a.idISTD( model.index( row, c_isISTD ).data().toInt() );
        std::vector< double > amounts;
        for ( int i = c_level_0; i <= c_level_last; ++i ) {
            auto data = model.index( row, i ).data();
            if ( !data.isNull() )
                amounts.push_back( data.toDouble() );
        }
        a.amounts( amounts.data(), amounts.size() );
        c << a;
    }
    return false;
}

bool
CompoundsTable::setContents( const adcontrols::QuanCompounds& c )
{
    QStandardItemModel& model = *model_;
    model.setRowCount( int( c.size() ) + 1 ); // add last empty line

    const adcontrols::QuanMethod& qm = QuanDocument::instance()->quanMethod();

    int row = 0;
    for ( auto& comp: c ) {
        std::wstring formula = comp.formula();
        model.setData( model.index( row, c_formula ), QString::fromStdWString( formula ) );
        model.setData( model.index( row, c_mass ), comp.mass() );
        model.setData( model.index( row, c_tR ), comp.tR() );
        model.setData( model.index( row, c_description ), QString::fromStdWString( comp.description() ) );

        // if ( auto cbx = model.item( row, c_isISTD ) )
        //     cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
        model.setData( model.index( row, c_isISTD ), comp.isISTD() );
        model.setData( model.index( row, c_idISTD ), comp.idISTD() );
        const double * amounts = comp.amounts();
        size_t i;
        for ( i = 0; i < comp.levels(); ++i )
            model.setData( model.index( row, int(c_level_0 + i) ), amounts[ i ] );

        ++row;
    }

    resizeColumnsToContents();
    resizeRowsToContents();
    return false;
}

void
CompoundsTable::handleQuanMethod( const adcontrols::QuanMethod& qm )
{
    levels_ = qm.levels();

    if ( !qm.isChromatogram() )
        setColumnHidden( c_tR, true );

    if ( !qm.isInternalStandard() ) {
        setColumnHidden( c_isISTD, true );
        setColumnHidden( c_idISTD, true );
    }
    else {
        setColumnHidden( c_isISTD, false );
        setColumnHidden( c_idISTD, false );
    }

    for ( int column = c_level_0; column <= c_level_last; ++column ) {
        uint32_t level = column - c_level_0;
        bool hidden = (level >= levels_) ? true : false;
        setColumnHidden( column, hidden );
    }
}

