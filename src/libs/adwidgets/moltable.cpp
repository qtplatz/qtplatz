/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "moltable.hpp"
#include "adducts_type.hpp"
#include "delegatehelper.hpp"
#include "htmlheaderview.hpp"
#include "moltablehelper.hpp"
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptides.hpp>
#include <adprot/peptide.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <adportable/index_of.hpp>
#include <QApplication>
#include <QByteArray>
#include <QByteArray>
#include <QClipboard>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/system/error_code.hpp>
#include <array>
#include <algorithm>
#include <functional>
#include <sstream>

using namespace adwidgets;
using adportable::index_of;


namespace {

    class delegate : public QStyledItemDelegate {
        enum {
            c_formula = index_of< col_formula, MolTable::column_list >::value
            , c_adducts = index_of< col_adducts, MolTable::column_list >::value
        };

        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            using adcontrols::ChemicalFormula;

            QStyleOptionViewItem opt(option);
            initStyleOption( &opt, index );
            opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

            if ( index.column() == index_of< col_formula, MolTable::column_list >::value ) {
                std::string formula = ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );
            } else if ( index.column() == index_of< col_adducts, MolTable::column_list >::value ) {
                std::string formula = ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );
            } else if ( index.column() == index_of< col_memo, MolTable::column_list >::value ) {
                DelegateHelper::render_html2( painter, opt, index.data().toString() );
            } else if ( index.column() == index_of< col_abundance, MolTable::column_list >::value ) {
                if ( index.data().toDouble() <= 0.002 ) {
                    painter->save();
                    painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x80 ) ); // tomato
                    QStyledItemDelegate::paint( painter, opt, index );
                    painter->restore();
                }
                QStyledItemDelegate::paint( painter, opt, index );
            } else if ( index.column() == index_of< col_mass, MolTable::column_list >::value ) {
                painter->save();
                using adwidgets::moltable::computeMass;
                double exactMass = std::get< 0 >( computeMass( index.model()->index( index.row(), c_formula ).data().toString()
                                                               , index.model()->index( index.row(), c_adducts ).data().toString() ) );
                double mass = index.data( Qt::EditRole ).toDouble();
                if ( adportable::compare<double>::approximatelyEqual( exactMass, mass ) )
                    painter->fillRect( option.rect, QColor( 0xf0, 0xf8, 0xff, 0x80 ) ); // AliceBlue
                else if ( mass < 0.9 )
                    painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x80 ) ); // tomato
                else
                    painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x40 ) ); // tomato
                painter->drawText( option.rect
                                   , option.displayAlignment
                                   , QString::number( index.data( Qt::EditRole ).toDouble(), 'f', 8  ) );
                // QStyledItemDelegate::paint( painter, opt, index );
                painter->restore();

            } else if ( index.column() == index_of< col_svg, MolTable::column_list >::value ) {
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
            if ( index.column() == index_of< col_mass, MolTable::column_list >::value ) {
                auto widget = new QDoubleSpinBox( parent );
                widget->setMinimum( 0 ); widget->setMaximum( 5000 ); widget->setSingleStep( 0.0001 ); widget->setDecimals( 7 );
                widget->setValue( index.data( Qt::EditRole ).toDouble() );
                return widget;
            } else {
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
        }

        QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.column() == index_of< col_svg, MolTable::column_list >::value && !index.data( Qt::EditRole ).toByteArray().isEmpty() ) {
                return QSize( 80, 80 );
            } else if ( index.column() == index_of< col_formula, MolTable::column_list >::value ) {
                return DelegateHelper::html_size_hint( option, index );
            } else {
                return QStyledItemDelegate::sizeHint( option, index );
            }
        }

    };

} // namespace

