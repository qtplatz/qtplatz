/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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

#include <boost/uuid/uuid.hpp>
#include <QString>
#include <memory>
#include <filesystem>
#include <vector>

namespace adcontrols {
    class MassSpectrum;
}

namespace adfs {
    class filesystem;
    class sqlite;
}

namespace tools {

    class document {
    public:
        static bool appendOnFile( const std::filesystem::path& path
                                  , const QString& title
                                  , const adcontrols::MassSpectrum& ms
                                  , QString& id );


        static std::shared_ptr< adcontrols::MassSpectrum>
        histogram( std::vector< size_t >& hist, const adcontrols::MassSpectrum& ms, double v_th );

        static document * instance();

        bool prepareStorage( adfs::filesystem& fs ) const;
        bool closingStorage( const boost::uuids::uuid&, adfs::filesystem& fs ) const;
    private:
        bool initStorage( const boost::uuids::uuid&, adfs::sqlite& db ) const;
    };
}
