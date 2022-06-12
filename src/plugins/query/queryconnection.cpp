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

#include "queryconnection.hpp"
#include <adfs/cpio.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/processmethod.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <dataproc/dataprocconstants.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <QMessageBox>
#include <QSqlDriver>
#include <QVariant>
#include <adfs/sqlite3.h>

using namespace query;

QueryConnection::~QueryConnection()
{
}

QueryConnection::QueryConnection() : db_( QSqlDatabase::addDatabase( "QSQLITE" ) )
{
}

bool
QueryConnection::connect( const std::wstring& database )
{
    db_.setDatabaseName( QString::fromStdWString( database ) );

    if ( db_.open() ) {
        filename_ = database;

        auto v = db_.driver()->handle();
        if ( v.isValid() && qstrcmp( v.typeName(), "sqlite3*" ) == 0 ) {
            sqlite3_initialize();
            if ( auto db = *static_cast< sqlite3 **>(v.data()) ) {
#if 1
                sqlite3_enable_load_extension( db, 1 );
                auto query = QSqlQuery( "SELECT load_extension( 'sqlite3-functions' )", db_ );
                if ( query.exec() ) {
                    ADDEBUG() << "sqlite3-functions loaded";
                }
                sqlite3_enable_load_extension( db, 0 );
#else
                int ret=0;
                if ( sqlite3_db_config( db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, &ret ) == adfs::sqlite_ok ) {
                    char * errmsg = 0;
                    if (auto code = sqlite3_load_extension( db, "sqlite3-functions.so", 0, &errmsg ) ) {
                        ADDEBUG() << errmsg << ", code=" << code;
                    }
                }
#endif
            }
        }

        return true;
    }

    QMessageBox::critical( 0
                           , QObject::tr( "Cannot open database" )
                           , QObject::tr( "Unable to establish a database connection.\nClick Cancel to exit." ), QMessageBox::Cancel );

    return false;
}

QSqlDatabase&
QueryConnection::sqlDatabase()
{
    return db_;
}

QSqlQuery
QueryConnection::sqlQuery( const QString& query )
{
    return QSqlQuery( query, db_ );
}
