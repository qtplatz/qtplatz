/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adducts_type.hpp"
#include "mschromatogramtable.hpp"
#include "moltablehelper.hpp"
#include "delegatehelper.hpp"
#include "htmlheaderview.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>

#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSvgRenderer>
#include <QUrl>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/system/error_code.hpp>
#include <array>
#include <algorithm>
#include <functional>
#include <sstream>

using namespace adwidgets;

namespace adwidgets {

    class MSChromatogramTable::impl  {
    public:
        impl() : current_polarity_{ adcontrols::polarity_positive }
               , model_( std::make_unique< QStandardItemModel >() ) {
        }

        ~impl() {
        }

        void formulaChanged( int row );
        void setValue( int row, const adcontrols::moltable::value_type& value );
        adcontrols::ion_polarity current_polarity_;
        std::unique_ptr< QStandardItemModel > model_;
    };
}

namespace {
    using adwidgets::moltable::computeMass;
    typedef MSChromatogramTable::column_list column_list;

    enum fields {
        c_formula         = adportable::index_of< col_formula,       column_list >::value
        , c_adducts       = adportable::index_of< col_adducts,       column_list >::value
        , c_mass          = adportable::index_of< col_mass,          column_list >::value
        , c_tR            = adportable::index_of< col_retentionTime, column_list >::value
        , c_msref         = adportable::index_of< col_msref,         column_list >::value
        , c_synonym       = adportable::index_of< col_synonym,       column_list >::value
        , c_svg           = adportable::index_of< col_svg,           column_list >::value
        , c_smiles        = adportable::index_of< col_smiles,        column_list >::value
        , c_description   = adportable::index_of< col_memo,          column_list >::value
    };

    class delegate : public QStyledItemDelegate {

        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            using adcontrols::ChemicalFormula;

            QStyleOptionViewItem opt(option);
            initStyleOption( &opt, index );
            opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

            if ( index.column() == index_of< col_formula, column_list >::value ) {
                std::string formula = ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );
            } else if ( index.column() == index_of< col_adducts, column_list >::value ) {
                std::string adduct = ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                DelegateHelper::render_html2( painter, opt, QString::fromStdString( adduct ) );
            } else if ( index.column() == index_of< col_memo, column_list >::value ) {
                DelegateHelper::render_html2( painter, opt, index.data().toString() );
            } else if ( index.column() == index_of< col_mass, column_list >::value ) {
                double exactMass = std::get< 0 >( computeMass( index.model()->index( index.row(), c_formula ).data().toString()
                                                               , index.model()->index( index.row(), c_adducts ).data().toString() ) );
                double mass = index.data( Qt::EditRole ).toDouble();
                if ( adportable::compare<double>::approximatelyEqual( exactMass, mass ) )
                    painter->fillRect( option.rect, QColor( 0xf0, 0xf8, 0xff, 0x80 ) ); // AliceBlue
                else
                    painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x40 ) ); // tomato
                painter->drawText( opt.rect
                                   , opt.displayAlignment
                                   , QString::number( index.data().toDouble(), 'f', 4  ) );
            } else if ( index.column() == index_of< col_retentionTime, column_list >::value ) {
                if ( index.data().isValid() ) {
                    painter->drawText( opt.rect
                                       , opt.displayAlignment
                                       , QString::number( index.data().toDouble(), 'f', 1  ) );
                } else {
                    painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x40 ) ); // tomato
                }
            } else if ( index.column() == index_of< col_svg, column_list >::value ) {
                painter->save();
                QSvgRenderer renderer( index.data().toByteArray() );
                painter->translate( option.rect.x(), option.rect.y() );
                painter->scale( 1.0, 1.0 );
                QRect target( 0, 0, option.rect.width(), option.rect.height() );
                renderer.render( painter, target );
                painter->restore();
            } else {
                opt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
                QStyledItemDelegate::paint( painter, opt, index );
            }
        }

        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {
            if ( index.column() == index_of< col_mass, column_list >::value ) {
                auto widget = new QDoubleSpinBox( parent );
                widget->setMinimum( 0 ); widget->setMaximum( 5000 ); widget->setSingleStep( 0.0001 ); widget->setDecimals( 7 );
                widget->setValue( index.data( Qt::EditRole ).toDouble() );
                return widget;
            } else {
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
        }

        QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.column() == index_of< col_svg, column_list >::value && !index.data( Qt::EditRole ).toByteArray().isEmpty() ) {
                return QSize( 80, 80 );
            } else if ( index.column() == index_of< col_formula, column_list >::value ) {
                return DelegateHelper::html_size_hint( option, index );
            } else {
                return QStyledItemDelegate::sizeHint( option, index );
            }
        }

    };
}

