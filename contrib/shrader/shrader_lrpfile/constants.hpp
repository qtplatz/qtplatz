// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2026 MS-Cheminformatics LLC
** Copyright (C) 2026 Toshinobu Hondo, Ph.D.
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

namespace shreader {
    namespace spectrometer {

        namespace names {
            constexpr const char * const iid_accutofspectrometer_plugin = "jp.ac.osaka-u.qtplatz.adplugins.massSpectrometer.infitof.lrp";
            constexpr const char * const objtext_massspectrometer = "lrp.infiTOF"; // (don't change)
            constexpr const char * const objtext_datainterpreter = "lrp.infiTOF"; // (don't change)
        }

        namespace iids {

            // 9568b15d-73b6-48ed-a1c7-ac56a308f712;
            constexpr const boost::uuids::uuid uuid_massspectrometer =
            {{ 0xA2, 0x7C, 0xBB, 0xCA, 0x19, 0x92, 0x4C, 0xCF
                 , 0xB2, 0x50, 0xC2, 0x8E, 0xB6, 0xF6, 0x20, 0x4F }};

            constexpr const boost::uuids::uuid uuid_datainterpreter =
            {{ 0xC3, 0xCE, 0x71, 0x47, 0x5E, 0x64, 0x4F, 0x67
                 , 0x91, 0x9D, 0xD8, 0xAF, 0x7A, 0x74, 0x6C, 0xA7 }};
        }
    }
}
