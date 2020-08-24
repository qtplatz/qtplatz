/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <adfs/folder.hpp>
#include <boost/filesystem/path.hpp>
#include <memory>
#include <vector>

namespace adfs { class filesystem; class folder; }


namespace adexport {

    class Export {
        std::unique_ptr< adfs::filesystem > fs_;
    public:
        ~Export();
        Export();
        bool open( const boost::filesystem::path& path );
        bool loadFolders();
        inline const std::vector< adfs::folder >& folders() const { return folders_; }
        bool out( const adfs::folder&, std::ostream& ) const;
        
    private:
        bool list( const boost::filesystem::path& path, const adfs::folder&, std::vector< adfs::folder >& ) const;
        bool out( const adfs::file& file, std::ostream& o, const std::string& header ) const;
        std::vector< adfs::folder > folders_;
    };

}

