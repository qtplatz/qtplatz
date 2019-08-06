// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

namespace adspectrometer {

    namespace names {
        constexpr const char * const iid_adspectrometer_plugin = "adspectrometer.plugin.ms-cheminfo.com";
        constexpr const char * const adspectrometer_objtext = "adspectrometer";
    }

    namespace iids {
        // constexpr const char * const iid_adspectrometer = "{e45d27e0-8478-414c-b33d-246f76cf62ad}";
        constexpr const boost::uuids::uuid uuid_massspectrometer = {{ 0xE4, 0x5D, 0x27, 0xE0, 0x84, 0x78, 0x41, 0x4C, 0xB3, 0x3D, 0x24, 0x6F, 0x76, 0xCF, 0x62, 0xAD }};
    }

}
