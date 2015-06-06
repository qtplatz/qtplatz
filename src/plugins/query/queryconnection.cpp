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
//#include "queryplotdata.hpp"
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

QueryConnection::QueryConnection()
{
}

bool
QueryConnection::connect( const std::wstring& database )
{
    if ( ( fs_ = std::make_shared< adfs::filesystem >() ) ) { // 
        if ( fs_->mount( database.c_str() ) ) {
            fs_->db().register_error_handler( [=](const char * msg){ QMessageBox::warning(0, "SQLite SQL Error", msg); });
            filename_ = database;
            return true;
        }
    }
    return false;
}

std::shared_ptr<QueryQuery>
QueryConnection::query()
{
    if ( fs_ )
        return std::make_shared<QueryQuery>( fs_->db() );
    return 0;
}

adfs::sqlite&
QueryConnection::db()
{
    if ( fs_ )
        return fs_->db();
    static adfs::sqlite dummy;
    return dummy;
}

adfs::file
QueryConnection::select_file( const std::wstring& dataGuid, const wchar_t * path )
{
    if ( auto folder = fs_->findFolder( path ) ) // L"/Processed/Spectra" | L"/Processed/Chromatograms
        return folder.selectFile( dataGuid );
    return adfs::file();
}

