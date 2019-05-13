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

#include "method.hpp"
#include "infitofcontrols_global.hpp"
#include <multumcontrols/scanlaw.hpp>
#include <adcontrols/metric/prefix.hpp>
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

namespace infitofcontrols {

    template<typename T = IonSource_EI_Method >
    class IonSource_EI_Method_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( _.einzel_voltage );
            ar & BOOST_SERIALIZATION_NVP( _.va_pulse_voltage );
            ar & BOOST_SERIALIZATION_NVP( _.potential_lift_pulse_voltage );
            ar & BOOST_SERIALIZATION_NVP( _.filament_current ); // mA
            ar & BOOST_SERIALIZATION_NVP( _.ionization_current );
            ar & BOOST_SERIALIZATION_NVP( _.ionization_voltage );
            ar & BOOST_SERIALIZATION_NVP( _.ei_temp );
            ar & BOOST_SERIALIZATION_NVP( _.gc_temp );
        }
    };

    template<> INFITOFCONTROLSSHARED_EXPORT void IonSource_EI_Method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        IonSource_EI_Method_archive<>().serialize( ar, *this, version );
    }
    template<> INFITOFCONTROLSSHARED_EXPORT void IonSource_EI_Method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        IonSource_EI_Method_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void IonSource_EI_Method::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        IonSource_EI_Method_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void IonSource_EI_Method::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        IonSource_EI_Method_archive<>().serialize( ar, *this, version );
    }

    template<typename T = IonSource_MALDI_Method >
    class IonSource_MALDI_Method_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int ) {
            using namespace boost::serialization;
            ar  & BOOST_SERIALIZATION_NVP( _.tba );
        }
    };
    template<> INFITOFCONTROLSSHARED_EXPORT void IonSource_MALDI_Method::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        IonSource_MALDI_Method_archive<>().serialize( ar, *this, version );
    }
    template<> INFITOFCONTROLSSHARED_EXPORT void IonSource_MALDI_Method::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        IonSource_MALDI_Method_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void IonSource_MALDI_Method::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        IonSource_MALDI_Method_archive<>().serialize( ar, *this, version );
    }

    template<> INFITOFCONTROLSSHARED_EXPORT void IonSource_MALDI_Method::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        IonSource_MALDI_Method_archive<>().serialize( ar, *this, version );
    }

}
