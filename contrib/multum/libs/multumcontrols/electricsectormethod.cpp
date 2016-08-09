/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "electricsectormethod.hpp"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/nvp.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

namespace multumcontrols {

    ///////////////////////////////////////////
    template<typename T = ElectricSectorMethod >
    class ElectricSectorMethod_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.outer_voltage );
            ar & BOOST_SERIALIZATION_NVP( _.inner_voltage );
            ar & BOOST_SERIALIZATION_NVP( _.enable );
        }
    };
    
    template<> MULTUMCONTROLSSHARED_EXPORT void ElectricSectorMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        ElectricSectorMethod_archive<>().serialize( ar, *this, version );
    }
    
    template<> MULTUMCONTROLSSHARED_EXPORT void ElectricSectorMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ElectricSectorMethod_archive<>().serialize( ar, *this, version );
    }
    
    template<> MULTUMCONTROLSSHARED_EXPORT void ElectricSectorMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        ElectricSectorMethod_archive<>().serialize( ar, *this, version );
    }
    
    template<> MULTUMCONTROLSSHARED_EXPORT void ElectricSectorMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ElectricSectorMethod_archive<>().serialize( ar, *this, version );
    }

} // namespace


using namespace multumcontrols;

//////////////////////

ElectricSectorMethod::ElectricSectorMethod() : outer_voltage( 0 ), inner_voltage( 0 ), enable( true )
{
}

