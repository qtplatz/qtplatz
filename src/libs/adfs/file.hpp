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
#include <boost/any.hpp>
#include <boost/cstdint.hpp>
#include "attributes.hpp"

namespace adfs {

    class folder;
    class sqlite;
    class ostreambuf;
    class istreambuf;

    // file class represents folium on portfolio library on adfs::filesystem, so that it is responsible
    // relatively low level io

    class file : public attributes {
    public:
        ~file();
        file();
        file( const file& );
        file( sqlite&, boost::int64_t rowid, const std::wstring& name, bool is_attachment = false );

    public:

        bool empty() const;
        void operator = ( boost::any& );
        operator boost::any& ();

        std::vector< file > attachments();
        const std::vector< file > attachments() const;
        folder getParentFolder();

        std::size_t write( std::size_t size, const char_t * );
        std::size_t read( std::size_t size, char_t * );
        std::size_t size() const;
        bool resize( const std::size_t );

        // --- create/modify
        file addAttachment( const std::wstring& name );
        inline sqlite& db() const { return *db_; }
        inline const std::wstring& name() const { return name_; }
        inline int64_t rowid() const { return rowid_; }  // rowid on table 'directory'

        template<typename T> bool fetch( T& t ) { // this may raise std::exception
            std::vector< adfs::char_t > iobuf( size() );
            if ( read( iobuf.size(), iobuf.data() ) == iobuf.size() )
                return adfs::cpio<T>::deserialize( t, iobuf.data(), iobuf.size() );
            return false;
        }

    private:
        sqlite * db_;
        std::wstring name_;
        int64_t rowid_;   // rowid on 'directory'
        int64_t fileid_;  // rowid on 'file'
        bool is_attachment_;
    };

    typedef std::vector< adfs::file > files;

}

