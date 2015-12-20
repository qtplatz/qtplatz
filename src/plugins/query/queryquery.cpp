/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "queryquery.hpp"
#include <adfs/sqlite.hpp>
#include <adfs/sqlite3.h>
#include <boost/uuid/uuid_io.hpp>
#include <workaround/boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>
#include <QObject>

using namespace query;

QueryQuery::QueryQuery( adfs::sqlite& db ) : sql_( db )
{
}

QueryQuery::QueryQuery( const QueryQuery& t ) : std::enable_shared_from_this< QueryQuery >( t )
                                           , sql_( t.sql_ )
{
}

bool
QueryQuery::prepare( const std::wstring& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

bool
QueryQuery::prepare( const std::string& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

adfs::sqlite_state
QueryQuery::step()
{
    state_ = sql_.step();
    return state_;
}

size_t
QueryQuery::column_count() const
{
    return sql_.column_count();
}

QString
QueryQuery::column_name( size_t idx ) const
{
    std::string name = sql_.column_name( int( idx ) );
    return QString::fromStdString( name );
}

QString
QueryQuery::column_name_tr( const QString& d )
{
    struct column_name {
        const QString loc_c;
        const QString i10n;
    };
    column_name names[] = {
            { "dataSource", QObject::tr( "dataSource" ) }
            , { "formula", QObject::tr( "formula" ) }
            , { "mass", QObject::tr( "mass" ) }
            , { "exact mass", QObject::tr( "exact mass" ) }
            , { "error(Da)", QObject::tr( "error(Da)" ) }
            , { "row", QObject::tr( "row" ) }
            , { "level", QObject::tr( "level" ) }
            , { "intensity", QObject::tr( "intensity" ) }
            , { "sampleType", QObject::tr( "sample type" ) }
            , { "name", QObject::tr( "name" ) }
            , { "description", QObject::tr( "description" ) }
            , { "amount", QObject::tr( "amount" ) }
            , { "idCompound", QObject::tr( "id" ) }
    };
    
    for ( auto& t : names ) {
        if ( t.loc_c == d )
            return t.i10n;
    }
    return d;
}


QVariant
QueryQuery::column_value( size_t idx ) const
{
    switch ( sql_.column_type( int( idx ) ) ) {
    case SQLITE_INTEGER: return QVariant( static_cast< qlonglong >( sql_.get_column_value< int64_t >( int( idx ) ) ) );
    case SQLITE_FLOAT:   return QVariant( sql_.get_column_value< double >( int( idx ) ) );
    case SQLITE_TEXT:    return QVariant( QString( sql_.get_column_value< std::string >( int( idx ) ).c_str() ) );
    case SQLITE_BLOB: {
        try {
            auto uuid = sql_.get_column_value< boost::uuids::uuid >( int( idx ) );
            return QVariant( QString( boost::lexical_cast<std::string>(uuid).c_str() ) );
        }
        catch ( boost::bad_lexical_cast& ) {
        }
    }
    case SQLITE_NULL:    return QVariant();
    }
    return QVariant();
}

