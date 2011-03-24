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

#pragma once

#include <string>

namespace adfs {

    class sqlite;
    class folium;
    class folder;

    class filesystem {
        sqlite * db_;
    public:
        ~filesystem();
        filesystem();
        bool create( const wchar_t * filename, size_t alloc = 0, size_t page_size = 8192 );
        bool mount( const wchar_t * filename );
        bool close();
        //
        folder addFolder( const wchar_t * path );
    private:
        bool prealloc( size_t size );
    };

    namespace internal {
        class fs {
        public:
            static bool format( sqlite& db, const std::wstring& filename );
            static bool format_superblock( sqlite& db, const std::wstring& filename );
            static bool format_directory( sqlite& db );
            static bool mount( sqlite& db );
            static bool prealloc( adfs::sqlite& db, unsigned long long size );

            static folder add_folder( adfs::sqlite& db, const std::wstring& fullpath );  // full path required
            static folder get_parent_folder( adfs::sqlite& db, boost::int64_t rowid );
            static folium add_folium( const folder&, const std::wstring& name );
            static folium add_attachment( const folium&, const std::wstring& name );
        };
    };

} // adfs

