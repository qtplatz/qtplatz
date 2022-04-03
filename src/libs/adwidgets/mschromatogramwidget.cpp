/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mschromatogramwidget.hpp"
#include "mschromatogramform.hpp"
#include "mschromatogramtable.hpp"
#include "moltableview.hpp"
#include "moltablehelper.hpp"
#include "targetingadducts.hpp"
#if HAVE_RDKit
# include <adchem/drawing.hpp>
# include <adchem/mol.hpp>
#endif
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/moltable.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <boost/exception/all.hpp>
#include <QSplitter>
#include <QStandardItemModel>
#include <QBoxLayout>
#include <QMenu>
#include <boost/json.hpp>
#include <fstream>

namespace {
    struct helper {
        static bool readRow( int row, adcontrols::moltable::value_type&, const QStandardItemModel& model );
        static bool setRow( int row, const adcontrols::moltable::value_type&, QStandardItemModel& model );
    };
}

namespace adwidgets {

    enum {
        c_formula
        , c_adducts
        , c_mass
        , c_tR
        , c_lockmass
        , c_protocol
        , c_synonym
        , c_memo
        , c_svg
        , c_smiles
        , column_count
    };

    class MSChromatogramWidget::impl {
    public:
        std::unique_ptr< QStandardItemModel > model_;

        impl() : model_( std::make_unique< QStandardItemModel >() ) {
        }
    };

}

using namespace adwidgets;

MSChromatogramWidget::MSChromatogramWidget(QWidget *parent) : QWidget(parent)
                                                            , impl_( std::make_unique< impl >() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new MSChromatogramForm ) );
            auto table = new MSChromatogramTable(); //new MolTableView();
            // setup( table );
            splitter->addWidget( table );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }

    if ( auto form = findChild< MSChromatogramForm * >() ) {

        if ( auto table = findChild< MolTableView *>() )
            connect( form, &MSChromatogramForm::onEnableLockMass
                     , [table]( bool enable ) { table->setColumnHidden( c_lockmass, !enable ); } );

        connect( form, &MSChromatogramForm::triggerProcess, [this] { run(); } );

        if ( auto table = findChild< MSChromatogramTable *>() ) {
            connect( form, &MSChromatogramForm::polarityToggled, table, &MSChromatogramTable::handlePolarity );
        }
    }
}

MSChromatogramWidget::~MSChromatogramWidget()
{
}

QWidget *
MSChromatogramWidget::create( QWidget * parent )
{
    return new MSChromatogramWidget( parent );
}

void
MSChromatogramWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSChromatogramWidget::OnInitialUpdate()
{
    if ( auto form = findChild< MSChromatogramForm * >() )
        form->OnInitialUpdate();

    if ( auto table = findChild< MolTableView *>() )
        table->onInitialUpdate();

    if ( auto table = findChild< MSChromatogramTable * >() )
        table->onInitialUpdate();
}

void
MSChromatogramWidget::onUpdate( boost::any&& )
{
}

void
MSChromatogramWidget::OnFinalClose()
{
}

bool
MSChromatogramWidget::getContents( boost::any& a ) const
{
    if ( auto pm = boost::any_cast<adcontrols::ProcessMethod *>( a ) ) {
        adcontrols::MSChromatogramMethod m;
        getContents( m );
        (*pm) *= m;
        return true;
    }
    return false;
}

bool
MSChromatogramWidget::setContents( boost::any&& a )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {

        const adcontrols::ProcessMethod& pm = boost::any_cast<adcontrols::ProcessMethod&>( a );

        if ( auto m = pm.find< adcontrols::MSChromatogramMethod >() ) {
            setContents( *m );
            return true;
        }
    }
    return false;
}

void
MSChromatogramWidget::setContents( const adcontrols::MSChromatogramMethod& m )
{
    if ( auto form = findChild< MSChromatogramForm * >() ) {
        form->setContents( m );
    }

    if ( auto table = findChild< MolTableView * >() ) {
        table->setColumnHidden( c_lockmass, !m.lockmass() );
    }
    if ( auto table = findChild< MSChromatogramTable * >() ) {
        table->setColumnHidden( col_lockmass{}, !m.lockmass() );
        table->setValue( m.molecules() );
    }

    size_t size = m.molecules().data().size();

    if ( size > impl_->model_->rowCount() )
        impl_->model_->setRowCount( m.molecules().data().size() );
    else
        impl_->model_->removeRows( size - 1, impl_->model_->rowCount() - size );

    int row( 0 );
    for ( auto& mol: m.molecules().data() )
        helper::setRow( row++, mol, *impl_->model_ );

    addRow();
}

