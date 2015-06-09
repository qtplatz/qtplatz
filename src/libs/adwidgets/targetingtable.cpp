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
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adportable/float.hpp>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <boost/format.hpp>
#include <qtwrapper/font.hpp>
#include <functional>

using namespace adwidgets;

namespace adwidgets {

    namespace detail {

        enum {
            c_peptide
            , c_formula
            , c_mass
            , c_description
            , nbrColums
        };

        class TargetingDelegate : public QStyledItemDelegate {
        
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {

                QStyleOptionViewItem opt(option);
                initStyleOption( &opt, index );
                opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

                if ( index.column() == c_formula ) {
                    
                    std::string formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                    DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );

                } else if ( index.column() == c_mass ) {

                    painter->save();
                    std::string formula = index.model()->index( index.row(), c_formula ).data( Qt::EditRole ).toString().toStdString();
                    double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula );
                    double mass = index.data( Qt::EditRole ).toDouble();
                    if ( !adportable::compare<double>::approximatelyEqual( exactMass, mass ) )
                        painter->fillRect( option.rect, QColor( 0xff, 0x66, 0x44, 0x40 ) );
                    QStyledItemDelegate::paint( painter, opt, index );
                    painter->restore();

                } else {

                    QStyledItemDelegate::paint( painter, opt, index );

                }
            }

            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
                QStyledItemDelegate::setModelData( editor, model, index );
                if ( valueChanged_ )
                    valueChanged_( index );
            }

            QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {
                if ( index.column() == c_mass ) {
                    auto widget = new QDoubleSpinBox( parent );
                    widget->setMinimum( 0 ); widget->setMaximum( 5000 ); widget->setSingleStep( 0.0001 ); widget->setDecimals( 7 );
                    widget->setValue( index.data( Qt::EditRole ).toDouble() );
                    return widget;
                } else {
                    return QStyledItemDelegate::createEditor( parent, option, index );
                }
            }
            
            //QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            //    if ( index.column() == 1 ) {
            //        return DelegateHelper::html_size_hint( option, index );
            //    } else {
            //        return QStyledItemDelegate::sizeHint( option, index );
            //    }
            //}
        public:
            void register_handler( std::function< void( const QModelIndex& ) > f ) {
                valueChanged_ = f;
            }
        private:
            std::function< void( const QModelIndex& ) > valueChanged_;
        };

    }
}

TargetingTable::TargetingTable(QWidget *parent) : TableView(parent)
                                                , model_( new QStandardItemModel() )
                                                , mass_editable_( false )
{
    setModel( model_ );
    auto delegate = new detail::TargetingDelegate;
    delegate->register_handler( [=]( const QModelIndex& index ){ handleValueChanged( index ); } );
	setItemDelegate( delegate );
    setSortingEnabled( true );
    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QTableView::customContextMenuRequested, this, &TargetingTable::handleContextMenu );

	model_->setColumnCount( detail::nbrColums );
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

    using namespace adwidgets::detail;

    // horizontalHeader()->setResizeMode( QHeaderView::Stretch ); // Depricated since 5.0
    horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch ); // Depricated since 5.0

    model.setColumnCount( nbrColums );
    model.setHeaderData( c_peptide,  Qt::Horizontal, QObject::tr( "peptide" ) );
    model.setHeaderData( c_formula,  Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_mass,  Qt::Horizontal, QObject::tr( "mass" ) );
    model.setHeaderData( c_description, Qt::Horizontal, QObject::tr( "memo" ) );

    setColumnHidden( c_peptide, true );

    resizeColumnsToContents();
    resizeRowsToContents();
}

void
TargetingTable::setContents( const adprot::digestedPeptides& )
{
}

