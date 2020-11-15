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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <boost/optional.hpp>
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
        , c_apparent_mass
        , ncols
    };
}

namespace admtwidgets {

    struct lapFinder {
        const adcontrols::ScanLaw& scanlaw_;
        const double mass_;
        const int laps_;
        const double time_;
        const double tlap_;
        lapFinder( const adcontrols::ScanLaw& law, double mass, double time, int laps )
            : scanlaw_( law ), mass_( mass ), time_( time ), laps_( laps )
            , tlap_( scanlaw_.getTime( mass, 2 ) - scanlaw_.getTime( mass, 1 ) ) {
        }

        std::pair< int, double > operator()( double mass ) const {
            if ( laps_ > 0 && mass > 0.5 ) {
                int laps = laps_;
                if ( mass < mass_ ) {
                    do {
                        double time_p = time_ - (tlap_/2);
                        double time = scanlaw_.getTime( mass, laps );
                        if ( time > time_p )
                            return { laps, time };
                    } while ( ++laps && laps < 1000 );
                } else {
                    do {
                        double time_n = time_ + (tlap_/2);
                        double time = scanlaw_.getTime( mass, laps );
                        if ( time < time_n )
                            return { laps, time };
                    } while ( laps && --laps );
                }
            }
            return { 0, 0 };
        }
    };

    class MolTableWidget::impl {
        MolTableWidget * this_;
        boost::optional< int > targetRow_;
    public:
        std::unique_ptr< QStandardItemModel > model_;
        std::weak_ptr< const adcontrols::MassSpectrometer > spectrometer_;
        bool enable_;
    public:
        impl( MolTableWidget * p ) : this_( p )
                                   , model_( std::make_unique< QStandardItemModel >() )
                                   , enable_( true ) {
            model_->setColumnCount( ncols );
            model_->setHeaderData( c_formula,    Qt::Horizontal, QObject::tr( "Formula" ) );
            model_->setHeaderData( c_mass,       Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
            model_->setHeaderData( c_time,       Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
            model_->setHeaderData( c_laps,       Qt::Horizontal, QObject::tr( "lap#" ) );
            model_->setHeaderData( c_tdiff,      Qt::Horizontal, QObject::tr( "dt" ) );
            model_->setHeaderData( c_apparent_mass,   Qt::Horizontal, QObject::tr( "Apparent <i>m/z</i>" ) );
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
                actions.emplace_back( menu.addAction( "set as target ion" ), [&](){ setTargetIon(); } );
                actions.emplace_back( menu.addAction( "add line" ), [&](){ addLine(); } );
                actions.emplace_back( menu.addAction( "find laps" ), [&](){ findLaps(); } );
                if ( QAction * selected = menu.exec( table->mapToGlobal( pt ) ) ) {
                    auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){ return t.first == selected; });
                    if ( it != actions.end() )
                        (it->second)();
                }
            }
        }

        //-----------
        void updateTarget() {
            int nlaps = model_->index( *targetRow_, c_laps ).data( Qt::EditRole ).toInt();
            if ( auto sp = spectrometer_.lock() ) {
                if ( auto scanlaw = sp->scanLaw() ) {
                    for ( int row = 0; row < model_->rowCount(); ++row ) {
                        auto xlaps = model_->index( row, c_laps ).data( Qt::EditRole ).toInt();
                        if ( xlaps > 0 ) {
                            double mass = model_->index( row, c_mass ).data( Qt::EditRole ).toDouble();
                            double time = scanlaw->getTime( mass, xlaps );
                            model_->setData( model_->index( row, c_apparent_mass ), scanlaw->getMass( time, nlaps ) );
                            for ( int column = 0; column < ncols; ++column )
                                model_->setData( model_->index( *targetRow_, column ), QColor( 0xff, 0x38, 0x3f, 0x40 ), Qt::BackgroundRole);
                        }
                    }
                }
            }
        }

