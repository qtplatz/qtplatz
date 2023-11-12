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

#pragma once

#include "isopeak.hpp"
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace adcontrols { class MassSpectrum; }

namespace lipidid {

    class candidate {
        double exact_mass_;
        std::string formula_;
        std::string adduct_;
        double mass_error_;
        std::vector< isoPeak > isotope_;
        std::vector< std::string > inchiKeys_;
    public:
        inline double exact_mass() const { return exact_mass_; }
        inline const std::string& formula() const { return formula_; }
        inline const std::string& adduct() const { return adduct_; }
        inline double mass_error() const { return mass_error_; }
        inline const std::vector< isoPeak >&  isotope() const { return isotope_; }
        inline const std::vector< std::string >& inchiKeys() const { return inchiKeys_; }
        ~candidate();
        candidate( double m = 0
                   , const std::string& f = {}        // formula
                   , const std::string& a = {}        // adduct/lose
                   , double e = 0                     // mass_error
                   , std::vector< isoPeak >&& i = {}     // isotope match
                   , std::vector< std::string >&& keys = {} );

        candidate( const candidate& t );
        friend void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const candidate& );
        friend candidate tag_invoke( const boost::json::value_to_tag< candidate >&, const boost::json::value& jv );
    };

}
