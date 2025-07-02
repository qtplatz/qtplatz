/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "datafile_signature.hpp"
#include <adfs/sqlite.hpp>
#include <adportable/date_time.hpp>
#include <adportable/debug.hpp>
#include <adportable/iso8601.hpp>
#include <adportfolio/node.hpp>
#include <chrono>
#include <sstream>
#include <boost/json.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace adutils;
using namespace adutils::data_signature;

datafileSignature::~datafileSignature()
{
}

datafileSignature::datafileSignature()
{
}

bool
datafileSignature::create_table( adfs::sqlite& db )
{
    adfs::stmt sql( db );

    if ( sql.exec(
             "CREATE TABLE DATAFILE_SIGNATURE (\
 id                   TEXT                     \
,type                 TEXT                     \
,value                TEXT                     \
,UNIQUE(id)                                    \
)" )
        )
        return true;
    return false;
}

// using datum_t = std::tuple< std::string // id
//                           , std::string // type ('json'|'xml'|'text'|'uuid'|'isodate')
//                           , std::string // value
//                           , boost::uuids::uuid
//                           >;

namespace adutils { namespace data_signature {

        namespace {
            template<class... Ts>
            struct overloaded : Ts... { using Ts::operator()...; };
            template<class... Ts>
            overloaded(Ts...) -> overloaded<Ts...>;

            std::string to_string( const pugi::xml_document& a ) {
                std::ostringstream o;
                a.print( o );
                return o.str();
            }

            std::string to_string( const boost::json::value& a ) {
                return boost::json::serialize( a );
            }

            std::string to_string( const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds >& tp ) {
                return adportable::date_time::to_iso< std::chrono::nanoseconds >( tp );
            }
        }

        //////////
        adfs::stmt&
        operator << ( adfs::stmt& sql, datum_t&& dt )
        {
            sql.prepare( "INSERT OR REPLACE INTO DATAFILE_SIGNATURE VALUES (?,?,?)" );
            auto t = std::visit( overloaded{
                    [&]( const std::string& a ){
                        return std::make_tuple( std::string("text"), a );
                    }
                        , [&]( const boost::uuids::uuid& a ){
                            return std::make_tuple( std::string("uuid"), boost::uuids::to_string( a ) );
                        }
                        , [&]( const boost::json::value& a ){
                            return std::make_tuple( std::string("json"), to_string( a ) );
                        }
                        , [&]( const std::shared_ptr< pugi::xml_document >& a ){
                            return std::make_tuple( std::string("xml"), to_string( *a ) );
                        }
                        , [&]( const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& a ){
                            return std::make_tuple( std::string("tp"), to_string( a ) );
                        }
                        }, std::get<1>(dt));

            int col = 1;
            sql.bind( col++ ) = std::get<0>(dt);   // id
            sql.bind( col++ ) = std::get<0>( t );  // value-type
            sql.bind( col++ ) = std::get<1>(t );   // value
            if ( sql.step() != adfs::sqlite_done ) {
                ADDEBUG() << sql.errmsg() << ": " << sql.expanded_sql();
            }
            return sql;
        }
        ///////////////////
        adfs::stmt&
        operator >> (adfs::stmt& sql, std::map< std::string, value_t >& t )
        {
            if ( sql.prepare( "SELECT * FROM DATAFILE_SIGNATURE VALUES (id,type,value)" ) ) {
                while ( sql.step() == adfs::sqlite_row ) {
                    const auto id = sql.get_column_value< std::string > ( 0 );
                    const auto dt = sql.get_column_value< std::string > ( 1 );
                    if ( dt == "text" ) {
                        t.emplace( id, sql.get_column_value< std::string > ( 2 ) );
                    } else if ( dt == "uuid" ) {
                        t.emplace( id, sql.get_column_value< boost::uuids::uuid >( 2 ) );
                    } else if ( dt == "json" ) {
                        boost::system::error_code ec;
                        auto doc = boost::json::parse( sql.get_column_value< std::string >( 2 ), ec );
                        t.emplace( id, doc );
                    } else if ( dt == "xml" ) {
                        auto doc = std::shared_ptr< pugi::xml_document >();
                        doc->load_string( sql.get_column_value< std::string >( 2 ).c_str() );
                        t.emplace( id, doc );
                    } else if ( dt == "tp" ) {
                        auto value = sql.get_column_value< std::string >( 2 );
                        if ( auto tp = adportable::iso8601::parse< std::string::const_iterator
                             , std::chrono::nanoseconds>( value.begin(), value.end() ) ) {
                            t.emplace( id, *tp );
                        }
                    }
                }
            }
            return sql;
        }


    }
}