bool
MSChromatogramWidget::getContents( adcontrols::MSChromatogramMethod& m ) const
{
    if ( auto form = findChild< MSChromatogramForm * >() )
        form->getContents( m );

    m.molecules().data().clear();

    for ( int row = 0; row < impl_->model_->rowCount(); ++row ) {
        adcontrols::moltable::value_type value;
        helper::readRow( row, value, *impl_->model_ );
        if ( !value.formula().empty() && value.mass() > 0 ) {
            // ADDEBUG() << boost::json::object{ { "row", row }, { "formula", value.formula() }
            //         , { "enable", value.enable() }, {"adducts", value.adducts() }, { "mass", value.mass() } };
            m.molecules() << value;
        }
    }

#if ! defined NDEBUG // || 1
    int cnt = 0;
    for ( const auto& value: m.molecules().data() ) {
        ADDEBUG() << boost::json::object{ { "row", cnt++}, { "formula", value.formula() }
                , { "enable", value.enable() }, {"adducts", value.adducts() }, { "mass", value.mass() } };
    }
#endif

    return true;
}

void
MSChromatogramWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Run generate chromatograms", this, SLOT( run() ) );
}

void
MSChromatogramWidget::run()
{
    emit triggerProcess( "MSChromatogramWidget" );
}

void
MSChromatogramWidget::setup( MolTableView * table )
{
    impl_->model_ = std::make_unique< QStandardItemModel >();
    impl_->model_->setColumnCount( column_count );

    impl_->model_->setHeaderData( c_formula,  Qt::Horizontal, QObject::tr( "Formula" ) );
    impl_->model_->setHeaderData( c_adducts,  Qt::Horizontal, QObject::tr( "adduct/lose" ) );
    impl_->model_->setHeaderData( c_mass,     Qt::Horizontal, QObject::tr( "<i>m/z<i>" ) );
    impl_->model_->setHeaderData( c_tR,       Qt::Horizontal, QObject::tr( "<i>t<sub>R</sub><i>" ) );
    impl_->model_->setHeaderData( c_lockmass, Qt::Horizontal, QObject::tr( "lock mass" ) );
    impl_->model_->setHeaderData( c_protocol, Qt::Horizontal, QObject::tr( "protocol#" ) );
    impl_->model_->setHeaderData( c_synonym,  Qt::Horizontal, QObject::tr( "synonym" ) );
    impl_->model_->setHeaderData( c_memo,     Qt::Horizontal, QObject::tr( "memo" ) );
    impl_->model_->setHeaderData( c_svg,      Qt::Horizontal, QObject::tr( "structure" ) );
    impl_->model_->setHeaderData( c_smiles,   Qt::Horizontal, QObject::tr( "SMILES" ) );

    table->setModel( impl_->model_.get() );

    //                                                          editable, checkable
    table->setColumnField( c_formula, ColumnState::f_formula,     true,  true );
    table->setColumnField( c_adducts, ColumnState::f_adducts,     true,  false );
    table->setColumnField( c_mass,    ColumnState::f_mass,        false, false );
    table->setColumnField( c_tR,      ColumnState::f_time,        true,  false );
    table->setColumnField( c_synonym, ColumnState::f_synonym,     false, false );
    table->setColumnField( c_memo,    ColumnState::f_description, false, false );
    table->setColumnField( c_protocol, ColumnState::f_protocol,   true,  false );
    table->setColumnField( c_svg,     ColumnState::f_svg );
    table->setColumnField( c_smiles,  ColumnState::f_smiles );

    table->setPrecision( c_tR, 3 );

    table->setContextMenuHandler( [&]( const QPoint& pt ){
            QMenu menu;
            menu.addAction( tr( "Add row" ), this, SLOT( addRow() ) );
            if ( auto table = findChild< MolTableView * >() )
                menu.exec( table->mapToGlobal( pt ) );
        });
    connect( impl_->model_.get(), &QAbstractItemModel::dataChanged, this, &MSChromatogramWidget::handleDataChanged );
    addRow();
}