namespace adwidgets {
    ////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    class MolTable::impl  {
    public:
        impl() : model_( new QStandardItemModel() )
               , current_polarity_( adcontrols::polarity_positive ) {
            using adportable::index_of;

            std::fill( editable_.begin(), editable_.end(), false );
            for ( auto& col : { index_of< col_formula, column_list >::value
                               , index_of< col_adducts, column_list >::value
                               , index_of< col_abundance, column_list >::value
                               , index_of< col_synonym, column_list >::value
                               , index_of< col_memo, column_list >::value
                               , index_of< col_smiles, column_list >::value } )
                editable_[ col ] = true;
        }

        ~impl() {
            delete model_;
        }

        QStandardItemModel * model_;
        std::array< bool, ncolumns > editable_;
        adcontrols::ion_polarity current_polarity_;

        void formulaChanged( int row );
        void setValue( int row, const adcontrols::moltable::value_type& );
        void adductChanged( int row );
    };
}


MolTable::MolTable(QWidget *parent) : TableView(parent)
                                    , impl_( new impl() )
{
    setModel( impl_->model_ );
	setItemDelegate( new delegate ); //( [this]( const QModelIndex& index ){
    connect( impl_->model_, &QStandardItemModel::dataChanged, this, &MolTable::handleDataChanged );

    setHorizontalHeader( new HtmlHeaderView );
    setSortingEnabled( true );
    setAcceptDrops( true );

    connect( this, &TableView::rowsDeleted, [this]() {
        if ( impl_->model_->rowCount() == 0 )
            impl_->model_->setRowCount( 1 );
    });

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QTableView::customContextMenuRequested, this, &MolTable::handleContextMenu );

    impl_->model_->setColumnCount( std::tuple_size< column_list >() );
    impl_->model_->setRowCount( 1 );
    //setColumnHidden( c_smiles, true );
}

MolTable::~MolTable()
{
}

void
MolTable::onInitialUpdate()
{
    using adportable::index_of;

    QStandardItemModel * model = impl_->model_;

    setHeaderData( model, column_list{} );

    //----------
    setColumnHidden( col_msref{}, true );
    setColumnHidden( col_nlaps(), true );
    setColumnHidden( col_apparent_mass(), true );
    setColumnHidden( col_tof{}, true );

    horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Interactive );
    horizontalHeader()->setStretchLastSection( true );
}

void
MolTable::setContents( const adcontrols::moltable& mols )
{
    using adportable::index_of;

    QStandardItemModel& model = *impl_->model_;
    adcontrols::ChemicalFormula cformula;

    model.setRowCount( int( mols.data().size() + 1 ) ); // add one free line for add formula

    impl_->current_polarity_ = mols.polarity();
    do {
        QSignalBlocker block( impl_->model_ );
        int row = 0;
        for ( auto& mol : mols.data() ) {
            impl_->setValue( row, mol );
            ++row;
        }
    } while ( 0 );

    resizeColumnToContents( index_of< col_formula, column_list >::value );
    this->viewport()->repaint();
}

void
MolTable::getContents( adcontrols::moltable& m )
{
    using adportable::index_of;
    auto model = impl_->model_;

    m.data().clear();

    for ( int row = 0; row < model->rowCount(); ++row ) {
        adcontrols::moltable::value_type mol;

        mol.formula() = model->index( row, index_of< col_formula, column_list >::value ).data( Qt::EditRole ).toString().toStdString();
        if ( !mol.formula().empty() ) {
            m << moltable::value_from( model, row, column_list{} );
        }
    }
}

