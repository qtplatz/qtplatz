/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "isotope.hpp"
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>

namespace adcontrols {

    namespace targeting {

        isotope::isotope()
            : idx(-1), mass(0), abundance_ratio(0), abundance_ratio_error(0), exact_mass(0), exact_abundance(0)
        {
        }

        isotope::isotope( size_t _1, double _2, double _3, double _4, double _5, double _6 )
            : idx(_1), mass(_2), abundance_ratio(_3), abundance_ratio_error(_4), exact_mass(_5), exact_abundance(_6)
        {
        }

        isotope::isotope( const isotope& t )
            : idx( t.idx ), mass( t.mass ), abundance_ratio( t.abundance_ratio ), abundance_ratio_error( t.abundance_ratio_error )
            , exact_mass( t.exact_mass ), exact_abundance(t.exact_abundance)
        {
        }


        void
        tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const isotope& t )
        {
            jv = boost::json::object{ { "isotope"
                    , {
                        {   "idx",                   t.idx }
                        , { "abundance_ratio",       t.abundance_ratio }
                        , { "abundance_ratio_error", t.abundance_ratio_error }
                        , { "exact_mass",            t.exact_mass }
                        , { "exact_abundance",       t.exact_abundance }
                    }
                }};
        }

        isotope
        tag_invoke( const boost::json::value_to_tag< isotope >&, const boost::json::value& jv )
        {
            isotope t;
            using namespace adportable::json;
            if ( jv.is_object() ) {
                auto obj = jv.as_object();
                extract( obj, t.idx,                   "idx" );
                extract( obj, t.abundance_ratio,       "abundance_ratio" );
                extract( obj, t.abundance_ratio_error, "abundance_ratio_error" );
                extract( obj, t.exact_mass,            "exact_mass" );
                extract( obj, t.exact_abundance,       "exact_abundance" );
            }
            return t;
        }

    }
}