MSChromatogramTable::MSChromatogramTable(QWidget *parent) : TableView( parent )
                                                          , impl_( std::make_unique< impl >() )
{
    setModel( impl_->model_.get() );
    setHorizontalHeader( new HtmlHeaderView );
    setItemDelegate( new delegate() );
    setSortingEnabled( true );

    impl_->model_->setColumnCount( std::tuple_size< column_list >() );
    connect( this, &TableView::rowsDeleted, [&]{
        ADDEBUG() << "rows deleted" << impl_->model_->rowCount();
        if ( impl_->model_->rowCount() == 0 )
            impl_->model_->setRowCount( 1 );
    });
}

MSChromatogramTable::~MSChromatogramTable()
{
}

void
MSChromatogramTable::onInitialUpdate()
{
    setHeaderData( impl_->model_.get(), column_list{} );
    connect( impl_->model_.get(), &QStandardItemModel::dataChanged, this, &MSChromatogramTable::handleDataChanged );
}

void
MSChromatogramTable::setValue( const adcontrols::moltable& t )
{
    auto model = impl_->model_.get();

    model->setRowCount( int( t.data().size() + 1 ) ); // add one free line for add formula
    impl_->current_polarity_ = t.polarity();

    int row(0);
    for ( const auto& mol: t.data() ) {
        impl_->setValue( row++, mol );
    }
    resizeColumnToContents( index_of< col_formula, column_list >::value );
    resizeColumnToContents( index_of< col_retentionTime, column_list >::value );
    resizeColumnToContents( index_of< col_protocol, column_list >::value );
    this->viewport()->repaint();
}

void
MSChromatogramTable::handlePolarity( adcontrols::ion_polarity polarity )
{
    auto model = impl_->model_.get();
    if ( impl_->current_polarity_ != polarity ) {
        impl_->current_polarity_ = polarity;

        for ( int row = 0; row < model->rowCount(); ++row ) {
            auto adducts = model->index( row, index_of< col_adducts, column_list >::value ).data( Qt::UserRole + 1 ).value< adducts_type >();
            model->setData( model->index( row, index_of< col_adducts, column_list >::value ), adducts.get( polarity ) );
            impl_->formulaChanged( row );
        }
    }
    this->viewport()->repaint();
}

void
MSChromatogramTable::handleDataChanged( const QModelIndex& index, const QModelIndex& )
{
    auto model = impl_->model_.get();
    QSignalBlocker block( model );

    if ( index.column() == index_of< col_adducts, column_list >::value ) {
        auto adducts = model->index( index.row(), index_of< col_adducts, column_list >::value ).data( Qt::UserRole + 1 ).value< adducts_type >();
        adducts.set( index.data().toString(), impl_->current_polarity_ );
        model->setData( model->index( index.row(), index_of< col_adducts, column_list >::value ), QVariant::fromValue( adducts ), Qt::UserRole + 1 );
        impl_->formulaChanged( index.row() );
    }
    if ( index.column() == index_of< col_formula, column_list >::value ) {
        impl_->formulaChanged( index.row() );
        resizeColumnToContents( index_of< col_formula, column_list >::value );
    }

    emit valueChanged();
}

