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

#include <vector>
#include <boost/cstdint.hpp>
#include "attributes.h"

namespace adfs {

    class folium;
    class sqlite;

    class folder : public internal::attributes {
    public:
        ~folder();
        folder();
        folder( const folder& );
        folder( sqlite&, boost::int64_t, const std::wstring& name );

        std::vector< folder > folders();
        std::vector< folium > folio();
        folium selectFolium( const std::wstring& );

        // --- add/modify features
        folium addFolium( const std::wstring& name );

        inline boost::int64_t rowid() const { return rowid_; }
        inline const std::wstring& name() const { return name_; }
        inline sqlite& db() const { return *db_; }
    private:
        sqlite * db_;
        std::wstring name_;
        boost::int64_t rowid_;
    };

}

