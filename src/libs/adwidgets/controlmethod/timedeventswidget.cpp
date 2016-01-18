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

#include "timedeventswidget.hpp"
#include "timedtableview.hpp"
#include <adportable/is_type.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/controlmethod/timedevent.hpp>
#include <adcontrols/controlmethod/timedevents.hpp>
#include <adcontrols/controlmethod/modulecap.hpp>
#include <QBoxLayout>
#include <QDebug>
#include <QSplitter>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QPushButton>

namespace adwidgets {

    class TimedEventsWidget::impl {
        TimedEventsWidget * this_;
    public:
        enum columns { c_clsid, c_item, c_time, c_model_name, c_item_name, c_value, ncolumns };
        
        impl( TimedEventsWidget * p ) : this_( p )
                                      , model_( new QStandardItemModel() ) {

            model_->setColumnCount( ncolumns );
            model_->setRowCount( 1 );
            
            model_->setHeaderData( c_clsid,        Qt::Horizontal, QObject::tr( "clsid" ) );
            model_->setHeaderData( c_item,         Qt::Horizontal, QObject::tr( "itemClass" ) );
            model_->setHeaderData( c_time,         Qt::Horizontal, QObject::tr( "Time(seconds)" ) );
            model_->setHeaderData( c_model_name,   Qt::Horizontal, QObject::tr( "Module" ) );
            model_->setHeaderData( c_item_name,    Qt::Horizontal, QObject::tr( "Function" ) );
            model_->setHeaderData( c_value,        Qt::Horizontal, QObject::tr( "Value" ) );
        }

        ~impl() {
        }

        void dataChanged( const QModelIndex& _1, const QModelIndex& _2 ) {
            emit this_->valueChanged();
        }

        void handleContextMenu( const QPoint& pt );
        void addLine();

        // QSqlDatabase createConnection();
        std::unique_ptr< QStandardItemModel > model_;
        std::vector < adcontrols::ControlMethod::ModuleCap > capList_;
    };
    
}

using namespace adwidgets;

TimedEventsWidget::TimedEventsWidget(QWidget *parent) : QWidget(parent)
                                                      , impl_( new impl( this ) )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new TimedTableView ) );
            splitter->addWidget( ( new QPushButton() ) ); 
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 5 );
            splitter->setOrientation( Qt::Vertical );
            layout->addWidget( splitter );
        }
    }

    if ( auto table = findChild< TimedTableView * >() ) {
        
        table->setModel( impl_->model_.get() );
        table->setContextMenuHandler( [this]( const QPoint& pt ){ impl_->handleContextMenu( pt ); } );
        table->setColumnHidden( impl::c_clsid, true );
        table->setColumnHidden( impl::c_item, true );
        //enum columns { c_clsid, c_item, c_time, c_item_display, c_value, ncolumns };

        table->setColumnField( impl::c_time, ColumnState::f_time );
        table->setPrecision( impl::c_time, 4 );

        std::vector< std::pair< QString, QVariant > > choice;
        choice.push_back( std::make_pair( "Area", 1 ) );
        choice.push_back( std::make_pair( "Height", 2 ) );
        choice.push_back( std::make_pair( "Counting", 3 ) );
        table->setChoice( impl::c_item_name, choice );
    }

    connect( impl_->model_.get(), &QStandardItemModel::dataChanged, [this] ( const QModelIndex& _1, const QModelIndex& _2 ) { impl_->dataChanged( _1, _2 ); } );
}

TimedEventsWidget::~TimedEventsWidget()
{
}

void
TimedEventsWidget::OnCreate( const adportable::Configuration& )
{
}

void
TimedEventsWidget::OnInitialUpdate()
{
    if ( auto table = findChild< TimedTableView *>() ) {
        table->onInitialUpdate();
        connect( table, &TimedTableView::onContextMenu, this, &TimedEventsWidget::handleContextMenu );
    }

}

void
TimedEventsWidget::onUpdate( boost::any& )
{
}

void
TimedEventsWidget::OnFinalClose()
{
}

bool
TimedEventsWidget::getContents( boost::any& a ) const
{
    adcontrols::ControlMethod::TimedEvents m;
#if 0
    getContents( m );

    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {
        auto ptr = boost::any_cast< std::shared_ptr< adcontrols::ControlMethod::Method > >( a );
        ptr->append( m );
    }
#endif
    return false;
}

bool
TimedEventsWidget::setContents( boost::any&& a )
{
    auto pi = adcontrols::ControlMethod::any_cast<>()( a, adcontrols::ControlMethod::TimedEvents::clsid() );
    if ( pi ) {
        adcontrols::ControlMethod::TimedEvents m;
        if ( pi->get( *pi, m ) ) {
            setContents( m );
            return true;
        }
    }
    return false;
}

bool
TimedEventsWidget::getContents( adcontrols::ControlMethod::TimedEvents& m ) const
{
    m.clear();

#if 0
    if ( auto form = findChild< TofChromatogramsForm *>() ) {
        form->getContents( m );
    }
#endif
    
    return true;
}

bool
TimedEventsWidget::setContents( const adcontrols::ControlMethod::TimedEvents& m )
{
    return true;
}

void
TimedEventsWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Simulate MS Spectrum", this, SLOT( run() ) );
}

void
TimedEventsWidget::impl::handleContextMenu( const QPoint& pt )
{
    QMenu menu;
    typedef std::pair< QAction *, std::function< void() > > action_type;
    
    if ( auto table = this_->findChild< TimedTableView * >() ) {

        std::vector< action_type > actions;
        actions.push_back( std::make_pair( menu.addAction( "add line" ), [this](){ addLine(); }) );
        //actions.push_back( std::make_pair( menu.addAction( "refresh" ), [this](){ model_->select(); }) );
        
        if ( QAction * selected = menu.exec( table->mapToGlobal( pt ) ) ) {
            auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){ return t.first == selected; });
            if ( it != actions.end() )
                (it->second)();
        }
    }
}

void
TimedEventsWidget::impl::addLine()
{
}

void
TimedEventsWidget::addModuleCap( const std::vector< adcontrols::ControlMethod::ModuleCap >& cap )
{
    //impl_->capList_.push_back( cap );
}
