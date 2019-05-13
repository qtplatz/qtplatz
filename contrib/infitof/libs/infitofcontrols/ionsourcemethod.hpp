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

#pragma once

#include "infitofcontrols_global.hpp"
#include <boost/serialization/version.hpp>
#include <boost/variant.hpp>
#include <cstdint>
#include <vector>

namespace boost { namespace serialization { class access; } }

namespace infitofcontrols {

    enum eIonSource {
        eIonSource_EI
        , eIonSource_MALDI
    };

    enum ePolarity { ePolarity_Indeterminate, ePolarity_Positive, ePolarity_Negative };

    class INFITOFCONTROLSSHARED_EXPORT IonSource_EI_Method {
    public:
        // EI
        double einzel_voltage;
        double va_pulse_voltage;
        double potential_lift_pulse_voltage;
        // Filament
        double filament_current; // mA
        double ionization_current; // := trap current (mA)
        double ionization_voltage; // V
        double ei_temp;
        double gc_temp;
    private:
        friend class boost::serialization::access;
        template< class Archive > void serialize( Archive& ar, const unsigned int );
    };

    class INFITOFCONTROLSSHARED_EXPORT IonSource_MALDI_Method {
    public:
        double tba;
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int );
    };

};
