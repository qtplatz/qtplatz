// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <vector>
#include <boost/cstdint.hpp>
#include "attributes.hpp"

namespace adfs {

    class file;
    class sqlite;

    class folder : public attributes {
    public:
        ~folder();
        folder();
        folder( const folder& );
        folder( sqlite&, boost::int64_t, const std::wstring& name );

        std::vector< folder > folders();
        const std::vector< folder > folders() const;
        std::vector< file > files();
        const std::vector< file > files() const;

        file selectFile( const std::wstring& );

        // --- add/modify features
        file addFile( const std::wstring& id, const std::wstring& title = L"" );

        inline boost::int64_t rowid() const { return rowid_; }
        inline const std::wstring& name() const { return name_; }
        inline sqlite& db() const { return *db_; }
    private:
        sqlite * db_;
        std::wstring name_;
        boost::int64_t rowid_;
    };

    typedef std::vector< adfs::folder > folders;

}

