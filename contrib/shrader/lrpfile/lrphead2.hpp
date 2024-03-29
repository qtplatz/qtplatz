/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <array>
#include <cstdint>
#include <string>

namespace shrader {

    class lrphead2 {
        enum {
            data_size = 256
            , istd_count = 13
        };
    public:
        ~lrphead2();
        lrphead2( std::istream& in, size_t fsize );
        inline operator bool () const { return loaded_; }

        int32_t flags() const;                  // Long 4 Record type code = 1;
        std::string descline1() const;          // Sample description
        std::string descline2() const;          // Second line of description
        std::string client() const;             // Submitting client
        const int32_t * istd() const; // Internal standard amounts ( in nanograms )
    private:
        std::array< char, data_size > data_;
        bool loaded_;
    };

}

