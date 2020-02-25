// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "folder.hpp"
#include "file.hpp"
#include "fs.hpp"
#include "adfs.hpp"
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>

using namespace adfs;

folder::~folder()
{
}

folder::folder() : db_( 0 ), rowid_( 0 )
{
}

folder::folder( const folder& t ) : attributes( t )
                                  , db_( t.db_ )
                                  , name_( t.name_ )
                                  , rowid_( t.rowid_ )
{
}

folder::folder( adfs::sqlite& db
                , boost::int64_t rowid
                , const std::wstring& name ) : db_( &db )
                                             , name_( name )
                                             , rowid_( rowid )
{
}

std::vector< folder >
folder::folders()
{
    std::vector< folder > folders;
    if ( db_ && rowid_ )
        internal::fs::select_folders( *db_, rowid_, folders );
    return folders;
}

const std::vector< folder >
folder::folders() const
{
    std::vector< folder > folders;
    if ( db_ && rowid_ )
        internal::fs::select_folders( *db_, rowid_, folders );
    return folders;
}

adfs::files
folder::files()
{
    adfs::files files;
    if ( db_ && rowid_ )
        internal::fs::select_files( *db_, rowid_, files );
    return files;
}

const adfs::files
folder::files() const
{
    adfs::files files;
    if ( db_ && rowid_ )
        internal::fs::select_files( *db_, rowid_, files );
    return files;
}

file
folder::selectFile( const std::wstring& id )
{
    adfs::file file;
    if ( db_ && rowid_ ) {
        internal::fs::select_file( *db_, rowid_, id, file );
    }
    return file;
}

namespace adfs {
    template<> ADFSSHARED_EXPORT const std::basic_string< char >
    folder::name() const
    {
        return adportable::utf::to_utf8( name_ );
    }

    template<> ADFSSHARED_EXPORT const std::basic_string< wchar_t >
    folder::name() const
    {
        return name_;
    }
}

/////////////////////////
file
folder::addFile( const std::wstring& id, const std::wstring& title )
{
    if ( db_ && rowid_ ) {
		adfs::file file = internal::fs::add_file( *this, id );
	    if ( file ) {
			file.id( id );
			if ( ! title.empty() )
				static_cast< attributes& >(file).name( title );
			return file;
		}
	}
    return file();
}

file
folder::addFile( const boost::uuids::uuid& id, const std::wstring& title )
{
    if ( db_ && rowid_ ) {
		adfs::file file = internal::fs::add_file( *this, id );
	    if ( file ) {
			file.id( adfs::to_string< wchar_t >( id ) );
			if ( ! title.empty() )
				static_cast< attributes& >(file).name( title );
			return file;
		}
	}
    return file();
}
