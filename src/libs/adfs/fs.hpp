// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "file.hpp"

namespace adfs {

    class sqlite;
    class folder;
	class stmt;

    namespace internal {

        enum dir_type { type_folder = 1, type_file = 2, type_attachment = 3 };

        struct dml {
            static bool insert_directory( adfs::sqlite& db, dir_type, boost::int64_t parent_id, const std::wstring& name );

			static int64_t select_directory( adfs::sqlite&, dir_type, int64_t parent_id, const std::wstring& name );
            static bool update_mtime( adfs::stmt&, int64_t fileid );
            static adfs::file insert_file( adfs::sqlite& db, dir_type, int64_t dirid, const std::wstring& name );
        };

        class fs {
        public:
            static bool format( sqlite& db, const std::wstring& filename );
            static bool format_superblock( sqlite& db, const std::wstring& filename );
            static bool format_directory( sqlite& db );
            static bool mount( sqlite& db );
            static bool prealloc( adfs::sqlite& db, uint64_t size );
            
            static folder add_folder( adfs::sqlite& db, const std::wstring& fullpath, bool create );  // full path required
            static folder find_folder( adfs::sqlite& db, const std::wstring& fullpath ); // full path required
            static folder get_parent_folder( adfs::sqlite& db, int64_t rowid );
            static file add_file( const folder&, const std::wstring& name );
            static file add_attachment( const file&, const std::wstring& name );
            
            static bool select_folders( adfs::sqlite& db, int64_t parent_id, std::vector<folder>& );
            static bool select_file( adfs::sqlite&, int64_t parent_id, const std::wstring& id, file& );
            static bool select_files( adfs::sqlite& db, int64_t parent_id, files& );
            
            static bool write( adfs::sqlite& db, int64_t fileid, size_t size, const char_t * pbuf );
            static int64_t rowid_from_fileid( adfs::sqlite&, int64_t fileid );
            static bool read( adfs::sqlite& db, int64_t rowid, size_t size, char_t * pbuf );
            static std::size_t size( adfs::sqlite& db, int64_t rowid );
        };

    } // internal
} // adfs

