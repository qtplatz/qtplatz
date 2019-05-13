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
#include "avgrmethod.hpp"
#include "ionsourcemethod.hpp"
#include <boost/serialization/version.hpp>
#include <boost/variant.hpp>
#include <cstdint>
#include <vector>

namespace boost {
    namespace serialization { class access; }
    namespace uuids { struct uuid; }
}

namespace infitofcontrols {

    // Time Event
    template<typename T> class VoltageMethod_archive;

    class INFITOFCONTROLSSHARED_EXPORT VoltageMethod {
    public:
        static const char * modelClass() { return "InfiTOF,HV"; }
        static const char * itemLabel() { return "Voltage"; }
        static const boost::uuids::uuid& clsid();

        VoltageMethod() : onoff_( false ) {}
        bool onoff() const { return onoff_; }
        void setOnOff( bool a ) { onoff_ = a; }

        static bool archive( std::ostream&, const VoltageMethod& );
        static bool restore( std::istream&, VoltageMethod& );

    private:
        bool onoff_;
        friend class VoltageMethod_archive < VoltageMethod > ;
        friend class VoltageMethod_archive < const VoltageMethod > ;
        friend class boost::serialization::access;
        template< class Archive > void serialize( Archive& ar, const unsigned int );
    };

};

BOOST_CLASS_VERSION( infitofcontrols::VoltageMethod, 0 )