void
MSChromatogramWidget::handleDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    QSignalBlocker block( impl_->model_.get() );

    if ( ( topLeft.column() <= c_formula && c_formula <= bottomRight.column() ) ||
         ( topLeft.column() <= c_adducts && c_adducts <= bottomRight.column() ) ) {

        for ( auto row = topLeft.row(); row <= bottomRight.row(); ++row ) {

            double mass = MolTableHelper::monoIsotopicMass( impl_->model_->index( row, c_formula ).data( Qt::EditRole ).toString()
                                                             , impl_->model_->index( row, c_adducts ).data( Qt::EditRole ).toString() );

            impl_->model_->setData( impl_->model_->index( row, c_mass ), mass, Qt::EditRole );
        }
    }

    if ( topLeft.column() <= c_smiles && c_smiles <= bottomRight.column() ) {

        for ( auto row = topLeft.row(); row <= bottomRight.row(); ++row ) {
            auto smiles = impl_->model_->index( row, c_smiles ).data( Qt::EditRole ).toString();
            if ( auto fsvg = MolTableHelper::SmilesToSVG()( smiles ) ) {
#if __cplusplus >= 201703L
                auto [ formula, svg ] = *fsvg;
#else
                QString formula;
                QByteArray svg;
                std::tie( formula, svg ) = *fsvg;
#endif
                double mass = MolTableHelper::monoIsotopicMass( formula, impl_->model_->index( row, c_adducts ).data( Qt::EditRole ).toString() );
                impl_->model_->setData( impl_->model_->index( row, c_svg ), svg );
                impl_->model_->setData( impl_->model_->index( row, c_formula ), formula, Qt::EditRole );
                impl_->model_->setData( impl_->model_->index( row, c_mass ), mass, Qt::EditRole );
                if ( auto item = impl_->model_->item( row, c_formula ) )
                    item->setEditable( false );
            }
        }

    }
}

void
MSChromatogramWidget::addRow()
{
    int row = impl_->model_->rowCount();
    impl_->model_->setRowCount( row + 1 );

    helper::setRow( row, adcontrols::moltable::value_type(), *impl_->model_ );
}

namespace {
    bool
    helper::readRow( int row, adcontrols::moltable::value_type& mol, const QStandardItemModel& model )
    {
        mol.formula() = model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdString();
        mol.enable()  = model.index( row, c_formula ).data( Qt::CheckStateRole ).toBool();
        mol.mass()    = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
        mol.abundance() = 1.0;
        mol.adducts() = model.index( row, c_adducts ).data( Qt::EditRole ).toString().toStdString();
        mol.synonym() = model.index( row, c_synonym ).data( Qt::EditRole ).toString().toStdString();
        mol.description() = model.index( row, c_memo ).data( Qt::EditRole ).toString().toStdWString();
        mol.setIsMSRef( model.index( row, c_lockmass ).data( Qt::CheckStateRole ).toBool() );
        mol.smiles()  = model.index( row, c_smiles ).data( Qt::EditRole ).toString().toStdString();
        int protocol = model.index( row, c_protocol ).data( Qt::EditRole ).toInt();
        mol.setProtocol( protocol >= 0 ? boost::optional< int32_t >( protocol ) : boost::none );

        double tR = model.index( row, c_tR ).data( Qt::EditRole ).toDouble();
        mol.set_tR( tR > 0 ? boost::optional< double >( tR ) : boost::none );

        return true;
    }

    bool
    ::helper::setRow( int row, const adcontrols::moltable::value_type& mol, QStandardItemModel& model )
    {
        {
            QSignalBlocker block( &model );

            if ( row <= model.rowCount() )
                model.setRowCount( row + 1 );

            if ( auto item = new QStandardItem() ) {
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                item->setEditable( true );
                item->setCheckState( mol.enable() ? Qt::Checked : Qt::Unchecked );
                model.setItem ( row, c_formula, item );
            }

            if ( !QString::fromStdString( mol.formula() ).isEmpty() && QString::fromStdString( mol.smiles() ).isEmpty() )
                model.setData( model.index( row, c_formula ), QString::fromStdString( mol.formula() ) );

            model.setData( model.index( row, c_adducts ), QString::fromStdString( mol.adducts() ) );
            model.setData( model.index( row, c_mass ), mol.mass() );
            model.setData( model.index( row, c_lockmass ), mol.isMSRef() );
            if ( auto item = model.item( row, c_lockmass ) ) {
                item->setEditable( false );
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                model.setData( model.index( row, c_lockmass ), mol.isMSRef() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
            } else {
                ADDEBUG() << "--------- empty item (c_lockmass) ------------- " << mol.formula();
            }
            model.setData( model.index( row, c_protocol ), mol.protocol() ? mol.protocol().get() : -1 );
            model.setData( model.index( row, c_tR ), mol.tR() ? mol.tR().get() : 0.0 );
            model.setData( model.index( row, c_synonym ), QString::fromStdString( mol.synonym() ) );
            model.setData( model.index( row, c_memo ), QString::fromStdWString( mol.description() ) );
        }

        if ( !QString::fromStdString( mol.smiles() ).isEmpty() )
            model.setData( model.index( row, c_smiles ), QString::fromStdString( mol.smiles() ) );

        return true;
    }
}
