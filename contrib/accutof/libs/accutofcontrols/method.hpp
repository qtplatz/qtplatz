/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC
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

#include "accutofcontrols_global.hpp"
#include <compiler/boost/workaround.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/uuid/uuid.hpp>
#include <cstdint>
#include <string>

namespace accutofcontrols {

    template<typename T> class method_archive;

    class ACCUTOFCONTROLSSHARED_EXPORT method;

    class method {
    public:
        // f8e6f1b6-58c7-461e-8ad8-87a2aa4bfbe3
        static constexpr boost::uuids::uuid __clsid = {{ 0xf8, 0xe6, 0xf1, 0xb6, 0x58, 0xc7, 0x46, 0x1e, 0x8a, 0xd8, 0x87, 0xa2, 0xaa, 0x4b, 0xfb, 0xe3 }};

        static const char * modelClass() { return "AccuTOF"; }
        static const char * itemLabel() { return "AccuTOF.Analyzer"; }
        static const boost::uuids::uuid& clsid() { return __clsid; }

    private:
        std::string json_;

    public:
        ~method();
        method();
        method( const method& t );

        static bool archive( std::ostream&, const method& );
        static bool restore( std::istream&, method& );

    private:
        friend class boost::serialization::access;
        template< class Archive > void serialize( Archive& ar, const unsigned int version ) {
            ar & json_;
        };
    };

};

BOOST_CLASS_VERSION( accutofcontrols::method, 1 )