        //-----------
        bool setTargetIon() {
            if ( auto table = this_->findChild< MolTableView * >() ) {
                if ( *targetRow_ ){
                    for ( int column = 0; column < ncols; ++column )
                        model_->setData( model_->index( *targetRow_, column ), QColor( Qt::white ), Qt::BackgroundRole);
                }
                targetRow_ = boost::none;
                QModelIndexList indices = table->selectionModel()->selectedIndexes();
                qSort( indices );
                if ( indices.size() ) {
                    targetRow_ = indices.first().row();
                    updateTarget();
                }
            }
            return bool( targetRow_ );
        }

        void addLine() {
            model_->insertRow( model_->rowCount() );
        }

        //-----
        void findLaps() {
            //if ( auto table = this_->findChild< MolTableView * >() ) {
            if ( ! setTargetIon() )
                return;
            if ( auto sp = spectrometer_.lock() ) {
                if ( auto scanlaw = sp->scanLaw() ) {
                    auto row = *targetRow_;
                    auto mass = model_->index( row, c_mass ).data( Qt::EditRole ).toDouble();
                    auto laps = model_->index( row, c_laps ).data( Qt::EditRole ).toUInt();
                    auto time = model_->index( row, c_time ).data( Qt::EditRole ).toDouble() / std::micro::den;
                    model_->setData( model_->index( row, c_apparent_mass), mass );

                    lapFinder finder( *scanlaw, mass, time, int(laps) );
                    for ( int i = 0; i < model_->rowCount(); ++i ) {
                        if ( i != *targetRow_ ) {
                            int lap;
                            double tof;
                            std::tie( lap, tof ) = finder( model_->index( i, c_mass ).data( Qt::EditRole ).toDouble() );
                            if ( lap > 0 ) {
                                model_->setData( model_->index( i, c_laps ), lap );
                                model_->setData( model_->index( i, c_time ), tof * std::micro::den );
                                model_->setData( model_->index( i, c_apparent_mass), scanlaw->getMass( tof, int(laps) ) );
                            }
                        }
                    }
                }
            }
        }

        //-------------------
        void setTime( int row, double mass ) {
            if ( auto sp = spectrometer_.lock() ) {
                if ( auto scanlaw = sp->scanLaw() ) {
                    if ( model_->index( row, c_laps ).data().isNull() )
                        model_->setData( model_->index( row, c_laps ), 10 );
                    int32_t nlaps =  model_->index( row, c_laps ).data( Qt::EditRole ).toUInt();
                    double tof = scanlaw->getTime( mass, nlaps );
                    model_->setData( model_->index( row, c_time ), tof * std::micro::den, Qt::EditRole );
                }
                deltaTime();
            }
        }

        void deltaTime() {
            if ( model_->rowCount() >= 2 ) {
                std::vector< double > times;
                for ( size_t i = 0; i < model_->rowCount(); ++i ) {
                    if ( model_->index( i, c_mass ).data( Qt::EditRole ).toDouble() > 0.5 )
                        times.emplace_back( model_->index( i, c_time ).data( Qt::EditRole ).toDouble() );
                }
                auto it = std::min_element( times.begin(), times.end() );
                for ( size_t i = 0; i < model_->rowCount(); ++i ) {
                    if ( model_->index( i, c_mass ).data( Qt::EditRole ).toDouble() > 0.5 )
                        model_->setData( model_->index( i, c_tdiff ), times[i] - *it );
                }
            }
        }

