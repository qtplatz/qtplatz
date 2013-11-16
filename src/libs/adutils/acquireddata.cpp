/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "acquireddata.hpp"
#include <adfs/sqlite.hpp>

using namespace adutils;

AcquiredData::AcquiredData()
{
}

// static
bool
AcquiredData::insert( adfs::sqlite& db
                      , uint64_t objId
                      , int64_t time
                      , int32_t pos
                      , int32_t fcn
                      , uint32_t events
                      , const int8_t * data
                      , size_t dsize
                      , const int8_t * meta
                      , size_t msize )
{
    adfs::stmt sql( db );

	sql.prepare( "INSERT INTO AcquiredData VALUES( :oid, :time, :npos, :fcn, :events, :data, :meta )" );

	sql.bind( 1 ) = objId;
	sql.bind( 2 ) = time;
	sql.bind( 3 ) = pos;
	sql.bind( 4 ) = fcn;
	sql.bind( 5 ) = events;
	sql.bind( 6 ) = adfs::blob( dsize, data );
	sql.bind( 7 ) = adfs::blob( msize, meta );

    return sql.step() == adfs::sqlite_done;
}

// static
bool
AcquiredData::create_table( adfs::sqlite& db )
{
    adfs::stmt sql( db );
    if ( sql.exec(
"CREATE TABLE AcquiredData \
(oid    INTEGER            \
,time   INTEGER            \
,npos   INTEGER            \
,fcn    INTEGER            \
,events INTEGER            \
,data   BLOB               \
,meta   BLOB               \
)"
             ) )
        return true;
    
    return false;
}
