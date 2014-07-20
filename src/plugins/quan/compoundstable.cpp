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
#include <adwidgets/delegatehelper.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/quanmethod.hpp>

#include <QHeaderView>
#include <QMenu>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <boost/format.hpp>
#include <qtwrapper/font.hpp>
#include <functional>

namespace quan {
    namespace compounds_table {

        enum {
            c_formula
            , c_mass
            , c_description
            , nbrColums
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
}

CompoundsTable::~CompoundsTable()
{
    delete model_;
}

void
CompoundsTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

    horizontalHeader()->setResizeMode( QHeaderView::Stretch );

    model.setColumnCount( nbrColums );
    model.setHeaderData( c_formula,  Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_mass,  Qt::Horizontal, QObject::tr( "mass" ) );
    model.setHeaderData( c_description, Qt::Horizontal, QObject::tr( "memo" ) );

    resizeColumnsToContents();
    resizeRowsToContents();
}

#if 0
void
CompoundsTable::setContents( const adcontrols::CompoundsMethod& method )
{
    QStandardItemModel& model = *model_;
    using namespace adwidgets::detail;
    adcontrols::ChemicalFormula cformula;

    model.setRowCount( int( method.formulae().size() + 1 ) ); // add one free line for add formula

    int row = 0;
    for ( auto& formula: method.formulae() ) {

        typedef adcontrols::CompoundsMethod::formula_data formula_data;

        model.setData( model.index( row, c_formula ), QString::fromStdString( formula_data::formula( formula ) ) );
        if ( auto item = model.item( row, c_formula ) ) {
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            item->setEditable( true );
            model.setData( model.index( row, c_formula ), formula_data::enable( formula ) ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
        model.setData( model.index( row, c_description ), QString::fromStdWString( formula_data::description( formula ) ) );

        double exactMass = cformula.getMonoIsotopicMass( formula_data::formula( formula ) );
        model_->setData( model_->index( row, c_mass ), exactMass );

        ++row;
    }

    resizeColumnsToContents();
    resizeRowsToContents();
}

void
CompoundsTable::getContents( adcontrols::CompoundsMethod& method )
{
    QStandardItemModel& model = *model_;
    using namespace adwidgets::detail;

    method.formulae().clear();

    for ( int row = 0; row < model.rowCount(); ++row ) {

        std::string formula = model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdString();
        bool enable = model.index( row, c_formula ).data( Qt::CheckStateRole ).toBool();
        std::wstring memo = model.index( row, c_description ).data( Qt::EditRole ).toString().toStdWString();

        if ( !formula.empty() ) {
            method.formulae().push_back( adcontrols::CompoundsMethod::formula_data( enable, formula, memo ) );
        }

    }
}
#endif

void
CompoundsTable::handleValueChanged( const QModelIndex& index )
{
    if ( index.column() == c_formula ) {

        std::string formula = index.data( Qt::EditRole ).toString().toStdString();

        if ( auto item = model_->item( index.row(), c_formula ) ) {
            if ( !(item->flags() & Qt::ItemIsUserCheckable) ) {
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                item->setEditable( true );
            }
        }

        model_->setData( index, formula.empty() ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole );

        adcontrols::ChemicalFormula cformula;
        double exactMass = cformula.getMonoIsotopicMass( formula );
        model_->setData( model_->index( index.row(), c_mass ), exactMass );
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
