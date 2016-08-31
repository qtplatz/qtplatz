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

#include "countingwidget.hpp"
#include "moltableview.hpp"
#include "targetingadducts.hpp"
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

namespace adwidgets {

    using adcontrols::CountingMethod;
    
    struct CountingHelper {
        static bool readRow( int row, CountingMethod::value_type&, const QStandardItemModel& model );
        static bool setRow( int row, const CountingMethod::value_type&, QStandardItemModel& model );
        static bool setTime( int row, double mass, std::shared_ptr< const adcontrols::MassSpectrometer > sp, QStandardItemModel& model );
    };

    enum {
        c_formula
        , c_mass
        , c_time
        , c_width
        , c_protocol
        , c_laps
    };

}

using namespace adwidgets;

CountingWidget::CountingWidget(QWidget *parent) : QWidget(parent)
{
    if ( QVBoxLayout * topLayout = new QVBoxLayout( this ) ) {

        topLayout->setMargin(5);
        topLayout->setSpacing(4);

        auto gbox = new QGroupBox;
        gbox->setTitle( tr( "Counting range list" ) );
        gbox->setCheckable( true );
        connect( gbox, &QGroupBox::clicked, [this]( bool checked ){ emit valueChanged( -1, CountingEnable, checked ); } );

        topLayout->addWidget( gbox );
        
        if ( QVBoxLayout * vboxLayout = new QVBoxLayout( gbox ) ) {
            vboxLayout->setMargin( 2 );
            vboxLayout->setSpacing( 2 );

            auto moltable = new MolTableView();
            setup( moltable );

            // QDoubleSpinBox -->
            connect( moltable, &MolTableView::valueChanged, this, [&]( const QModelIndex& index, double value ){
                    QPair< double, double > range;
                    if ( index.column() == c_time ) {
                        range.first = value / std::micro::den;
                        range.second = model_->index( index.row(), c_width ).data( Qt::EditRole ).toDouble() / std::micro::den;
                    } else if ( index.column() == c_width ) {
                        range.first = model_->index( index.row(), c_time ).data( Qt::EditRole ).toDouble() / std::micro::den;
                        range.second = value / std::micro::den;
                    }
                    emit editChanged( index.row(), column_type( index.column() ), QVariant::fromValue( range ) );
                } );

            // check box state changed
            connect( moltable, &MolTableView::stateChanged, [&]( const QModelIndex& index, Qt::CheckState state ){
                    emit valueChanged( index.row(), CountingWidget::CountingEnable, state == Qt::Checked );
                });

            // rows removed
            connect( model_.get(), &QStandardItemModel::rowsRemoved, [&]( const QModelIndex&, int start, int end ){
                    emit rowsRemoved( start, end );
                });

            connect( model_.get(), &QStandardItemModel::itemChanged, this, &CountingWidget::handleItemChanged );
            
            vboxLayout->addWidget( moltable );
        }
    }
}

CountingWidget::~CountingWidget()
{
}

void
CountingWidget::OnCreate( const adportable::Configuration& )
{
}

void
CountingWidget::OnInitialUpdate()
{
    if ( auto table = findChild< MolTableView *>() ) {
        table->onInitialUpdate();
    }
}

void
CountingWidget::onUpdate( boost::any&& )
{
}

void
CountingWidget::OnFinalClose()
{
}

bool
CountingWidget::getContents( boost::any& a ) const
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
CountingWidget::setContents( boost::any&& a )
{
    if ( auto pi = adcontrols::ControlMethod::any_cast<>( )( a, adcontrols::CountingMethod::clsid() ) ) {

        adcontrols::CountingMethod m;

        if ( pi->get<>( *pi, m ) )
            return setContents( m );

    }
    return false;
}

bool
CountingWidget::getContents( adcontrols::CountingMethod& t ) const
{
    t.clear();

    adcontrols::CountingMethod::value_type v;
    for ( int i = 0; i < model_->rowCount(); ++i ) {
        CountingHelper::readRow( i, v, *model_ );
        t << std::move( v );
    }

    if ( auto gbox = findChild< QGroupBox * >() )
        t.setEnable( gbox->isChecked() );

    return true;
}

bool
CountingWidget::setContents( const adcontrols::CountingMethod& t )
{
    model_->setRowCount( t.size() );

    int idx(0);

    for ( const auto& value: t ) 
        CountingHelper::setRow( idx++, value, *model_ );

    if ( auto gbox = findChild< QGroupBox * >() )
        gbox->setChecked( t.enable() );

    return true;    
}

