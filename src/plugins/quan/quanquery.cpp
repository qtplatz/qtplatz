/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanquery.hpp"
#include <adfs/sqlite.hpp>
#include <adfs/sqlite3.h>
#include <adportable/debug.hpp>
#include <workaround/boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <QObject>
#include <sstream>

#ifdef _MSC_VER 
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

using namespace quan;

QuanQuery::QuanQuery( adfs::sqlite& db ) : sql_( db )
{
}

QuanQuery::QuanQuery( const QuanQuery& t ) : std::enable_shared_from_this< QuanQuery >( t )
                                           , sql_( t.sql_ )
{
}

bool
QuanQuery::prepare( const std::wstring& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

bool
QuanQuery::prepare( const std::string& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

adfs::sqlite_state
QuanQuery::step()
{
    state_ = sql_.step();
    return state_;
}

size_t
QuanQuery::column_count() const
{
    return sql_.column_count();
}

QString
QuanQuery::column_name( size_t idx ) const
{
    std::string name = sql_.column_name( int( idx ) );
    return QString::fromStdString( name );
}

bool
QuanQuery::hasColumn( const std::string& column, const std::string& table )
{
    sql_.reset();
    sql_.prepare( "PRAGMA table_info('" + table + "')" );
    while ( sql_.step() == adfs::sqlite_row ) {
        std::string a = sql_.get_column_value< std::string >( 1 );
        if ( strcasecmp(a.c_str(), column.c_str() ) ) {
            sql_.reset();
            return true;
        }
    }
    return false;
}

QString
QuanQuery::column_name_tr( const QString& d )
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
QuanQuery::column_value( size_t idx ) const
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

bool
QuanQuery::buildQuery( std::string& query, int idx, bool isCounting, bool isISTD, const std::string& additional )
{
    if ( isCounting )
        return buildCountingQuery( query, idx, isISTD, additional );
    else
        return buildQuantifyQuery( query, idx, isISTD, additional );
}

bool
QuanQuery::buildCountingQuery( std::string& query, int idx, bool isISTD, const std::string& additional )
{
    std::string fmtstr;

    if ( isISTD ) {
        bool hasIsCounting = hasColumn( "isCounting", "QuanCompound" );
        if ( hasIsCounting ) {
            fmtstr = 
                "SELECT t1.uuid as 'uuid'"
                ", t1.id as id"
                ", t1.idSample as idSample"
                ", t1.name as name"
                ", t1.sampleType as sampletype"
                ", t1.level as level"
                ", t1.isCounting as isCounting"
                ", t1.formula as formula"
                ", t1.mass as mass"
                ", t1.error as 'error(mDa)'"
                ", t1.CountRate as CountRate"
                ", t2.formula as formula"
                ", t2.CountRate as CountRate"
                ", t1.CountRate/t2.CountRate AS 'Ratio'"
                ", amount"
                ", trigCounts"
                ", dataSource"
                " FROM "
                "(SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name,idSample"
                ", sampleType, QuanSample.level, QuanCompound.formula"
                ", QuanCompound.isCounting"
                ", QuanCompound.mass AS 'exact mass', QuanResponse.mass"
                ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error'"
                ", timeCounts * 100.0 / trigCounts as 'CountRate', trigCounts, QuanResponse.amount, QuanCompound.description, dataSource"
                " FROM QuanSample, QuanResponse, QuanCompound"
                " WHERE QuanSample.id = idSample"
                " AND QuanResponse.idCmpd = QuanCompound.uuid %1%) t1"
                " LEFT JOIN"
                " (SELECT idSample, timeCounts * 100.0 / trigCounts as 'CountRate',QuanResponse.formula,QuanResponse.mass"
                " FROM QuanResponse,QuanCompound"
                " WHERE QuanResponse.idCmpd=QuanCompound.uuid AND isISTD=1) t2"
                " ON t1.idSample=t2.idSample ORDER BY t1.idSample";
        } else {
            fmtstr = 
                "SELECT t1.uuid as 'uuid'"
                ", t1.id as id"
                ", t1.idSample as idSample"
                ", t1.name as name"
                ", t1.sampleType as sampletype"
                ", t1.level as level"
                ", t1.formula as formula"
                ", t1.mass as mass"
                ", t1.error as 'error(mDa)'"
                ", t1.CountRate"
                ", t2.formula as formula"
                ", t2.CountRate as CountRate"
                ", t1.CountRate/t2.CountRate AS 'Ratio'"
                ", amount"
                ", trigCounts,dataSource"
                " FROM "
                "(SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name,idSample"
                ", sampleType, QuanSample.level, QuanCompound.formula"
                ", QuanCompound.mass AS 'exact mass', QuanResponse.mass"
                ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error'"
                ", timeCounts * 100.0 / trigCounts as 'CountRate', trigCounts, QuanResponse.amount, QuanCompound.description, dataSource"
                " FROM QuanSample, QuanResponse, QuanCompound"
                " WHERE QuanSample.id = idSample"
                " AND QuanResponse.idCmpd = QuanCompound.uuid %1%) t1"
                " LEFT JOIN"
                " (SELECT idSample, timeCounts * 100.0 / trigCounts as 'CountRate',QuanResponse.formula,QuanResponse.mass"
                " FROM QuanResponse,QuanCompound"
                " WHERE QuanResponse.idCmpd=QuanCompound.uuid AND isISTD=1) t2"
                " ON t1.idSample=t2.idSample ORDER BY t1.idSample";
        }
    } else {
        fmtstr =
            "SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name"
            ", sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\""
            ", QuanResponse.mass"
            ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(Da)'"
            ", timeCounts * 100.0 / trigCounts as 'CountRate', QuanResponse.amount, QuanCompound.description, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " %1%"
            " ORDER BY QuanCompound.id";
    }
    if ( idx == 0 ) { // All
        query = ( boost::format( fmtstr ) % additional ).str();
    } else if ( idx == 1 ) { // Unknown
        query = ( boost::format( fmtstr ) % (( boost::format( "%1% AND sampleType = 0" ) % additional ).str()) ).str();
    } else if ( idx == 2 ) { // Standard
        query = ( boost::format( fmtstr ) % (( boost::format( "%1% AND sampleType = 1" ) % additional ).str()) ).str();
    } else if ( idx == 3 ) { // QC
        fmtstr = 
            "SELECT QuanCompound.uuid, QuanSample.name, sampleType, QuanCompound.formula"
            ", QuanCompound.mass AS \"exact mass\", QuanResponse.mass "
            ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
            ", timeCounts * 100.0 / trigCounts as 'count rate', QuanSample.level, QuanAmount.amount, QuanCompound.description"
            ", sampleType, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " AND sampleType = 2"
            " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
            " %1%"
            " ORDER BY QuanCompound.id, QuanSample.level";
        query = ( boost::format( fmtstr ) % additional ).str();
    } else if ( idx == 4 ) { // Blank
        fmtstr = 
            "SELECT QuanCompound.uuid, QuanSample.name, sampleType, QuanCompound.formula"
            ", QuanCompound.mass AS \"exact mass\", QuanResponse.mass"
            ", (QuanCompound.mass - QuanResponse.mass)*1000 AS 'error(mDa)'"
            ", timeCounts * 100.0 / trigCounts as 'counts/min', QuanSample.level, QuanAmount.amount, QuanCompound.description"
            ", sampleType, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " AND sampleType = 3"
            " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
            " %1%"
            " ORDER BY QuanCompound.id, QuanSample.level";
        query = ( boost::format( fmtstr ) % additional ).str();
    }
    return true;
}

bool
QuanQuery::buildQuantifyQuery( std::string& query, int idx, bool isISTD, const std::string& additional )
{
    std::string fmtstr;
    
    if ( idx == 0 ) { // All
        
        fmtstr = "SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name"
            ", sampleType, QuanSample.level, QuanCompound.formula"
            ", QuanCompound.mass AS \"exact mass\", QuanResponse.mass"
            ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
            ", intensity, QuanResponse.amount, QuanCompound.description, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " %1%"
            " ORDER BY QuanCompound.id, QuanSample.level";

    } else if ( idx == 1 ) { // Unknown
        
        fmtstr = "SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name"
            ", sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\""
            ", QuanResponse.mass"
            ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
            ", intensity, QuanResponse.amount, QuanCompound.description, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " AND sampleType = 0 "
            " %1%"            
            " ORDER BY QuanCompound.id";
    }
    else if ( idx == 2 ) { // Standard

        fmtstr = "SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name"
            ", sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\""
            ", QuanResponse.mass"
            ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
            ", intensity, QuanSample.level, QuanAmount.amount, QuanCompound.description, sampleType, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " AND sampleType = 1 "
            " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
            " %1%"
            " ORDER BY QuanCompound.id, QuanSample.level";
    }
    else if ( idx == 3 ) { // QC

        fmtstr = "SELECT QuanCompound.uuid, QuanSample.name, sampleType, QuanCompound.formula"
            ", QuanCompound.mass AS \"exact mass\", QuanResponse.mass "
            ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
            ", intensity, QuanSample.level, QuanAmount.amount, QuanCompound.description, sampleType, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " AND sampleType = 2"
            " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
            " %1%"
            " ORDER BY QuanCompound.id, QuanSample.level";
        
    } else if ( idx == 4 ) { // Blank

        fmtstr = "SELECT QuanCompound.uuid, QuanSample.name, sampleType, QuanCompound.formula"
            ", QuanCompound.mass AS \"exact mass\", QuanResponse.mass"
            ", (QuanCompound.mass - QuanResponse.mass)*1000 AS 'error(mDa)'"
            ", intensity, QuanSample.level, QuanAmount.amount, QuanCompound.description, sampleType, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " AND sampleType = 3"
            " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
            " %1%"            
            " ORDER BY QuanCompound.id, QuanSample.level";
    }

    if ( fmtstr.empty() )
        return false;

    query = ( boost::format( fmtstr ) % additional ).str();
    
    return true;
}
