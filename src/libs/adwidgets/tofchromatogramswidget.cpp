/**************************************************************************
** Copyright (C) 2010- Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "tofchromatogramswidget.hpp"
#include "tofchromatogramsform.hpp"
#include "moltableview.hpp"
#include "moltablehelper.hpp"
#include <adportable/is_type.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <QBoxLayout>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStandardItemModel>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <ratio>
#include <cmath>

#if defined _DEBUG
# include <fstream>
#endif

namespace adwidgets {

    class TofChromatogramsWidget::impl {
        TofChromatogramsWidget * this_;
    public:
        // enum columns { c_id, c_formula, c_mass, c_masswindow, c_time, c_timewindow, c_algo, c_protocol, ncolumns };

        QString connString_;

        impl( TofChromatogramsWidget * p ) : this_( p )
                                           , model_( std::make_unique< QStandardItemModel >() ) {
            model_->setColumnCount( ncolumns );
            model_->setHeaderData( c_id,         Qt::Horizontal, QObject::tr( "id" ) );
            model_->setHeaderData( c_formula,    Qt::Horizontal, QObject::tr( "Formula" ) );
            model_->setHeaderData( c_mass,       Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
            model_->setHeaderData( c_masswindow, Qt::Horizontal, QObject::tr( "Window(Da)" ) );
            model_->setHeaderData( c_time,       Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
            model_->setHeaderData( c_timewindow, Qt::Horizontal, QObject::tr( "Window(&mu;s)" ) );
            model_->setHeaderData( c_algo,       Qt::Horizontal, QObject::tr( "Method" ) );
            model_->setHeaderData( c_protocol,   Qt::Horizontal, QObject::tr( "Protocol#" ) );
        }

        ~impl() {
        }

        void dataChanged( const QModelIndex& _1, const QModelIndex& _2 ) {
            int row = _1.row();
            QSignalBlocker block( model_.get() );
            if ( _1.column() == c_formula ) {
                adcontrols::ChemicalFormula cf;
                auto formulae = _1.data( Qt::EditRole ).toString().toStdString();
                auto mc = cf.getMonoIsotopicMass( cf.split( formulae ) );
                if ( mc.first > 0.7 ) {
                    model_->setData( model_->index( row, c_mass ), mc.first );
                    if ( model_->data( model_->index( row, c_masswindow ), Qt::EditRole ).toDouble() < 0.0001 )
                        model_->setData( model_->index( row, c_masswindow ), 0.100 );
                    if ( auto sp = spectrometer_ ) {
                        double time = sp->timeFromMass( mc.first );
                        model_->setData( model_->index( row, c_time ), time * std::micro::den );
                    }
                } else {
                    for ( auto& id : { c_mass, c_masswindow, c_time, c_timewindow } )
                        model_->setData( model_->index( row, id ), QVariant() );
                }
                if ( auto tv = this_->findChild< QTableView * >() )
                    tv->resizeColumnToContents( c_formula );
            } else if ( _1.column() == c_time ) {
                if ( auto sp = spectrometer_ ) {
                    double mass = sp->assignMass( _1.data( Qt::EditRole ).toDouble() / std::micro::den );
                    model_->setData( model_->index( row, c_mass ), mass );
                }
            } else if ( _1.column() == c_timewindow ) {
                if ( auto sp = spectrometer_ ) {
                    double t1 = model_->data( model_->index( row, c_time ), Qt::EditRole ).toDouble() / std::micro::den;
                    double t2 = t1 + _1.data( Qt::EditRole ).toDouble() / std::micro::den;
                    double dm = std::abs( sp->assignMass( t2 ) - sp->assignMass( t1 ) );
                    model_->setData( model_->index( row, c_masswindow ), dm );
                }
            } else if ( _1.column() == c_mass && _1.isValid() ) {
                if ( auto sp = spectrometer_ ) {
                    double mass = _1.data( Qt::EditRole ).toDouble();
                    if ( mass > 1.0 && mass < 100000 ) {
                        double time = sp->timeFromMass( _1.data( Qt::EditRole ).toDouble() );
                        model_->setData( model_->index( row, c_time ), time * std::micro::den );
                    }
                }
            } else if ( _1.column() == c_masswindow ) {
                if ( auto sp = spectrometer_ ) {
                    double m1 = model_->data( model_->index( row, c_mass ), Qt::EditRole ).toDouble();
                    double m2 = m1 + _1.data( Qt::EditRole ).toDouble();
                    double dt = std::abs( sp->timeFromMass( m2 ) - sp->timeFromMass( m1 ) );
                    model_->setData( model_->index( row, c_timewindow ), ( dt * std::micro::den ) );
                }
            }
            emit this_->valueChanged();
        }

        void handleContextMenu( const QPoint& pt );
        void addLine();

        std::unique_ptr< QStandardItemModel > model_;
        std::shared_ptr< const adcontrols::MassSpectrometer > spectrometer_;
    };

}

using namespace adwidgets;

TofChromatogramsWidget::TofChromatogramsWidget(QWidget *parent) : QWidget(parent)
                                                                , impl_( new impl( this ) )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setContentsMargins( {} );
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new TofChromatogramsForm ) );
            splitter->addWidget( ( new MolTableView ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }

    if ( auto table = findChild< MolTableView * >() ) {

        table->setModel( impl_->model_.get() );
        table->setContextMenuHandler( [this]( const QPoint& pt ){ impl_->handleContextMenu( pt ); } );

        table->setColumnField( c_formula, ColumnState::f_any, true, true );
        table->setPrecision( c_mass, 4 );
        table->setPrecision( c_time, 3 );
        table->setPrecision( c_timewindow, 3 );
        {
            std::vector< std::pair< QString, QVariant > > choice;
            choice.emplace_back( "Area", QVariant( adcontrols::xic::ePeakAreaOnProfile ) );
            choice.emplace_back( "Height", QVariant( adcontrols::xic::ePeakHeightOnProfile ) );
            choice.emplace_back( "Counting", QVariant( adcontrols::xic::eCounting ) );
            table->setChoice( c_algo, choice );
        }
        {
            std::vector< std::pair< QString, QVariant > > choice;
            choice.emplace_back( "0", QVariant( 0 ) );
            choice.emplace_back( "1", QVariant( 1 ) );
            choice.emplace_back( "3", QVariant( 2 ) );
            choice.emplace_back( "4", QVariant( 3 ) );
            table->setChoice( c_protocol, choice );
        }

        table->setColumnHidden( c_id, true );

        connect( table, &MolTableView::valueChanged, [&]( const QModelIndex& index, double value ){
                if ( index.column() == c_time || index.column() == c_timewindow )
                    value /= std::micro::den;  // us -> s
                emit editorValueChanged( index, value );
            });
    }

    if ( auto form = findChild< TofChromatogramsForm * >() )  {
        connect( form, &TofChromatogramsForm::valueChanged, [this](){ emit valueChanged(); } );
    }

    connect( impl_->model_.get()
             , &QStandardItemModel::dataChanged
             , [&] ( const QModelIndex& _1, const QModelIndex& _2 ) { impl_->dataChanged( _1, _2 ); } );
}

TofChromatogramsWidget::~TofChromatogramsWidget()
{
}

void
TofChromatogramsWidget::OnCreate( const adportable::Configuration& )
{
}

void
TofChromatogramsWidget::OnInitialUpdate()
{
    if ( auto form = findChild< TofChromatogramsForm * >() )
        form->OnInitialUpdate();

    if ( auto table = findChild< MolTableView *>() ) {
        table->onInitialUpdate();
        // connect( table, &MolTableView::onContextMenu, this, &TofChromatogramsWidget::handleContextMenu );
    }

    setContents( adcontrols::TofChromatogramsMethod() );
}

void
TofChromatogramsWidget::onUpdate( boost::any&& )
{
}

void
TofChromatogramsWidget::OnFinalClose()
{
}

bool
TofChromatogramsWidget::getContents( boost::any& a ) const
{
    adcontrols::TofChromatogramsMethod m;
    getContents( m );

    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {
        auto ptr = boost::any_cast< std::shared_ptr< adcontrols::ControlMethod::Method > >( a );
        ptr->append( m );
    }
    return false;
}

bool
TofChromatogramsWidget::setContents( boost::any&& a )
{
    auto pi = adcontrols::ControlMethod::any_cast<>()( a, adcontrols::TofChromatogramsMethod::clsid() );
    if ( pi ) {
        adcontrols::TofChromatogramsMethod m;
        if ( pi->get( *pi, m ) ) {
            setContents( m );
            return true;
        }
    }
    return false;
}

adcontrols::TofChromatogramsMethod
TofChromatogramsWidget::method() const
{
    adcontrols::TofChromatogramsMethod m;
    getContents( m );
    return m;
}


bool
TofChromatogramsWidget::getContents( adcontrols::TofChromatogramsMethod& m ) const
{
    m.clear();

    if ( auto form = findChild< TofChromatogramsForm *>() )
        form->getContents( m );

    //QSqlDatabase db = QSqlDatabase::database( impl_->connString_ );
    auto& model = *impl_->model_;
    for ( int row = 0; row < model.rowCount(); ++row ) {
        adcontrols::TofChromatogramMethod item;
        namespace xic = adcontrols::xic;

        item.setEnable( model.index( row, c_formula ).data( Qt::CheckStateRole ) == Qt::Checked );
        item.setFormula( model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdString() );
        item.setMass( model.index( row, c_mass ).data( Qt::EditRole ).toDouble() );
        item.setMassWindow( model.index( row, c_masswindow ).data( Qt::EditRole ).toDouble() );
		item.setTime( model.index( row, c_time ).data( Qt::EditRole ).toDouble() / std::micro::den );
        item.setTimeWindow( model.index( row, c_timewindow ).data( Qt::EditRole ).toDouble() / std::micro::den );
        item.setIntensityAlgorithm( xic::eIntensityAlgorithm(  model.index( row, c_algo ).data( Qt::EditRole ).toInt() ) );
        item.setProtocol( model.index( row, c_protocol ).data( Qt::EditRole ).toInt() );
        m << item;
    }

    return true;
}

bool
TofChromatogramsWidget::setContents( const adcontrols::TofChromatogramsMethod& m )
{
    QSignalBlocker block( this );

    if ( auto form = findChild< TofChromatogramsForm *>() )
        form->setContents( m );

    auto& model = *impl_->model_;
    model.setRowCount( m.size() );

    int row(0);
    for ( auto& trace: m ) {
        model.setData( model.index( row, c_formula ), QString::fromStdString( trace.formula() ) );

        if ( auto item = model.item( row, c_formula ) ) {
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            model.setData( model.index( row, c_formula ), trace.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

        model.setData( model.index( row, c_mass ), trace.mass() );
        model.setData( model.index( row, c_masswindow ), trace.massWindow() );
        model.setData( model.index( row, c_time ), trace.time() * std::micro::den );
        model.setData( model.index( row, c_timewindow ), trace.timeWindow() * std::micro::den );
        model.setData( model.index( row, c_algo ), trace.intensityAlgorithm() );
        model.setData( model.index( row, c_protocol ), trace.protocol() );
        ++row;
    }
    if ( auto table = findChild< MolTableView * >() ) {
        table->resizeColumnToContents( c_formula );
    }
    return true;

}

MolTableView *
TofChromatogramsWidget::molTableView()
{
    return findChild< MolTableView * >();
}

void
TofChromatogramsWidget::setDigitizerMode( bool softAverage )
{
    if ( auto form = findChild< TofChromatogramsForm * >() ) {
        form->setDigitizerMode( softAverage );
    }
}

void
TofChromatogramsWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Simulate MS Spectrum", this, SLOT( run() ) );
}

void
TofChromatogramsWidget::impl::handleContextMenu( const QPoint& pt )
{
    QMenu menu;
    typedef std::pair< QAction *, std::function< void() > > action_type;

    if ( auto table = this_->findChild< MolTableView * >() ) {

        std::vector< action_type > actions;
        actions.push_back( std::make_pair( menu.addAction( "add line" ), [this](){ addLine(); }) );

        if ( QAction * selected = menu.exec( table->mapToGlobal( pt ) ) ) {
            auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){ return t.first == selected; });
            if ( it != actions.end() )
                (it->second)();
        }
    }
}

void
TofChromatogramsWidget::impl::addLine()
{
    model_->insertRow( model_->rowCount() );
}

void
TofChromatogramsWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > sp )
{
    impl_->spectrometer_ = sp;
}

void
TofChromatogramsWidget::handleScanLawChanged()
{

}

QByteArray
TofChromatogramsWidget::readJson() const
{
    QJsonObject jtop;
    QJsonArray jmols;

    adcontrols::TofChromatogramsMethod m;
    getContents( m );

    jtop[ "numberOfTriggers" ]         = int( m.numberOfTriggers() );
    jtop[ "refreshHistogram" ]         = m.refreshHistogram();

    auto& model = *impl_->model_;
    for ( int row = 0; row < model.rowCount(); ++row ) {
        using adcontrols::TofChromatogramMethod;
        adcontrols::TofChromatogramMethod item;
        QJsonObject jitem;

        jitem[ "enable"     ] = model.index( row, c_formula ).data( Qt::CheckStateRole ) == Qt::Checked;
        jitem[ "formula"    ] = model.index( row, c_formula ).data( Qt::EditRole ).toString();
        jitem[ "mass"       ] = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
        jitem[ "massWindow" ] = model.index( row, c_masswindow ).data( Qt::EditRole ).toDouble();
        jitem[ "time"       ] = model.index( row, c_time ).data( Qt::EditRole ).toDouble() / std::micro::den;
        jitem[ "timeWindow" ] = model.index( row, c_timewindow ).data( Qt::EditRole ).toDouble() / std::micro::den;
        jitem[ "intensAlgo" ] = model.index( row, c_algo ).data( Qt::EditRole ).toInt();
        jitem[ "protocol"   ] = model.index( row, c_protocol ).data( Qt::EditRole ).toInt();
        if ( row == 0 || jitem[ "mass" ].toDouble() > 0 || jitem[ "time" ].toDouble() > 0 )
            jmols.append( jitem );
    }

    jtop[ "list" ] = jmols;

    QJsonObject jobj;
    jobj[ QString::fromStdString( adcontrols::TofChromatogramsMethod::modelClass() ) ] = jtop;

    QJsonDocument jdoc( jobj );

    return QByteArray( jdoc.toJson( /* QJsonDocument::Indented */ ) );
}

