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

#pragma once

#include "multumcontrols_global.hpp"
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <vector>
#include <string>

namespace boost { namespace serialization { class access; } }

namespace multumcontrols {
    
    class MULTUMCONTROLSSHARED_EXPORT ElectricSectorMethod {
    public:
        double outer_voltage;
        double inner_voltage;
        bool enable;
        ElectricSectorMethod();
    private:
        friend class boost::serialization::access;
        template< class Archive >  
            void serialize( Archive& ar, const unsigned int );
    };

#if defined _MSC_VER
    MULTUMCONTROLSSHARED_TEMPLATE_EXPORT template class MULTUMCONTROLSSHARED_EXPORT std::vector< ElectricSectorMethod >;
#endif

}

BOOST_CLASS_VERSION( multumcontrols::ElectricSectorMethod, 0 )