void
MolTable::handleDataChanged( const QModelIndex& index, const QModelIndex& last )
{
    using adportable::index_of;
    QString stdFormula;
    auto model = impl_->model_;

    if ( index.column() == index_of< col_formula, column_list >::value ) {
        impl_->formulaChanged( index.row() );
        resizeColumnToContents( index_of< col_formula, column_list >::value );
    }

    if ( index.column() == index_of< col_adducts, column_list >::value ) {
        auto adducts = model->index( index.row(), index_of< col_adducts, column_list >::value ).data( Qt::UserRole + 1 ).value< adducts_type >();
        adducts.set( index.data().toString(), impl_->current_polarity_ ); // update adducts_type
        model->setData( model->index( index.row(), index_of< col_adducts, column_list >::value ), QVariant::fromValue( adducts ), Qt::UserRole + 1 ); // restore
        impl_->formulaChanged( index.row() );
    }
    if ( !stdFormula.isEmpty() )
        model->setData( model->index( index.row(), index_of< col_memo, column_list >::value ), stdFormula );

    if ( index.column() == index_of< col_smiles, column_list >::value ) {
        auto smiles = index.data( Qt::EditRole ).toString();
        if ( smiles.isEmpty() ) {
            model->setData( model->index( index.row(), index_of< col_svg, column_list >::value ), QByteArray() );
        } else {
            if ( auto d = MolTableHelper::SmilesToSVG()( smiles ) ) {
                model->setData( model->index( index.row(), index_of< col_svg, column_list >::value ), std::get< 1 >( *d ) );
                model->setData( model->index( index.row(), index_of< col_formula, column_list >::value ), std::get< 0 >( *d ) );
                impl_->formulaChanged( index.row() );
            }
        }
    }

    if ( index.row() == model->rowCount() - 1 &&
         !model->index( index.row(), index_of< col_formula, column_list >::value ).data( Qt::EditRole ).toString().isEmpty() ) {
        model->insertRow( index.row() + 1 );
    }

    emit onValueChanged();
}

void
MolTable::setColumnEditable( int column, bool hide )
{
    if ( column >= 0 && column < impl_->editable_.size() )
        impl_->editable_[ column ] = hide;
}

bool
MolTable::isColumnEditable( int column ) const
{
    if ( column >= 0 && column < impl_->editable_.size() )
        return impl_->editable_[ column ];
    return false;
}

void
MolTable::handleContextMenu( const QPoint& pt )
{
    QMenu menu;

    emit onContextMenu( menu, pt );

    typedef std::pair< QAction *, std::function< void() > > action_type;

    std::vector< action_type > actions;

    actions.emplace_back( menu.addAction( "Enable all" ), [=,this](){ enable_all( true ); } );
    actions.emplace_back( menu.addAction( "Disable all" ), [=,this](){ enable_all( false ); } );

    TableView::addActionsToContextMenu( menu, pt );

    if ( QAction * selected = menu.exec( mapToGlobal( pt ) ) ) {
        auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){
                return t.first == selected;
            });
        if ( it != actions.end() )
            (it->second)();
    }
}

void
MolTable::enable_all( bool enable )
{
    using adportable::index_of;
    QStandardItemModel& model = *impl_->model_;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( ! model.index( row, index_of< col_formula, column_list >::value ).data().toString().isEmpty() )
            model.setData( model.index( row, index_of< col_formula, column_list >::value ), enable ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }

}

void
MolTable::dragEnterEvent( QDragEnterEvent * event )
{
	const QMimeData * mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
        for ( auto& url: urlList ) {
            QFileInfo path( url.toLocalFile() );
            if ( path.suffix() == "sdf" || path.suffix() == "mol" ) {
                event->accept();
                return;
            }
        }
	}
}

void
MolTable::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
MolTable::dragLeaveEvent( QDragLeaveEvent * event )
{
	event->accept();
}

void
MolTable::dropEvent( QDropEvent * event )
{
	const QMimeData * mimeData = event->mimeData();
    auto model = impl_->model_;

    QModelIndex beg;

	if ( mimeData->hasUrls() ) {

        QSignalBlocker block( this );

        int row = model->rowCount() == 0 ? 0 : model->rowCount() - 1;
        beg = model->index( row, 0 );

        QList<QUrl> urlList = mimeData->urls();
        for ( auto& url : urlList ) {
            auto vec = MolTableHelper::SDMolSupplier()( url );
            model->insertRows( row, vec.size() );
            for ( const auto& d: vec ) {
#if __cplusplus >= 201703L
                auto [ formula, smiles, svg ] = d;
#else
                QString formula, smiles;
                QByteArray svg;
                std::tie( formula, smiles, svg ) = d;
#endif
                // impl_->setData( *this, row, formula, impl_->polarity_, QString(), smiles, svg, QString() );
                ++row;
            }
        }
        event->accept();
	}

    if ( beg.isValid() ) {
        emit dataChanged( beg, model->index( model->rowCount() - 1, model->columnCount() - 1 ) );
    }
}

