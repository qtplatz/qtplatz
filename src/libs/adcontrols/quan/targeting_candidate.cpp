/**************************************************************************
** Copyright (C) 2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "targeting_candidate.hpp"

#include "../adcontrols_global.h"
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>

namespace adcontrols {
    namespace quan {

        // targeting_candidate
        ADCONTROLSSHARED_EXPORT
        void tag_invoke( const boost::json::value_from_tag
                         , boost::json::value& jv, const targeting_candidate& t )
        {
            jv = {
                { "matchedMass",  t.matchedMass }
                , { "mass_error", t.mass_error }
                , { "idx",        t.idx }
                , { "fcn",        t.fcn }
                , { "charge",     t.charge }
                , { "formula",    t.formula }
            };
        }

        ADCONTROLSSHARED_EXPORT
        targeting_candidate tag_invoke( const boost::json::value_to_tag< targeting_candidate >&
                                        , const boost::json::value& jv )
        {
            targeting_candidate t;
            using namespace adportable::json;
            if ( jv.is_object() ) {
                auto obj = jv.as_object();
                extract( obj, t.matchedMass, "matchedMass" );
                extract( obj, t.mass_error,  "mass_error" );
                extract( obj, t.idx,         "idx" );
                extract( obj, t.charge,      "charge" );
                extract( obj, t.formula,     "formula" );
            }
            return t;
        }

    }
}
