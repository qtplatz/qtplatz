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
#include <tuple>
#include <optional>

namespace shrader {

    using cal_data = std::tuple< int32_t, float, double, double >;
    enum { cal_mass, cal_intens, cal_coeff_a, cal_coeff_b };

    class lrpcalib {
        enum {
            data_size = 256
            , record_type_code = 3
        };
    public:
        enum { cal_size = 10 };

        ~lrpcalib();
        lrpcalib();
        lrpcalib( const lrpcalib& );

        bool load( std::istream& in, size_t fsize );
        inline operator bool () const { return loaded_; }

        int32_t flags() const;
        std::string type() const;
        cal_data cal_data( size_t idx ) const;

    private:
        bool loaded_;
        std::array< char, data_size > data_;
    };

}
