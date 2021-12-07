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

#pragma once

#include "../adcontrols_global.h"
#include <boost/json/fwd.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <boost/uuid/uuid.hpp>
#include <string>

namespace adcontrols {
    namespace quan {

        struct ADCONTROLSSHARED_EXPORT targeting_candidate;

        struct targeting_candidate {
            double matchedMass;
            double mass_error;
            int32_t idx;
            int32_t fcn;
            uint32_t charge;
            std::string formula;
            targeting_candidate( double mass = 0
                                   , double error = 0
                                   , int32_t _idx = 0
                                   , int32_t _fcn = 0
                                   , uint32_t _charge = 1
                                   , const std::string& _formula = "")
                : matchedMass(mass), mass_error(error), idx(_idx), fcn(_fcn), charge(_charge), formula(_formula) {
            }
            targeting_candidate( const targeting_candidate& t )
                : matchedMass(t.matchedMass)
                , mass_error(t.mass_error)
                , idx(t.idx)
                , fcn(t.fcn)
                , charge(t.charge)
                , formula(t.formula) {
            }
        };

        // targeting_candidate
        ADCONTROLSSHARED_EXPORT
        void tag_invoke( boost::json::value_from_tag
                         , boost::json::value&, const targeting_candidate& );

        ADCONTROLSSHARED_EXPORT
        targeting_candidate tag_invoke( boost::json::value_to_tag< targeting_candidate >&
                             , const boost::json::value& jv );

    }
}
