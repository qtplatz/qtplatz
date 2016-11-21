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
#include "queryquery.hpp"
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

