// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include "adfs.hpp"
#include "sqlite.hpp"
#include "portfolio.hpp"

#include "folder.hpp"
#include "folium.hpp"
#include "filesystem.hpp"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#define BOOST_LIB_NAME boost_filesystem
#include <boost/config/auto_link.hpp>

using namespace adfs;

portfolio::~portfolio()
{
}

portfolio::portfolio()
{
}

portfolio::portfolio( const portfolio& t ) : db_( t.db_ )
{
}

std::vector< folder >
portfolio::folders()
{
    std::vector< folder > folders;
    internal::fs::select_folders( *db_, 1, folders ); // find under '/' directory
    return folders;
}

folium
portfolio::findFolium( const std::wstring& id )
{
    // although id is the name, it should be unique due to original xml based design
    folium folium;
    if ( db_ )
        internal::fs::select_folium( *db_, id, folium );  
    return folium;
}

/////////////
bool
portfolio::create( const wchar_t * filename, size_t alloc, size_t page_size )
{
    boost::filesystem::path filepath( filename );

    if ( boost::filesystem::exists( filepath ) ) {
        boost::system::error_code ec;
        if ( ! boost::filesystem::remove( filepath, ec ) ) {
            throw adfs::exception( ec.message(), ec.category().name() );
            return false;
        }
    }

    db_.reset( new sqlite() );
    if ( db_->open( filepath.c_str() ) ) {
        adfs::stmt sql( *db_ );
        if ( page_size )
            sql.exec( ( boost::format( "PRAGMA page_size = %1%" ) % page_size ).str() );
        if ( alloc )
            internal::fs::prealloc( *db_, alloc );
        return internal::fs::format( *db_, filename );
    }
    db_.reset();
    return false;
}

bool
portfolio::mount( const wchar_t * filename )
{
    if ( db_ )
        db_.reset();

    boost::filesystem::path filepath( filename );

    db_.reset( new sqlite() );
    if ( db_->open( filepath.c_str() ) ) {
        if ( internal::fs::mount( *db_ ) )
            return true;
    }
    db_.reset();
    return false;
}

folder
portfolio::addFolder( const std::wstring& name, bool uniq )
{
    (void)uniq; // always unique
    return internal::fs::add_folder( *db_, name );
}

folder
portfolio::findFolder( const std::wstring& name ) const
{
    return internal::fs::find_folder( *db_, name );
}

