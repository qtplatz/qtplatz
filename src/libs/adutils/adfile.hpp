/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <memory>

namespace adfs { class filesystem; class folder; class file; }
namespace portfolio { class Portfolio; class Folium; }
namespace boost { namespace filesystem { class path; } }
namespace adcontrols { class datafile; }

namespace adutils {
	
    // high level data file io over adfs::filesystem and adfs::file

	class adfile {
    public:
        ~adfile();
        adfile();
        adfile( const boost::filesystem::path& filename );
        operator bool () const;
        bool open( const boost::filesystem::path& );
        bool append( const portfolio::Folium&, const adcontrols::datafile& );
    private:
        std::shared_ptr< adfs::filesystem > fs_;
    };

}

