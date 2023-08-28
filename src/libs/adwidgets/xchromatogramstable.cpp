/**************************************************************************
** Copyright (C) 2022-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "xchromatogramstable.hpp"
#include "delegatehelper.hpp"
#include "htmlheaderview.hpp"
#include "moltablecolumns.hpp"
#include "moltablehelper.hpp"
#include "spin_t.hpp"
#include <GraphMol/MolDraw2D/MolDraw2DHelpers.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qobject.h>
#include <QtGui/qstandarditemmodel.h>
#include <adplot/color_table.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptides.hpp>
#include <adprot/peptide.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/xchromatogramsmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <adportable/index_of.hpp>
#include <QApplication>
#include <QByteArray>
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
#include <QDebug>
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

namespace {

    using adcontrols::ion_polarity;

    struct adducts_type {
        adducts_type() {}
        adducts_type( const std::tuple< std::string, std::string >& t )
            : adducts( t ) {}

        QString get( ion_polarity polarity ) const {
            return polarity == adcontrols::polarity_positive ?
                QString::fromStdString( std::get< adcontrols::polarity_positive >( adducts ) )
                : QString::fromStdString( std::get< adcontrols::polarity_negative >( adducts ) );
        }

        void set( const QString& adduct, ion_polarity polarity ) {
            ( polarity == adcontrols::polarity_positive
                ? std::get< adcontrols::polarity_positive >( adducts )
              : std::get< adcontrols::polarity_negative >( adducts ) )  = adduct.toStdString();
        }
        std::tuple< std::string, std::string > adducts;
    };

    ///////////////////////////////////////
    struct paste_handler {
        adcontrols::ion_polarity current_polarity_;
        paste_handler( adcontrols::ion_polarity pol ) : current_polarity_( pol ) {}
        adcontrols::xic::xic_method operator()( const adcontrols::moltable::value_type& value ) const {
            adcontrols::xic::xic_method line;
            line.mol_         = std::make_tuple( value.enable(), value.synonym(), value.formula(), value.smiles() );
            line.adduct_      = value.adducts();
            line.mass_window_ = std::make_pair( value.mass(), 0.1 );
            line.algo_        = adcontrols::xic::ePeakAreaOnProfile;
            line.protocol( value.protocol() ? *value.protocol() : 0 );
            return line;
        }
    };

}

Q_DECLARE_METATYPE( adducts_type );

namespace adwidgets {

    class XChromatogramsTable::impl  {
    public:
        impl() : model_( new QStandardItemModel )
               , current_polarity_{ adcontrols::polarity_positive } {
        }

        ~impl() {
        }

        void formulaChanged( int row );
        void massChanged( int row );
        void timeChanged( int row );

        QStandardItemModel * model_;
        std::weak_ptr< const adcontrols::MassSpectrometer > massSpectrometer_;
        adcontrols::ion_polarity current_polarity_;
    };
}

namespace {

    typedef std::tuple<
        col_synonym
        , col_formula
        , col_adducts
        , col_mass
        , col_massWindow
        , col_tof
        , col_tofWindow
        , col_xicMethod
        , col_svg
        , col_protocol
        , col_smiles
        > column_list;

    enum columns { c_synonym       = adportable::index_of< col_synonym,   column_list >::value
                   , c_formula     = adportable::index_of< col_formula,   column_list >::value
                   , c_adducts     = adportable::index_of< col_adducts,   column_list >::value
                   , c_mass        = adportable::index_of< col_mass,      column_list >::value
                   , c_masswindow  = adportable::index_of< col_massWindow,column_list >::value
                   , c_time        = adportable::index_of< col_tof,       column_list >::value
                   , c_timewindow  = adportable::index_of< col_tofWindow, column_list >::value
                   , c_algo        = adportable::index_of< col_xicMethod, column_list >::value
                   , c_svg         = adportable::index_of< col_svg,       column_list >::value
                   , c_protocol    = adportable::index_of< col_protocol,  column_list >::value
                   , c_smiles      = adportable::index_of< col_smiles,    column_list >::value
                   , ncolumns      = std::tuple_size< column_list >()
    };

    struct color_legend {
        static void update_color_legends( QAbstractItemModel * model, int first ) {
            for ( int row = first; row < model->rowCount(); ++row ) {
                if ( row < 7 ) {
                    const auto& color = adplot::constants::chromatogram::color_table[ row + 1 ];
                    model->setData( model->index( row, c_svg ), QBrush(color), Qt::BackgroundRole );
                } else {
                    model->setData( model->index( row, c_svg ), QVariant{}, Qt::BackgroundRole );
                }
            }
        }
    };


    using namespace adwidgets::spin_initializer;

    template< typename ratio >
    class TimeSpinBox : public QDoubleSpinBox {
    public:
        TimeSpinBox( QWidget * parent ) : QDoubleSpinBox( parent ) {}
        double valueFromText(const QString &text) const override {
            return QDoubleSpinBox::valueFromText( text ) / ratio::den;
        }
        QString textFromValue(double val) const override {
            return QDoubleSpinBox::textFromValue( val * ratio::den );
        }
    };

    class delegate : public QStyledItemDelegate {
        adcontrols::ChemicalFormula cf_;
        XChromatogramsTable * table_;
    public:
        delegate( XChromatogramsTable * table ) : table_( table ) {}

        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            using adportable::index_of;
            QStyleOptionViewItem opt(option);
            initStyleOption( &opt, index );
            opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            if ( auto item = qobject_cast< const QStandardItemModel * >(index.model())->itemFromIndex( index ) ) {
                painter->fillRect( option.rect, item->background() );
            }
            if ( index.column() == c_formula || index.column() == c_adducts ) {
                std::string display_formula = cf_.formatFormulae( index.data().toString().toStdString() );
                DelegateHelper::render_html2( painter, opt, QString::fromStdString( display_formula ) );
            } else if ( index.column() == c_time ) {
                painter->drawText( option.rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( index.data().toDouble() * std::micro::den, 'f', 3 ) );
            } else if ( index.column() == c_timewindow ) {
                painter->drawText( option.rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( index.data().toDouble() * std::nano::den, 'f', 1 ) );
            } else if ( index.column() == index_of< col_xicMethod, column_list >::value ) {
                QString text;
                switch( index.data().toInt() ) {
                case adcontrols::xic::ePeakAreaOnProfile:   text = "Area"; break;
                case adcontrols::xic::ePeakHeightOnProfile: text = "Height"; break;
                case adcontrols::xic::eCounting:            text = "Counts"; break;
                }
                painter->drawText( opt.rect, Qt::AlignCenter, text );
            } else if ( index.column() == index_of< col_svg, column_list >::value ) {
                painter->save();
                QSvgRenderer renderer( index.data().toByteArray() );
                painter->translate( option.rect.x(), option.rect.y() );
                painter->scale( 1.0, 1.0 );
                QRect target( 2, 1, option.rect.width() - 4, option.rect.height() - 2 );
                renderer.render( painter, target );
                painter->restore();
            } else {
                QStyledItemDelegate::paint( painter, opt, index );
            }
        }

        QWidget * createEditor( QWidget * parent
                                , const QStyleOptionViewItem &option
                                , const QModelIndex& index ) const override {
            if ( index.column() == c_algo ) {
                auto cbx = new QComboBox( parent );
                cbx->addItems( { "Area", "Height", "Counts" } );
                cbx->setCurrentIndex( index.data().toInt() );
                return cbx;
            } else if ( index.column() == c_mass ) {
                qDebug() << "-------- createEditor ------- " << index;
                auto spin = new QDoubleSpinBox( parent );
                spin_init( spin, std::make_tuple( Decimals{3}, SingleStep{ 0.010 }, Minimum{1.00}, Maximum{4000.0} ) );
                connect( spin, qOverload< double >(&QDoubleSpinBox::valueChanged)
                         , [index,this](double value){ emit table_->editorValueChanged( index, value ); });
                return spin;
            } else if ( index.column() == c_masswindow ) {
                qDebug() << "-------- createEditor ------- " << index;
                auto spin = new QDoubleSpinBox( parent );
                spin_init( spin, std::make_tuple( Decimals{3}, SingleStep{ 0.001 }, Minimum{0.001}, Maximum{10.0} ) );
                connect( spin, qOverload< double >(&QDoubleSpinBox::valueChanged)
                         , [index,this](double value){ emit table_->editorValueChanged( index, value ); });
                return spin;
            } else if ( index.column() == c_time ) {
                auto spin = new TimeSpinBox<std::micro>( parent );
                spin_init( spin, std::make_tuple( Decimals{7}, Maximum{1e6} ) );
                return spin;
            } else if ( index.column() == c_timewindow ) {
                auto spin = new TimeSpinBox<std::nano>( parent );
                spin_init( spin, std::make_tuple( Decimals{1}, Maximum{100} ) );
                return spin;
            } else {
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
        }

        void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
            if ( index.column() == c_algo ) {
                model->setData( index, qobject_cast< QComboBox * >( editor )->currentIndex() );
            } else if ( index.column() == c_time || index.column() == c_timewindow ) {
                model->setData( index, qobject_cast< QDoubleSpinBox * >( editor )->value() );
            } else {
                QStyledItemDelegate::setModelData( editor, model, index );
            }
        }

    };
}


XChromatogramsTable::XChromatogramsTable(QWidget *parent) : TableView(parent)
                                                          , impl_( std::make_unique< impl >() )
{
    setItemDelegate( new delegate( this ) );

    setHorizontalHeader( new HtmlHeaderView );
    setSortingEnabled( false );

    verticalHeader()->setSectionsMovable( true );
    verticalHeader()->setDragEnabled( true );
    verticalHeader()->setDragDropMode( QAbstractItemView::InternalMove );

    setContextMenuPolicy( Qt::CustomContextMenu );

    impl_->model_->setColumnCount( ncolumns );
    impl_->model_->setRowCount( 8 );
    setModel( impl_->model_ );
}

XChromatogramsTable::~XChromatogramsTable()
{
}

void
XChromatogramsTable::onInitialUpdate()
{
    auto model = impl_->model_;

    moltable::setHeaderData( model, column_list{} );
    setColumnHidden( adportable::index_of< col_protocol, column_list>::value, true );
    setColumnHidden( adportable::index_of< col_tof, column_list>::value, true );

    for ( int i = 0; i < 7; ++i ) {
        setValue( i, adcontrols::xic::xic_method{}, impl_->current_polarity_ );
        // 0 (blue) := TIC
        // using adplot::constants::chromatogram::color_table;
        // model->setHeaderData( i, Qt::Vertical, QBrush( color_table[ i + 1 ] ), Qt::BackgroundRole );
        // model->setHeaderData( i, Qt::Vertical, i + 1 );
    }

    connect( this, &QTableView::customContextMenuRequested, this, &XChromatogramsTable::handleContextMenu );
    connect( model, &QStandardItemModel::dataChanged, this, &XChromatogramsTable::handleDataChanged );

    connect( verticalHeader(),  &QHeaderView::sectionMoved
             , this
             , [&](int logicalIndex, int oldVisualIndex, int newVisualIndex){
                 QSignalBlocker block( verticalHeader() );
                 auto m = getValue();
                 verticalHeader()->moveSection( newVisualIndex, oldVisualIndex );
                 setValue( m );
                 emit valueChanged();
             });

    connect( model, &QAbstractItemModel::rowsInserted, this, [&](const QModelIndex&, int first, int last ){
        color_legend::update_color_legends( impl_->model_, first );
        emit valueChanged();
    });

    connect( model, &QAbstractItemModel::rowsRemoved, this, [&](const QModelIndex&, int first, int last ){
        color_legend::update_color_legends( impl_->model_, first );
        emit valueChanged();
    });
}

void
XChromatogramsTable::setValue( int row, const adcontrols::xic::xic_method& m, adcontrols::ion_polarity polarity )
{
    using adportable::index_of;
    auto model = impl_->model_;

    QSignalBlocker block( model );

    if ( row >= model->rowCount() ) {
        model->setRowCount( row + 1 );
    }

    using adplot::constants::chromatogram::color_table;

    impl_->current_polarity_ = polarity;
    model->setData( model->index( row, c_synonym ), QString::fromStdString( m.synonym() ) );
    model->setData( model->index( row, c_formula ), QString::fromStdString( m.formula() ) );
    if ( auto item = model->item( row, c_formula ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable | item->flags() );
        model->setData( model->index( row, c_formula ), m.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }
    adducts_type adducts( m.adduct_ );
    model->setData( model->index( row, c_adducts ), QVariant::fromValue( adducts ), Qt::UserRole + 1 );
    model->setData( model->index( row, c_adducts ), adducts.get( polarity ) );

    model->setData( model->index( row, c_mass ), m.mass() );
    model->setData( model->index( row, c_masswindow ), m.mass_window() );
    model->setData( model->index( row, c_time ), m.time() );
    model->setData( model->index( row, c_timewindow ), m.time_window() );
    model->setData( model->index( row, c_algo ), m.algo() );
    model->setData( model->index( row, c_protocol ), m.protocol() );
    model->setData( model->index( row, c_smiles ), QString::fromStdString( m.smiles() ) );

    if ( !m.smiles().empty() ) {
        if ( auto d = MolTableHelper::SmilesToSVG()( m.smiles() ) ) {
            auto [formula,svg] = *d;
            model->setData( model->index( row, adportable::index_of< col_svg, column_list >::value ), svg );
         }
    }

    if ( row < 7 ) {
        const auto& color = color_table[ row + 1 ];
        model->setData( model->index( row, adportable::index_of< col_svg, column_list >::value ), QBrush(color), Qt::BackgroundRole );
    } else {
        model->setData( model->index( row, c_svg ), QVariant{}, Qt::BackgroundRole );
    }
    resizeColumnToContents( c_formula );
}

void
XChromatogramsTable::setValue( const adcontrols::XChromatogramsMethod& xm )
{
    QSignalBlocker block( impl_->model_ );

    if ( verticalHeader()->sectionsMoved() ) {
        ADDEBUG() << "========== setValue sectionsMoved: " << verticalHeader()->sectionsMoved();
    }

    size_t row(0);
    for ( const auto& m: xm.xics() )
        setValue( row++, m, xm.polarity() );
}

adcontrols::XChromatogramsMethod
XChromatogramsTable::getValue() const
{
    adcontrols::XChromatogramsMethod m;
    getContents( m );
    return m;
}

void
XChromatogramsTable::getContents( adcontrols::XChromatogramsMethod& xm ) const
{
    auto model = impl_->model_;
    // xm.xics().resize( model->rowCount() );
    xm.xics().clear();

    for ( size_t i = 0; i < model->rowCount(); ++i ) {

        int row = verticalHeader()->logicalIndex( i );

        auto synonym = model->index( row, c_synonym ).data().toString().toStdString();
        auto formula = model->index( row, c_formula ).data().toString().toStdString();
        bool enable = model->index( row, c_formula ).data( Qt::CheckStateRole ).toBool();
        auto adducts = model->index( row, c_adducts ).data( Qt::UserRole + 1 ).value< adducts_type >();
        auto smiles = model->index( row, c_smiles ).data().toString().toStdString();
        auto mass = model->index( row, c_mass ).data().toDouble();
        auto mass_window = model->index( row, c_masswindow ).data().toDouble();
        auto time = model->index( row, c_time ).data().toDouble();
        auto time_window = model->index( row, c_timewindow ).data().toDouble();
        auto algo = model->index( row, c_algo ).data().toInt();

        adcontrols::xic::xic_method x;

        x.mol_ = std::make_tuple( enable, synonym, formula, smiles );
        x.adduct_ = adducts.adducts; // std::tuple

        x.mass_window_ = std::make_pair( mass, mass_window );
        x.time_window_ = std::make_pair( time, time_window );
        x.algo( adcontrols::xic::eIntensityAlgorithm( algo ) );

        xm.xics().emplace_back( x );
    }
}

void
XChromatogramsTable::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > sp )
{
    impl_->massSpectrometer_ = sp;
}

void
XChromatogramsTable::handlePolarity( adcontrols::ion_polarity polarity )
{
    if ( impl_->current_polarity_ != polarity ) {
        impl_->current_polarity_ = polarity;

        auto model = impl_->model_;

        for ( int row = 0; row < model->rowCount(); ++row ) {
            auto adducts = model->index( row, c_adducts ).data( Qt::UserRole + 1 ).value< adducts_type >();
            model->setData( model->index( row, c_adducts ), adducts.get( polarity ) );
            impl_->formulaChanged( row );
        }
    }
    this->viewport()->repaint();
}

void
XChromatogramsTable::handleDataChanged( const QModelIndex& index, const QModelIndex& indexLast )
{
    QSignalBlocker block( impl_->model_ );

    if ( index.column() == c_adducts ) {
        auto model = impl_->model_;
        auto adducts = model->index( index.row(), c_adducts ).data( Qt::UserRole + 1 ).value< adducts_type >();
        adducts.set( index.data().toString(), impl_->current_polarity_ );
        model->setData( model->index( index.row(), c_adducts ), QVariant::fromValue( adducts ), Qt::UserRole + 1 );
        impl_->formulaChanged( index.row() );
    }
    if ( index.column() == c_formula ) {
        impl_->formulaChanged( index.row() );
        resizeColumnToContents( c_formula );
    }

    if ( index.column() == c_mass || index.column() == c_masswindow ) {
        impl_->massChanged( index.row() );
    }

    if ( index.column() == c_time || index.column() == c_timewindow ) {
        impl_->timeChanged( index.row() );
    }
    emit valueChanged();
}

void
XChromatogramsTable::impl::formulaChanged( int row )
{
    bool enable = model_->index( row, c_formula ).data( Qt::CheckStateRole ).toBool();
    auto formula = model_->index( row, c_formula ).data().toString().toStdString();
    auto adduct  = model_->index( row, c_adducts ).data().toString().toStdString();

    if ( auto item = model_->item( row, c_formula ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        model_->setData( model_->index( row, c_formula ), enable ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }

    auto vec = adcontrols::ChemicalFormula::standardFormulae( formula, adduct );
    if ( !vec.empty() ) {
        const double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( vec.at( 0 ) );
        model_->setData( model_->index( row, c_mass ), mass );
        massChanged( row );
    }
}

void
XChromatogramsTable::impl::massChanged( int row )
{
    const double mass = model_->index( row, c_mass ).data().toDouble();
    const double mass_window = model_->index( row, c_masswindow ).data().toDouble();

    // update time, time_window
    if ( auto sp = massSpectrometer_.lock() ) {
        double time = sp->timeFromMass( model_->index( row, c_mass ).data().toDouble() );
        model_->setData( model_->index( row, c_time ), time );

        double time_window = sp->timeFromMass( mass + mass_window / 2.0 ) - sp->timeFromMass( mass - mass_window / 2.0 );
        model_->setData( model_->index( row, c_timewindow ), time_window );
    }
}

void
XChromatogramsTable::impl::timeChanged( int row )
{
    const double time = model_->index( row, c_time ).data().toDouble();   // display role
    const double time_window = model_->index( row, c_timewindow ).data().toDouble(); // display role

    if ( auto sp = massSpectrometer_.lock() ) {
        double mass = sp->assignMass( time );
        model_->setData( model_->index( row, c_mass ), mass );

        double mass_window = sp->assignMass( time + time_window / 2.0 ) - sp->assignMass( time - time_window / 2.0 );
        model_->setData( model_->index( row, c_masswindow ), mass_window );
    }
}


void
XChromatogramsTable::handleContextMenu( const QPoint& pt )
{
    QMenu menu;

    menu.addAction( tr( "Set adducts if empty." ), this, SLOT( handleSetAdducts() ) );

    addActionsToContextMenu( menu, pt );

    menu.exec( mapToGlobal( pt ) );
}

void
XChromatogramsTable::handleSetAdducts()
{
    auto model = impl_->model_;
    QSignalBlocker block( model );
    for ( int row = 0; row < model->rowCount(); ++row ) {
        auto adducts = model->index( row, c_adducts ).data( Qt::UserRole + 1 ).value< adducts_type >();
        bool dirty( false );
        if ( adducts.get( adcontrols::polarity_positive ).isEmpty() ) {
            dirty = true;
            adducts.set( "+[H]+", adcontrols::polarity_positive );
        } else if ( adducts.get( adcontrols::polarity_negative ).isEmpty() ) {
            dirty = true;
            adducts.set( "-[H]+", adcontrols::polarity_negative );
        }
        if ( dirty ) {
            model->setData( model->index( row, c_adducts ), QVariant::fromValue( adducts ), Qt::UserRole + 1 );
            model->setData( model->index( row, c_adducts ), adducts.get( impl_->current_polarity_ ) );
            impl_->formulaChanged( row );
        }
    }
}

void
XChromatogramsTable::handlePaste()
{
    auto model = impl_->model_;

    int row = model->rowCount() - 1;

    if ( selectionModel()->hasSelection() && selectionModel()->currentIndex().isValid() )
        row = selectionModel()->currentIndex().row();

    if ( auto mols = MolTableHelper::paste() ) {
        model->insertRows( row, mols->data().size() );

        paste_handler paste( impl_->current_polarity_ );

        QSignalBlocker block( model );
        for ( const auto& value: mols->data() )
            setValue( row++, paste( value ), impl_->current_polarity_ );
    }

    color_legend::update_color_legends( impl_->model_, row );
}


void
XChromatogramsTable::handleCopyToClipboard()
{
    auto model = impl_->model_;
    std::set< int > rows;
	for ( const auto& index: selectionModel()->selectedIndexes() )
        rows.emplace( index.row() );

    adcontrols::moltable mol;
    for ( const auto& i: rows ) {
        int row = verticalHeader()->logicalIndex( i );
        adcontrols::moltable::value_type value;
        auto synonym = model->index( row, c_synonym ).data().toString().toStdString();
        auto formula = model->index( row, c_formula ).data().toString().toStdString();
        bool enable = model->index( row, c_formula ).data( Qt::CheckStateRole ).toBool();
        auto adducts = model->index( row, c_adducts ).data( Qt::UserRole + 1 ).value< adducts_type >();
        auto smiles = model->index( row, c_smiles ).data().toString().toStdString();
        auto mass = model->index( row, c_mass ).data().toDouble();
        auto mass_window = model->index( row, c_masswindow ).data().toDouble();
        auto time = model->index( row, c_time ).data().toDouble();
        auto time_window = model->index( row, c_timewindow ).data().toDouble();
        auto algo = model->index( row, c_algo ).data().toInt();
        value.enable() = enable;
        value.synonym() = synonym;
        value.formula() = formula;
        value.adducts() = adducts.adducts;
        value.mass()    = mass;
        value.smiles()  = smiles;
        mol << value;
    }

    auto json = QString::fromStdString( boost::json::serialize( boost::json::value_from( mol ) ) );

    if ( auto md = new QMimeData() ) {
        md->setData( QLatin1String( "application/json" ), json.toUtf8() );
        // workaround for x11
        if ( QApplication::keyboardModifiers() & Qt::ShiftModifier )
            md->setText( json );
        // else
        //     md->setText( selected_text );
        QApplication::clipboard()->setMimeData( md, QClipboard::Clipboard );
    }
}

///////