        QByteArray readJson() const {
            QJsonArray a;
            for ( size_t i = 0; i < model_->rowCount(); ++i ) {
                auto formula = model_->index( i, c_formula ).data( Qt::EditRole ).toString();
                if ( ! formula.isEmpty() ) {
                    double mass = model_->index( i, c_mass ).data( Qt::EditRole ).toDouble();
                    double tof = model_->index( i, c_time ).data( Qt::EditRole ).toDouble();
                    int nlaps = model_->index( i, c_laps ).data( Qt::EditRole ).toUInt();
                    QJsonObject obj{{"formula", formula }, { "mass", mass }, { "tof", tof }, { "nlaps", nlaps }};
                    a.push_back( obj );
                }
            }

            QJsonObject top{{ "tofCalculator", QJsonObject{{ "enable", enable_}, { "tofdata", a }} }};
            return QJsonDocument( top ).toJson( QJsonDocument::Compact );
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
        gbox->setTitle( tr( "Enable TOF Calculator" ) );
        gbox->setCheckable( true );
        gbox->setChecked( true );
        connect( gbox, &QGroupBox::clicked
                 , [&]( bool checked ){
                       impl_->enable_ = checked;
                       emit valueChanged( impl_->readJson() );
                   } );

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

            connect( impl_->model_.get(), &QAbstractItemModel::rowsRemoved, [&]( const QModelIndex& index, int first, int last ) {
                    emit valueChanged( impl_->readJson() );
                } );
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
    a = impl_->readJson();
    return true;
}

bool
MolTableWidget::setContents( boost::any&& a )
{
    if ( a.type() == typeid( QByteArray ) ) {
        QSignalBlocker block( impl_->model_.get() );

        auto& model = *impl_->model_;
        model.setRowCount( 0 );
        auto obj = QJsonDocument::fromJson( boost::any_cast< QByteArray >( a ) ).object();
        auto top = obj.value( "tofCalculator" );
        bool enable = top[ "enable" ].toBool();
        auto data = top[ "tofdata" ];
        if ( data.isArray() ) {
            for ( const auto& x: data.toArray() ) {
                auto item = x.toObject();
                size_t row = model.rowCount();
                if ( ! item[ "formula" ].toString().isEmpty() ) {
                    model.insertRow( row );
                    model.setData( model.index( row, c_formula ), item[ "formula" ].toString() );
                    model.setData( model.index( row, c_mass ),    item[ "mass" ].toDouble() );
                    model.setData( model.index( row, c_time ),    item[ "tof" ].toDouble() );
                    model.setData( model.index( row, c_laps ),    item[ "nlaps" ].toInt() );
                }
            }
            impl_->deltaTime();
        }
        if ( auto gbox = findChild< QGroupBox * >() )
            gbox->setChecked( enable );
        impl_->addLine();
        return true;
    }
    return false;
}

void
MolTableWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p )
{
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

    // ADDEBUG() << "------- " << __FUNCTION__ << "  index: " << index.row() << ", " << index.column();
    auto& model = *impl_->model_;
    {
        QSignalBlocker block( impl_->model_.get() );

        if ( index.column() == c_formula ) {
            double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( model.data( index, Qt::EditRole ).toString().toStdString() );
            model.setData( model.index( index.row(), c_mass ), mass, Qt::EditRole );
            impl_->setTime( index.row(), mass );
        }
        if ( index.column() == c_laps ) {
            double mass = model.index( index.row(), c_mass ).data( Qt::EditRole ).toDouble();
            impl_->setTime( index.row(), mass );
        }
        if ( index.row() == model.rowCount() - 1 )
            impl_->addLine();
    }
    auto json = impl_->readJson();
    emit valueChanged( json );
}

void
MolTableWidget::handleScanLawChanged()
{
    if ( auto sp = impl_->spectrometer_.lock() ) {
        if ( auto scanlaw = sp->scanLaw() ) {

            QSignalBlocker block( impl_->model_.get() );

            for ( int i = 0; i < impl_->model_->rowCount(); ++i ) {

                auto formula = impl_->model_->index( i, c_formula ).data( Qt::EditRole ).toString();
                auto mass = impl_->model_->index( i, c_mass ).data( Qt::EditRole ).toDouble();
                auto laps = impl_->model_->index( i, c_laps ).data( Qt::EditRole ).toUInt();
                if ( mass > 0.5 )
                    impl_->setTime( i, mass );
            }
            auto json = impl_->readJson();
            emit valueChanged( json );
        }
    }
}
