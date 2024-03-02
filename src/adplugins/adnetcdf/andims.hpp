// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include <map>
#include <optional>
#include <string>
#include <vector>
#include <memory>

namespace adcontrols {
    class Chromatogram;
}

namespace adnetcdf {

    namespace netcdf {
        class ncfile;
    }

    namespace nc = adnetcdf::netcdf;

    class AndiMS {
        AndiMS( const AndiMS& ) = delete;
        AndiMS& operator = ( const AndiMS& ) = delete;
    public:
        ~AndiMS();
        AndiMS();
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > import( const nc::ncfile& file ) const;

        std::optional< std::string > find_global_attribute( const std::string& ) const;

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
