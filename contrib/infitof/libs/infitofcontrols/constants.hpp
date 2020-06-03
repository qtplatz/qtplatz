/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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

#include <boost/uuid/uuid.hpp>

namespace infitofcontrols {
    namespace Constants {
        static constexpr size_t max_protocol = 4;
    };
}

namespace infitof {

    namespace Constants {
        constexpr const char * const httpd_dg = "httpd-dg";
        constexpr const char * const httpd_hv = "httpd-hv";
    };

    namespace names {
        constexpr const char * const objtext_massspectrometer = "InfiTOF"; // historical name (don't change)
        constexpr const char * const objtext_datainterpreter = "InfiTOF"; // historical name (don't change)
        // used be defined in adplugins/infitofspectrometer/massspectrometer
        //constexpr const char * massspectrometer_clsid_text = "{90BB510B-5DC2-43AB-89EF-2E108E99EAAA}";
        //constexpr const char * massspectrometer_class_name = "InfiTOF"; // historical name, don't change
    }

    namespace iids {
        constexpr const boost::uuids::uuid uuid_massspectrometer = {{ 0x90, 0xBB, 0x51, 0x0B, 0x5D, 0xC2, 0x43, 0xAB, 0x89, 0xEF, 0x2E, 0x10, 0x8E, 0x99, 0xEA, 0xAA }};

        // "{06C98B98-9BF7-4056-BB31-0CF42E33FB36}"
        constexpr const boost::uuids::uuid uuid_datainterpreter = {{ 0x06, 0xC9, 0x8B, 0x98, 0x9B, 0xF7, 0x40, 0x56, 0xBB, 0x31, 0x0C, 0xF4, 0x2E, 0x33, 0xFB, 0x36 }};
    }
}
