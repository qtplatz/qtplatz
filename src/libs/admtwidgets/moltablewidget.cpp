/**************************************************************************
** Copyright (C) 2010- Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "moltablewidget.hpp"
#include <adwidgets/moltableview.hpp>
#include <adwidgets/targetingadducts.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/countingmethod.hpp>
#include <adcontrols/element.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adportable/is_type.hpp>
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QMenu>
#include <QStandardItemModel>
#include <QSplitter>
#include <QGroupBox>
#include <ratio>
#include <cmath>

using namespace adwidgets;
using namespace admtwidgets;

namespace {
    enum {
        c_formula
        , c_mass
        , c_time
        , c_laps
        , c_tdiff
        , ncols
    };
}

namespace admtwidgets {

    class MolTableWidget::impl {
        MolTableWidget * this_;
    public:
        std::unique_ptr< QStandardItemModel > model_;
        std::weak_ptr< const adcontrols::MassSpectrometer > spectrometer_;
    public:
        impl( MolTableWidget * p ) : this_( p )
                                   , model_( std::make_unique< QStandardItemModel >() ) {
            model_->setColumnCount( ncols );
            model_->setHeaderData( c_formula,    Qt::Horizontal, QObject::tr( "Formula" ) );
            model_->setHeaderData( c_mass,       Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
            model_->setHeaderData( c_time,       Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
            model_->setHeaderData( c_laps,       Qt::Horizontal, QObject::tr( "lap#" ) );
            model_->setHeaderData( c_tdiff,      Qt::Horizontal, QObject::tr( "dt" ) );
        }

        void dataChanged( const QModelIndex& _1, const QModelIndex& _2 ) {
            ADDEBUG() << _1.row() << ", " << _1.column();
        }

        //-----------
        void handleContextMenu( const QPoint& pt ) {
            QMenu menu;
            typedef std::pair< QAction *, std::function< void() > > action_type;

            if ( auto table = this_->findChild< MolTableView * >() ) {
                std::vector< action_type > actions;
                actions.push_back( std::make_pair( menu.addAction( "add line" ), [&](){ addLine(); }) );
                if ( QAction * selected = menu.exec( table->mapToGlobal( pt ) ) ) {
                    auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){ return t.first == selected; });
                    if ( it != actions.end() )
                        (it->second)();
                }
            }
        }

        //-----------
        void addLine() {
            model_->insertRow( model_->rowCount() );
        }

        //-------------------
        void setTime( int row, double mass ) {
            if ( auto sp = spectrometer_.lock() ) {
                if ( auto scanlaw = sp->scanLaw() ) {
                    int32_t nlaps =  model_->index( row, c_laps ).data( Qt::EditRole ).toUInt();
                    double tof = scanlaw->getTime( mass, nlaps );
                    model_->setData( model_->index( row, c_time ), tof * std::micro::den, Qt::EditRole );
                }
                deltaTime();
            }
        }

        void deltaTime() {
            if ( model_->rowCount() > 2 ) {
                std::vector< double > times;
                for ( size_t i = 0; i < model_->rowCount(); ++i )
                    times.emplace_back( model_->index( i, c_time ).data( Qt::EditRole ).toDouble() );
                auto it = std::min_element( times.begin(), times.end() );
                for ( size_t i = 0; i < model_->rowCount(); ++i )
                    model_->setData( model_->index( i, c_tdiff ), times[i] - *it );
            }
        }
    };

}

MolTableWidget::MolTableWidget(QWidget *parent) : QWidget(parent)
                                                , impl_( std::make_unique< impl >( this ) )
{
    if ( QVBoxLayout * topLayout = new QVBoxLayout( this ) ) {

        topLayout->setMargin(5);
        topLayout->setSpacing(4);

        auto gbox = new QGroupBox;
        gbox->setTitle( tr( "Enable ion list" ) );
        gbox->setCheckable( true );
        gbox->setChecked( true );
        connect( gbox, &QGroupBox::clicked, [this]( bool checked ){ emit valueChanged( -1, MolTableEnable, checked ); } );

        topLayout->addWidget( gbox );

        if ( QVBoxLayout * vboxLayout = new QVBoxLayout( gbox ) ) {
            vboxLayout->setMargin( 2 );
            vboxLayout->setSpacing( 2 );

            auto table = new MolTableView();
            vboxLayout->addWidget( table );

            table->setModel( impl_->model_.get() );
            table->setContextMenuHandler( [this]( const QPoint& pt ){ impl_->handleContextMenu( pt ); } );
            table->setColumnField( c_formula, ColumnState::f_formula, true, true );
            table->setPrecision( c_mass, 4 );
            table->setPrecision( c_time, 3 );
            table->setColumnField( c_laps, ColumnState::f_uint, true, false );

            // item changed
            connect( impl_->model_.get(), &QStandardItemModel::itemChanged, this, &MolTableWidget::handleItemChanged );

        }
    }
}

MolTableWidget::~MolTableWidget()
{
}

void
MolTableWidget::OnCreate( const adportable::Configuration& )
{
}

void
MolTableWidget::OnInitialUpdate()
{
    if ( auto table = findChild< MolTableView *>() ) {
        table->onInitialUpdate();
    }
}

void
MolTableWidget::onUpdate( boost::any&& )
{
}

void
MolTableWidget::OnFinalClose()
{
}

bool
MolTableWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>( a );

        adcontrols::CountingMethod m;
        if ( getContents( m ) )
            ptr->append( m );

        return true;
    }

    return false;
}

bool
MolTableWidget::setContents( boost::any&& a )
{
    if ( auto pi = adcontrols::ControlMethod::any_cast<>( )( a, adcontrols::CountingMethod::clsid() ) ) {

        adcontrols::CountingMethod m;

        if ( pi->get<>( *pi, m ) )
            return setContents( m );

    }
    return false;
}

bool
MolTableWidget::getContents( adcontrols::CountingMethod& t ) const
{
    // t.clear();

    // adcontrols::CountingMethod::value_type v;
    // for ( int i = 0; i < model_->rowCount(); ++i ) {
    //     // CountingHelper::readRow( i, v, *model_ );
    //     t << std::move( v );
    // }

    // if ( auto gbox = findChild< QGroupBox * >() )
    //     t.setEnable( gbox->isChecked() );
    return true;
}

bool
MolTableWidget::setContents( const adcontrols::CountingMethod& t )
{
    // model_->setRowCount( int( t.size() ) );

    // int idx(0);

    // for ( const auto& value: t )
    //     CountingHelper::setRow( idx++, value, *model_ );

    if ( auto gbox = findChild< QGroupBox * >() )
        gbox->setChecked( t.enable() );

    return true;
}

// void
// MolTableWidget::setup( MolTableView * table )
// {
    // model_ = std::make_unique< QStandardItemModel >();
    // model_->setColumnCount( 6 );

    // model_->setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "Formula" ) );
    // model_->setHeaderData( c_mass, Qt::Horizontal, QObject::tr( "<i>m/z<i>" ) );
    // model_->setHeaderData( c_time, Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
    // model_->setHeaderData( c_laps, Qt::Horizontal, QObject::tr( "#lap" ) );

    // table->setModel( model_.get() );

    // table->setColumnField( c_formula, ColumnState::f_formula, true, true ); // editable, checkable
    // table->setColumnField( c_mass, ColumnState::f_mass, false, false );     // not editable
    // table->setColumnField( c_time, ColumnState::f_time );
    // table->setPrecision( c_time, 3 );
    // table->setColumnField( c_laps, ColumnState::f_uint, true, false );

    // table->setContextMenuHandler( [&]( const QPoint& pt ){
    //         QMenu menu;
    //         menu.addAction( tr( "Add row" ), this, SLOT( addRow() ) );
    //         menu.addAction( tr( "Compute time-of-flight" ), this, SLOT( handleComputeTof() ) );
    //         if ( auto table = findChild< MolTableView * >() )
    //             menu.exec( table->mapToGlobal( pt ) );
    //     });
//}

void
MolTableWidget::addRow()
{
    // int row = model_->rowCount();
    // model_->setRowCount( row + 1 );
    // adcontrols::CountingMethod::value_type v( std::make_tuple( true, std::string(), std::make_pair( 0.0, 0.1e-6 ), 0 ) );
    // if ( row > 0 )
    //     CountingHelper::readRow( row - 1, v, *model_ );
    // CountingHelper::setRow( row, v, *model_ );
}

void
MolTableWidget::handleComputeTof()
{
    ADDEBUG() << "handleComputeTof";
    if ( auto table = findChild< MolTableView *>() ) {
        auto index = table->currentIndex();
        if ( index.isValid() ) {
            //double mass = model_->index( index.row(), c_mass ).data( Qt::EditRole ).toDouble();
            if ( auto sp = impl_->spectrometer_.lock() )
                // CountingHelper::setTime( index.row(), mass, sp, *model_ );
                ;
        }
    }
}

void
MolTableWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p )
{
    ADDEBUG() << __FUNCTION__;
    impl_->spectrometer_ = p;
}

void
MolTableWidget::setValue( int row, column_type column, const QVariant& value )
{
    // QSignalBlocker block( model_.get() );

    // if ( column == MolTableWidget::CountingEnable ) {
    //     model_->setData( model_->index( row, c_formula ), value, Qt::CheckStateRole );
    // } else if ( column == MolTableWidget::CountingFormula ) {
    //     model_->setData( model_->index( row, c_formula ), value, Qt::EditRole );
    // } else if ( column == MolTableWidget::CountingRangeFirst ) {
    //     model_->setData( model_->index( row, c_time ), value, Qt::EditRole );
    // } else if ( column == MolTableWidget::CountingRangeWidth ) {
    //     model_->setData( model_->index( row, c_width ), value, Qt::EditRole );
    // } else {
    //     model_->setData( model_->index( row, int(column) ), value, Qt::EditRole );
    // }
}

QVariant
MolTableWidget::value( int row, column_type column ) const
{
    // if ( column == MolTableWidget::CountingEnable )
    //     return model_->index( row, c_formula ).data( Qt::CheckStateRole );
    // else if ( column == MolTableWidget::CountingFormula )
    //     return model_->index( row, c_formula ).data( Qt::EditRole );
    // else if ( column == MolTableWidget::CountingRangeFirst || column == MolTableWidget::CountingRangeWidth )
    //     return model_->index( row, c_formula ).data( Qt::EditRole );
    // else
    //     return model_->index( row, int( column ) ).data( Qt::EditRole );
    return {};
}

void
MolTableWidget::handleItemChanged( const QStandardItem * item )
{
    auto index = item->index();

    ADDEBUG() << "------- " << __FUNCTION__ << "  index: " << index.row() << ", " << index.column();

    // int prev = model_->index( index.row(), c_formula ).data( Qt::CheckStateRole ).toInt();
    auto& model = *impl_->model_;

    if ( index.column() == c_formula ) {
        QSignalBlocker block( impl_->model_.get() );
        ADDEBUG() << index.data().toString().toStdString();
        std::vector< adcontrols::mol::element > elements;
        int charge;
        if ( adcontrols::ChemicalFormula::getComposition( elements, index.data().toString().toStdString(), charge ) && charge == 0 ) {
            QString f = QString( "[%1]+" ).arg( index.data().toString() );
            model.setData( index, f, Qt::EditRole );
        }
        double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( impl_->model_->data( index, Qt::EditRole ).toString().toStdString() );
        //double prev = impl_->model_->data( model.index( index.row(), c_mass ), Qt::EditRole ).toDouble();
        model.setData( model.index( index.row(), c_mass ), mass, Qt::EditRole );
        impl_->setTime( index.row(), mass );
    }
    if ( index.column() == c_laps ) {
        double mass = model.index( index.row(), c_mass ).data( Qt::EditRole ).toDouble();
        impl_->setTime( index.row(), mass );
    }

    // } else if ( index.column() == c_time ) {
    //     emit valueChanged( index.row(), column_type( index.column() ), index.data( Qt::EditRole ).toDouble() / std::micro::den );
    // } else
    //     emit valueChanged( index.row(), column_type( index.column() ), index.data( Qt::EditRole ) );
}

// bool
// CountingHelper::readRow( int row, adcontrols::CountingMethod::value_type& v, const QStandardItemModel& model )
// {
//     using adcontrols::CountingMethod;

//     std::get< CountingMethod::CountingEnable >( v ) = model.index( row, 0 ).data( Qt::CheckStateRole ).toBool();
//     std::get< CountingMethod::CountingFormula >( v ) = model.index( row, 0 ).data( Qt::EditRole ).toString().toStdString();
//     std::get< CountingMethod::CountingRange >( v ).first = model.index( row, 2 ).data( Qt::EditRole ).toDouble() / std::micro::den;
//     std::get< CountingMethod::CountingRange >( v ).second = model.index( row, 3 ).data( Qt::EditRole ).toDouble() / std::micro::den;
//     std::get< CountingMethod::CountingProtocol >( v ) = model.index( row, 4 ).data( Qt::EditRole ).toInt();

//     return true;
// }

// bool
// CountingHelper::setRow( int row, const adcontrols::CountingMethod::value_type& v, QStandardItemModel& model )
// {
//     using adcontrols::CountingMethod;

//     QSignalBlocker block( &model );

//     auto formula = std::get< CountingMethod::CountingFormula >( v );
//     bool checked = std::get< CountingMethod::CountingEnable >( v );

//     model.setData( model.index( row, c_formula ), QString::fromStdString( formula ) );

//     if ( auto item = model.item( row, c_formula ) ) {
//         item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
//         model.setData( model.index( row, c_formula ), checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
//     }

//     if ( !formula.empty() )
//         model.setData( model.index( row, c_mass ), adcontrols::ChemicalFormula().getMonoIsotopicMass( formula ) );

//     model.setData( model.index( row, c_time ), std::get< CountingMethod::CountingRange >( v ).first * std::micro::den, Qt::EditRole );
//     model.setData( model.index( row, c_laps ), 0, Qt::EditRole );

//     if ( auto item = model.item( row, c_mass ) )
//         item->setEditable( false );

//     if ( auto item = model.item( row, c_laps ) )
//         item->setEditable( false );

//     return true;
// }
