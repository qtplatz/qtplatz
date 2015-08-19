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

#include "chemquery.hpp"
#include <adfs/sqlite.hpp>
#include <adfs/sqlite3.h>
#include <adportable/uuid.hpp>
#include <workaround/boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>
#include <QByteArray>
#include <QObject>

using namespace chemistry;

ChemQuery::ChemQuery(adfs::sqlite& db) : sql_( db )
{
}

ChemQuery::ChemQuery( const ChemQuery& t ) : std::enable_shared_from_this< ChemQuery >( t )
                                           , sql_( t.sql_ )
{
}

bool
ChemQuery::prepare( const std::wstring& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

bool
ChemQuery::prepare( const std::string& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

adfs::sqlite_state
ChemQuery::step()
{
    state_ = sql_.step();
    return state_;
}

size_t
ChemQuery::column_count() const
{
    return sql_.column_count();
}

QString
ChemQuery::column_name( size_t idx ) const
{
    std::string name = sql_.column_name( int( idx ) );
    return QString::fromStdString( name );
}

QString
ChemQuery::column_name_tr( const QString& d )
{
    struct column_name {
        const QString loc_c;
        const QString i10n;
    };
    column_name names[] = {
        { "smiles", QObject::tr( "SMILES" ) }
        , { "formula", QObject::tr( "Formula" ) }
        , { "mass", QObject::tr( "Mass" ) }
        , { "svg", QObject::tr( "Structure" ) }
        , { "synonym", QObject::tr( "Synonym" ) }
        , { "casrn", QObject::tr( "CAS NO." ) }
        , { "SystematicName", QObject::tr( "Systematic Name" ) }
    };
    
    for ( auto& t : names ) {
        if ( t.loc_c == d )
            return t.i10n;
    }
    return d;
}


QVariant
ChemQuery::column_value( size_t idx ) const
{
    switch ( sql_.column_type( int( idx ) ) ) {
    case SQLITE_INTEGER: return QVariant( static_cast< qlonglong >( sql_.get_column_value< int64_t >( int( idx ) ) ) );
    case SQLITE_FLOAT:   return QVariant( sql_.get_column_value< double >( int( idx ) ) );
    case SQLITE_TEXT:    return QVariant( QString( sql_.get_column_value< std::string >( int( idx ) ).c_str() ) );
    case SQLITE_BLOB: {
        try {
            if ( sql_.column_name( int( idx ) ) == "svg" ) {
                auto blob = sql_.get_column_value < adfs::blob >( int( idx ) );
                return QByteArray( reinterpret_cast<const char *>( blob.data() ), int( blob.size() ) );
            } else if ( sql_.column_name( idx ) == "uuid" ) {
                auto uuid = sql_.get_column_value< boost::uuids::uuid >( int( idx ) );
                return QVariant( QString( boost::lexical_cast<std::string>( uuid ).c_str() ) );
            }
        }
        catch ( boost::bad_lexical_cast& ) {
        }
    }
    case SQLITE_NULL:    return QVariant();
    }
    return QVariant();
}

boost::uuids::uuid
ChemQuery::insert_mol( const std::string& smiles, const std::string& svg, const std::string& formula, double mass, const std::string& synonym )
{
    if ( sql_.prepare( "INSERT INTO mols (uuid,smiles,svg,formula,mass,synonym) VALUES(?,?,?,?,?,?)" ) ) {

        auto uuid = adportable::uuid()();

        sql_.bind( 1 ) = uuid;
        sql_.bind( 2 ) = smiles;
        sql_.bind( 3 ) = adfs::blob( svg.size(), svg.data() );
        sql_.bind( 4 ) = formula;
        sql_.bind( 5 ) = mass;
        sql_.bind( 6 ) = synonym;

        if ( sql_.step() == adfs::sqlite_done ) {
            insert_synonym( uuid, synonym );
            return uuid;
        }
        
    }
    return boost::uuids::uuid();
}

bool
ChemQuery::insert_synonym( const boost::uuids::uuid&, const std::string& synonym )
{
    if ( sql_.prepare( "INSERT INTO synonyms (uuid,synonym) VALUES(?,?)" ) ) {

        auto uuid = adportable::uuid()();

        sql_.bind( 1 ) = uuid;
        sql_.bind( 2 ) = synonym;

        return sql_.step() == adfs::sqlite_done;

    }

    return false;
}

