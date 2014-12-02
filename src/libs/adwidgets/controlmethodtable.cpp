/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "controlmethodtable.hpp"
#include "controlmethodwidget.hpp"
#include <adcontrols/controlmethod.hpp>
#include <qtwrapper/font.hpp>

#include <QStandardItemModel>
#include <QHeaderView>
#include <QMenu>
#include <boost/exception/all.hpp>

using namespace adwidgets;

ControlMethodTable::ControlMethodTable( ControlMethodWidget * parent ) : adwidgets::TableView( parent )
                                                                       , model_( new QStandardItemModel )
                                                                       , parent_( parent )
{
    setModel( model_ );

    setSelectionMode( QAbstractItemView::SingleSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );

    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );

    onInitialUpdate();
}

QStandardItemModel& 
ControlMethodTable::model()
{
    return *model_;
}

void
ControlMethodTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

	model.setColumnCount( 5 );
    model.setRowCount( 1 );

    model.setHeaderData( 0,  Qt::Horizontal, QObject::tr( "time(min)" ) );
    model.setHeaderData( 1,  Qt::Horizontal, QObject::tr( "module" ) );
    model.setHeaderData( 2,  Qt::Horizontal, QObject::tr( "function" ) );
    model.setHeaderData( 3,  Qt::Horizontal, QObject::tr( "description" ) );
    model.setHeaderData( 4,  Qt::Horizontal, QObject::tr( "index" ) );

    resizeColumnsToContents();
    resizeRowsToContents();
    setColumnHidden( 4, true );

    horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
}

void
ControlMethodTable::setSharedPointer( std::shared_ptr< adcontrols::ControlMethod > ptr )
{
    method_ = ptr;
    setContents( *method_ );
}

bool 
ControlMethodTable::setContents( const adcontrols::ControlMethod& m )
{
    QStandardItemModel& model = *model_;
    model.setRowCount( int( m.size() ) );
    auto it = m.begin();
    for ( int row = 0; row < m.size(); ++row ) {
        model.setData( model.index( row, 0 ), it->time() );
        model.setData( model.index( row, 1 ), QString::fromStdString( it->modelname() ) );
        model.setData( model.index( row, 2 ), it->funcid() );
        model.setData( model.index( row, 3 ), QString::fromStdString( it->itemLabel() ) );
        model.setData( model.index( row, 4 ), row );
        ++it;
    }
    return true;
}

void
ControlMethodTable::addItem( const QString& text )
{
    items_ << text;
}

void
ControlMethodTable::currentChanged( const QModelIndex& curr, const QModelIndex& prev )
{
    if ( prev.isValid() ) {
        int idx = model_->index( prev.row(), 4 ).data().toInt();
        auto it = method_->begin() + idx;
        parent_->getMethod( *it );
    }
    if ( curr.isValid() ) {
        int idx = model_->index( curr.row(), 4 ).data().toInt();
        auto it = method_->begin() + idx;
        parent_->setMethod( *it );
    }
}

void
ControlMethodTable::showContextMenu( const QPoint& pt )
{
    std::vector< QAction * > actions;
    QAction * delete_action;
    QMenu menu;

    for ( auto item : items_ )
        actions.push_back( menu.addAction( "Add line: " + item ) );
    delete_action = menu.addAction( "Delete line" );

    if ( QAction * selected = menu.exec( this->mapToGlobal( pt ) ) ) {
        if ( selected == delete_action )
            ;
        else {
            for ( size_t i = 0; i < actions.size(); ++i ) {
                if ( actions[ i ] == selected ) {
                    emit onAddMethod( items_[ int( i ) ] );
                    break;
                }
            }
        }

    }

}

void
ControlMethodTable::insert( const QString& title
                            , const adcontrols::controlmethod::MethodItem& mi
                            , const QModelIndex& index )
{
    QStandardItemModel& model = *model_;

    int row = index.isValid() ? index.row() : model.rowCount();
    model.insertRow( row );

    model.setData( model.index( row, 0 ), mi.time() );
    model.setData( model.index( row, 1 ), QString::fromStdString( mi.modelname() ) );
    model.setData( model.index( row, 2 ), mi.funcid() );
    model.setData( model.index( row, 3 ), QString::fromStdString( mi.itemLabel() ) );
    size_t idx = std::distance( method_->begin(), method_->insert( mi ) );
    model.setData( model.index( row, 4 ), idx );
}

bool
ControlMethodTable::append( const adcontrols::controlmethod::MethodItem& mi )
{
    QStandardItemModel& model = *model_;

    int row = model.rowCount();
    model.setRowCount( row + 1 );

    model.setData( model.index( row, 0 ), mi.time() );
    model.setData( model.index( row, 1 ), QString::fromStdString( mi.modelname() ) );
    model.setData( model.index( row, 2 ), mi.funcid() );
    model.setData( model.index( row, 3 ), QString::fromStdString( mi.itemLabel() ) );
    size_t idx = std::distance( method_->begin(), method_->insert( mi ) );
    model.setData( model.index( row, 4 ), idx );
    return true;
}