void
TofChromatogramsWidget::setJson( const QByteArray& json )
{
    auto doc = QJsonDocument::fromJson( json );
    auto jtop = doc.object()[ QString::fromStdString( adcontrols::TofChromatogramsMethod::modelClass() ) ].toObject();
    auto jlist = jtop[ "list" ].toArray();

    adcontrols::TofChromatogramsMethod m;

    m.setNumberOfTriggers( jtop[ "numberOfTriggers" ].toInt() );
    m.setRefreshHistogram( jtop[ "refreshHistogram" ].toBool() );
    if ( auto form = findChild< TofChromatogramsForm *>() )
        form->setContents( m );

    auto& model = *impl_->model_;
    model.setRowCount( jlist.count() );
    int row(0);
    for ( const auto& x: jlist ) {
        auto jtrace = x.toObject();
        model.setData( model.index( row, c_formula ), jtrace[ "formula" ].toString() );

        if ( auto item = model.item( row, c_formula ) ) {
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            model.setData( model.index( row, c_formula ), jtrace[ "enable" ].toBool() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

        model.setData( model.index( row, c_mass ), jtrace[ "mass" ].toDouble() );
        model.setData( model.index( row, c_masswindow ), jtrace[ "massWindow" ].toDouble() );
        model.setData( model.index( row, c_time ), jtrace[ "time " ].toDouble() * std::micro::den );
        model.setData( model.index( row, c_timewindow ), jtrace[ "timeWindow" ].toDouble() * std::micro::den );
        model.setData( model.index( row, c_algo ), jtrace[ "intensAlgo" ].toInt() );
        model.setData( model.index( row, c_protocol ), jtrace[ "protocol" ].toInt() );

        ++row;
    }
}


QStandardItemModel *
TofChromatogramsWidget::model()
{
    return impl_->model_.get();
}
