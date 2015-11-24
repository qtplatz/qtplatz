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

#include "tofchromatogramswidget.hpp"
#include "tofchromatogramsform.hpp"
#include "moltableview.hpp"
#include <adportable/is_type.hpp>
#include <adcontrols/tofchromatogrammethod.hpp>
#include <adcontrols/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <QBoxLayout>
#include <QDebug>
#include <QSplitter>
#include <QMenu>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#if defined _DEBUG
# include <fstream>
#endif

namespace adwidgets {

    static const char * const ConnectionString = "TofChromatogramsWidget";
    
    class TofChromatogramsWidget::impl {
        TofChromatogramsWidget * this_;
    public:
        enum columns { c_id, c_formula, c_mass, c_masswindow, c_time, c_timewindow, c_algo };
        
        impl( TofChromatogramsWidget * p ) : this_( p ) {

            auto db = createConnection();
            if ( db.isValid() ) {
                QSqlQuery query( db );

                model_.reset( new QSqlTableModel( 0, db ) );
                
                model_->setEditStrategy( QSqlTableModel::OnRowChange );
                
                model_->setTable( "tofChromatograms" );
                model_->setHeaderData( c_id,         Qt::Horizontal, QObject::tr( "id" ) );
                model_->setHeaderData( c_formula,    Qt::Horizontal, QObject::tr( "Formula" ) );
                model_->setHeaderData( c_mass,       Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
                model_->setHeaderData( c_masswindow, Qt::Horizontal, QObject::tr( "Window(Da)" ) );
                model_->setHeaderData( c_time,       Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
                model_->setHeaderData( c_timewindow, Qt::Horizontal, QObject::tr( "Window(ns)" ) );
                model_->setHeaderData( c_algo,       Qt::Horizontal, QObject::tr( "Method" ) );
                //model_->setHeaderData( 6, Qt::Horizontal, QObject::tr( "Mol" ) );
                //model_->setHeaderData( 7, Qt::Horizontal, QObject::tr( "SMILES" ) );
            }
        }

        ~impl() {
            auto db = QSqlDatabase::database( ConnectionString );
            if ( db.isValid() ) {
                db.close();
                QSqlDatabase::removeDatabase( ConnectionString );
            }
        }

        void dataChanged( const QModelIndex& _1, const QModelIndex& _2 ) {

            if ( _1.column() == c_formula ) {
                int row = _1.row();
                auto record = model_->record( row );
                
                size_t id = record.value( "id" ).toLongLong();
                double exactMass = MolTableView::getMonoIsotopicMass( record.value( "formula" ).toString() );
                if ( exactMass > 0.7 ) {
                    model_->setData( model_->index( row, c_mass ), exactMass );
                    for ( auto& id : { std::make_pair( c_masswindow, 0.005 )
                                     , std::make_pair( c_time, 0.0 )
                                     , std::make_pair( c_timewindow, 10.0 ) } ) {
                        if ( record.isNull( id.first ) )
                            model_->setData( model_->index( row, id.first ), id.second );
                    }
                } else {
                    for ( auto& id : { c_mass, c_masswindow, c_time, c_timewindow } )
                        model_->setData( model_->index( row, id ), QVariant() );
                }
            }
            emit this_->valueChanged();
        }

        void handleContextMenu( const QPoint& pt );
        void addLine();

        QSqlDatabase createConnection();
        std::unique_ptr< QSqlTableModel > model_;
    };
    
}

using namespace adwidgets;

TofChromatogramsWidget::TofChromatogramsWidget(QWidget *parent) : QWidget(parent)
                                                                , impl_( new impl( this ) )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
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
        table->setColumnHidden( impl::c_id, true );
        table->setColumnField( impl::c_formula, ColumnState::f_formula, true, false );
        table->setColumnField( impl::c_mass, ColumnState::f_mass );
        table->setColumnField( impl::c_time, ColumnState::f_time );
        table->setColumnField( impl::c_timewindow, ColumnState::f_time );
        table->setPrecision( impl::c_time, 4 );
        table->setPrecision( impl::c_timewindow, 2 );

        std::vector< std::pair< QString, QVariant > > choice;
        choice.push_back( std::make_pair( "Area", QVariant( adcontrols::TofChromatogramMethod::ePeakAreaOnProfile ) ) );
        choice.push_back( std::make_pair( "Height", QVariant( adcontrols::TofChromatogramMethod::ePeakHeightOnProfile ) ) );
        choice.push_back( std::make_pair( "Counting", QVariant( adcontrols::TofChromatogramMethod::eCounting ) ) );
        table->setChoice( impl::c_algo, choice );
    }

    if ( auto form = findChild< TofChromatogramsForm * >() ) 
        connect( form, &TofChromatogramsForm::applyTriggered, [this](){ emit applyTriggered(); } );

    connect( impl_->model_.get(), &QSqlTableModel::dataChanged, [this] ( const QModelIndex& _1, const QModelIndex& _2 ) { impl_->dataChanged( _1, _2 ); } );
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
        connect( table, &MolTableView::onContextMenu, this, &TofChromatogramsWidget::handleContextMenu );
    }

    impl_->model_->select();
}

