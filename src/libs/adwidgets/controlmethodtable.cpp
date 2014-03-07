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

    model.setHeaderData( 0,  Qt::Horizontal, QObject::tr( "time(s)" ) );
    model.setHeaderData( 1,  Qt::Horizontal, QObject::tr( "module" ) );
    model.setHeaderData( 2,  Qt::Horizontal, QObject::tr( "description" ) );

    resizeColumnsToContents();
    resizeRowsToContents();

    horizontalHeader()->setResizeMode( QHeaderView::Stretch );
}

void
ControlMethodTable::setContents( const adcontrols::ControlMethod& cm )
{
    method_ = std::make_shared< adcontrols::ControlMethod >( cm );
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
    if ( row >= method_->size() || row < 0 ) {
        BOOST_THROW_EXCEPTION( error() << info( "subscript out of range" ) );
    }

    return *method_->begin();
}