void
CountingWidget::setup( MolTableView * table )
{
    model_ = std::make_unique< QStandardItemModel >();
    model_->setColumnCount( 6 );

    model_->setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "Formula" ) );
    model_->setHeaderData( c_mass, Qt::Horizontal, QObject::tr( "<i>m/z<i>" ) );
    model_->setHeaderData( c_time, Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
    model_->setHeaderData( c_width, Qt::Horizontal, QObject::tr( "Width(&mu;s)" ) );
    model_->setHeaderData( c_protocol, Qt::Horizontal, QObject::tr( "Protocol" ) );
    model_->setHeaderData( c_laps, Qt::Horizontal, QObject::tr( "#lap" ) );

    table->setModel( model_.get() );
    
    table->setColumnField( c_formula, ColumnState::f_formula, true, true ); // checkable
    table->setColumnField( c_mass, ColumnState::f_mass, false, false );     // not editable
    table->setColumnField( c_time, ColumnState::f_time );
    table->setPrecision( c_time, 3 );
    table->setColumnField( c_width, ColumnState::f_time );
    table->setPrecision( c_width, 3 );
    table->setColumnField( c_protocol, ColumnState::f_protocol );
    table->setColumnField( c_laps, ColumnState::f_protocol, false, false );

    table->setContextMenuHandler( [&]( const QPoint& pt ){
            QMenu menu;
            menu.addAction( tr( "Add row" ), this, SLOT( addRow() ) );
            menu.addAction( tr( "Compute time-of-flight" ), this, SLOT( handleComputeTof() ) );
            if ( auto table = findChild< MolTableView * >() )
                menu.exec( table->mapToGlobal( pt ) );
        });
}

void
CountingWidget::addRow()
{
    int row = model_->rowCount();
    model_->setRowCount( row + 1 );
    adcontrols::CountingMethod::value_type v( std::make_tuple( true, std::string(), std::make_pair( 0.0, 0.1e-6 ), 0 ) );
    if ( row > 0 )
        CountingHelper::readRow( row - 1, v, *model_ );
    CountingHelper::setRow( row, v, *model_ );
}

void
CountingWidget::handleComputeTof()
{
    if ( auto table = findChild< MolTableView *>() ) {
        auto index = table->currentIndex();
        if ( index.isValid() ) {
            double mass = model_->index( index.row(), c_mass ).data( Qt::EditRole ).toDouble();
            if ( auto sp = spectrometer_.lock() )
                CountingHelper::setTime( index.row(), mass, sp, *model_ );
        }
    }
}

void
CountingWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p )
{
    spectrometer_ = p;
}

void
CountingWidget::setValue( int row, column_type column, const QVariant& value )
{
    QSignalBlocker block( model_.get() );
    
    if ( column == CountingWidget::CountingEnable ) {
        model_->setData( model_->index( row, c_formula ), value, Qt::CheckStateRole );
    } else if ( column == CountingWidget::CountingFormula ) {
        model_->setData( model_->index( row, c_formula ), value, Qt::EditRole );
    } else if ( column == CountingWidget::CountingRangeFirst ) {
        model_->setData( model_->index( row, c_time ), value, Qt::EditRole );
    } else if ( column == CountingWidget::CountingRangeWidth ) {
        model_->setData( model_->index( row, c_width ), value, Qt::EditRole );        
    } else {
        model_->setData( model_->index( row, int(column) ), value, Qt::EditRole );
    }
}

QVariant
CountingWidget::value( int row, column_type column ) const
{
    if ( column == CountingWidget::CountingEnable )
        return model_->index( row, c_formula ).data( Qt::CheckStateRole );
    else if ( column == CountingWidget::CountingFormula )
        return model_->index( row, c_formula ).data( Qt::EditRole );
    else if ( column == CountingWidget::CountingRangeFirst || column == CountingWidget::CountingRangeWidth )
        return model_->index( row, c_formula ).data( Qt::EditRole );
    else
        return model_->index( row, int( column ) ).data( Qt::EditRole );
}