void
MolTable::handleCopyToClipboard()
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
MolTable::handlePaste()
{
    auto model = impl_->model_;

    int row = model->rowCount() - 1;

    if ( auto mols = MolTableHelper::paste() ) {
        model->setRowCount( model->rowCount() + mols->data().size() );

        QSignalBlocker block( model );
        for ( const auto& value: mols->data() ) {
            // ADDEBUG() << "row: " << row << "\t" << value.mass() << ", " << value.synonym();
            impl_->setValue( row++, value );
        }
    }
}

void
MolTable::setColumHide( const std::vector< std::pair< fields, bool > >& hides )
{
    for ( auto& hide: hides )
        QTableView::setColumnHidden( hide.first, hide.second );
}

void
MolTable::handlePolarity( adcontrols::ion_polarity polarity )
{
    using adportable::index_of;
    auto model = impl_->model_;
    QSignalBlocker block( model );

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

////////////////////////////////

void
MolTable::impl::formulaChanged( int row )
{
    using adportable::index_of;

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
MolTable::impl::setValue( int row, const adcontrols::moltable::value_type& value )
{
    using adportable::index_of;

    auto smiles = QString::fromStdString( value.smiles() );
    adducts_type adducts( value.adducts_ );

    model_->setData( model_->index( row, index_of< col_adducts,   column_list >::value ), QVariant::fromValue( adducts ), Qt::UserRole + 1 );
    model_->setData( model_->index( row, index_of< col_adducts,   column_list >::value ), adducts.get( current_polarity_ ), Qt::EditRole );

    model_->setData( model_->index( row, index_of< col_smiles,    column_list >::value ), smiles );
    model_->setData( model_->index( row, index_of< col_formula,   column_list >::value ), QString::fromStdString( value.formula() ) );
    model_->setData( model_->index( row, index_of< col_synonym,   column_list >::value ), QString::fromStdString( value.synonym() ) );
    model_->setData( model_->index( row, index_of< col_abundance, column_list >::value ), value.abundance() );
    model_->setData( model_->index( row, index_of< col_mass,      column_list >::value ), value.mass() );
    model_->setData( model_->index( row, index_of< col_msref,     column_list >::value ), value.isMSRef() );
    model_->setData( model_->index( row, index_of< col_memo,      column_list >::value ), QString::fromStdWString( value.description() ) );

    if ( !smiles.isEmpty() ) {
        if ( auto d = MolTableHelper::SmilesToSVG()( smiles ) ) {
            auto formula = std::get< 0 >( *d );
            auto svg = std::get< 1 >( *d );
            model_->setData( model_->index( row, index_of< col_svg, column_list >::value ), svg );
            if ( auto logP = MolTableHelper::logP( smiles ) )
                model_->setData( model_->index( row, index_of< col_logp, column_list >::value ), logP->first );
        }
        if ( auto item = model_->item( row, index_of< col_formula, column_list >::value ) )
            item->setEditable( false );
    }
    if ( auto item = model_->item( row, index_of< col_formula, column_list >::value ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        model_->setData( model_->index( row, index_of< col_formula, column_list >::value ), value.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }
    if ( auto item = model_->item( row, index_of< col_svg, column_list >::value ) ) // has structure data
        item->setEditable( false );
    if ( auto item = model_->item( row, index_of< col_mass, column_list >::value ) )
        item->setEditable( editable_[ index_of< col_mass, column_list >::value ] );
}
