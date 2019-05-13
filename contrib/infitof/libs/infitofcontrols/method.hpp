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

namespace adcontrols { class TofProtocol; }

namespace infitofcontrols {

    template<typename T> class method_archive;

    class INFITOFCONTROLSSHARED_EXPORT method {
    public:
        static const char * modelClass() { return "InfiTOF2"; }
        static const char * itemLabel() { return "Analyzer"; }
        static const boost::uuids::uuid& clsid();

        std::vector<int32_t>& arp_hv();
        const std::vector<int32_t>& arp_hv() const;

        AvgrMethod& tof();
        const AvgrMethod& tof() const;

        std::string& description();
        const std::string& description() const;

    private:
        std::string description_;
        AvgrMethod tof_;                 // Timing protocol(s)
        std::vector< int32_t > arp_hv_;  // equivalent to infitofcontrols::arp::HVSetpts

    public:
        ~method();
        method();
        method( const method& t );

        static bool archive( std::ostream&, const method& );
        static bool restore( std::istream&, method& );
        static bool xml_archive( std::wostream&, const method& );
        static bool xml_restore( std::wistream&, method& );
        static void setup_default( method& m );

        static void copy_protocols( const AvgrMethod&, std::vector< adcontrols::TofProtocol >& );
        void copy_protocols( std::vector< adcontrols::TofProtocol >& ) const;

    private:
        friend class method_archive < method > ;
        friend class method_archive < const method > ;
        friend class boost::serialization::access;
        template< class Archive >
            void serialize( Archive& ar, const unsigned int version );
    };

};

BOOST_CLASS_VERSION( infitofcontrols::method, 4 )