void
MSChromatogramTable::impl::formulaChanged( int row )
{
    bool enable = model_->index( row, index_of< col_formula, column_list >::value ).data( Qt::CheckStateRole ).toBool();
    auto formula = model_->index( row, index_of< col_formula, column_list >::value ).data().toString().toStdString();
    auto adduct  = model_->index( row, index_of< col_adducts, column_list >::value ).data().toString().toStdString();

    if ( auto item = model_->item( row, index_of< col_formula, column_list >::value ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        model_->setData( model_->index( row, index_of< col_formula, column_list >::value ), enable ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }

    auto vec = adcontrols::ChemicalFormula::standardFormulae( formula, adduct );
    if ( !vec.empty() ) {
        const double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( vec.at( 0 ) );
        model_->setData( model_->index( row, index_of< col_mass, column_list >::value ), mass );
        auto mol = adcontrols::ChemicalFormula::toMolecule( formula, adduct );

        auto charge_string = (mol.charge() == 1 || mol.charge() == -1) ? QString() : QString("%1").arg(QString::number(std::abs(mol.charge())));
        charge_string += mol.charge() > 0 ? "+" : mol.charge() < 0 ? "-" : "";

        auto adduct_string = QString::fromStdString( adcontrols::ChemicalFormula::neutralize( adduct ).first );

        QString display_formula = QString("[M%1]<sup>%2</sup>").arg( adduct_string, charge_string);
        model_->setData( model_->index( row, index_of< col_memo, column_list >::value ), display_formula );
    }
}

void
MSChromatogramTable::impl::setValue( int row, const adcontrols::moltable::value_type& value )
{
    auto smiles = QString::fromStdString( value.smiles() );
    adducts_type adducts( value.adducts_ );

    model_->setData( model_->index( row, index_of< col_adducts, column_list >::value ),    QVariant::fromValue( adducts ), Qt::UserRole + 1 );
    model_->setData( model_->index( row, index_of< col_adducts, column_list >::value ),    adducts.get( current_polarity_ ), Qt::EditRole );
    model_->setData( model_->index( row, index_of< col_smiles,  column_list >::value ),    smiles );
    model_->setData( model_->index( row, index_of< col_formula, column_list >::value ),    QString::fromStdString( value.formula() ) );
    model_->setData( model_->index( row, index_of< col_synonym, column_list >::value ),    QString::fromStdString( value.synonym() ) );
    model_->setData( model_->index( row, index_of< col_mass, column_list >::value ),       value.mass() );

    model_->setData( model_->index( row, index_of< col_retentionTime, column_list >::value ), value.tR() ? *value.tR() : QVariant{} );

    model_->setData( model_->index( row, index_of< col_msref, column_list >::value ), value.isMSRef() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    if ( auto item = model_->item( row, index_of< col_msref, column_list >::value ) ) {
        item->setEditable( false );
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
    } else {
        ADDEBUG() << "-------- empty item -----------";
    }

    model_->setData( model_->index( row, index_of< col_protocol, column_list >::value ), value.protocol() ? *value.protocol() : QVariant{} );

    if ( !smiles.isEmpty() ) {
        if ( auto d = MolTableHelper::SmilesToSVG()( smiles ) ) {
            model_->setData( model_->index( row, index_of< col_formula, column_list >::value ), std::get< 0 >( *d ) ); // formula
            model_->setData( model_->index( row, index_of< col_svg, column_list >::value ), std::get< 1 >( *d ) ); // svg
        }
        if ( auto item = model_->item( row, index_of< col_formula, column_list >::value ) )
            item->setEditable( false );
    }
    if ( auto item = model_->item( row, index_of< col_formula, column_list >::value ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        model_->setData( model_->index( row, index_of< col_formula, column_list >::value )
                         , value.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }
    if ( auto item = model_->item( row, index_of< col_svg, column_list >::value ) ) // has structure data
        item->setEditable( false );
}

void
MSChromatogramTable::handleCopyToClipboard()
{
    using adportable::index_of;
	QModelIndexList indices = selectionModel()->selectedIndexes();

    std::sort( indices.begin(), indices.end() );
    if ( indices.size() < 1 )
        return;

    auto mol = moltable::copy( indices, column_list{} );
    auto json = QString::fromStdString( boost::json::serialize( boost::json::value_from( mol ) ) );
    // text
    QString selected_text;
    std::pair< QModelIndexList::const_iterator, QModelIndexList::const_iterator > range{ indices.begin(), {} };
    while ( range.first != indices.end() ) {
        range = equal_range( indices.begin(), indices.end(), *range.first, [](const auto& a, const auto& b){ return a.row() < b.row(); });
        // per line
        for ( auto it = range.first; it != range.second; ++it ) {
            if ( !isColumnHidden( it->column() ) && ( it->column() != index_of< col_svg, column_list >::value ) )  {
                auto text = it->data( Qt::EditRole ).toString();
                selected_text.append( text );
                selected_text.append( '\t' );
            }
        }
        selected_text.append( '\n' );
        range.first = range.second;
    }
    if ( auto md = new QMimeData() ) {
        md->setData( QLatin1String( "application/json" ), json.toUtf8() );
        // workaround for x11
        if ( QApplication::keyboardModifiers() & ( Qt::ShiftModifier | Qt::ControlModifier ) )
            md->setText( json );
        else
            md->setText( selected_text );
        QApplication::clipboard()->setMimeData( md, QClipboard::Clipboard );
    }
}

void
MSChromatogramTable::handlePaste()
{
    auto model = impl_->model_.get();

    int row = model->rowCount() - 1;

    if ( auto mols = MolTableHelper::paste() ) {
        model->setRowCount( model->rowCount() + mols->data().size() );

        QSignalBlocker block( model );
        for ( const auto& value: mols->data() ) {
            impl_->setValue( row++, value );
        }
    }
}

void
MSChromatogramTable::handleAutoTagetingEnabled( bool enable )
{
    setColumnHidden( col_retentionTime{}, !enable );
    setColumnHidden( col_protocol{}, !enable );
}

void
MSChromatogramTable::getContents( adcontrols::moltable& m )
{
    using adportable::index_of;
    auto model = impl_->model_.get();
    m.data().clear();
    m.setPolarity( impl_->current_polarity_ );
    for ( int row = 0; row < model->rowCount(); ++row ) {
        auto formula = model->index( row, index_of< col_formula, column_list >::value ).data( Qt::EditRole ).toString().toStdString();
        if ( !formula.empty() )
            m << moltable::value_from( model, row, column_list{} );
    }

}

void
MSChromatogramTable::addActionsToContextMenu( QMenu& menu, const QPoint& pt ) const
{
    TableView::addActionsToContextMenu( menu, pt );
    // menu.addAction( tr( "Add a line" ), this, [&]{ impl_->model_->setRowCount( impl_->model_->rowCount() + 1 ); } );
}
