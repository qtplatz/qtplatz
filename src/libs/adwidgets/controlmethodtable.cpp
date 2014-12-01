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
#include <adcontrols/controlmethod.hpp>
#include <qtwrapper/font.hpp>

#include <QStandardItemModel>
#include <QHeaderView>
#include <QMenu>
#include <boost/exception/all.hpp>

using namespace adwidgets;

ControlMethodTable::ControlMethodTable( QWidget *parent ) : adwidgets::TableView( parent )
                                                          , model_( new QStandardItemModel )
{
    setModel( model_ );

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

	model.setColumnCount( 3 );
    model.setRowCount( 1 );

    model.setHeaderData( 0,  Qt::Horizontal, QObject::tr( "time(min)" ) );
    model.setHeaderData( 1,  Qt::Horizontal, QObject::tr( "module" ) );
    model.setHeaderData( 2,  Qt::Horizontal, QObject::tr( "description" ) );

    resizeColumnsToContents();
    resizeRowsToContents();

    horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
}

void
ControlMethodTable::setSharedPointer( std::shared_ptr< adcontrols::ControlMethod > ptr )
{
    method_ = ptr;
    setContents( *method_ );
}

bool 
ControlMethodTable::getContents( adcontrols::ControlMethod& cm )
{
    if ( method_ ) {
        cm = *method_;
        return true;
    }
    return false;
}

bool 
ControlMethodTable::setContents( const adcontrols::ControlMethod& m )
{
    QStandardItemModel& model = *model_;
    model.setRowCount( m.size() );
    auto it = m.begin();
    for ( int row = 0; row < m.size(); ++row ) {
        model.setData( model.index( row, 0 ), it->time() );
        model.setData( model.index( row, 1 ), QString::fromStdString( it->modelname() ) );
        ++it;
    }
    return true;
}

bool
ControlMethodTable::append( const adcontrols::controlmethod::MethodItem& item )
{
    (void)item;
    return false;
}



const adcontrols::controlmethod::MethodItem&
ControlMethodTable::operator [] ( int row ) const
{
    typedef boost::error_info< struct tag_errmsg, std::string > info;
    struct error : virtual boost::exception, virtual std::exception {};

    if ( !method_ ) {
        BOOST_THROW_EXCEPTION( error() << info( "no method" ) );
    }
    if ( unsigned(row) >= method_->size() || row < 0 ) {
        BOOST_THROW_EXCEPTION( error() << info( "subscript out of range" ) );
    }

    return *method_->begin();
}

void
ControlMethodTable::addItem( const QString& text )
{
    items_ << text;
}

void
ControlMethodTable::showContextMenu( const QPoint& pt )
{
    std::vector< QAction * > actions;
    QMenu menu;
    
    actions.push_back( menu.addAction( "Add to peak table" ) );
    QAction * selected = menu.exec( this->mapToGlobal( pt ) );
}
