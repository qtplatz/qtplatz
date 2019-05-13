/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "avgr_u5303a.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

namespace infitof {
    namespace u5303a {

        template< typename T = Descriptors >
        class Descriptors_archive {
        public:
            template<class Archive>
            void serialize( Archive& ar, T& _, const unsigned int version ) {
                using namespace boost::serialization;

                if ( version == 0 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.method_ );
                    ar & BOOST_SERIALIZATION_NVP( _.meta_ );
                } else {
                    ar & BOOST_SERIALIZATION_NVP( _.method_ );
                    ar & BOOST_SERIALIZATION_NVP( _.meta_ );
                }
            }
        };

        template<> void Descriptors::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
        {
            Descriptors_archive<>().serialize( ar, *this, version );
        }
        template<> void Descriptors::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
        {
            Descriptors_archive<>().serialize( ar, *this, version );
        }
    
        template<> void Descriptors::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            Descriptors_archive<>().serialize( ar, *this, version );
        }
    
        template<> void Descriptors::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            Descriptors_archive<>().serialize( ar, *this, version );
        }
        
    }
}

