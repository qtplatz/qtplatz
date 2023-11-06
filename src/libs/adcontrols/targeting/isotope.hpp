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
#include <string>
#include <vector>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <tuple>
#include <memory>

namespace adcontrols {

    namespace targeting {

        struct ADCONTROLSSHARED_EXPORT isotope;

        struct isotope {
            int32_t idx;
            double mass;
            double abundance_ratio;
            double abundance_ratio_error;
            double exact_mass;
            double exact_abundance;
            isotope();
            isotope( size_t idx, double mass, double ra, double ra_error, double exact_mass, double exact_abundance );
            isotope( const isotope& t );

        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, unsigned int ) {
                ar & BOOST_SERIALIZATION_NVP( idx );
                ar & BOOST_SERIALIZATION_NVP( mass );
                ar & BOOST_SERIALIZATION_NVP( abundance_ratio );
                ar & BOOST_SERIALIZATION_NVP( abundance_ratio_error );
                ar & BOOST_SERIALIZATION_NVP( exact_mass );
                ar & BOOST_SERIALIZATION_NVP( exact_abundance );
            }
        };

        ADCONTROLSSHARED_EXPORT
        void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const isotope& );

        ADCONTROLSSHARED_EXPORT
        isotope tag_invoke( const boost::json::value_to_tag< isotope >&, const boost::json::value& jv );
    }
}

BOOST_CLASS_VERSION( adcontrols::targeting::isotope, 0 )
