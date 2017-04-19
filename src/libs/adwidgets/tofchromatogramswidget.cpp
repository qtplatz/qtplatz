/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/debug.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <QBoxLayout>
#include <QDebug>
#include <QSplitter>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <ratio>

#if defined _DEBUG
# include <fstream>
#endif

namespace adwidgets {

    class TofChromatogramsWidget::impl {
        TofChromatogramsWidget * this_;
    public:
        enum columns { c_id, c_formula, c_mass, c_masswindow, c_time, c_timewindow, c_algo, c_protocol, ncolumns };

        QString connString_;
        
        impl( TofChromatogramsWidget * p ) : this_( p )
                                           , model_( std::make_unique< QStandardItemModel >() ) {
            
            model_->setColumnCount( ncolumns );
            model_->setHeaderData( c_id,         Qt::Horizontal, QObject::tr( "id" ) );
            model_->setHeaderData( c_formula,    Qt::Horizontal, QObject::tr( "Formula" ) );
            model_->setHeaderData( c_mass,       Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
            model_->setHeaderData( c_masswindow, Qt::Horizontal, QObject::tr( "Window(Da)" ) );
            model_->setHeaderData( c_time,       Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
            model_->setHeaderData( c_timewindow, Qt::Horizontal, QObject::tr( "Window(ns)" ) );
            model_->setHeaderData( c_algo,       Qt::Horizontal, QObject::tr( "Method" ) );
            model_->setHeaderData( c_protocol,   Qt::Horizontal, QObject::tr( "Protocol#" ) );
        }

        ~impl() {
        }

        void dataChanged( const QModelIndex& _1, const QModelIndex& _2 ) {
            if ( _1.column() == c_formula ) {
                int row = _1.row();
                double exactMass = MolTableView::getMonoIsotopicMass( _1.data( Qt::EditRole ).toString() );
                if ( exactMass > 0.7 ) {
                    model_->setData( model_->index( row, c_mass ), exactMass );
                    model_->setData( model_->index( row, c_masswindow ), 0.005 );
                    model_->setData( model_->index( row, c_time ), 0.0 );
                    model_->setData( model_->index( row, c_timewindow ), 10.0 );
                } else {
                    for ( auto& id : { c_mass, c_masswindow, c_time, c_timewindow } )
                        model_->setData( model_->index( row, id ), QVariant() );
                }
            }
            emit this_->valueChanged();
        }

        void handleContextMenu( const QPoint& pt );
        void addLine();

        std::unique_ptr< QStandardItemModel > model_;
        std::weak_ptr< const adcontrols::MassSpectrometer > spectrometer_;
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

        table->setColumnField( impl::c_formula, ColumnState::f_any, true, true );
        table->setPrecision( impl::c_mass, 4 );

        {
            std::vector< std::pair< QString, QVariant > > choice;
            choice.emplace_back( "Area", QVariant( adcontrols::TofChromatogramMethod::ePeakAreaOnProfile ) );
            choice.emplace_back( "Height", QVariant( adcontrols::TofChromatogramMethod::ePeakHeightOnProfile ) );
            choice.emplace_back( "Counting", QVariant( adcontrols::TofChromatogramMethod::eCounting ) );
            table->setChoice( impl::c_algo, choice );
        }
        {
            std::vector< std::pair< QString, QVariant > > choice;
            choice.emplace_back( "0", QVariant( 0 ) );
            choice.emplace_back( "1", QVariant( 1 ) );
            choice.emplace_back( "3", QVariant( 2 ) );
            choice.emplace_back( "4", QVariant( 3 ) );
            table->setChoice( impl::c_protocol, choice );
        }
        
        table->setColumnHidden( impl::c_id, true );
        table->setColumnHidden( impl::c_time, true );
        table->setColumnHidden( impl::c_timewindow, true );
    }

    if ( auto form = findChild< TofChromatogramsForm * >() )  {
        connect( form, &TofChromatogramsForm::applyTriggered, [this](){ emit applyTriggered(); } );
        connect( form, &TofChromatogramsForm::valueChanged, [this](){ emit valueChanged(); } );
    }

    connect( impl_->model_.get(), &QStandardItemModel::dataChanged
             , [this] ( const QModelIndex& _1, const QModelIndex& _2 ) { impl_->dataChanged( _1, _2 ); } );
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

bool
TofChromatogramsWidget::getContents( adcontrols::TofChromatogramsMethod& m ) const
{
    m.clear();

    if ( auto form = findChild< TofChromatogramsForm *>() )
        form->getContents( m );
    
    //QSqlDatabase db = QSqlDatabase::database( impl_->connString_ );
    auto& model = *impl_->model_;
    for ( int row = 0; row < model.rowCount(); ++row ) {
        using adcontrols::TofChromatogramMethod;
        adcontrols::TofChromatogramMethod item;

        item.setEnable( model.index( row, impl::c_formula ).data( Qt::CheckStateRole ) == Qt::Checked );
        item.setFormula( model.index( row, impl::c_formula ).data( Qt::EditRole ).toString().toStdString() );
        item.setMass( model.index( row, impl::c_mass ).data( Qt::EditRole ).toDouble() );
        item.setMassWindow( model.index( row, impl::c_masswindow ).data( Qt::EditRole ).toDouble() );
		item.setTime( model.index( row, impl::c_time ).data( Qt::EditRole ).toDouble() / std::micro::den );
        item.setTimeWindow( model.index( row, impl::c_timewindow ).data( Qt::EditRole ).toDouble() / std::nano::den );
        item.setIntensityAlgorithm( TofChromatogramMethod::eIntensityAlgorishm(  model.index( row, impl::c_algo ).data( Qt::EditRole ).toInt() ) );
        item.setProtocol( model.index( row, impl::c_protocol ).data( Qt::EditRole ).toInt() );
        m << item;
    }

    int row(0);
    for ( auto& i: m ) {
        ADDEBUG() << "row[" << row << "] formula: " << i.formula() << " enable: " << i.enable();
        ++row;
    }
    
    return true;
}

bool
TofChromatogramsWidget::setContents( const adcontrols::TofChromatogramsMethod& m )
{
    if ( auto form = findChild< TofChromatogramsForm *>() )
        form->setContents( m );
    
    auto& model = *impl_->model_;
    model.setRowCount( m.size() );

    int row(0);
    for ( auto& trace: m ) {
        model.setData( model.index( row, impl::c_formula ), QString::fromStdString( trace.formula() ) );
        
        if ( auto item = model.item( row, impl::c_formula ) ) {
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            model.setData( model.index( row, impl::c_formula ), trace.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
        
        model.setData( model.index( row, impl::c_mass ), trace.mass() );
        model.setData( model.index( row, impl::c_masswindow ), trace.massWindow() );
        model.setData( model.index( row, impl::c_time ), trace.time() * std::micro::den );
        model.setData( model.index( row, impl::c_timewindow ), trace.timeWindow() * std::nano::den );
        model.setData( model.index( row, impl::c_algo ), trace.intensityAlgorithm() );
        model.setData( model.index( row, impl::c_protocol ), trace.protocol() );
        ++row;
    }
    return true;

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

