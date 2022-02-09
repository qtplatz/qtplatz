// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
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

#include "adportable_global.h"
#include <boost/variant.hpp>
#include <boost/spirit/home/x3.hpp>
#include <memory>
#include <vector>

namespace adportable {
    namespace csv {
        namespace x3 = boost::spirit::x3;

        typedef boost::variant< x3::unused_type, std::string, double, int > variant_type;
        typedef std::vector< variant_type > list_type;

        class ADPORTABLESHARED_EXPORT csv_reader;

        class csv_reader {
            csv_reader( const csv_reader& ) = delete;
            csv_reader& operator = ( const csv_reader& ) = delete;
        public:
            ~csv_reader();
            csv_reader( const std::string& file );
            bool read( list_type& );

        private:
            class impl;
            std::unique_ptr< impl > impl_;
        };

    }
}
