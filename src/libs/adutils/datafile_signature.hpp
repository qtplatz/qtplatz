/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <boost/uuid/uuid.hpp>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include "adutils_global.h"
#include <tuple>

namespace adfs { class sqlite; class stmt; }

namespace adutils {

    namespace data_signature {

        using datum_t = std::tuple< std::string // id
                                    , std::string // type ('json'|'xml'|'text'|'uuid'|'isodate')
                                    , std::string // value
                                    , boost::uuids::uuid
                                    >;

        class ADUTILSSHARED_EXPORT datafileSignature {
        public:
            ~datafileSignature();
            datafileSignature();
            static void create_table( const adfs::sqlite& );
        };

        adfs::stmt& operator << (adfs::stmt& sql, datum_t );

    };
}
