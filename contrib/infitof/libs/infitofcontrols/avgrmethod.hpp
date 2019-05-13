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
#include <cstdint>
#include <iostream>
#include <vector>

namespace boost {
    namespace serialization { class access; }
    namespace uuids { struct uuid; }
}

namespace multumcontrols { class OrbitProtocol; }

namespace infitofcontrols {

    class INFITOFCONTROLSSHARED_EXPORT AvgrMethod;

    template<typename T = AvgrMethod > class AvgrMethod_archive;

    class AvgrMethod {
    public:
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
        std::vector< multumcontrols::OrbitProtocol > protocols;
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
