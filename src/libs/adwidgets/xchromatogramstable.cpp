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

#include "xchromatogramstable.hpp"
#include "delegatehelper.hpp"
#include "htmlheaderview.hpp"
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
#include <adcontrols/targetingmethod.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <QApplication>
#include <QByteArray>
#include <QByteArray>
#include <QClipboard>
#include <QComboBox>
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

    enum columns { c_synonym, c_formula, c_adducts, c_mass, c_masswindow, c_time, c_timewindow, c_algo, c_protocol, c_smiles, ncolumns };

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
    public:
        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
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
            } else if ( index.column() == c_algo ) {
                QString text;
                switch( index.data().toInt() ) {
                case adcontrols::xic::ePeakAreaOnProfile:   text = "Area"; break;
                case adcontrols::xic::ePeakHeightOnProfile: text = "Height"; break;
                case adcontrols::xic::eCounting:            text = "Counts"; break;
                }
                painter->drawText( opt.rect, Qt::AlignCenter, text );
            } else {
                QStyledItemDelegate::paint( painter, opt, index );
            }
        }

        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {
            if ( index.column() == c_algo ) {
                auto cbx = new QComboBox( parent );
                cbx->addItems( { "Area", "Height", "Counts" } );
                cbx->setCurrentIndex( index.data().toInt() );
                return cbx;
            } else if ( index.column() == c_time ) {
                auto spin = new TimeSpinBox<std::micro>( parent );
                spin->setDecimals( 9 );  // ns resolution; QAbstractSpinBox rounds up value by specified decimals here.
                spin->setMaximum( 1e9 ); // 1000s
                return spin;
            } else if ( index.column() == c_timewindow ) {
                auto spin = new TimeSpinBox<std::nano>( parent );
                spin->setDecimals( 10 ); // 0.1ns resolution
                spin->setMaximum( 1e9 ); // 1000s
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
    impl_->model_->setColumnCount( ncolumns );
    impl_->model_->setRowCount( 8 );

    setModel( impl_->model_ );
    setItemDelegate( new delegate );

    setHorizontalHeader( new HtmlHeaderView );
    setSortingEnabled( false );
    // setAcceptDrops( true );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QTableView::customContextMenuRequested, this, &XChromatogramsTable::handleContextMenu );

    // //setColumnHidden( c_smiles, true );
}

XChromatogramsTable::~XChromatogramsTable()
{
}

void
XChromatogramsTable::onInitialUpdate()
{
    auto model = impl_->model_;
    model->setHeaderData( c_synonym,    Qt::Horizontal, QObject::tr( "Synonym" ) );
    model->setHeaderData( c_formula,    Qt::Horizontal, QObject::tr( "Formula" ) );
    model->setHeaderData( c_adducts,    Qt::Horizontal, QObject::tr( "Adducts" ) );
    model->setHeaderData( c_mass,       Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
    model->setHeaderData( c_masswindow, Qt::Horizontal, QObject::tr( "Window(Da)" ) );
    model->setHeaderData( c_time,       Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
    model->setHeaderData( c_timewindow, Qt::Horizontal, QObject::tr( "Window(ns)" ) );
    model->setHeaderData( c_algo,       Qt::Horizontal, QObject::tr( "Method(algo)" ) );
    model->setHeaderData( c_protocol,   Qt::Horizontal, QObject::tr( "Prot.#" ) );
    model->setHeaderData( c_smiles,     Qt::Horizontal, QObject::tr( "SMILES" ) );

    for ( int i = 0; i < 8; ++i )
        setValue( i, adcontrols::xic::xic_method{}, impl_->current_polarity_ );

    connect( model, &QStandardItemModel::dataChanged, this, &XChromatogramsTable::handleDataChanged );
}


void
XChromatogramsTable::setValue( int row, const adcontrols::xic::xic_method& m, adcontrols::ion_polarity polarity )
{
    auto model = impl_->model_;

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
    resizeColumnToContents( c_formula );
}

void
XChromatogramsTable::setValue( const adcontrols::XChromatogramsMethod& xm )
{
    size_t row(0);
    for ( const auto& m: xm.xics() ) {
        setValue( row++, m, xm.polarity() );
    }
}

void
XChromatogramsTable::getContents( adcontrols::XChromatogramsMethod& xm )
{
    auto model = impl_->model_;
    size_t nRows = std::min( 8, model->rowCount() );
    xm.xics().resize( nRows );
    for ( size_t row = 0; row < nRows; ++row ) {
        auto& x = xm.xics().at( row );

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

        x.mol_ = std::make_tuple( enable, synonym, formula, smiles );
        x.adduct_ = adducts.adducts; // std::tuple

        x.mass_window_ = std::make_pair( mass, mass_window );
        x.time_window_ = std::make_pair( time, time_window );
        x.algo( adcontrols::xic::eIntensityAlgorithm( algo ) );
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
