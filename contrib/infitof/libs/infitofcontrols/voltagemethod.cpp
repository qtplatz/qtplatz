/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "voltagemethod.hpp"
#include "infitofcontrols_global.hpp"
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
#include <boost/uuid/uuid_generators.hpp>

namespace infitofcontrols {

    template<typename T = VoltageMethod >
    class VoltageMethod_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.onoff_ );
        }
    };
    template<> INFITOFCONTROLSSHARED_EXPORT void VoltageMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        VoltageMethod_archive<>().serialize( ar, *this, version );
    }
    template<> INFITOFCONTROLSSHARED_EXPORT void VoltageMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        VoltageMethod_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void VoltageMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        VoltageMethod_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void VoltageMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        VoltageMethod_archive<>().serialize( ar, *this, version );
    }

}

using namespace infitofcontrols;

bool
VoltageMethod::archive( std::ostream& os, const VoltageMethod& t )
{
    try {
        portable_binary_oarchive ar( os );
        ar & boost::serialization::make_nvp( "m", t );
        return true;
    } catch ( std::exception& ex ) {
        BOOST_THROW_EXCEPTION( ex );
    }
    return false;
}

bool
VoltageMethod::restore( std::istream& is, VoltageMethod& t )
{
    try {
        portable_binary_iarchive ar( is );
        ar & boost::serialization::make_nvp( "m", t );
        return true;
    } catch ( std::exception& ex ) {
        BOOST_THROW_EXCEPTION( ex );
    }
    return false;
}

const boost::uuids::uuid&
VoltageMethod::clsid()
{
    static boost::uuids::uuid baseid = boost::uuids::string_generator()( "{3D2F180E-18E9-43D3-9A37-9E981B509CAA}" );
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( baseid )( "infwidgets::VoltageMethod" );
    return myclsid;
}