void
TofChromatogramsWidget::onUpdate( boost::any& )
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
TofChromatogramsWidget::setContents( boost::any& a )
{
   adcontrols::TofChromatogramsMethod m;

    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ||
         adportable::a_type< std::shared_ptr< const adcontrols::ControlMethod::Method > >::is_a( a ) ) {
        auto ptr = boost::any_cast< std::shared_ptr< const adcontrols::ControlMethod::Method > >( a );
        auto it = ptr->find( ptr->begin(), ptr->end(), adcontrols::TofChromatogramsMethod::modelClass() );
        if ( it != ptr->end() ) {
            if ( it->get( *it, m ) ) {
                setContents( m );
                return true;
            }
        }
    }
    
    return false;
}

bool
TofChromatogramsWidget::getContents( adcontrols::TofChromatogramsMethod& m ) const
{
    m.clear();

    if ( auto form = findChild< TofChromatogramsForm *>() ) {
        form->getContents( m );
    }
    
    QSqlDatabase db = QSqlDatabase::database( ConnectionString );
    QSqlQuery query( "SELECT * from tofChromatograms", db );
    while ( query.next() ) {
        adcontrols::TofChromatogramMethod item;
        item.setFormula( query.value( "formula" ).toString().toStdString() );
        item.setMass( query.value( "mass" ).toDouble() );
        item.setMassWindow( query.value( "masswindow" ).toDouble() );
		item.setTime( query.value( "time" ).toDouble() * 1.0e-6 ); // us -> s
        item.setTimeWindow( query.value( "timewindow" ).toDouble() * 1.0e-9 ); // ns -> s
        item.setIntensityAlgorithm( adcontrols::TofChromatogramMethod::eIntensityAlgorishm(  query.value( "algorithm" ).toInt() ) );
        m << item;
    }
    return true;
}

bool
TofChromatogramsWidget::setContents( const adcontrols::TofChromatogramsMethod& m )
{
    if ( auto form = findChild< TofChromatogramsForm *>() ) {
        form->setContents( m );
    }

    QSqlDatabase db = QSqlDatabase::database( ConnectionString );
    QSqlQuery query( db );

    if ( !query.exec( "DELETE FROM tofChromatograms" ) )
        qDebug() << "error: " << query.lastError();

    if ( !query.prepare( "INSERT into tofChromatograms (formula,mass,masswindow,time,timewindow,algorithm) VALUES (?,?,?,?,?,?)" ) )
        qDebug() << "error: " << query.lastError();

    for ( auto& item: m ) {
        query.addBindValue( QString::fromStdString( item.formula() ) );
        query.addBindValue( item.mass() );
        query.addBindValue( item.massWindow() );
        query.addBindValue( item.time() * 1.0e6 );       // s -> us
		query.addBindValue( item.timeWindow() * 1.0e9 ); // s -> ns
        query.addBindValue( item.intensityAlgorithm() );
        if ( !query.exec() )
            qDebug() << "error: " << query.lastError();
    }
    impl_->model_->select();
    return true;

}

#if 0
void
TofChromatogramsWidget::setTimeSquaredScanLaw( double flength, double acceleratorVoltage, double tdelay )
{
    if ( auto form = findChild< MSSimulatorForm * >() ) {
        adcontrols::MSSimulatorMethod m;
        form->getContents( m );
        m.setLength( flength );
        m.setAcceleratorVoltage( acceleratorVoltage );
        m.setTDelay( tdelay );
        form->setContents( m );
    }
}
#endif

void
TofChromatogramsWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Simulate MS Spectrum", this, SLOT( run() ) );
}

QSqlDatabase
TofChromatogramsWidget::impl::createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", ConnectionString );
    db.setDatabaseName(":memory:");
    
    if (!db.open()) {
        QMessageBox::critical(0, tr("Cannot open database"),
                              tr("Unable to establish a database connection.\nClick Cancel to exit."), QMessageBox::Cancel);
        return db;
    }

    QSqlQuery query(db);
    if ( !query.exec( "CREATE TABLE tofChromatograms ("
                      "id INTEGER PRIMARY KEY"
                      ",formula TEXT"
                      ",mass REAL"
                      ",masswindow REAL"
                      ",time REAL"
                      ",timewindow REAL"
                      ",algorithm INTEGER )" ) ) {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();        
        return db;        
    }
    if ( !query.exec( "INSERT into tofChromatograms (formula,algorithm) VALUES( \"TIC\",0)" ) ) {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }
    return db;
}

void
TofChromatogramsWidget::impl::handleContextMenu( const QPoint& pt )
{
    QMenu menu;
    typedef std::pair< QAction *, std::function< void() > > action_type;
    
    if ( auto table = this_->findChild< MolTableView * >() ) {

        std::vector< action_type > actions;
        actions.push_back( std::make_pair( menu.addAction( "add line" ), [this](){ addLine(); }) );
        actions.push_back( std::make_pair( menu.addAction( "refresh" ), [this](){ model_->select(); }) );
        
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
    QSqlDatabase db = QSqlDatabase::database( ConnectionString );
    QSqlQuery query( db );
    if ( query.exec( "INSERT into tofChromatograms (formula,algorithm) VALUES( \"\",0)" ) ) {
        model_->select();
    } else {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }    
}
