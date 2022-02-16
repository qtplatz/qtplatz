/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "isopeak.hpp"


namespace lipidid {

    isoPeak::isoPeak( size_t _0, double _1, double _2 ) : idx(_0), rel_ra(_1), mass_error(_2)
    {
    }

    isoPeak::isoPeak( const isoPeak& t ) : idx(t.idx), rel_ra(t.rel_ra), mass_error(t.mass_error)
    {
    }

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const isoPeak& t )
    {
        jv = boost::json::object{{ "idx", t.idx }, { "rel_ra", t.rel_ra }, { "mass_error", t.mass_error }};
    }
}