void
TargetingTable::setContents( const adcontrols::TargetingMethod& method )
{
    QStandardItemModel& model = *model_;
    using namespace adwidgets::detail;
    adcontrols::ChemicalFormula cformula;

    model.setRowCount( int( method.formulae().size() + 1 ) ); // add one free line for add formula

    int row = 0;
    for ( auto& formula: method.formulae() ) {

        typedef adcontrols::TargetingMethod::formula_data formula_data;

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
TargetingTable::getContents( adcontrols::TargetingMethod& method )
{
    QStandardItemModel& model = *model_;
    using namespace adwidgets::detail;

    method.formulae().clear();

    for ( int row = 0; row < model.rowCount(); ++row ) {

        std::string formula = model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdString();
        bool enable = model.index( row, c_formula ).data( Qt::CheckStateRole ).toBool();
        std::wstring memo = model.index( row, c_description ).data( Qt::EditRole ).toString().toStdWString();

        if ( !formula.empty() ) {
            method.formulae().push_back( adcontrols::TargetingMethod::formula_data( enable, formula, memo ) );
        }

    }
}

void
TargetingTable::setContents( const adcontrols::MSChromatogramMethod& m )
{
    QStandardItemModel& model = *model_;
    using namespace adwidgets::detail;
    adcontrols::ChemicalFormula cformula;

    mass_editable_ = true;

    model.setRowCount( int( m.targets().size() + 1 ) ); // add one free line for add formula

    int row = 0;
    for ( auto& value: m.targets() ) {

        model.setData( model.index( row, c_formula ), QString::fromStdString( value.formula ) );
        if ( auto item = model.item( row, c_formula ) ) {
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            item->setEditable( true );
            model.setData( model.index( row, c_formula ), value.enable ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

        if ( value.mass < 0.7 )
            model.setData( model.index( row, c_mass ), cformula.getMonoIsotopicMass( value.formula ) );
        else
            model.setData( model.index( row, c_mass ), value.mass );

        model.item( row, c_mass )->setEditable( true );

        model.setData( model.index( row, c_description ), QString::fromStdWString( value.memo ) );
        model.item( row, c_description )->setEditable( true );

        ++row;
    }

}

void
TargetingTable::getContents( adcontrols::MSChromatogramMethod& m )
{
    QStandardItemModel& model = *model_;
    using namespace adwidgets::detail;

    std::vector< adcontrols::MSChromatogramMethod::value_type > vec;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        adcontrols::MSChromatogramMethod::value_type value;

        value.formula = model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdString();
        value.enable = model.index( row, c_formula ).data( Qt::CheckStateRole ).toBool();
        value.memo = model.index( row, c_description ).data( Qt::EditRole ).toString().toStdWString();
        value.mass = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();

        if ( !value.formula.empty() )
            vec.push_back( value );
    }
    m.targets( vec );
}

void
TargetingTable::handleValueChanged( const QModelIndex& index )
{
    using namespace adwidgets::detail;

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
        if ( mass_editable_ )
            model_->item( index.row(), c_mass )->setEditable( true );
    }
    if ( index.row() == model_->rowCount() - 1 ) {

        model_->insertRow( index.row() + 1 );

    }

    // resizeColumnsToContents();
    // resizeRowsToContents();
}

void
TargetingTable::handleContextMenu( const QPoint& pt )
{
    QMenu menu;

    emit onContextMenu( menu, pt );
    
    typedef std::pair< QAction *, std::function< void() > > action_type;

    std::vector< action_type > actions;
    actions.push_back( std::make_pair( menu.addAction( "Enable all" ), [=](){ enable_all( true ); }) );
    actions.push_back( std::make_pair( menu.addAction( "Disable all" ), [=](){ enable_all( false ); }) );

    if ( QAction * selected = menu.exec( mapToGlobal( pt ) ) ) {
        auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){
                return t.first == selected;
            });
        if ( it != actions.end() )
            (it->second)();
    }
    

}

void
TargetingTable::enable_all( bool enable )
{
    QStandardItemModel& model = *model_;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( !model.index( row, detail::c_formula ).data().toString().isEmpty() )
            model_->setData( model.index( row, detail::c_formula ), enable ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );        
    }

}

void
TargetingTable::setEditable( fields id, bool enable )
{
    if ( id == idMass )
        mass_editable_ = enable;
}

