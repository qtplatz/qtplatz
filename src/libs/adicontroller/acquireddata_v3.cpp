/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "acquireddata_v3.hpp"
#include <adicontroller/signalobserver.hpp>
#include <adfs/sqlite.hpp>
#include <boost/uuid/uuid.hpp>

using namespace adicontroller::v3;

AcquiredData::AcquiredData()
{
}

// static
bool
AcquiredData::insert( adfs::sqlite& db
                         , const boost::uuids::uuid& objid
                         , const adicontroller::SignalObserver::DataReadBuffer& rb )
{
    adfs::stmt sql( db );

	sql.prepare( "INSERT INTO AcquiredData VALUES( :objuuid, :elapsed_time, :epoch_time, :npos, :fcn, :events, :data, :meta )" );

    sql.bind( 1 ) = objid;
    sql.bind( 2 ) = rb.elapsed_time();
    sql.bind( 3 ) = rb.epoch_time();
	sql.bind( 4 ) = rb.pos();
	sql.bind( 5 ) = rb.fcn();
	sql.bind( 6 ) = rb.events();
    sql.bind( 7 ) = adfs::blob( rb.xdata().size(), reinterpret_cast<const int8_t *>( rb.xdata().data() ) );
    sql.bind( 8 ) = adfs::blob( rb.xmeta().size(), reinterpret_cast<const int8_t *>( rb.xmeta().data() ) );

    return sql.step() == adfs::sqlite_done;
}

// static
bool
AcquiredData::create_table_v3( adfs::sqlite& db )
{
    adfs::stmt sql( db );
    if ( sql.exec(
             "CREATE TABLE \
 AcquiredData              \
(objuuid      UUID         \
,elapsed_time INTEGER      \
,epoch_time   INTEGER      \
,npos         INTEGER      \
,fcn          INTEGER      \
,events       INTEGER      \
,data         BLOB         \
,meta         BLOB         \
)"
             ) )
        return true;
    
    return false;
}
