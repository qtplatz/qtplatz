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
#include "adfs_global.h"
#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include <mutex>

namespace boost { namespace filesystem { class path; } }

namespace adfs {

    class sqlite;
    class file;
    class folder;

    class ADFSSHARED_EXPORT filesystem {
        pragma_msvc_warning_push_disable_4251
        std::shared_ptr< adfs::sqlite > db_;
        std::mutex mutex_;
        std::string filename_;
        pragma_msvc_warning_pop
        int format_version_;
    public:
        ~filesystem();
        filesystem();
		filesystem( const filesystem& );

        bool create( const boost::filesystem::path&, size_t alloc = 0, size_t page_size = 8192 );
        bool create( const wchar_t * filename, size_t alloc = 0, size_t page_size = 8192 );
        bool create( const char * filename, size_t alloc = 0, size_t page_size = 8192 );        
        bool mount( const boost::filesystem::path& filename );
        bool mount( const wchar_t * filename );
        bool mount( const char * filename );        
        bool close();
        const std::string& filename() const;
        //
        folder addFolder( const std::wstring& name, bool create = true );
        folder findFolder( const std::wstring& name ) const;
        file findFile( const folder&, const std::wstring& id );
        std::vector< folder > folders();
        inline sqlite& db() const { return *db_; }
        inline std::shared_ptr< adfs::sqlite > _ptr() const { return db_; }
        int format_version() const;
    };

} // adfs