void
CountingWidget::handleItemChanged( const QStandardItem * item )
{
    auto index = item->index();

    int prev = model_->index( index.row(), c_formula ).data( Qt::CheckStateRole ).toInt();
    
    if ( index.column() == c_formula ) {
        QSignalBlocker block( model_.get() );        

        std::vector< adcontrols::mol::element > elements;
        int charge;
        if ( adcontrols::ChemicalFormula::getComposition( elements, index.data().toString().toStdString(), charge ) && charge == 0 ) {
            QString f = QString( "[%1]+" ).arg( index.data().toString() );
            model_->setData( index, f, Qt::EditRole );
        }
        double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( model_->data( index, Qt::EditRole ).toString().toStdString() );
        double prev = model_->data( model_->index( index.row(), c_mass ), Qt::EditRole ).toDouble();

        model_->setData( model_->index( index.row(), c_mass ), mass, Qt::EditRole );        

        if ( std::abs( mass - prev ) >= 1.0 ) {
            CountingHelper::setTime( index.row(), mass, spectrometer_.lock(), *model_ );
            emit valueChanged( index.row(), CountingWidget::CountingFormula, index.data( Qt::EditRole ) );
        }

    } else if ( index.column() == c_protocol ) {

        int prev = model_->data( model_->index( index.row(), c_laps ), Qt::EditRole ).toInt();

        emit valueChanged( index.row(), column_type( index.column() ), index.data( Qt::EditRole ) );

        if ( prev != model_->data( model_->index( index.row(), c_laps ), Qt::EditRole ).toInt() ) {
            double mass = model_->data( model_->index( index.row(), c_mass ), Qt::EditRole ).toDouble();
            CountingHelper::setTime( index.row(), mass, spectrometer_.lock(), *model_ );
        }

    } else if ( index.column() == c_time || index.column() == c_width ) {
        emit valueChanged( index.row(), column_type( index.column() ), index.data( Qt::EditRole ).toDouble() / std::micro::den );
    } else
        emit valueChanged( index.row(), column_type( index.column() ), index.data( Qt::EditRole ) );
}

bool
CountingHelper::readRow( int row, adcontrols::CountingMethod::value_type& v, const QStandardItemModel& model )
{
    using adcontrols::CountingMethod;

    std::get< CountingMethod::CountingEnable >( v ) = model.index( row, 0 ).data( Qt::CheckStateRole ).toBool();
    std::get< CountingMethod::CountingFormula >( v ) = model.index( row, 0 ).data( Qt::EditRole ).toString().toStdString();
    std::get< CountingMethod::CountingRange >( v ).first = model.index( row, 2 ).data( Qt::EditRole ).toDouble() / std::micro::den;
    std::get< CountingMethod::CountingRange >( v ).second = model.index( row, 3 ).data( Qt::EditRole ).toDouble() / std::micro::den;
    std::get< CountingMethod::CountingProtocol >( v ) = model.index( row, 4 ).data( Qt::EditRole ).toInt();

    return true;
}

bool
CountingHelper::setRow( int row, const adcontrols::CountingMethod::value_type& v, QStandardItemModel& model )
{
    using adcontrols::CountingMethod;

    QSignalBlocker block( &model );

    auto formula = std::get< CountingMethod::CountingFormula >( v );
    bool checked = std::get< CountingMethod::CountingEnable >( v );

    model.setData( model.index( row, c_formula ), QString::fromStdString( formula ) );
    
    if ( auto item = model.item( row, c_formula ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        model.setData( model.index( row, c_formula ), checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }

    if ( !formula.empty() )
        model.setData( model.index( row, c_mass ), adcontrols::ChemicalFormula().getMonoIsotopicMass( formula ) );
     
    model.setData( model.index( row, c_time ), std::get< CountingMethod::CountingRange >( v ).first * std::micro::den, Qt::EditRole );
    model.setData( model.index( row, c_width ), std::get< CountingMethod::CountingRange >( v ).second * std::micro::den, Qt::EditRole );
    model.setData( model.index( row, c_protocol ), std::get< CountingMethod::CountingProtocol >( v ), Qt::EditRole );
    model.setData( model.index( row, c_laps ), 0, Qt::EditRole );

    if ( auto item = model.item( row, c_mass ) )
        item->setEditable( false );    

    if ( auto item = model.item( row, c_laps ) )
        item->setEditable( false );

    return true;
}

bool
CountingHelper::setTime( int row, double mass, std::shared_ptr< const adcontrols::MassSpectrometer > sp, QStandardItemModel& model )
{
    if ( sp ) {
        if ( auto scanlaw = sp->scanLaw() ) {
            double tof = scanlaw->getTime( mass, model.data( model.index( row, c_laps ) ).toInt() );
            model.setData( model.index( row, c_time ), tof * std::micro::den, Qt::EditRole );
        }
    }
}
    
