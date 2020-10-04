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
#include <admtcontrols/orbitprotocol.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/uuid.hpp>
#include <cstdint>
#include <iostream>
#include <vector>

namespace boost {
    namespace serialization { class access; }
    namespace uuids { struct uuid; }
}

namespace admtcontrols { class OrbitProtocol; }

namespace infitofcontrols {

    class INFITOFCONTROLSSHARED_EXPORT AvgrMethod;

    template<typename T = AvgrMethod > class AvgrMethod_archive;

    class AvgrMethod {
    public:
        // dialog "{0EB051AB-7820-4FB5-B5A4-7359A13D8CD1}";
        static constexpr boost::uuids::uuid __dlg_clsid = {{ 0x0e, 0xb0, 0x51, 0xab, 0x78, 0x20, 0x4f, 0xb5, 0xb5, 0xa4, 0x73, 0x59, 0xa1, 0x3d, 0x8c, 0xd1 }};

        // this 9dc75ee5-16e2-50a2-b82a-fb31898ec105
        static constexpr boost::uuids::uuid __clsid = {{ 0x9d, 0xc7, 0x5e, 0xe5, 0x16, 0xe2, 0x50, 0xa2, 0xb8, 0x2a, 0xfb, 0x31, 0x89, 0x8e, 0xc1, 0x05 }};

        static const char * modelClass() { return "InfiTOF,Avgr"; }
        static const char * itemLabel()  { return "Avgr"; }
        static const boost::uuids::uuid& clsid();

        bool isMaxNumAverage; // deprecated
        int32_t numAverage;
        int32_t gain;
    private:
        int32_t trigInterval; // microseconds (1000 := 1kHz)
        uint32_t nReplicates; // deprecated -- use protocols.replicates()
    public:
        uint32_t nTurn;       // deprecated -- use protocols nturns
        std::vector< admtcontrols::OrbitProtocol > protocols;
        uint32_t autoBackground;
        bool enableGateWindow;  // for acqprotocoldetail UI behavior change
        double gateWindow;      // ibid.

        void setTrigRate( int32_t v ) { trigInterval = v; };
        int32_t trigRate() const { return trigInterval; }

        AvgrMethod();
        AvgrMethod( const AvgrMethod& t );

        static bool archive( std::ostream&, const AvgrMethod& );
        static bool restore( std::istream&, AvgrMethod& );
        static bool xml_archive( std::wostream&, const AvgrMethod& );
        static bool xml_restore( std::wistream&, AvgrMethod& );

    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int version );

        friend class AvgrMethod_archive< AvgrMethod >;
        friend class AvgrMethod_archive< const AvgrMethod >;
    };

};

BOOST_CLASS_VERSION( infitofcontrols::AvgrMethod, 6 )
