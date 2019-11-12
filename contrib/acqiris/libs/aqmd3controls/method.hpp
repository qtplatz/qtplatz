/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "aqmd3controls_global.hpp"
#include "device_method.hpp"
#include <adcontrols/threshold_method.hpp>
#include <adcontrols/threshold_action.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>
#include <iostream>

namespace boost {
    namespace serialization { class access; }
    namespace uuids { struct uuid; }
}


namespace aqmd3controls {

    class method;
    template<typename T = method > class method_archive;

    class AQMD3CONTROLSSHARED_EXPORT method;

    class method {
    public:
        method();
        method( const method& t );
        static const char * modelClass() { return "aqmd3"; }
        static const char * itemLabel() { return "aqmd3"; }
        static const boost::uuids::uuid& clsid();

        enum class DigiMode : uint32_t { Digitizer = 0, Averager = 2 };

        uint32_t channels() const;
        void setChannels( uint32_t );
        DigiMode mode() const;  // 0 := digitizer, 2 := averager
        void setMode( DigiMode );

        // By default, U5303A prohibit to run w/o calibration when change delay, width or other paramters.
        // In order to success 'rapid protocol' acquisition, need following environmental variable set
        // AGMD2_SKIP_CAL_REQUIRED_CHECKS=1

        const aqmd3controls::device_method& device_method() const;
        aqmd3controls::device_method& device_method();

        uint32_t protocolIndex() const;
        bool setProtocolIndex( uint32_t, bool modifyDeviceMethod );

        std::vector< adcontrols::TofProtocol >& protocols();
        const std::vector< adcontrols::TofProtocol >& protocols() const;

        static bool archive( std::ostream&, const method& );
        static bool restore( std::istream&, method& );
        static bool xml_archive( std::wostream&, const method& );
        static bool xml_restore( std::wistream&, method& );

    private:
        uint32_t channels_;
        DigiMode mode_;  // 0 := digitizer, 2 := averager
        aqmd3controls::device_method device_method_;

        uint32_t protocolIndex_;
        std::vector< adcontrols::TofProtocol > protocols_;

        friend class method_archive< method >;
        friend class method_archive< const method >;
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

}

BOOST_CLASS_VERSION( aqmd3controls::method, 1 )
