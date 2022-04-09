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

#pragma once

#include "../adcontrols_global.h"
#include "../constants_fwd.hpp"
#include "isotope.hpp"
#include <string>
#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <tuple>
#include <memory>

namespace adcontrols {
    namespace targeting {

        struct isotope;
        struct ADCONTROLSSHARED_EXPORT Candidate;

        struct Candidate {
            uint32_t idx;                       // peak index on mass-spectrum
            uint32_t fcn;                       // protocol (aka segment) id
            int32_t  charge;
            double   mass;                      // This used to an error from exact mass, change it to found mass at V2 (2019-AUG-15)
            std::string formula;                // this is the exact formula matched with the peak (contains adducts)
            double   exact_mass;                // V2
            int32_t  score;                     // V2
            std::vector< isotope > isotopes;    // V2

            Candidate();
            Candidate( const Candidate& );
            Candidate( uint32_t idx, uint32_t fcn, int32_t charge, double mass, double exact_mass, const std::string& formula );
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, unsigned int version ) {
                ar & BOOST_SERIALIZATION_NVP( idx );
                ar & BOOST_SERIALIZATION_NVP( fcn );
                ar & BOOST_SERIALIZATION_NVP( charge );
                ar & BOOST_SERIALIZATION_NVP( mass );
                ar & BOOST_SERIALIZATION_NVP( formula );
                if ( version >= 2 ) {
                    ar & BOOST_SERIALIZATION_NVP( exact_mass );
                    ar & BOOST_SERIALIZATION_NVP( score );
                    ar & BOOST_SERIALIZATION_NVP( isotopes );
                }
            }
        };

    }
}

BOOST_CLASS_VERSION( adcontrols::targeting::Candidate, 2 )
