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

#ifndef QUERYCONNECTION_HPP
#define QUERYCONNECTION_HPP

#include <adfs/sqlite.hpp>
#include <adfs/file.hpp>
#include <boost/variant.hpp>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace adfs { class filesystem; class stmt; class sqlite; }

namespace chemistry {

    class ChemQuery;

    class ChemConnection : public std::enable_shared_from_this < ChemConnection > {
        ChemConnection( const ChemConnection& ) = delete;
    public:
        ChemConnection();
        ~ChemConnection();

        bool connect( const std::filesystem::path& database );

        adfs::sqlite& db();

        const std::filesystem::path& filepath() const { return filename_; }

    private:
        std::filesystem::path filename_;
        std::shared_ptr< adfs::filesystem > fs_;
    };

}

#endif // QUERYCONNECTION_HPP
