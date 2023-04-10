/**************************************************************************
** Copyright (C) 2022-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcontrols/make_combination.hpp>
#include <filesystem>

namespace adcontrols { class IonReactionMethod; }

namespace lipidid {

    class SQLExport {
        SQLExport( const SQLExport& ) = delete;
        SQLExport& operator = (const SQLExport& ) = delete;
    public:
        SQLExport();
        SQLExport( adcontrols::IonReactionMethod&& );
        ~SQLExport();

        bool create_database( const std::filesystem::path& filename );
        bool create_tables();
        bool drop_tables();

        bool export_ion_reactions( adcontrols::IonReactionMethod&& t, bool testing );

    private:
        bool insert_molecule( int64_t molid, const adcontrols::mol::molecule&, adcontrols::ion_polarity ) const;
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
